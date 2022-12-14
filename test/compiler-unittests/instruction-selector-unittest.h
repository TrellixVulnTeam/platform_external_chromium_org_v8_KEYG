// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_COMPILER_UNITTESTS_INSTRUCTION_SELECTOR_UNITTEST_H_
#define V8_COMPILER_UNITTESTS_INSTRUCTION_SELECTOR_UNITTEST_H_

#include <deque>
#include <set>

#include "src/base/utils/random-number-generator.h"
#include "src/compiler/instruction-selector.h"
#include "src/compiler/raw-machine-assembler.h"
#include "test/compiler-unittests/compiler-unittests.h"

namespace v8 {
namespace internal {
namespace compiler {

class InstructionSelectorTest : public CompilerTest {
 public:
  InstructionSelectorTest();
  virtual ~InstructionSelectorTest() {}

  base::RandomNumberGenerator* rng() { return &rng_; }

 protected:
  class Stream;

  enum StreamBuilderMode { kAllInstructions, kTargetInstructions };

  class StreamBuilder V8_FINAL : public RawMachineAssembler {
   public:
    StreamBuilder(InstructionSelectorTest* test, MachineType return_type)
        : RawMachineAssembler(new (test->zone()) Graph(test->zone()),
                              CallDescriptorBuilder(test->zone(), return_type)),
          test_(test) {}
    StreamBuilder(InstructionSelectorTest* test, MachineType return_type,
                  MachineType parameter0_type)
        : RawMachineAssembler(new (test->zone()) Graph(test->zone()),
                              CallDescriptorBuilder(test->zone(), return_type,
                                                    parameter0_type)),
          test_(test) {}
    StreamBuilder(InstructionSelectorTest* test, MachineType return_type,
                  MachineType parameter0_type, MachineType parameter1_type)
        : RawMachineAssembler(
              new (test->zone()) Graph(test->zone()),
              CallDescriptorBuilder(test->zone(), return_type, parameter0_type,
                                    parameter1_type)),
          test_(test) {}
    StreamBuilder(InstructionSelectorTest* test, MachineType return_type,
                  MachineType parameter0_type, MachineType parameter1_type,
                  MachineType parameter2_type)
        : RawMachineAssembler(
              new (test->zone()) Graph(test->zone()),
              CallDescriptorBuilder(test->zone(), return_type, parameter0_type,
                                    parameter1_type, parameter2_type)),
          test_(test) {}

    Stream Build(CpuFeature feature) {
      return Build(InstructionSelector::Features(feature));
    }
    Stream Build(CpuFeature feature1, CpuFeature feature2) {
      return Build(InstructionSelector::Features(feature1, feature2));
    }
    Stream Build(StreamBuilderMode mode = kTargetInstructions) {
      return Build(InstructionSelector::Features(), mode);
    }
    Stream Build(InstructionSelector::Features features,
                 StreamBuilderMode mode = kTargetInstructions);

   private:
    MachineCallDescriptorBuilder* CallDescriptorBuilder(
        Zone* zone, MachineType return_type) {
      return new (zone) MachineCallDescriptorBuilder(return_type, 0, NULL);
    }

    MachineCallDescriptorBuilder* CallDescriptorBuilder(
        Zone* zone, MachineType return_type, MachineType parameter0_type) {
      MachineType* parameter_types = zone->NewArray<MachineType>(1);
      parameter_types[0] = parameter0_type;
      return new (zone)
          MachineCallDescriptorBuilder(return_type, 1, parameter_types);
    }

    MachineCallDescriptorBuilder* CallDescriptorBuilder(
        Zone* zone, MachineType return_type, MachineType parameter0_type,
        MachineType parameter1_type) {
      MachineType* parameter_types = zone->NewArray<MachineType>(2);
      parameter_types[0] = parameter0_type;
      parameter_types[1] = parameter1_type;
      return new (zone)
          MachineCallDescriptorBuilder(return_type, 2, parameter_types);
    }

    MachineCallDescriptorBuilder* CallDescriptorBuilder(
        Zone* zone, MachineType return_type, MachineType parameter0_type,
        MachineType parameter1_type, MachineType parameter2_type) {
      MachineType* parameter_types = zone->NewArray<MachineType>(3);
      parameter_types[0] = parameter0_type;
      parameter_types[1] = parameter1_type;
      parameter_types[2] = parameter2_type;
      return new (zone)
          MachineCallDescriptorBuilder(return_type, 3, parameter_types);
    }

   private:
    InstructionSelectorTest* test_;
  };

  class Stream V8_FINAL {
   public:
    size_t size() const { return instructions_.size(); }
    const Instruction* operator[](size_t index) const {
      EXPECT_LT(index, size());
      return instructions_[index];
    }

    bool IsDouble(int virtual_register) const {
      return doubles_.find(virtual_register) != doubles_.end();
    }

    bool IsReference(int virtual_register) const {
      return references_.find(virtual_register) != references_.end();
    }

    int32_t ToInt32(const InstructionOperand* operand) const {
      return ToConstant(operand).ToInt32();
    }

    int ToVreg(const InstructionOperand* operand) const {
      EXPECT_EQ(InstructionOperand::UNALLOCATED, operand->kind());
      return UnallocatedOperand::cast(operand)->virtual_register();
    }

   private:
    Constant ToConstant(const InstructionOperand* operand) const {
      ConstantMap::const_iterator i;
      if (operand->IsConstant()) {
        i = constants_.find(operand->index());
        EXPECT_FALSE(constants_.end() == i);
      } else {
        EXPECT_EQ(InstructionOperand::IMMEDIATE, operand->kind());
        i = immediates_.find(operand->index());
        EXPECT_FALSE(immediates_.end() == i);
      }
      EXPECT_EQ(operand->index(), i->first);
      return i->second;
    }

    friend class StreamBuilder;

    typedef std::map<int, Constant> ConstantMap;

    ConstantMap constants_;
    ConstantMap immediates_;
    std::deque<Instruction*> instructions_;
    std::set<int> doubles_;
    std::set<int> references_;
  };

  base::RandomNumberGenerator rng_;
};


template <typename T>
class InstructionSelectorTestWithParam
    : public InstructionSelectorTest,
      public ::testing::WithParamInterface<T> {};

}  // namespace compiler
}  // namespace internal
}  // namespace v8

#endif  // V8_COMPILER_UNITTESTS_INSTRUCTION_SELECTOR_UNITTEST_H_
