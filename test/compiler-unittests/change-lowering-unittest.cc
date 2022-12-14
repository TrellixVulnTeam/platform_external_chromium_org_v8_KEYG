// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/compiler/change-lowering.h"
#include "src/compiler/js-graph.h"
#include "src/compiler/node-properties-inl.h"
#include "src/compiler/simplified-operator.h"
#include "src/compiler/typer.h"
#include "test/compiler-unittests/graph-unittest.h"
#include "testing/gmock-support.h"

using testing::_;
using testing::AllOf;
using testing::Capture;
using testing::CaptureEq;

namespace v8 {
namespace internal {
namespace compiler {

// TODO(bmeurer): Find a new home for these functions.
inline std::ostream& operator<<(std::ostream& os, const MachineType& type) {
  OStringStream ost;
  ost << type;
  return os << ost.c_str();
}


class ChangeLoweringTest : public GraphTest {
 public:
  ChangeLoweringTest() : simplified_(zone()) {}
  virtual ~ChangeLoweringTest() {}

  virtual MachineType WordRepresentation() const = 0;

 protected:
  int HeapNumberValueOffset() const {
    STATIC_ASSERT(HeapNumber::kValueOffset % kApiPointerSize == 0);
    return (HeapNumber::kValueOffset / kApiPointerSize) * PointerSize() -
           kHeapObjectTag;
  }
  bool Is32() const { return WordRepresentation() == kRepWord32; }
  int PointerSize() const {
    switch (WordRepresentation()) {
      case kRepWord32:
        return 4;
      case kRepWord64:
        return 8;
      default:
        break;
    }
    UNREACHABLE();
    return 0;
  }
  int SmiMaxValue() const { return -(SmiMinValue() + 1); }
  int SmiMinValue() const {
    return static_cast<int>(0xffffffffu << (SmiValueSize() - 1));
  }
  int SmiShiftAmount() const { return kSmiTagSize + SmiShiftSize(); }
  int SmiShiftSize() const {
    // TODO(turbofan): Work-around for weird GCC 4.6 linker issue:
    // src/compiler/change-lowering.cc:46: undefined reference to
    // `v8::internal::SmiTagging<4u>::kSmiShiftSize'
    // src/compiler/change-lowering.cc:46: undefined reference to
    // `v8::internal::SmiTagging<8u>::kSmiShiftSize'
    STATIC_ASSERT(SmiTagging<4>::kSmiShiftSize == 0);
    STATIC_ASSERT(SmiTagging<8>::kSmiShiftSize == 31);
    return Is32() ? 0 : 31;
  }
  int SmiValueSize() const {
    // TODO(turbofan): Work-around for weird GCC 4.6 linker issue:
    // src/compiler/change-lowering.cc:46: undefined reference to
    // `v8::internal::SmiTagging<4u>::kSmiValueSize'
    // src/compiler/change-lowering.cc:46: undefined reference to
    // `v8::internal::SmiTagging<8u>::kSmiValueSize'
    STATIC_ASSERT(SmiTagging<4>::kSmiValueSize == 31);
    STATIC_ASSERT(SmiTagging<8>::kSmiValueSize == 32);
    return Is32() ? 31 : 32;
  }

  Node* Parameter(int32_t index = 0) {
    return graph()->NewNode(common()->Parameter(index), graph()->start());
  }

  Reduction Reduce(Node* node) {
    Typer typer(zone());
    JSGraph jsgraph(graph(), common(), &typer);
    CompilationInfo info(isolate(), zone());
    Linkage linkage(&info);
    MachineOperatorBuilder machine(zone(), WordRepresentation());
    ChangeLowering reducer(&jsgraph, &linkage, &machine);
    return reducer.Reduce(node);
  }

  SimplifiedOperatorBuilder* simplified() { return &simplified_; }

  Matcher<Node*> IsAllocateHeapNumber(const Matcher<Node*>& effect_matcher,
                                      const Matcher<Node*>& control_matcher) {
    return IsCall(
        _, IsHeapConstant(PrintableUnique<HeapObject>::CreateImmovable(
               zone(), CEntryStub(isolate(), 1).GetCode())),
        IsExternalConstant(ExternalReference(
            Runtime::FunctionForId(Runtime::kAllocateHeapNumber), isolate())),
        IsInt32Constant(0), IsNumberConstant(0.0), effect_matcher,
        control_matcher);
  }
  Matcher<Node*> IsFalse() {
    return IsHeapConstant(PrintableUnique<HeapObject>::CreateImmovable(
        zone(), factory()->false_value()));
  }
  Matcher<Node*> IsTrue() {
    return IsHeapConstant(PrintableUnique<HeapObject>::CreateImmovable(
        zone(), factory()->true_value()));
  }
  Matcher<Node*> IsWordEqual(const Matcher<Node*>& lhs_matcher,
                             const Matcher<Node*>& rhs_matcher) {
    return Is32() ? IsWord32Equal(lhs_matcher, rhs_matcher)
                  : IsWord64Equal(lhs_matcher, rhs_matcher);
  }

 private:
  SimplifiedOperatorBuilder simplified_;
};


// -----------------------------------------------------------------------------
// Common.


class ChangeLoweringCommonTest
    : public ChangeLoweringTest,
      public ::testing::WithParamInterface<MachineType> {
 public:
  virtual ~ChangeLoweringCommonTest() {}

  virtual MachineType WordRepresentation() const V8_FINAL V8_OVERRIDE {
    return GetParam();
  }
};


TARGET_TEST_P(ChangeLoweringCommonTest, ChangeBitToBool) {
  Node* val = Parameter(0);
  Node* node = graph()->NewNode(simplified()->ChangeBitToBool(), val);
  Reduction reduction = Reduce(node);
  ASSERT_TRUE(reduction.Changed());

  Node* phi = reduction.replacement();
  Capture<Node*> branch;
  EXPECT_THAT(phi,
              IsPhi(IsTrue(), IsFalse(),
                    IsMerge(IsIfTrue(AllOf(CaptureEq(&branch),
                                           IsBranch(val, graph()->start()))),
                            IsIfFalse(CaptureEq(&branch)))));
}


TARGET_TEST_P(ChangeLoweringCommonTest, ChangeBoolToBit) {
  Node* val = Parameter(0);
  Node* node = graph()->NewNode(simplified()->ChangeBoolToBit(), val);
  Reduction reduction = Reduce(node);
  ASSERT_TRUE(reduction.Changed());

  EXPECT_THAT(reduction.replacement(), IsWordEqual(val, IsTrue()));
}


TARGET_TEST_P(ChangeLoweringCommonTest, ChangeFloat64ToTagged) {
  Node* val = Parameter(0);
  Node* node = graph()->NewNode(simplified()->ChangeFloat64ToTagged(), val);
  Reduction reduction = Reduce(node);
  ASSERT_TRUE(reduction.Changed());

  Node* finish = reduction.replacement();
  Capture<Node*> heap_number;
  EXPECT_THAT(
      finish,
      IsFinish(
          AllOf(CaptureEq(&heap_number),
                IsAllocateHeapNumber(IsValueEffect(val), graph()->start())),
          IsStore(kMachFloat64, kNoWriteBarrier, CaptureEq(&heap_number),
                  IsInt32Constant(HeapNumberValueOffset()), val,
                  CaptureEq(&heap_number), graph()->start())));
}


TARGET_TEST_P(ChangeLoweringCommonTest, StringAdd) {
  Node* node =
      graph()->NewNode(simplified()->StringAdd(), Parameter(0), Parameter(1));
  Reduction reduction = Reduce(node);
  EXPECT_FALSE(reduction.Changed());
}


INSTANTIATE_TEST_CASE_P(ChangeLoweringTest, ChangeLoweringCommonTest,
                        ::testing::Values(kRepWord32, kRepWord64));


// -----------------------------------------------------------------------------
// 32-bit


class ChangeLowering32Test : public ChangeLoweringTest {
 public:
  virtual ~ChangeLowering32Test() {}
  virtual MachineType WordRepresentation() const V8_FINAL V8_OVERRIDE {
    return kRepWord32;
  }
};


TARGET_TEST_F(ChangeLowering32Test, ChangeInt32ToTagged) {
  Node* val = Parameter(0);
  Node* node = graph()->NewNode(simplified()->ChangeInt32ToTagged(), val);
  Reduction reduction = Reduce(node);
  ASSERT_TRUE(reduction.Changed());

  Node* phi = reduction.replacement();
  Capture<Node*> add, branch, heap_number, if_true;
  EXPECT_THAT(
      phi,
      IsPhi(IsFinish(
                AllOf(CaptureEq(&heap_number),
                      IsAllocateHeapNumber(_, CaptureEq(&if_true))),
                IsStore(kMachFloat64, kNoWriteBarrier, CaptureEq(&heap_number),
                        IsInt32Constant(HeapNumberValueOffset()),
                        IsChangeInt32ToFloat64(val), CaptureEq(&heap_number),
                        CaptureEq(&if_true))),
            IsProjection(
                0, AllOf(CaptureEq(&add), IsInt32AddWithOverflow(val, val))),
            IsMerge(AllOf(CaptureEq(&if_true), IsIfTrue(CaptureEq(&branch))),
                    IsIfFalse(AllOf(CaptureEq(&branch),
                                    IsBranch(IsProjection(1, CaptureEq(&add)),
                                             graph()->start()))))));
}


TARGET_TEST_F(ChangeLowering32Test, ChangeTaggedToFloat64) {
  STATIC_ASSERT(kSmiTag == 0);
  STATIC_ASSERT(kSmiTagSize == 1);

  Node* val = Parameter(0);
  Node* node = graph()->NewNode(simplified()->ChangeTaggedToFloat64(), val);
  Reduction reduction = Reduce(node);
  ASSERT_TRUE(reduction.Changed());

  Node* phi = reduction.replacement();
  Capture<Node*> branch, if_true;
  EXPECT_THAT(
      phi,
      IsPhi(
          IsLoad(kMachFloat64, val, IsInt32Constant(HeapNumberValueOffset()),
                 IsControlEffect(CaptureEq(&if_true))),
          IsChangeInt32ToFloat64(
              IsWord32Sar(val, IsInt32Constant(SmiShiftAmount()))),
          IsMerge(
              AllOf(CaptureEq(&if_true),
                    IsIfTrue(AllOf(
                        CaptureEq(&branch),
                        IsBranch(IsWord32And(val, IsInt32Constant(kSmiTagMask)),
                                 graph()->start())))),
              IsIfFalse(CaptureEq(&branch)))));
}


TARGET_TEST_F(ChangeLowering32Test, ChangeTaggedToInt32) {
  STATIC_ASSERT(kSmiTag == 0);
  STATIC_ASSERT(kSmiTagSize == 1);

  Node* val = Parameter(0);
  Node* node = graph()->NewNode(simplified()->ChangeTaggedToInt32(), val);
  Reduction reduction = Reduce(node);
  ASSERT_TRUE(reduction.Changed());

  Node* phi = reduction.replacement();
  Capture<Node*> branch, if_true;
  EXPECT_THAT(
      phi,
      IsPhi(IsTruncateFloat64ToInt32(IsLoad(
                kMachFloat64, val, IsInt32Constant(HeapNumberValueOffset()),
                IsControlEffect(CaptureEq(&if_true)))),
            IsWord32Sar(val, IsInt32Constant(SmiShiftAmount())),
            IsMerge(AllOf(CaptureEq(&if_true), IsIfTrue(CaptureEq(&branch))),
                    IsIfFalse(AllOf(
                        CaptureEq(&branch),
                        IsBranch(IsWord32And(val, IsInt32Constant(kSmiTagMask)),
                                 graph()->start()))))));
}


TARGET_TEST_F(ChangeLowering32Test, ChangeTaggedToUint32) {
  STATIC_ASSERT(kSmiTag == 0);
  STATIC_ASSERT(kSmiTagSize == 1);

  Node* val = Parameter(0);
  Node* node = graph()->NewNode(simplified()->ChangeTaggedToUint32(), val);
  Reduction reduction = Reduce(node);
  ASSERT_TRUE(reduction.Changed());

  Node* phi = reduction.replacement();
  Capture<Node*> branch, if_true;
  EXPECT_THAT(
      phi,
      IsPhi(IsTruncateFloat64ToInt32(IsLoad(
                kMachFloat64, val, IsInt32Constant(HeapNumberValueOffset()),
                IsControlEffect(CaptureEq(&if_true)))),
            IsWord32Sar(val, IsInt32Constant(SmiShiftAmount())),
            IsMerge(AllOf(CaptureEq(&if_true), IsIfTrue(CaptureEq(&branch))),
                    IsIfFalse(AllOf(
                        CaptureEq(&branch),
                        IsBranch(IsWord32And(val, IsInt32Constant(kSmiTagMask)),
                                 graph()->start()))))));
}


TARGET_TEST_F(ChangeLowering32Test, ChangeUint32ToTagged) {
  STATIC_ASSERT(kSmiTag == 0);
  STATIC_ASSERT(kSmiTagSize == 1);

  Node* val = Parameter(0);
  Node* node = graph()->NewNode(simplified()->ChangeUint32ToTagged(), val);
  Reduction reduction = Reduce(node);
  ASSERT_TRUE(reduction.Changed());

  Node* phi = reduction.replacement();
  Capture<Node*> branch, heap_number, if_false;
  EXPECT_THAT(
      phi,
      IsPhi(
          IsWord32Shl(val, IsInt32Constant(SmiShiftAmount())),
          IsFinish(
              AllOf(CaptureEq(&heap_number),
                    IsAllocateHeapNumber(_, CaptureEq(&if_false))),
              IsStore(kMachFloat64, kNoWriteBarrier, CaptureEq(&heap_number),
                      IsInt32Constant(HeapNumberValueOffset()),
                      IsChangeUint32ToFloat64(val), CaptureEq(&heap_number),
                      CaptureEq(&if_false))),
          IsMerge(
              IsIfTrue(AllOf(CaptureEq(&branch),
                             IsBranch(IsUint32LessThanOrEqual(
                                          val, IsInt32Constant(SmiMaxValue())),
                                      graph()->start()))),
              AllOf(CaptureEq(&if_false), IsIfFalse(CaptureEq(&branch))))));
}


// -----------------------------------------------------------------------------
// 64-bit


class ChangeLowering64Test : public ChangeLoweringTest {
 public:
  virtual ~ChangeLowering64Test() {}
  virtual MachineType WordRepresentation() const V8_FINAL V8_OVERRIDE {
    return kRepWord64;
  }
};


TARGET_TEST_F(ChangeLowering64Test, ChangeInt32ToTagged) {
  Node* val = Parameter(0);
  Node* node = graph()->NewNode(simplified()->ChangeInt32ToTagged(), val);
  Reduction reduction = Reduce(node);
  ASSERT_TRUE(reduction.Changed());

  EXPECT_THAT(reduction.replacement(),
              IsWord64Shl(IsChangeInt32ToInt64(val),
                          IsInt32Constant(SmiShiftAmount())));
}


TARGET_TEST_F(ChangeLowering64Test, ChangeTaggedToFloat64) {
  STATIC_ASSERT(kSmiTag == 0);
  STATIC_ASSERT(kSmiTagSize == 1);

  Node* val = Parameter(0);
  Node* node = graph()->NewNode(simplified()->ChangeTaggedToFloat64(), val);
  Reduction reduction = Reduce(node);
  ASSERT_TRUE(reduction.Changed());

  Node* phi = reduction.replacement();
  Capture<Node*> branch, if_true;
  EXPECT_THAT(
      phi,
      IsPhi(
          IsLoad(kMachFloat64, val, IsInt32Constant(HeapNumberValueOffset()),
                 IsControlEffect(CaptureEq(&if_true))),
          IsChangeInt32ToFloat64(IsTruncateInt64ToInt32(
              IsWord64Sar(val, IsInt32Constant(SmiShiftAmount())))),
          IsMerge(
              AllOf(CaptureEq(&if_true),
                    IsIfTrue(AllOf(
                        CaptureEq(&branch),
                        IsBranch(IsWord64And(val, IsInt32Constant(kSmiTagMask)),
                                 graph()->start())))),
              IsIfFalse(CaptureEq(&branch)))));
}


TARGET_TEST_F(ChangeLowering64Test, ChangeTaggedToInt32) {
  STATIC_ASSERT(kSmiTag == 0);
  STATIC_ASSERT(kSmiTagSize == 1);

  Node* val = Parameter(0);
  Node* node = graph()->NewNode(simplified()->ChangeTaggedToInt32(), val);
  Reduction reduction = Reduce(node);
  ASSERT_TRUE(reduction.Changed());

  Node* phi = reduction.replacement();
  Capture<Node*> branch, if_true;
  EXPECT_THAT(
      phi,
      IsPhi(IsTruncateFloat64ToInt32(IsLoad(
                kMachFloat64, val, IsInt32Constant(HeapNumberValueOffset()),
                IsControlEffect(CaptureEq(&if_true)))),
            IsTruncateInt64ToInt32(
                IsWord64Sar(val, IsInt32Constant(SmiShiftAmount()))),
            IsMerge(AllOf(CaptureEq(&if_true), IsIfTrue(CaptureEq(&branch))),
                    IsIfFalse(AllOf(
                        CaptureEq(&branch),
                        IsBranch(IsWord64And(val, IsInt32Constant(kSmiTagMask)),
                                 graph()->start()))))));
}


TARGET_TEST_F(ChangeLowering64Test, ChangeTaggedToUint32) {
  STATIC_ASSERT(kSmiTag == 0);
  STATIC_ASSERT(kSmiTagSize == 1);

  Node* val = Parameter(0);
  Node* node = graph()->NewNode(simplified()->ChangeTaggedToUint32(), val);
  Reduction reduction = Reduce(node);
  ASSERT_TRUE(reduction.Changed());

  Node* phi = reduction.replacement();
  Capture<Node*> branch, if_true;
  EXPECT_THAT(
      phi,
      IsPhi(IsTruncateFloat64ToInt32(IsLoad(
                kMachFloat64, val, IsInt32Constant(HeapNumberValueOffset()),
                IsControlEffect(CaptureEq(&if_true)))),
            IsTruncateInt64ToInt32(
                IsWord64Sar(val, IsInt32Constant(SmiShiftAmount()))),
            IsMerge(AllOf(CaptureEq(&if_true), IsIfTrue(CaptureEq(&branch))),
                    IsIfFalse(AllOf(
                        CaptureEq(&branch),
                        IsBranch(IsWord64And(val, IsInt32Constant(kSmiTagMask)),
                                 graph()->start()))))));
}


TARGET_TEST_F(ChangeLowering64Test, ChangeUint32ToTagged) {
  STATIC_ASSERT(kSmiTag == 0);
  STATIC_ASSERT(kSmiTagSize == 1);

  Node* val = Parameter(0);
  Node* node = graph()->NewNode(simplified()->ChangeUint32ToTagged(), val);
  Reduction reduction = Reduce(node);
  ASSERT_TRUE(reduction.Changed());

  Node* phi = reduction.replacement();
  Capture<Node*> branch, heap_number, if_false;
  EXPECT_THAT(
      phi,
      IsPhi(
          IsWord64Shl(IsChangeUint32ToUint64(val),
                      IsInt32Constant(SmiShiftAmount())),
          IsFinish(
              AllOf(CaptureEq(&heap_number),
                    IsAllocateHeapNumber(_, CaptureEq(&if_false))),
              IsStore(kMachFloat64, kNoWriteBarrier, CaptureEq(&heap_number),
                      IsInt32Constant(HeapNumberValueOffset()),
                      IsChangeUint32ToFloat64(val), CaptureEq(&heap_number),
                      CaptureEq(&if_false))),
          IsMerge(
              IsIfTrue(AllOf(CaptureEq(&branch),
                             IsBranch(IsUint32LessThanOrEqual(
                                          val, IsInt32Constant(SmiMaxValue())),
                                      graph()->start()))),
              AllOf(CaptureEq(&if_false), IsIfFalse(CaptureEq(&branch))))));
}

}  // namespace compiler
}  // namespace internal
}  // namespace v8
