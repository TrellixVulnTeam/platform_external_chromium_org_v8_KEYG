// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "test/compiler-unittests/instruction-selector-unittest.h"

namespace v8 {
namespace internal {
namespace compiler {

namespace {

typedef RawMachineAssembler::Label MLabel;
typedef Node* (RawMachineAssembler::*Constructor)(Node*, Node*);


// Data processing instructions.
struct DPI {
  Constructor constructor;
  const char* constructor_name;
  ArchOpcode arch_opcode;
  ArchOpcode reverse_arch_opcode;
  ArchOpcode test_arch_opcode;
};


std::ostream& operator<<(std::ostream& os, const DPI& dpi) {
  return os << dpi.constructor_name;
}


static const DPI kDPIs[] = {
    {&RawMachineAssembler::Word32And, "Word32And", kArmAnd, kArmAnd, kArmTst},
    {&RawMachineAssembler::Word32Or, "Word32Or", kArmOrr, kArmOrr, kArmOrr},
    {&RawMachineAssembler::Word32Xor, "Word32Xor", kArmEor, kArmEor, kArmTeq},
    {&RawMachineAssembler::Int32Add, "Int32Add", kArmAdd, kArmAdd, kArmCmn},
    {&RawMachineAssembler::Int32Sub, "Int32Sub", kArmSub, kArmRsb, kArmCmp}};


// Data processing instructions with overflow.
struct ODPI {
  Constructor constructor;
  const char* constructor_name;
  ArchOpcode arch_opcode;
  ArchOpcode reverse_arch_opcode;
};


std::ostream& operator<<(std::ostream& os, const ODPI& odpi) {
  return os << odpi.constructor_name;
}


static const ODPI kODPIs[] = {{&RawMachineAssembler::Int32AddWithOverflow,
                               "Int32AddWithOverflow", kArmAdd, kArmAdd},
                              {&RawMachineAssembler::Int32SubWithOverflow,
                               "Int32SubWithOverflow", kArmSub, kArmRsb}};


// Shifts.
struct Shift {
  Constructor constructor;
  const char* constructor_name;
  int32_t i_low;          // lowest possible immediate
  int32_t i_high;         // highest possible immediate
  AddressingMode i_mode;  // Operand2_R_<shift>_I
  AddressingMode r_mode;  // Operand2_R_<shift>_R
};


std::ostream& operator<<(std::ostream& os, const Shift& shift) {
  return os << shift.constructor_name;
}


static const Shift kShifts[] = {
    {&RawMachineAssembler::Word32Sar, "Word32Sar", 1, 32,
     kMode_Operand2_R_ASR_I, kMode_Operand2_R_ASR_R},
    {&RawMachineAssembler::Word32Shl, "Word32Shl", 0, 31,
     kMode_Operand2_R_LSL_I, kMode_Operand2_R_LSL_R},
    {&RawMachineAssembler::Word32Shr, "Word32Shr", 1, 32,
     kMode_Operand2_R_LSR_I, kMode_Operand2_R_LSR_R},
    {&RawMachineAssembler::Word32Ror, "Word32Ror", 1, 31,
     kMode_Operand2_R_ROR_I, kMode_Operand2_R_ROR_R}};


// Immediates (random subset).
static const int32_t kImmediates[] = {
    -2147483617, -2147483606, -2113929216, -2080374784, -1996488704,
    -1879048192, -1459617792, -1358954496, -1342177265, -1275068414,
    -1073741818, -1073741777, -855638016,  -805306368,  -402653184,
    -268435444,  -16777216,   0,           35,          61,
    105,         116,         171,         245,         255,
    692,         1216,        1248,        1520,        1600,
    1888,        3744,        4080,        5888,        8384,
    9344,        9472,        9792,        13312,       15040,
    15360,       20736,       22272,       23296,       32000,
    33536,       37120,       45824,       47872,       56320,
    59392,       65280,       72704,       101376,      147456,
    161792,      164864,      167936,      173056,      195584,
    209920,      212992,      356352,      655360,      704512,
    716800,      851968,      901120,      1044480,     1523712,
    2572288,     3211264,     3588096,     3833856,     3866624,
    4325376,     5177344,     6488064,     7012352,     7471104,
    14090240,    16711680,    19398656,    22282240,    28573696,
    30408704,    30670848,    43253760,    54525952,    55312384,
    56623104,    68157440,    115343360,   131072000,   187695104,
    188743680,   195035136,   197132288,   203423744,   218103808,
    267386880,   268435470,   285212672,   402653185,   415236096,
    595591168,   603979776,   603979778,   629145600,   1073741835,
    1073741855,  1073741861,  1073741884,  1157627904,  1476395008,
    1476395010,  1610612741,  2030043136,  2080374785,  2097152000};

}  // namespace


// -----------------------------------------------------------------------------
// Data processing instructions.


typedef InstructionSelectorTestWithParam<DPI> InstructionSelectorDPITest;


TEST_P(InstructionSelectorDPITest, Parameters) {
  const DPI dpi = GetParam();
  StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32);
  m.Return((m.*dpi.constructor)(m.Parameter(0), m.Parameter(1)));
  Stream s = m.Build();
  ASSERT_EQ(1U, s.size());
  EXPECT_EQ(dpi.arch_opcode, s[0]->arch_opcode());
  EXPECT_EQ(kMode_Operand2_R, s[0]->addressing_mode());
  EXPECT_EQ(2U, s[0]->InputCount());
  EXPECT_EQ(1U, s[0]->OutputCount());
}


TEST_P(InstructionSelectorDPITest, Immediate) {
  const DPI dpi = GetParam();
  TRACED_FOREACH(int32_t, imm, kImmediates) {
    StreamBuilder m(this, kMachInt32, kMachInt32);
    m.Return((m.*dpi.constructor)(m.Parameter(0), m.Int32Constant(imm)));
    Stream s = m.Build();
    ASSERT_EQ(1U, s.size());
    EXPECT_EQ(dpi.arch_opcode, s[0]->arch_opcode());
    EXPECT_EQ(kMode_Operand2_I, s[0]->addressing_mode());
    ASSERT_EQ(2U, s[0]->InputCount());
    EXPECT_EQ(imm, s.ToInt32(s[0]->InputAt(1)));
    EXPECT_EQ(1U, s[0]->OutputCount());
  }
  TRACED_FOREACH(int32_t, imm, kImmediates) {
    StreamBuilder m(this, kMachInt32, kMachInt32);
    m.Return((m.*dpi.constructor)(m.Int32Constant(imm), m.Parameter(0)));
    Stream s = m.Build();
    ASSERT_EQ(1U, s.size());
    EXPECT_EQ(dpi.reverse_arch_opcode, s[0]->arch_opcode());
    EXPECT_EQ(kMode_Operand2_I, s[0]->addressing_mode());
    ASSERT_EQ(2U, s[0]->InputCount());
    EXPECT_EQ(imm, s.ToInt32(s[0]->InputAt(1)));
    EXPECT_EQ(1U, s[0]->OutputCount());
  }
}


TEST_P(InstructionSelectorDPITest, ShiftByParameter) {
  const DPI dpi = GetParam();
  TRACED_FOREACH(Shift, shift, kShifts) {
    StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32, kMachInt32);
    m.Return((m.*dpi.constructor)(
        m.Parameter(0),
        (m.*shift.constructor)(m.Parameter(1), m.Parameter(2))));
    Stream s = m.Build();
    ASSERT_EQ(1U, s.size());
    EXPECT_EQ(dpi.arch_opcode, s[0]->arch_opcode());
    EXPECT_EQ(shift.r_mode, s[0]->addressing_mode());
    EXPECT_EQ(3U, s[0]->InputCount());
    EXPECT_EQ(1U, s[0]->OutputCount());
  }
  TRACED_FOREACH(Shift, shift, kShifts) {
    StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32, kMachInt32);
    m.Return((m.*dpi.constructor)(
        (m.*shift.constructor)(m.Parameter(0), m.Parameter(1)),
        m.Parameter(2)));
    Stream s = m.Build();
    ASSERT_EQ(1U, s.size());
    EXPECT_EQ(dpi.reverse_arch_opcode, s[0]->arch_opcode());
    EXPECT_EQ(shift.r_mode, s[0]->addressing_mode());
    EXPECT_EQ(3U, s[0]->InputCount());
    EXPECT_EQ(1U, s[0]->OutputCount());
  }
}


TEST_P(InstructionSelectorDPITest, ShiftByImmediate) {
  const DPI dpi = GetParam();
  TRACED_FOREACH(Shift, shift, kShifts) {
    TRACED_FORRANGE(int32_t, imm, shift.i_low, shift.i_high) {
      StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32);
      m.Return((m.*dpi.constructor)(
          m.Parameter(0),
          (m.*shift.constructor)(m.Parameter(1), m.Int32Constant(imm))));
      Stream s = m.Build();
      ASSERT_EQ(1U, s.size());
      EXPECT_EQ(dpi.arch_opcode, s[0]->arch_opcode());
      EXPECT_EQ(shift.i_mode, s[0]->addressing_mode());
      ASSERT_EQ(3U, s[0]->InputCount());
      EXPECT_EQ(imm, s.ToInt32(s[0]->InputAt(2)));
      EXPECT_EQ(1U, s[0]->OutputCount());
    }
  }
  TRACED_FOREACH(Shift, shift, kShifts) {
    TRACED_FORRANGE(int32_t, imm, shift.i_low, shift.i_high) {
      StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32);
      m.Return((m.*dpi.constructor)(
          (m.*shift.constructor)(m.Parameter(0), m.Int32Constant(imm)),
          m.Parameter(1)));
      Stream s = m.Build();
      ASSERT_EQ(1U, s.size());
      EXPECT_EQ(dpi.reverse_arch_opcode, s[0]->arch_opcode());
      EXPECT_EQ(shift.i_mode, s[0]->addressing_mode());
      ASSERT_EQ(3U, s[0]->InputCount());
      EXPECT_EQ(imm, s.ToInt32(s[0]->InputAt(2)));
      EXPECT_EQ(1U, s[0]->OutputCount());
    }
  }
}


TEST_P(InstructionSelectorDPITest, BranchWithParameters) {
  const DPI dpi = GetParam();
  StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32);
  MLabel a, b;
  m.Branch((m.*dpi.constructor)(m.Parameter(0), m.Parameter(1)), &a, &b);
  m.Bind(&a);
  m.Return(m.Int32Constant(1));
  m.Bind(&b);
  m.Return(m.Int32Constant(0));
  Stream s = m.Build();
  ASSERT_EQ(1U, s.size());
  EXPECT_EQ(dpi.test_arch_opcode, s[0]->arch_opcode());
  EXPECT_EQ(kMode_Operand2_R, s[0]->addressing_mode());
  EXPECT_EQ(kFlags_branch, s[0]->flags_mode());
  EXPECT_EQ(kNotEqual, s[0]->flags_condition());
}


TEST_P(InstructionSelectorDPITest, BranchWithImmediate) {
  const DPI dpi = GetParam();
  TRACED_FOREACH(int32_t, imm, kImmediates) {
    StreamBuilder m(this, kMachInt32, kMachInt32);
    MLabel a, b;
    m.Branch((m.*dpi.constructor)(m.Parameter(0), m.Int32Constant(imm)), &a,
             &b);
    m.Bind(&a);
    m.Return(m.Int32Constant(1));
    m.Bind(&b);
    m.Return(m.Int32Constant(0));
    Stream s = m.Build();
    ASSERT_EQ(1U, s.size());
    EXPECT_EQ(dpi.test_arch_opcode, s[0]->arch_opcode());
    EXPECT_EQ(kMode_Operand2_I, s[0]->addressing_mode());
    EXPECT_EQ(kFlags_branch, s[0]->flags_mode());
    EXPECT_EQ(kNotEqual, s[0]->flags_condition());
  }
  TRACED_FOREACH(int32_t, imm, kImmediates) {
    StreamBuilder m(this, kMachInt32, kMachInt32);
    MLabel a, b;
    m.Branch((m.*dpi.constructor)(m.Int32Constant(imm), m.Parameter(0)), &a,
             &b);
    m.Bind(&a);
    m.Return(m.Int32Constant(1));
    m.Bind(&b);
    m.Return(m.Int32Constant(0));
    Stream s = m.Build();
    ASSERT_EQ(1U, s.size());
    EXPECT_EQ(dpi.test_arch_opcode, s[0]->arch_opcode());
    EXPECT_EQ(kMode_Operand2_I, s[0]->addressing_mode());
    EXPECT_EQ(kFlags_branch, s[0]->flags_mode());
    EXPECT_EQ(kNotEqual, s[0]->flags_condition());
  }
}


TEST_P(InstructionSelectorDPITest, BranchWithShiftByParameter) {
  const DPI dpi = GetParam();
  TRACED_FOREACH(Shift, shift, kShifts) {
    StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32, kMachInt32);
    MLabel a, b;
    m.Branch((m.*dpi.constructor)(
                 m.Parameter(0),
                 (m.*shift.constructor)(m.Parameter(1), m.Parameter(2))),
             &a, &b);
    m.Bind(&a);
    m.Return(m.Int32Constant(1));
    m.Bind(&b);
    m.Return(m.Int32Constant(0));
    Stream s = m.Build();
    ASSERT_EQ(1U, s.size());
    EXPECT_EQ(dpi.test_arch_opcode, s[0]->arch_opcode());
    EXPECT_EQ(shift.r_mode, s[0]->addressing_mode());
    EXPECT_EQ(kFlags_branch, s[0]->flags_mode());
    EXPECT_EQ(kNotEqual, s[0]->flags_condition());
  }
  TRACED_FOREACH(Shift, shift, kShifts) {
    StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32, kMachInt32);
    MLabel a, b;
    m.Branch((m.*dpi.constructor)(
                 (m.*shift.constructor)(m.Parameter(0), m.Parameter(1)),
                 m.Parameter(2)),
             &a, &b);
    m.Bind(&a);
    m.Return(m.Int32Constant(1));
    m.Bind(&b);
    m.Return(m.Int32Constant(0));
    Stream s = m.Build();
    ASSERT_EQ(1U, s.size());
    EXPECT_EQ(dpi.test_arch_opcode, s[0]->arch_opcode());
    EXPECT_EQ(shift.r_mode, s[0]->addressing_mode());
    EXPECT_EQ(kFlags_branch, s[0]->flags_mode());
    EXPECT_EQ(kNotEqual, s[0]->flags_condition());
  }
}


TEST_P(InstructionSelectorDPITest, BranchWithShiftByImmediate) {
  const DPI dpi = GetParam();
  TRACED_FOREACH(Shift, shift, kShifts) {
    TRACED_FORRANGE(int32_t, imm, shift.i_low, shift.i_high) {
      StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32);
      MLabel a, b;
      m.Branch((m.*dpi.constructor)(m.Parameter(0),
                                    (m.*shift.constructor)(
                                        m.Parameter(1), m.Int32Constant(imm))),
               &a, &b);
      m.Bind(&a);
      m.Return(m.Int32Constant(1));
      m.Bind(&b);
      m.Return(m.Int32Constant(0));
      Stream s = m.Build();
      ASSERT_EQ(1U, s.size());
      EXPECT_EQ(dpi.test_arch_opcode, s[0]->arch_opcode());
      EXPECT_EQ(shift.i_mode, s[0]->addressing_mode());
      ASSERT_EQ(5U, s[0]->InputCount());
      EXPECT_EQ(imm, s.ToInt32(s[0]->InputAt(2)));
      EXPECT_EQ(kFlags_branch, s[0]->flags_mode());
      EXPECT_EQ(kNotEqual, s[0]->flags_condition());
    }
  }
  TRACED_FOREACH(Shift, shift, kShifts) {
    TRACED_FORRANGE(int32_t, imm, shift.i_low, shift.i_high) {
      StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32);
      MLabel a, b;
      m.Branch((m.*dpi.constructor)(
                   (m.*shift.constructor)(m.Parameter(0), m.Int32Constant(imm)),
                   m.Parameter(1)),
               &a, &b);
      m.Bind(&a);
      m.Return(m.Int32Constant(1));
      m.Bind(&b);
      m.Return(m.Int32Constant(0));
      Stream s = m.Build();
      ASSERT_EQ(1U, s.size());
      EXPECT_EQ(dpi.test_arch_opcode, s[0]->arch_opcode());
      EXPECT_EQ(shift.i_mode, s[0]->addressing_mode());
      ASSERT_EQ(5U, s[0]->InputCount());
      EXPECT_EQ(imm, s.ToInt32(s[0]->InputAt(2)));
      EXPECT_EQ(kFlags_branch, s[0]->flags_mode());
      EXPECT_EQ(kNotEqual, s[0]->flags_condition());
    }
  }
}


TEST_P(InstructionSelectorDPITest, BranchIfZeroWithParameters) {
  const DPI dpi = GetParam();
  StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32);
  MLabel a, b;
  m.Branch(m.Word32Equal((m.*dpi.constructor)(m.Parameter(0), m.Parameter(1)),
                         m.Int32Constant(0)),
           &a, &b);
  m.Bind(&a);
  m.Return(m.Int32Constant(1));
  m.Bind(&b);
  m.Return(m.Int32Constant(0));
  Stream s = m.Build();
  ASSERT_EQ(1U, s.size());
  EXPECT_EQ(dpi.test_arch_opcode, s[0]->arch_opcode());
  EXPECT_EQ(kMode_Operand2_R, s[0]->addressing_mode());
  EXPECT_EQ(kFlags_branch, s[0]->flags_mode());
  EXPECT_EQ(kEqual, s[0]->flags_condition());
}


TEST_P(InstructionSelectorDPITest, BranchIfNotZeroWithParameters) {
  const DPI dpi = GetParam();
  StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32);
  MLabel a, b;
  m.Branch(
      m.Word32NotEqual((m.*dpi.constructor)(m.Parameter(0), m.Parameter(1)),
                       m.Int32Constant(0)),
      &a, &b);
  m.Bind(&a);
  m.Return(m.Int32Constant(1));
  m.Bind(&b);
  m.Return(m.Int32Constant(0));
  Stream s = m.Build();
  ASSERT_EQ(1U, s.size());
  EXPECT_EQ(dpi.test_arch_opcode, s[0]->arch_opcode());
  EXPECT_EQ(kMode_Operand2_R, s[0]->addressing_mode());
  EXPECT_EQ(kFlags_branch, s[0]->flags_mode());
  EXPECT_EQ(kNotEqual, s[0]->flags_condition());
}


TEST_P(InstructionSelectorDPITest, BranchIfZeroWithImmediate) {
  const DPI dpi = GetParam();
  TRACED_FOREACH(int32_t, imm, kImmediates) {
    StreamBuilder m(this, kMachInt32, kMachInt32);
    MLabel a, b;
    m.Branch(m.Word32Equal(
                 (m.*dpi.constructor)(m.Parameter(0), m.Int32Constant(imm)),
                 m.Int32Constant(0)),
             &a, &b);
    m.Bind(&a);
    m.Return(m.Int32Constant(1));
    m.Bind(&b);
    m.Return(m.Int32Constant(0));
    Stream s = m.Build();
    ASSERT_EQ(1U, s.size());
    EXPECT_EQ(dpi.test_arch_opcode, s[0]->arch_opcode());
    EXPECT_EQ(kMode_Operand2_I, s[0]->addressing_mode());
    EXPECT_EQ(kFlags_branch, s[0]->flags_mode());
    EXPECT_EQ(kEqual, s[0]->flags_condition());
  }
  TRACED_FOREACH(int32_t, imm, kImmediates) {
    StreamBuilder m(this, kMachInt32, kMachInt32);
    MLabel a, b;
    m.Branch(m.Word32Equal(
                 (m.*dpi.constructor)(m.Int32Constant(imm), m.Parameter(0)),
                 m.Int32Constant(0)),
             &a, &b);
    m.Bind(&a);
    m.Return(m.Int32Constant(1));
    m.Bind(&b);
    m.Return(m.Int32Constant(0));
    Stream s = m.Build();
    ASSERT_EQ(1U, s.size());
    EXPECT_EQ(dpi.test_arch_opcode, s[0]->arch_opcode());
    EXPECT_EQ(kMode_Operand2_I, s[0]->addressing_mode());
    EXPECT_EQ(kFlags_branch, s[0]->flags_mode());
    EXPECT_EQ(kEqual, s[0]->flags_condition());
  }
}


TEST_P(InstructionSelectorDPITest, BranchIfNotZeroWithImmediate) {
  const DPI dpi = GetParam();
  TRACED_FOREACH(int32_t, imm, kImmediates) {
    StreamBuilder m(this, kMachInt32, kMachInt32);
    MLabel a, b;
    m.Branch(m.Word32NotEqual(
                 (m.*dpi.constructor)(m.Parameter(0), m.Int32Constant(imm)),
                 m.Int32Constant(0)),
             &a, &b);
    m.Bind(&a);
    m.Return(m.Int32Constant(1));
    m.Bind(&b);
    m.Return(m.Int32Constant(0));
    Stream s = m.Build();
    ASSERT_EQ(1U, s.size());
    EXPECT_EQ(dpi.test_arch_opcode, s[0]->arch_opcode());
    EXPECT_EQ(kMode_Operand2_I, s[0]->addressing_mode());
    EXPECT_EQ(kFlags_branch, s[0]->flags_mode());
    EXPECT_EQ(kNotEqual, s[0]->flags_condition());
  }
  TRACED_FOREACH(int32_t, imm, kImmediates) {
    StreamBuilder m(this, kMachInt32, kMachInt32);
    MLabel a, b;
    m.Branch(m.Word32NotEqual(
                 (m.*dpi.constructor)(m.Int32Constant(imm), m.Parameter(0)),
                 m.Int32Constant(0)),
             &a, &b);
    m.Bind(&a);
    m.Return(m.Int32Constant(1));
    m.Bind(&b);
    m.Return(m.Int32Constant(0));
    Stream s = m.Build();
    ASSERT_EQ(1U, s.size());
    EXPECT_EQ(dpi.test_arch_opcode, s[0]->arch_opcode());
    EXPECT_EQ(kMode_Operand2_I, s[0]->addressing_mode());
    EXPECT_EQ(kFlags_branch, s[0]->flags_mode());
    EXPECT_EQ(kNotEqual, s[0]->flags_condition());
  }
}


INSTANTIATE_TEST_CASE_P(InstructionSelectorTest, InstructionSelectorDPITest,
                        ::testing::ValuesIn(kDPIs));


// -----------------------------------------------------------------------------
// Data processing instructions with overflow.


typedef InstructionSelectorTestWithParam<ODPI> InstructionSelectorODPITest;


TEST_P(InstructionSelectorODPITest, OvfWithParameters) {
  const ODPI odpi = GetParam();
  StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32);
  m.Return(
      m.Projection(1, (m.*odpi.constructor)(m.Parameter(0), m.Parameter(1))));
  Stream s = m.Build();
  ASSERT_EQ(1U, s.size());
  EXPECT_EQ(odpi.arch_opcode, s[0]->arch_opcode());
  EXPECT_EQ(kMode_Operand2_R, s[0]->addressing_mode());
  EXPECT_EQ(2U, s[0]->InputCount());
  EXPECT_LE(1U, s[0]->OutputCount());
  EXPECT_EQ(kFlags_set, s[0]->flags_mode());
  EXPECT_EQ(kOverflow, s[0]->flags_condition());
}


TEST_P(InstructionSelectorODPITest, OvfWithImmediate) {
  const ODPI odpi = GetParam();
  TRACED_FOREACH(int32_t, imm, kImmediates) {
    StreamBuilder m(this, kMachInt32, kMachInt32);
    m.Return(m.Projection(
        1, (m.*odpi.constructor)(m.Parameter(0), m.Int32Constant(imm))));
    Stream s = m.Build();
    ASSERT_EQ(1U, s.size());
    EXPECT_EQ(odpi.arch_opcode, s[0]->arch_opcode());
    EXPECT_EQ(kMode_Operand2_I, s[0]->addressing_mode());
    ASSERT_EQ(2U, s[0]->InputCount());
    EXPECT_EQ(imm, s.ToInt32(s[0]->InputAt(1)));
    EXPECT_LE(1U, s[0]->OutputCount());
    EXPECT_EQ(kFlags_set, s[0]->flags_mode());
    EXPECT_EQ(kOverflow, s[0]->flags_condition());
  }
  TRACED_FOREACH(int32_t, imm, kImmediates) {
    StreamBuilder m(this, kMachInt32, kMachInt32);
    m.Return(m.Projection(
        1, (m.*odpi.constructor)(m.Int32Constant(imm), m.Parameter(0))));
    Stream s = m.Build();
    ASSERT_EQ(1U, s.size());
    EXPECT_EQ(odpi.reverse_arch_opcode, s[0]->arch_opcode());
    EXPECT_EQ(kMode_Operand2_I, s[0]->addressing_mode());
    ASSERT_EQ(2U, s[0]->InputCount());
    EXPECT_EQ(imm, s.ToInt32(s[0]->InputAt(1)));
    EXPECT_LE(1U, s[0]->OutputCount());
    EXPECT_EQ(kFlags_set, s[0]->flags_mode());
    EXPECT_EQ(kOverflow, s[0]->flags_condition());
  }
}


TEST_P(InstructionSelectorODPITest, OvfWithShiftByParameter) {
  const ODPI odpi = GetParam();
  TRACED_FOREACH(Shift, shift, kShifts) {
    StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32, kMachInt32);
    m.Return(m.Projection(
        1, (m.*odpi.constructor)(
               m.Parameter(0),
               (m.*shift.constructor)(m.Parameter(1), m.Parameter(2)))));
    Stream s = m.Build();
    ASSERT_EQ(1U, s.size());
    EXPECT_EQ(odpi.arch_opcode, s[0]->arch_opcode());
    EXPECT_EQ(shift.r_mode, s[0]->addressing_mode());
    EXPECT_EQ(3U, s[0]->InputCount());
    EXPECT_LE(1U, s[0]->OutputCount());
    EXPECT_EQ(kFlags_set, s[0]->flags_mode());
    EXPECT_EQ(kOverflow, s[0]->flags_condition());
  }
  TRACED_FOREACH(Shift, shift, kShifts) {
    StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32, kMachInt32);
    m.Return(m.Projection(
        1, (m.*odpi.constructor)(
               (m.*shift.constructor)(m.Parameter(0), m.Parameter(1)),
               m.Parameter(0))));
    Stream s = m.Build();
    ASSERT_EQ(1U, s.size());
    EXPECT_EQ(odpi.reverse_arch_opcode, s[0]->arch_opcode());
    EXPECT_EQ(shift.r_mode, s[0]->addressing_mode());
    EXPECT_EQ(3U, s[0]->InputCount());
    EXPECT_LE(1U, s[0]->OutputCount());
    EXPECT_EQ(kFlags_set, s[0]->flags_mode());
    EXPECT_EQ(kOverflow, s[0]->flags_condition());
  }
}


TEST_P(InstructionSelectorODPITest, OvfWithShiftByImmediate) {
  const ODPI odpi = GetParam();
  TRACED_FOREACH(Shift, shift, kShifts) {
    TRACED_FORRANGE(int32_t, imm, shift.i_low, shift.i_high) {
      StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32);
      m.Return(m.Projection(
          1, (m.*odpi.constructor)(m.Parameter(0),
                                   (m.*shift.constructor)(
                                       m.Parameter(1), m.Int32Constant(imm)))));
      Stream s = m.Build();
      ASSERT_EQ(1U, s.size());
      EXPECT_EQ(odpi.arch_opcode, s[0]->arch_opcode());
      EXPECT_EQ(shift.i_mode, s[0]->addressing_mode());
      ASSERT_EQ(3U, s[0]->InputCount());
      EXPECT_EQ(imm, s.ToInt32(s[0]->InputAt(2)));
      EXPECT_LE(1U, s[0]->OutputCount());
      EXPECT_EQ(kFlags_set, s[0]->flags_mode());
      EXPECT_EQ(kOverflow, s[0]->flags_condition());
    }
  }
  TRACED_FOREACH(Shift, shift, kShifts) {
    TRACED_FORRANGE(int32_t, imm, shift.i_low, shift.i_high) {
      StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32);
      m.Return(m.Projection(
          1, (m.*odpi.constructor)(
                 (m.*shift.constructor)(m.Parameter(1), m.Int32Constant(imm)),
                 m.Parameter(0))));
      Stream s = m.Build();
      ASSERT_EQ(1U, s.size());
      EXPECT_EQ(odpi.reverse_arch_opcode, s[0]->arch_opcode());
      EXPECT_EQ(shift.i_mode, s[0]->addressing_mode());
      ASSERT_EQ(3U, s[0]->InputCount());
      EXPECT_EQ(imm, s.ToInt32(s[0]->InputAt(2)));
      EXPECT_LE(1U, s[0]->OutputCount());
      EXPECT_EQ(kFlags_set, s[0]->flags_mode());
      EXPECT_EQ(kOverflow, s[0]->flags_condition());
    }
  }
}


TEST_P(InstructionSelectorODPITest, ValWithParameters) {
  const ODPI odpi = GetParam();
  StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32);
  m.Return(
      m.Projection(0, (m.*odpi.constructor)(m.Parameter(0), m.Parameter(1))));
  Stream s = m.Build();
  ASSERT_EQ(1U, s.size());
  EXPECT_EQ(odpi.arch_opcode, s[0]->arch_opcode());
  EXPECT_EQ(kMode_Operand2_R, s[0]->addressing_mode());
  EXPECT_EQ(2U, s[0]->InputCount());
  EXPECT_LE(1U, s[0]->OutputCount());
  EXPECT_EQ(kFlags_none, s[0]->flags_mode());
}


TEST_P(InstructionSelectorODPITest, ValWithImmediate) {
  const ODPI odpi = GetParam();
  TRACED_FOREACH(int32_t, imm, kImmediates) {
    StreamBuilder m(this, kMachInt32, kMachInt32);
    m.Return(m.Projection(
        0, (m.*odpi.constructor)(m.Parameter(0), m.Int32Constant(imm))));
    Stream s = m.Build();
    ASSERT_EQ(1U, s.size());
    EXPECT_EQ(odpi.arch_opcode, s[0]->arch_opcode());
    EXPECT_EQ(kMode_Operand2_I, s[0]->addressing_mode());
    ASSERT_EQ(2U, s[0]->InputCount());
    EXPECT_EQ(imm, s.ToInt32(s[0]->InputAt(1)));
    EXPECT_LE(1U, s[0]->OutputCount());
    EXPECT_EQ(kFlags_none, s[0]->flags_mode());
  }
  TRACED_FOREACH(int32_t, imm, kImmediates) {
    StreamBuilder m(this, kMachInt32, kMachInt32);
    m.Return(m.Projection(
        0, (m.*odpi.constructor)(m.Int32Constant(imm), m.Parameter(0))));
    Stream s = m.Build();
    ASSERT_EQ(1U, s.size());
    EXPECT_EQ(odpi.reverse_arch_opcode, s[0]->arch_opcode());
    EXPECT_EQ(kMode_Operand2_I, s[0]->addressing_mode());
    ASSERT_EQ(2U, s[0]->InputCount());
    EXPECT_EQ(imm, s.ToInt32(s[0]->InputAt(1)));
    EXPECT_LE(1U, s[0]->OutputCount());
    EXPECT_EQ(kFlags_none, s[0]->flags_mode());
  }
}


TEST_P(InstructionSelectorODPITest, ValWithShiftByParameter) {
  const ODPI odpi = GetParam();
  TRACED_FOREACH(Shift, shift, kShifts) {
    StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32, kMachInt32);
    m.Return(m.Projection(
        0, (m.*odpi.constructor)(
               m.Parameter(0),
               (m.*shift.constructor)(m.Parameter(1), m.Parameter(2)))));
    Stream s = m.Build();
    ASSERT_EQ(1U, s.size());
    EXPECT_EQ(odpi.arch_opcode, s[0]->arch_opcode());
    EXPECT_EQ(shift.r_mode, s[0]->addressing_mode());
    EXPECT_EQ(3U, s[0]->InputCount());
    EXPECT_LE(1U, s[0]->OutputCount());
    EXPECT_EQ(kFlags_none, s[0]->flags_mode());
  }
  TRACED_FOREACH(Shift, shift, kShifts) {
    StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32, kMachInt32);
    m.Return(m.Projection(
        0, (m.*odpi.constructor)(
               (m.*shift.constructor)(m.Parameter(0), m.Parameter(1)),
               m.Parameter(0))));
    Stream s = m.Build();
    ASSERT_EQ(1U, s.size());
    EXPECT_EQ(odpi.reverse_arch_opcode, s[0]->arch_opcode());
    EXPECT_EQ(shift.r_mode, s[0]->addressing_mode());
    EXPECT_EQ(3U, s[0]->InputCount());
    EXPECT_LE(1U, s[0]->OutputCount());
    EXPECT_EQ(kFlags_none, s[0]->flags_mode());
  }
}


TEST_P(InstructionSelectorODPITest, ValWithShiftByImmediate) {
  const ODPI odpi = GetParam();
  TRACED_FOREACH(Shift, shift, kShifts) {
    TRACED_FORRANGE(int32_t, imm, shift.i_low, shift.i_high) {
      StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32);
      m.Return(m.Projection(
          0, (m.*odpi.constructor)(m.Parameter(0),
                                   (m.*shift.constructor)(
                                       m.Parameter(1), m.Int32Constant(imm)))));
      Stream s = m.Build();
      ASSERT_EQ(1U, s.size());
      EXPECT_EQ(odpi.arch_opcode, s[0]->arch_opcode());
      EXPECT_EQ(shift.i_mode, s[0]->addressing_mode());
      ASSERT_EQ(3U, s[0]->InputCount());
      EXPECT_EQ(imm, s.ToInt32(s[0]->InputAt(2)));
      EXPECT_LE(1U, s[0]->OutputCount());
      EXPECT_EQ(kFlags_none, s[0]->flags_mode());
    }
  }
  TRACED_FOREACH(Shift, shift, kShifts) {
    TRACED_FORRANGE(int32_t, imm, shift.i_low, shift.i_high) {
      StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32);
      m.Return(m.Projection(
          0, (m.*odpi.constructor)(
                 (m.*shift.constructor)(m.Parameter(1), m.Int32Constant(imm)),
                 m.Parameter(0))));
      Stream s = m.Build();
      ASSERT_EQ(1U, s.size());
      EXPECT_EQ(odpi.reverse_arch_opcode, s[0]->arch_opcode());
      EXPECT_EQ(shift.i_mode, s[0]->addressing_mode());
      ASSERT_EQ(3U, s[0]->InputCount());
      EXPECT_EQ(imm, s.ToInt32(s[0]->InputAt(2)));
      EXPECT_LE(1U, s[0]->OutputCount());
      EXPECT_EQ(kFlags_none, s[0]->flags_mode());
    }
  }
}


TEST_P(InstructionSelectorODPITest, BothWithParameters) {
  const ODPI odpi = GetParam();
  StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32);
  Node* n = (m.*odpi.constructor)(m.Parameter(0), m.Parameter(1));
  m.Return(m.Word32Equal(m.Projection(0, n), m.Projection(1, n)));
  Stream s = m.Build();
  ASSERT_LE(1U, s.size());
  EXPECT_EQ(odpi.arch_opcode, s[0]->arch_opcode());
  EXPECT_EQ(kMode_Operand2_R, s[0]->addressing_mode());
  EXPECT_EQ(2U, s[0]->InputCount());
  EXPECT_EQ(2U, s[0]->OutputCount());
  EXPECT_EQ(kFlags_set, s[0]->flags_mode());
  EXPECT_EQ(kOverflow, s[0]->flags_condition());
}


TEST_P(InstructionSelectorODPITest, BothWithImmediate) {
  const ODPI odpi = GetParam();
  TRACED_FOREACH(int32_t, imm, kImmediates) {
    StreamBuilder m(this, kMachInt32, kMachInt32);
    Node* n = (m.*odpi.constructor)(m.Parameter(0), m.Int32Constant(imm));
    m.Return(m.Word32Equal(m.Projection(0, n), m.Projection(1, n)));
    Stream s = m.Build();
    ASSERT_LE(1U, s.size());
    EXPECT_EQ(odpi.arch_opcode, s[0]->arch_opcode());
    EXPECT_EQ(kMode_Operand2_I, s[0]->addressing_mode());
    ASSERT_EQ(2U, s[0]->InputCount());
    EXPECT_EQ(imm, s.ToInt32(s[0]->InputAt(1)));
    EXPECT_EQ(2U, s[0]->OutputCount());
    EXPECT_EQ(kFlags_set, s[0]->flags_mode());
    EXPECT_EQ(kOverflow, s[0]->flags_condition());
  }
  TRACED_FOREACH(int32_t, imm, kImmediates) {
    StreamBuilder m(this, kMachInt32, kMachInt32);
    Node* n = (m.*odpi.constructor)(m.Int32Constant(imm), m.Parameter(0));
    m.Return(m.Word32Equal(m.Projection(0, n), m.Projection(1, n)));
    Stream s = m.Build();
    ASSERT_LE(1U, s.size());
    EXPECT_EQ(odpi.reverse_arch_opcode, s[0]->arch_opcode());
    EXPECT_EQ(kMode_Operand2_I, s[0]->addressing_mode());
    ASSERT_EQ(2U, s[0]->InputCount());
    EXPECT_EQ(imm, s.ToInt32(s[0]->InputAt(1)));
    EXPECT_EQ(2U, s[0]->OutputCount());
    EXPECT_EQ(kFlags_set, s[0]->flags_mode());
    EXPECT_EQ(kOverflow, s[0]->flags_condition());
  }
}


TEST_P(InstructionSelectorODPITest, BothWithShiftByParameter) {
  const ODPI odpi = GetParam();
  TRACED_FOREACH(Shift, shift, kShifts) {
    StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32, kMachInt32);
    Node* n = (m.*odpi.constructor)(
        m.Parameter(0), (m.*shift.constructor)(m.Parameter(1), m.Parameter(2)));
    m.Return(m.Word32Equal(m.Projection(0, n), m.Projection(1, n)));
    Stream s = m.Build();
    ASSERT_LE(1U, s.size());
    EXPECT_EQ(odpi.arch_opcode, s[0]->arch_opcode());
    EXPECT_EQ(shift.r_mode, s[0]->addressing_mode());
    EXPECT_EQ(3U, s[0]->InputCount());
    EXPECT_EQ(2U, s[0]->OutputCount());
    EXPECT_EQ(kFlags_set, s[0]->flags_mode());
    EXPECT_EQ(kOverflow, s[0]->flags_condition());
  }
  TRACED_FOREACH(Shift, shift, kShifts) {
    StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32, kMachInt32);
    Node* n = (m.*odpi.constructor)(
        (m.*shift.constructor)(m.Parameter(0), m.Parameter(1)), m.Parameter(2));
    m.Return(m.Word32Equal(m.Projection(0, n), m.Projection(1, n)));
    Stream s = m.Build();
    ASSERT_LE(1U, s.size());
    EXPECT_EQ(odpi.reverse_arch_opcode, s[0]->arch_opcode());
    EXPECT_EQ(shift.r_mode, s[0]->addressing_mode());
    EXPECT_EQ(3U, s[0]->InputCount());
    EXPECT_EQ(2U, s[0]->OutputCount());
    EXPECT_EQ(kFlags_set, s[0]->flags_mode());
    EXPECT_EQ(kOverflow, s[0]->flags_condition());
  }
}


TEST_P(InstructionSelectorODPITest, BothWithShiftByImmediate) {
  const ODPI odpi = GetParam();
  TRACED_FOREACH(Shift, shift, kShifts) {
    TRACED_FORRANGE(int32_t, imm, shift.i_low, shift.i_high) {
      StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32);
      Node* n = (m.*odpi.constructor)(
          m.Parameter(0),
          (m.*shift.constructor)(m.Parameter(1), m.Int32Constant(imm)));
      m.Return(m.Word32Equal(m.Projection(0, n), m.Projection(1, n)));
      Stream s = m.Build();
      ASSERT_LE(1U, s.size());
      EXPECT_EQ(odpi.arch_opcode, s[0]->arch_opcode());
      EXPECT_EQ(shift.i_mode, s[0]->addressing_mode());
      ASSERT_EQ(3U, s[0]->InputCount());
      EXPECT_EQ(imm, s.ToInt32(s[0]->InputAt(2)));
      EXPECT_EQ(2U, s[0]->OutputCount());
      EXPECT_EQ(kFlags_set, s[0]->flags_mode());
      EXPECT_EQ(kOverflow, s[0]->flags_condition());
    }
  }
  TRACED_FOREACH(Shift, shift, kShifts) {
    TRACED_FORRANGE(int32_t, imm, shift.i_low, shift.i_high) {
      StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32);
      Node* n = (m.*odpi.constructor)(
          (m.*shift.constructor)(m.Parameter(0), m.Int32Constant(imm)),
          m.Parameter(1));
      m.Return(m.Word32Equal(m.Projection(0, n), m.Projection(1, n)));
      Stream s = m.Build();
      ASSERT_LE(1U, s.size());
      EXPECT_EQ(odpi.reverse_arch_opcode, s[0]->arch_opcode());
      EXPECT_EQ(shift.i_mode, s[0]->addressing_mode());
      ASSERT_EQ(3U, s[0]->InputCount());
      EXPECT_EQ(imm, s.ToInt32(s[0]->InputAt(2)));
      EXPECT_EQ(2U, s[0]->OutputCount());
      EXPECT_EQ(kFlags_set, s[0]->flags_mode());
      EXPECT_EQ(kOverflow, s[0]->flags_condition());
    }
  }
}


TEST_P(InstructionSelectorODPITest, BranchWithParameters) {
  const ODPI odpi = GetParam();
  StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32);
  MLabel a, b;
  Node* n = (m.*odpi.constructor)(m.Parameter(0), m.Parameter(1));
  m.Branch(m.Projection(1, n), &a, &b);
  m.Bind(&a);
  m.Return(m.Int32Constant(0));
  m.Bind(&b);
  m.Return(m.Projection(0, n));
  Stream s = m.Build();
  ASSERT_EQ(1U, s.size());
  EXPECT_EQ(odpi.arch_opcode, s[0]->arch_opcode());
  EXPECT_EQ(kMode_Operand2_R, s[0]->addressing_mode());
  EXPECT_EQ(4U, s[0]->InputCount());
  EXPECT_EQ(1U, s[0]->OutputCount());
  EXPECT_EQ(kFlags_branch, s[0]->flags_mode());
  EXPECT_EQ(kOverflow, s[0]->flags_condition());
}


TEST_P(InstructionSelectorODPITest, BranchWithImmediate) {
  const ODPI odpi = GetParam();
  TRACED_FOREACH(int32_t, imm, kImmediates) {
    StreamBuilder m(this, kMachInt32, kMachInt32);
    MLabel a, b;
    Node* n = (m.*odpi.constructor)(m.Parameter(0), m.Int32Constant(imm));
    m.Branch(m.Projection(1, n), &a, &b);
    m.Bind(&a);
    m.Return(m.Int32Constant(0));
    m.Bind(&b);
    m.Return(m.Projection(0, n));
    Stream s = m.Build();
    ASSERT_EQ(1U, s.size());
    EXPECT_EQ(odpi.arch_opcode, s[0]->arch_opcode());
    EXPECT_EQ(kMode_Operand2_I, s[0]->addressing_mode());
    ASSERT_EQ(4U, s[0]->InputCount());
    EXPECT_EQ(imm, s.ToInt32(s[0]->InputAt(1)));
    EXPECT_EQ(1U, s[0]->OutputCount());
    EXPECT_EQ(kFlags_branch, s[0]->flags_mode());
    EXPECT_EQ(kOverflow, s[0]->flags_condition());
  }
  TRACED_FOREACH(int32_t, imm, kImmediates) {
    StreamBuilder m(this, kMachInt32, kMachInt32);
    MLabel a, b;
    Node* n = (m.*odpi.constructor)(m.Int32Constant(imm), m.Parameter(0));
    m.Branch(m.Projection(1, n), &a, &b);
    m.Bind(&a);
    m.Return(m.Int32Constant(0));
    m.Bind(&b);
    m.Return(m.Projection(0, n));
    Stream s = m.Build();
    ASSERT_EQ(1U, s.size());
    EXPECT_EQ(odpi.reverse_arch_opcode, s[0]->arch_opcode());
    EXPECT_EQ(kMode_Operand2_I, s[0]->addressing_mode());
    ASSERT_EQ(4U, s[0]->InputCount());
    EXPECT_EQ(imm, s.ToInt32(s[0]->InputAt(1)));
    EXPECT_EQ(1U, s[0]->OutputCount());
    EXPECT_EQ(kFlags_branch, s[0]->flags_mode());
    EXPECT_EQ(kOverflow, s[0]->flags_condition());
  }
}


TEST_P(InstructionSelectorODPITest, BranchIfZeroWithParameters) {
  const ODPI odpi = GetParam();
  StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32);
  MLabel a, b;
  Node* n = (m.*odpi.constructor)(m.Parameter(0), m.Parameter(1));
  m.Branch(m.Word32Equal(m.Projection(1, n), m.Int32Constant(0)), &a, &b);
  m.Bind(&a);
  m.Return(m.Projection(0, n));
  m.Bind(&b);
  m.Return(m.Int32Constant(0));
  Stream s = m.Build();
  ASSERT_EQ(1U, s.size());
  EXPECT_EQ(odpi.arch_opcode, s[0]->arch_opcode());
  EXPECT_EQ(kMode_Operand2_R, s[0]->addressing_mode());
  EXPECT_EQ(4U, s[0]->InputCount());
  EXPECT_EQ(1U, s[0]->OutputCount());
  EXPECT_EQ(kFlags_branch, s[0]->flags_mode());
  EXPECT_EQ(kNotOverflow, s[0]->flags_condition());
}


TEST_P(InstructionSelectorODPITest, BranchIfNotZeroWithParameters) {
  const ODPI odpi = GetParam();
  StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32);
  MLabel a, b;
  Node* n = (m.*odpi.constructor)(m.Parameter(0), m.Parameter(1));
  m.Branch(m.Word32NotEqual(m.Projection(1, n), m.Int32Constant(0)), &a, &b);
  m.Bind(&a);
  m.Return(m.Projection(0, n));
  m.Bind(&b);
  m.Return(m.Int32Constant(0));
  Stream s = m.Build();
  ASSERT_EQ(1U, s.size());
  EXPECT_EQ(odpi.arch_opcode, s[0]->arch_opcode());
  EXPECT_EQ(kMode_Operand2_R, s[0]->addressing_mode());
  EXPECT_EQ(4U, s[0]->InputCount());
  EXPECT_EQ(1U, s[0]->OutputCount());
  EXPECT_EQ(kFlags_branch, s[0]->flags_mode());
  EXPECT_EQ(kOverflow, s[0]->flags_condition());
}


INSTANTIATE_TEST_CASE_P(InstructionSelectorTest, InstructionSelectorODPITest,
                        ::testing::ValuesIn(kODPIs));


// -----------------------------------------------------------------------------
// Shifts.


typedef InstructionSelectorTestWithParam<Shift> InstructionSelectorShiftTest;


TEST_P(InstructionSelectorShiftTest, Parameters) {
  const Shift shift = GetParam();
  StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32);
  m.Return((m.*shift.constructor)(m.Parameter(0), m.Parameter(1)));
  Stream s = m.Build();
  ASSERT_EQ(1U, s.size());
  EXPECT_EQ(kArmMov, s[0]->arch_opcode());
  EXPECT_EQ(shift.r_mode, s[0]->addressing_mode());
  EXPECT_EQ(2U, s[0]->InputCount());
  EXPECT_EQ(1U, s[0]->OutputCount());
}


TEST_P(InstructionSelectorShiftTest, Immediate) {
  const Shift shift = GetParam();
  TRACED_FORRANGE(int32_t, imm, shift.i_low, shift.i_high) {
    StreamBuilder m(this, kMachInt32, kMachInt32);
    m.Return((m.*shift.constructor)(m.Parameter(0), m.Int32Constant(imm)));
    Stream s = m.Build();
    ASSERT_EQ(1U, s.size());
    EXPECT_EQ(kArmMov, s[0]->arch_opcode());
    EXPECT_EQ(shift.i_mode, s[0]->addressing_mode());
    ASSERT_EQ(2U, s[0]->InputCount());
    EXPECT_EQ(imm, s.ToInt32(s[0]->InputAt(1)));
    EXPECT_EQ(1U, s[0]->OutputCount());
  }
}


TEST_P(InstructionSelectorShiftTest, Word32EqualWithParameter) {
  const Shift shift = GetParam();
  {
    StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32, kMachInt32);
    m.Return(
        m.Word32Equal(m.Parameter(0),
                      (m.*shift.constructor)(m.Parameter(1), m.Parameter(2))));
    Stream s = m.Build();
    ASSERT_EQ(1U, s.size());
    EXPECT_EQ(kArmCmp, s[0]->arch_opcode());
    EXPECT_EQ(shift.r_mode, s[0]->addressing_mode());
    EXPECT_EQ(3U, s[0]->InputCount());
    EXPECT_EQ(1U, s[0]->OutputCount());
    EXPECT_EQ(kFlags_set, s[0]->flags_mode());
    EXPECT_EQ(kEqual, s[0]->flags_condition());
  }
  {
    StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32, kMachInt32);
    m.Return(
        m.Word32Equal((m.*shift.constructor)(m.Parameter(1), m.Parameter(2)),
                      m.Parameter(0)));
    Stream s = m.Build();
    ASSERT_EQ(1U, s.size());
    EXPECT_EQ(kArmCmp, s[0]->arch_opcode());
    EXPECT_EQ(shift.r_mode, s[0]->addressing_mode());
    EXPECT_EQ(3U, s[0]->InputCount());
    EXPECT_EQ(1U, s[0]->OutputCount());
    EXPECT_EQ(kFlags_set, s[0]->flags_mode());
    EXPECT_EQ(kEqual, s[0]->flags_condition());
  }
}


TEST_P(InstructionSelectorShiftTest, Word32EqualWithParameterAndImmediate) {
  const Shift shift = GetParam();
  TRACED_FORRANGE(int32_t, imm, shift.i_low, shift.i_high) {
    StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32);
    m.Return(m.Word32Equal(
        (m.*shift.constructor)(m.Parameter(1), m.Int32Constant(imm)),
        m.Parameter(0)));
    Stream s = m.Build();
    ASSERT_EQ(1U, s.size());
    EXPECT_EQ(kArmCmp, s[0]->arch_opcode());
    EXPECT_EQ(shift.i_mode, s[0]->addressing_mode());
    ASSERT_EQ(3U, s[0]->InputCount());
    EXPECT_EQ(imm, s.ToInt32(s[0]->InputAt(2)));
    EXPECT_EQ(1U, s[0]->OutputCount());
    EXPECT_EQ(kFlags_set, s[0]->flags_mode());
    EXPECT_EQ(kEqual, s[0]->flags_condition());
  }
  TRACED_FORRANGE(int32_t, imm, shift.i_low, shift.i_high) {
    StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32);
    m.Return(m.Word32Equal(
        m.Parameter(0),
        (m.*shift.constructor)(m.Parameter(1), m.Int32Constant(imm))));
    Stream s = m.Build();
    ASSERT_EQ(1U, s.size());
    EXPECT_EQ(kArmCmp, s[0]->arch_opcode());
    EXPECT_EQ(shift.i_mode, s[0]->addressing_mode());
    ASSERT_EQ(3U, s[0]->InputCount());
    EXPECT_EQ(imm, s.ToInt32(s[0]->InputAt(2)));
    EXPECT_EQ(1U, s[0]->OutputCount());
    EXPECT_EQ(kFlags_set, s[0]->flags_mode());
    EXPECT_EQ(kEqual, s[0]->flags_condition());
  }
}


TEST_P(InstructionSelectorShiftTest, Word32EqualToZeroWithParameters) {
  const Shift shift = GetParam();
  StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32);
  m.Return(
      m.Word32Equal(m.Int32Constant(0),
                    (m.*shift.constructor)(m.Parameter(0), m.Parameter(1))));
  Stream s = m.Build();
  ASSERT_EQ(1U, s.size());
  EXPECT_EQ(kArmMov, s[0]->arch_opcode());
  EXPECT_EQ(shift.r_mode, s[0]->addressing_mode());
  EXPECT_EQ(2U, s[0]->InputCount());
  EXPECT_EQ(2U, s[0]->OutputCount());
  EXPECT_EQ(kFlags_set, s[0]->flags_mode());
  EXPECT_EQ(kEqual, s[0]->flags_condition());
}


TEST_P(InstructionSelectorShiftTest, Word32EqualToZeroWithImmediate) {
  const Shift shift = GetParam();
  TRACED_FORRANGE(int32_t, imm, shift.i_low, shift.i_high) {
    StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32);
    m.Return(m.Word32Equal(
        m.Int32Constant(0),
        (m.*shift.constructor)(m.Parameter(0), m.Int32Constant(imm))));
    Stream s = m.Build();
    ASSERT_EQ(1U, s.size());
    EXPECT_EQ(kArmMov, s[0]->arch_opcode());
    EXPECT_EQ(shift.i_mode, s[0]->addressing_mode());
    ASSERT_EQ(2U, s[0]->InputCount());
    EXPECT_EQ(imm, s.ToInt32(s[0]->InputAt(1)));
    EXPECT_EQ(2U, s[0]->OutputCount());
    EXPECT_EQ(kFlags_set, s[0]->flags_mode());
    EXPECT_EQ(kEqual, s[0]->flags_condition());
  }
}


TEST_P(InstructionSelectorShiftTest, Word32NotWithParameters) {
  const Shift shift = GetParam();
  StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32);
  m.Return(m.Word32Not((m.*shift.constructor)(m.Parameter(0), m.Parameter(1))));
  Stream s = m.Build();
  ASSERT_EQ(1U, s.size());
  EXPECT_EQ(kArmMvn, s[0]->arch_opcode());
  EXPECT_EQ(shift.r_mode, s[0]->addressing_mode());
  EXPECT_EQ(2U, s[0]->InputCount());
  EXPECT_EQ(1U, s[0]->OutputCount());
}


TEST_P(InstructionSelectorShiftTest, Word32NotWithImmediate) {
  const Shift shift = GetParam();
  TRACED_FORRANGE(int32_t, imm, shift.i_low, shift.i_high) {
    StreamBuilder m(this, kMachInt32, kMachInt32);
    m.Return(m.Word32Not(
        (m.*shift.constructor)(m.Parameter(0), m.Int32Constant(imm))));
    Stream s = m.Build();
    ASSERT_EQ(1U, s.size());
    EXPECT_EQ(kArmMvn, s[0]->arch_opcode());
    EXPECT_EQ(shift.i_mode, s[0]->addressing_mode());
    ASSERT_EQ(2U, s[0]->InputCount());
    EXPECT_EQ(imm, s.ToInt32(s[0]->InputAt(1)));
    EXPECT_EQ(1U, s[0]->OutputCount());
  }
}


TEST_P(InstructionSelectorShiftTest, Word32AndWithWord32NotWithParameters) {
  const Shift shift = GetParam();
  StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32, kMachInt32);
  m.Return(m.Word32And(m.Parameter(0), m.Word32Not((m.*shift.constructor)(
                                           m.Parameter(1), m.Parameter(2)))));
  Stream s = m.Build();
  ASSERT_EQ(1U, s.size());
  EXPECT_EQ(kArmBic, s[0]->arch_opcode());
  EXPECT_EQ(shift.r_mode, s[0]->addressing_mode());
  EXPECT_EQ(3U, s[0]->InputCount());
  EXPECT_EQ(1U, s[0]->OutputCount());
}


TEST_P(InstructionSelectorShiftTest, Word32AndWithWord32NotWithImmediate) {
  const Shift shift = GetParam();
  TRACED_FORRANGE(int32_t, imm, shift.i_low, shift.i_high) {
    StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32);
    m.Return(m.Word32And(m.Parameter(0),
                         m.Word32Not((m.*shift.constructor)(
                             m.Parameter(1), m.Int32Constant(imm)))));
    Stream s = m.Build();
    ASSERT_EQ(1U, s.size());
    EXPECT_EQ(kArmBic, s[0]->arch_opcode());
    EXPECT_EQ(shift.i_mode, s[0]->addressing_mode());
    ASSERT_EQ(3U, s[0]->InputCount());
    EXPECT_EQ(imm, s.ToInt32(s[0]->InputAt(2)));
    EXPECT_EQ(1U, s[0]->OutputCount());
  }
}


INSTANTIATE_TEST_CASE_P(InstructionSelectorTest, InstructionSelectorShiftTest,
                        ::testing::ValuesIn(kShifts));


// -----------------------------------------------------------------------------
// Miscellaneous.


TEST_F(InstructionSelectorTest, Int32AddWithInt32Mul) {
  {
    StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32, kMachInt32);
    m.Return(
        m.Int32Add(m.Parameter(0), m.Int32Mul(m.Parameter(1), m.Parameter(2))));
    Stream s = m.Build();
    ASSERT_EQ(1U, s.size());
    EXPECT_EQ(kArmMla, s[0]->arch_opcode());
    EXPECT_EQ(3U, s[0]->InputCount());
    EXPECT_EQ(1U, s[0]->OutputCount());
  }
  {
    StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32, kMachInt32);
    m.Return(
        m.Int32Add(m.Int32Mul(m.Parameter(1), m.Parameter(2)), m.Parameter(0)));
    Stream s = m.Build();
    ASSERT_EQ(1U, s.size());
    EXPECT_EQ(kArmMla, s[0]->arch_opcode());
    EXPECT_EQ(3U, s[0]->InputCount());
    EXPECT_EQ(1U, s[0]->OutputCount());
  }
}


TEST_F(InstructionSelectorTest, Int32DivWithParameters) {
  StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32);
  m.Return(m.Int32Div(m.Parameter(0), m.Parameter(1)));
  Stream s = m.Build();
  ASSERT_EQ(4U, s.size());
  EXPECT_EQ(kArmVcvtF64S32, s[0]->arch_opcode());
  ASSERT_EQ(1U, s[0]->OutputCount());
  EXPECT_EQ(kArmVcvtF64S32, s[1]->arch_opcode());
  ASSERT_EQ(1U, s[1]->OutputCount());
  EXPECT_EQ(kArmVdivF64, s[2]->arch_opcode());
  ASSERT_EQ(2U, s[2]->InputCount());
  ASSERT_EQ(1U, s[2]->OutputCount());
  EXPECT_EQ(s.ToVreg(s[0]->Output()), s.ToVreg(s[2]->InputAt(0)));
  EXPECT_EQ(s.ToVreg(s[1]->Output()), s.ToVreg(s[2]->InputAt(1)));
  EXPECT_EQ(kArmVcvtS32F64, s[3]->arch_opcode());
  ASSERT_EQ(1U, s[3]->InputCount());
  EXPECT_EQ(s.ToVreg(s[2]->Output()), s.ToVreg(s[3]->InputAt(0)));
}


TEST_F(InstructionSelectorTest, Int32DivWithParametersForSUDIV) {
  StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32);
  m.Return(m.Int32Div(m.Parameter(0), m.Parameter(1)));
  Stream s = m.Build(SUDIV);
  ASSERT_EQ(1U, s.size());
  EXPECT_EQ(kArmSdiv, s[0]->arch_opcode());
}


TEST_F(InstructionSelectorTest, Int32ModWithParameters) {
  StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32);
  m.Return(m.Int32Mod(m.Parameter(0), m.Parameter(1)));
  Stream s = m.Build();
  ASSERT_EQ(6U, s.size());
  EXPECT_EQ(kArmVcvtF64S32, s[0]->arch_opcode());
  ASSERT_EQ(1U, s[0]->OutputCount());
  EXPECT_EQ(kArmVcvtF64S32, s[1]->arch_opcode());
  ASSERT_EQ(1U, s[1]->OutputCount());
  EXPECT_EQ(kArmVdivF64, s[2]->arch_opcode());
  ASSERT_EQ(2U, s[2]->InputCount());
  ASSERT_EQ(1U, s[2]->OutputCount());
  EXPECT_EQ(s.ToVreg(s[0]->Output()), s.ToVreg(s[2]->InputAt(0)));
  EXPECT_EQ(s.ToVreg(s[1]->Output()), s.ToVreg(s[2]->InputAt(1)));
  EXPECT_EQ(kArmVcvtS32F64, s[3]->arch_opcode());
  ASSERT_EQ(1U, s[3]->InputCount());
  EXPECT_EQ(s.ToVreg(s[2]->Output()), s.ToVreg(s[3]->InputAt(0)));
  EXPECT_EQ(kArmMul, s[4]->arch_opcode());
  ASSERT_EQ(1U, s[4]->OutputCount());
  ASSERT_EQ(2U, s[4]->InputCount());
  EXPECT_EQ(s.ToVreg(s[3]->Output()), s.ToVreg(s[4]->InputAt(0)));
  EXPECT_EQ(s.ToVreg(s[1]->InputAt(0)), s.ToVreg(s[4]->InputAt(1)));
  EXPECT_EQ(kArmSub, s[5]->arch_opcode());
  ASSERT_EQ(1U, s[5]->OutputCount());
  ASSERT_EQ(2U, s[5]->InputCount());
  EXPECT_EQ(s.ToVreg(s[0]->InputAt(0)), s.ToVreg(s[5]->InputAt(0)));
  EXPECT_EQ(s.ToVreg(s[4]->Output()), s.ToVreg(s[5]->InputAt(1)));
}


TEST_F(InstructionSelectorTest, Int32ModWithParametersForSUDIV) {
  StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32);
  m.Return(m.Int32Mod(m.Parameter(0), m.Parameter(1)));
  Stream s = m.Build(SUDIV);
  ASSERT_EQ(3U, s.size());
  EXPECT_EQ(kArmSdiv, s[0]->arch_opcode());
  ASSERT_EQ(1U, s[0]->OutputCount());
  ASSERT_EQ(2U, s[0]->InputCount());
  EXPECT_EQ(kArmMul, s[1]->arch_opcode());
  ASSERT_EQ(1U, s[1]->OutputCount());
  ASSERT_EQ(2U, s[1]->InputCount());
  EXPECT_EQ(s.ToVreg(s[0]->Output()), s.ToVreg(s[1]->InputAt(0)));
  EXPECT_EQ(s.ToVreg(s[0]->InputAt(1)), s.ToVreg(s[1]->InputAt(1)));
  EXPECT_EQ(kArmSub, s[2]->arch_opcode());
  ASSERT_EQ(1U, s[2]->OutputCount());
  ASSERT_EQ(2U, s[2]->InputCount());
  EXPECT_EQ(s.ToVreg(s[0]->InputAt(0)), s.ToVreg(s[2]->InputAt(0)));
  EXPECT_EQ(s.ToVreg(s[1]->Output()), s.ToVreg(s[2]->InputAt(1)));
}


TEST_F(InstructionSelectorTest, Int32ModWithParametersForSUDIVAndMLS) {
  StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32);
  m.Return(m.Int32Mod(m.Parameter(0), m.Parameter(1)));
  Stream s = m.Build(MLS, SUDIV);
  ASSERT_EQ(2U, s.size());
  EXPECT_EQ(kArmSdiv, s[0]->arch_opcode());
  ASSERT_EQ(1U, s[0]->OutputCount());
  ASSERT_EQ(2U, s[0]->InputCount());
  EXPECT_EQ(kArmMls, s[1]->arch_opcode());
  ASSERT_EQ(1U, s[1]->OutputCount());
  ASSERT_EQ(3U, s[1]->InputCount());
  EXPECT_EQ(s.ToVreg(s[0]->Output()), s.ToVreg(s[1]->InputAt(0)));
  EXPECT_EQ(s.ToVreg(s[0]->InputAt(1)), s.ToVreg(s[1]->InputAt(1)));
  EXPECT_EQ(s.ToVreg(s[0]->InputAt(0)), s.ToVreg(s[1]->InputAt(2)));
}


TEST_F(InstructionSelectorTest, Int32MulWithParameters) {
  StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32);
  m.Return(m.Int32Mul(m.Parameter(0), m.Parameter(1)));
  Stream s = m.Build();
  ASSERT_EQ(1U, s.size());
  EXPECT_EQ(kArmMul, s[0]->arch_opcode());
  EXPECT_EQ(2U, s[0]->InputCount());
  EXPECT_EQ(1U, s[0]->OutputCount());
}


TEST_F(InstructionSelectorTest, Int32MulWithImmediate) {
  // x * (2^k + 1) -> x + (x >> k)
  TRACED_FORRANGE(int32_t, k, 1, 30) {
    StreamBuilder m(this, kMachInt32, kMachInt32);
    m.Return(m.Int32Mul(m.Parameter(0), m.Int32Constant((1 << k) + 1)));
    Stream s = m.Build();
    ASSERT_EQ(1U, s.size());
    EXPECT_EQ(kArmAdd, s[0]->arch_opcode());
    EXPECT_EQ(kMode_Operand2_R_LSL_I, s[0]->addressing_mode());
    ASSERT_EQ(3U, s[0]->InputCount());
    EXPECT_EQ(s.ToVreg(s[0]->InputAt(0)), s.ToVreg(s[0]->InputAt(1)));
    EXPECT_EQ(k, s.ToInt32(s[0]->InputAt(2)));
    EXPECT_EQ(1U, s[0]->OutputCount());
  }
  // x * (2^k - 1) -> -x + (x >> k)
  TRACED_FORRANGE(int32_t, k, 3, 30) {
    StreamBuilder m(this, kMachInt32, kMachInt32);
    m.Return(m.Int32Mul(m.Parameter(0), m.Int32Constant((1 << k) - 1)));
    Stream s = m.Build();
    ASSERT_EQ(1U, s.size());
    EXPECT_EQ(kArmRsb, s[0]->arch_opcode());
    EXPECT_EQ(kMode_Operand2_R_LSL_I, s[0]->addressing_mode());
    ASSERT_EQ(3U, s[0]->InputCount());
    EXPECT_EQ(s.ToVreg(s[0]->InputAt(0)), s.ToVreg(s[0]->InputAt(1)));
    EXPECT_EQ(k, s.ToInt32(s[0]->InputAt(2)));
    EXPECT_EQ(1U, s[0]->OutputCount());
  }
  // (2^k + 1) * x -> x + (x >> k)
  TRACED_FORRANGE(int32_t, k, 1, 30) {
    StreamBuilder m(this, kMachInt32, kMachInt32);
    m.Return(m.Int32Mul(m.Int32Constant((1 << k) + 1), m.Parameter(0)));
    Stream s = m.Build();
    ASSERT_EQ(1U, s.size());
    EXPECT_EQ(kArmAdd, s[0]->arch_opcode());
    EXPECT_EQ(kMode_Operand2_R_LSL_I, s[0]->addressing_mode());
    ASSERT_EQ(3U, s[0]->InputCount());
    EXPECT_EQ(s.ToVreg(s[0]->InputAt(0)), s.ToVreg(s[0]->InputAt(1)));
    EXPECT_EQ(k, s.ToInt32(s[0]->InputAt(2)));
    EXPECT_EQ(1U, s[0]->OutputCount());
  }
  // x * (2^k - 1) -> -x + (x >> k)
  TRACED_FORRANGE(int32_t, k, 3, 30) {
    StreamBuilder m(this, kMachInt32, kMachInt32);
    m.Return(m.Int32Mul(m.Int32Constant((1 << k) - 1), m.Parameter(0)));
    Stream s = m.Build();
    ASSERT_EQ(1U, s.size());
    EXPECT_EQ(kArmRsb, s[0]->arch_opcode());
    EXPECT_EQ(kMode_Operand2_R_LSL_I, s[0]->addressing_mode());
    ASSERT_EQ(3U, s[0]->InputCount());
    EXPECT_EQ(s.ToVreg(s[0]->InputAt(0)), s.ToVreg(s[0]->InputAt(1)));
    EXPECT_EQ(k, s.ToInt32(s[0]->InputAt(2)));
    EXPECT_EQ(1U, s[0]->OutputCount());
  }
}


TEST_F(InstructionSelectorTest, Int32SubWithInt32Mul) {
  StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32, kMachInt32);
  m.Return(
      m.Int32Sub(m.Parameter(0), m.Int32Mul(m.Parameter(1), m.Parameter(2))));
  Stream s = m.Build();
  ASSERT_EQ(2U, s.size());
  EXPECT_EQ(kArmMul, s[0]->arch_opcode());
  ASSERT_EQ(1U, s[0]->OutputCount());
  EXPECT_EQ(kArmSub, s[1]->arch_opcode());
  ASSERT_EQ(2U, s[1]->InputCount());
  EXPECT_EQ(s.ToVreg(s[0]->Output()), s.ToVreg(s[1]->InputAt(1)));
}


TEST_F(InstructionSelectorTest, Int32SubWithInt32MulForMLS) {
  StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32, kMachInt32);
  m.Return(
      m.Int32Sub(m.Parameter(0), m.Int32Mul(m.Parameter(1), m.Parameter(2))));
  Stream s = m.Build(MLS);
  ASSERT_EQ(1U, s.size());
  EXPECT_EQ(kArmMls, s[0]->arch_opcode());
  EXPECT_EQ(1U, s[0]->OutputCount());
  EXPECT_EQ(3U, s[0]->InputCount());
}


TEST_F(InstructionSelectorTest, Int32UDivWithParameters) {
  StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32);
  m.Return(m.Int32UDiv(m.Parameter(0), m.Parameter(1)));
  Stream s = m.Build();
  ASSERT_EQ(4U, s.size());
  EXPECT_EQ(kArmVcvtF64U32, s[0]->arch_opcode());
  ASSERT_EQ(1U, s[0]->OutputCount());
  EXPECT_EQ(kArmVcvtF64U32, s[1]->arch_opcode());
  ASSERT_EQ(1U, s[1]->OutputCount());
  EXPECT_EQ(kArmVdivF64, s[2]->arch_opcode());
  ASSERT_EQ(2U, s[2]->InputCount());
  ASSERT_EQ(1U, s[2]->OutputCount());
  EXPECT_EQ(s.ToVreg(s[0]->Output()), s.ToVreg(s[2]->InputAt(0)));
  EXPECT_EQ(s.ToVreg(s[1]->Output()), s.ToVreg(s[2]->InputAt(1)));
  EXPECT_EQ(kArmVcvtU32F64, s[3]->arch_opcode());
  ASSERT_EQ(1U, s[3]->InputCount());
  EXPECT_EQ(s.ToVreg(s[2]->Output()), s.ToVreg(s[3]->InputAt(0)));
}


TEST_F(InstructionSelectorTest, Int32UDivWithParametersForSUDIV) {
  StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32);
  m.Return(m.Int32UDiv(m.Parameter(0), m.Parameter(1)));
  Stream s = m.Build(SUDIV);
  ASSERT_EQ(1U, s.size());
  EXPECT_EQ(kArmUdiv, s[0]->arch_opcode());
}


TEST_F(InstructionSelectorTest, Int32UModWithParameters) {
  StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32);
  m.Return(m.Int32UMod(m.Parameter(0), m.Parameter(1)));
  Stream s = m.Build();
  ASSERT_EQ(6U, s.size());
  EXPECT_EQ(kArmVcvtF64U32, s[0]->arch_opcode());
  ASSERT_EQ(1U, s[0]->OutputCount());
  EXPECT_EQ(kArmVcvtF64U32, s[1]->arch_opcode());
  ASSERT_EQ(1U, s[1]->OutputCount());
  EXPECT_EQ(kArmVdivF64, s[2]->arch_opcode());
  ASSERT_EQ(2U, s[2]->InputCount());
  ASSERT_EQ(1U, s[2]->OutputCount());
  EXPECT_EQ(s.ToVreg(s[0]->Output()), s.ToVreg(s[2]->InputAt(0)));
  EXPECT_EQ(s.ToVreg(s[1]->Output()), s.ToVreg(s[2]->InputAt(1)));
  EXPECT_EQ(kArmVcvtU32F64, s[3]->arch_opcode());
  ASSERT_EQ(1U, s[3]->InputCount());
  EXPECT_EQ(s.ToVreg(s[2]->Output()), s.ToVreg(s[3]->InputAt(0)));
  EXPECT_EQ(kArmMul, s[4]->arch_opcode());
  ASSERT_EQ(1U, s[4]->OutputCount());
  ASSERT_EQ(2U, s[4]->InputCount());
  EXPECT_EQ(s.ToVreg(s[3]->Output()), s.ToVreg(s[4]->InputAt(0)));
  EXPECT_EQ(s.ToVreg(s[1]->InputAt(0)), s.ToVreg(s[4]->InputAt(1)));
  EXPECT_EQ(kArmSub, s[5]->arch_opcode());
  ASSERT_EQ(1U, s[5]->OutputCount());
  ASSERT_EQ(2U, s[5]->InputCount());
  EXPECT_EQ(s.ToVreg(s[0]->InputAt(0)), s.ToVreg(s[5]->InputAt(0)));
  EXPECT_EQ(s.ToVreg(s[4]->Output()), s.ToVreg(s[5]->InputAt(1)));
}


TEST_F(InstructionSelectorTest, Int32UModWithParametersForSUDIV) {
  StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32);
  m.Return(m.Int32UMod(m.Parameter(0), m.Parameter(1)));
  Stream s = m.Build(SUDIV);
  ASSERT_EQ(3U, s.size());
  EXPECT_EQ(kArmUdiv, s[0]->arch_opcode());
  ASSERT_EQ(1U, s[0]->OutputCount());
  ASSERT_EQ(2U, s[0]->InputCount());
  EXPECT_EQ(kArmMul, s[1]->arch_opcode());
  ASSERT_EQ(1U, s[1]->OutputCount());
  ASSERT_EQ(2U, s[1]->InputCount());
  EXPECT_EQ(s.ToVreg(s[0]->Output()), s.ToVreg(s[1]->InputAt(0)));
  EXPECT_EQ(s.ToVreg(s[0]->InputAt(1)), s.ToVreg(s[1]->InputAt(1)));
  EXPECT_EQ(kArmSub, s[2]->arch_opcode());
  ASSERT_EQ(1U, s[2]->OutputCount());
  ASSERT_EQ(2U, s[2]->InputCount());
  EXPECT_EQ(s.ToVreg(s[0]->InputAt(0)), s.ToVreg(s[2]->InputAt(0)));
  EXPECT_EQ(s.ToVreg(s[1]->Output()), s.ToVreg(s[2]->InputAt(1)));
}


TEST_F(InstructionSelectorTest, Int32UModWithParametersForSUDIVAndMLS) {
  StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32);
  m.Return(m.Int32UMod(m.Parameter(0), m.Parameter(1)));
  Stream s = m.Build(MLS, SUDIV);
  ASSERT_EQ(2U, s.size());
  EXPECT_EQ(kArmUdiv, s[0]->arch_opcode());
  ASSERT_EQ(1U, s[0]->OutputCount());
  ASSERT_EQ(2U, s[0]->InputCount());
  EXPECT_EQ(kArmMls, s[1]->arch_opcode());
  ASSERT_EQ(1U, s[1]->OutputCount());
  ASSERT_EQ(3U, s[1]->InputCount());
  EXPECT_EQ(s.ToVreg(s[0]->Output()), s.ToVreg(s[1]->InputAt(0)));
  EXPECT_EQ(s.ToVreg(s[0]->InputAt(1)), s.ToVreg(s[1]->InputAt(1)));
  EXPECT_EQ(s.ToVreg(s[0]->InputAt(0)), s.ToVreg(s[1]->InputAt(2)));
}


TEST_F(InstructionSelectorTest, Word32AndWithUbfxImmediateForARMv7) {
  TRACED_FORRANGE(int32_t, width, 1, 32) {
    StreamBuilder m(this, kMachInt32, kMachInt32);
    m.Return(m.Word32And(m.Parameter(0),
                         m.Int32Constant(0xffffffffu >> (32 - width))));
    Stream s = m.Build(ARMv7);
    ASSERT_EQ(1U, s.size());
    EXPECT_EQ(kArmUbfx, s[0]->arch_opcode());
    ASSERT_EQ(3U, s[0]->InputCount());
    EXPECT_EQ(0, s.ToInt32(s[0]->InputAt(1)));
    EXPECT_EQ(width, s.ToInt32(s[0]->InputAt(2)));
  }
  TRACED_FORRANGE(int32_t, width, 1, 32) {
    StreamBuilder m(this, kMachInt32, kMachInt32);
    m.Return(m.Word32And(m.Int32Constant(0xffffffffu >> (32 - width)),
                         m.Parameter(0)));
    Stream s = m.Build(ARMv7);
    ASSERT_EQ(1U, s.size());
    EXPECT_EQ(kArmUbfx, s[0]->arch_opcode());
    ASSERT_EQ(3U, s[0]->InputCount());
    EXPECT_EQ(0, s.ToInt32(s[0]->InputAt(1)));
    EXPECT_EQ(width, s.ToInt32(s[0]->InputAt(2)));
  }
}


TEST_F(InstructionSelectorTest, Word32AndWithBfcImmediateForARMv7) {
  TRACED_FORRANGE(int32_t, lsb, 0, 31) {
    TRACED_FORRANGE(int32_t, width, 1, (32 - lsb) - 1) {
      StreamBuilder m(this, kMachInt32, kMachInt32);
      m.Return(m.Word32And(
          m.Parameter(0),
          m.Int32Constant(~((0xffffffffu >> (32 - width)) << lsb))));
      Stream s = m.Build(ARMv7);
      ASSERT_EQ(1U, s.size());
      EXPECT_EQ(kArmBfc, s[0]->arch_opcode());
      ASSERT_EQ(1U, s[0]->OutputCount());
      EXPECT_TRUE(
          UnallocatedOperand::cast(s[0]->Output())->HasSameAsInputPolicy());
      ASSERT_EQ(3U, s[0]->InputCount());
      EXPECT_EQ(lsb, s.ToInt32(s[0]->InputAt(1)));
      EXPECT_EQ(width, s.ToInt32(s[0]->InputAt(2)));
    }
  }
  TRACED_FORRANGE(int32_t, lsb, 0, 31) {
    TRACED_FORRANGE(int32_t, width, 1, (32 - lsb) - 1) {
      StreamBuilder m(this, kMachInt32, kMachInt32);
      m.Return(
          m.Word32And(m.Int32Constant(~((0xffffffffu >> (32 - width)) << lsb)),
                      m.Parameter(0)));
      Stream s = m.Build(ARMv7);
      ASSERT_EQ(1U, s.size());
      EXPECT_EQ(kArmBfc, s[0]->arch_opcode());
      ASSERT_EQ(1U, s[0]->OutputCount());
      EXPECT_TRUE(
          UnallocatedOperand::cast(s[0]->Output())->HasSameAsInputPolicy());
      ASSERT_EQ(3U, s[0]->InputCount());
      EXPECT_EQ(lsb, s.ToInt32(s[0]->InputAt(1)));
      EXPECT_EQ(width, s.ToInt32(s[0]->InputAt(2)));
    }
  }
}


TEST_F(InstructionSelectorTest, Word32ShrWithWord32AndWithImmediateForARMv7) {
  TRACED_FORRANGE(int32_t, lsb, 0, 31) {
    TRACED_FORRANGE(int32_t, width, 1, 32 - lsb) {
      uint32_t max = 1 << lsb;
      if (max > static_cast<uint32_t>(kMaxInt)) max -= 1;
      uint32_t jnk = rng()->NextInt(max);
      uint32_t msk = ((0xffffffffu >> (32 - width)) << lsb) | jnk;
      StreamBuilder m(this, kMachInt32, kMachInt32);
      m.Return(m.Word32Shr(m.Word32And(m.Parameter(0), m.Int32Constant(msk)),
                           m.Int32Constant(lsb)));
      Stream s = m.Build(ARMv7);
      ASSERT_EQ(1U, s.size());
      EXPECT_EQ(kArmUbfx, s[0]->arch_opcode());
      ASSERT_EQ(3U, s[0]->InputCount());
      EXPECT_EQ(lsb, s.ToInt32(s[0]->InputAt(1)));
      EXPECT_EQ(width, s.ToInt32(s[0]->InputAt(2)));
    }
  }
  TRACED_FORRANGE(int32_t, lsb, 0, 31) {
    TRACED_FORRANGE(int32_t, width, 1, 32 - lsb) {
      uint32_t max = 1 << lsb;
      if (max > static_cast<uint32_t>(kMaxInt)) max -= 1;
      uint32_t jnk = rng()->NextInt(max);
      uint32_t msk = ((0xffffffffu >> (32 - width)) << lsb) | jnk;
      StreamBuilder m(this, kMachInt32, kMachInt32);
      m.Return(m.Word32Shr(m.Word32And(m.Int32Constant(msk), m.Parameter(0)),
                           m.Int32Constant(lsb)));
      Stream s = m.Build(ARMv7);
      ASSERT_EQ(1U, s.size());
      EXPECT_EQ(kArmUbfx, s[0]->arch_opcode());
      ASSERT_EQ(3U, s[0]->InputCount());
      EXPECT_EQ(lsb, s.ToInt32(s[0]->InputAt(1)));
      EXPECT_EQ(width, s.ToInt32(s[0]->InputAt(2)));
    }
  }
}


TEST_F(InstructionSelectorTest, Word32AndWithWord32Not) {
  {
    StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32);
    m.Return(m.Word32And(m.Parameter(0), m.Word32Not(m.Parameter(1))));
    Stream s = m.Build();
    ASSERT_EQ(1U, s.size());
    EXPECT_EQ(kArmBic, s[0]->arch_opcode());
    EXPECT_EQ(kMode_Operand2_R, s[0]->addressing_mode());
    EXPECT_EQ(2U, s[0]->InputCount());
    EXPECT_EQ(1U, s[0]->OutputCount());
  }
  {
    StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32);
    m.Return(m.Word32And(m.Word32Not(m.Parameter(0)), m.Parameter(1)));
    Stream s = m.Build();
    ASSERT_EQ(1U, s.size());
    EXPECT_EQ(kArmBic, s[0]->arch_opcode());
    EXPECT_EQ(kMode_Operand2_R, s[0]->addressing_mode());
    EXPECT_EQ(2U, s[0]->InputCount());
    EXPECT_EQ(1U, s[0]->OutputCount());
  }
}


TEST_F(InstructionSelectorTest, Word32EqualWithParameters) {
  StreamBuilder m(this, kMachInt32, kMachInt32, kMachInt32);
  m.Return(m.Word32Equal(m.Parameter(0), m.Parameter(1)));
  Stream s = m.Build();
  ASSERT_EQ(1U, s.size());
  EXPECT_EQ(kArmCmp, s[0]->arch_opcode());
  EXPECT_EQ(kMode_Operand2_R, s[0]->addressing_mode());
  EXPECT_EQ(2U, s[0]->InputCount());
  EXPECT_EQ(1U, s[0]->OutputCount());
  EXPECT_EQ(kFlags_set, s[0]->flags_mode());
  EXPECT_EQ(kEqual, s[0]->flags_condition());
}


TEST_F(InstructionSelectorTest, Word32EqualWithImmediate) {
  TRACED_FOREACH(int32_t, imm, kImmediates) {
    if (imm == 0) continue;
    StreamBuilder m(this, kMachInt32, kMachInt32);
    m.Return(m.Word32Equal(m.Parameter(0), m.Int32Constant(imm)));
    Stream s = m.Build();
    ASSERT_EQ(1U, s.size());
    EXPECT_EQ(kArmCmp, s[0]->arch_opcode());
    EXPECT_EQ(kMode_Operand2_I, s[0]->addressing_mode());
    ASSERT_EQ(2U, s[0]->InputCount());
    EXPECT_EQ(imm, s.ToInt32(s[0]->InputAt(1)));
    EXPECT_EQ(1U, s[0]->OutputCount());
    EXPECT_EQ(kFlags_set, s[0]->flags_mode());
    EXPECT_EQ(kEqual, s[0]->flags_condition());
  }
  TRACED_FOREACH(int32_t, imm, kImmediates) {
    if (imm == 0) continue;
    StreamBuilder m(this, kMachInt32, kMachInt32);
    m.Return(m.Word32Equal(m.Int32Constant(imm), m.Parameter(0)));
    Stream s = m.Build();
    ASSERT_EQ(1U, s.size());
    EXPECT_EQ(kArmCmp, s[0]->arch_opcode());
    EXPECT_EQ(kMode_Operand2_I, s[0]->addressing_mode());
    ASSERT_EQ(2U, s[0]->InputCount());
    EXPECT_EQ(imm, s.ToInt32(s[0]->InputAt(1)));
    EXPECT_EQ(1U, s[0]->OutputCount());
    EXPECT_EQ(kFlags_set, s[0]->flags_mode());
    EXPECT_EQ(kEqual, s[0]->flags_condition());
  }
}


TEST_F(InstructionSelectorTest, Word32EqualWithZero) {
  {
    StreamBuilder m(this, kMachInt32, kMachInt32);
    m.Return(m.Word32Equal(m.Parameter(0), m.Int32Constant(0)));
    Stream s = m.Build();
    ASSERT_EQ(1U, s.size());
    EXPECT_EQ(kArmTst, s[0]->arch_opcode());
    EXPECT_EQ(kMode_Operand2_R, s[0]->addressing_mode());
    ASSERT_EQ(2U, s[0]->InputCount());
    EXPECT_EQ(s.ToVreg(s[0]->InputAt(0)), s.ToVreg(s[0]->InputAt(1)));
    EXPECT_EQ(1U, s[0]->OutputCount());
    EXPECT_EQ(kFlags_set, s[0]->flags_mode());
    EXPECT_EQ(kEqual, s[0]->flags_condition());
  }
  {
    StreamBuilder m(this, kMachInt32, kMachInt32);
    m.Return(m.Word32Equal(m.Int32Constant(0), m.Parameter(0)));
    Stream s = m.Build();
    ASSERT_EQ(1U, s.size());
    EXPECT_EQ(kArmTst, s[0]->arch_opcode());
    EXPECT_EQ(kMode_Operand2_R, s[0]->addressing_mode());
    ASSERT_EQ(2U, s[0]->InputCount());
    EXPECT_EQ(s.ToVreg(s[0]->InputAt(0)), s.ToVreg(s[0]->InputAt(1)));
    EXPECT_EQ(1U, s[0]->OutputCount());
    EXPECT_EQ(kFlags_set, s[0]->flags_mode());
    EXPECT_EQ(kEqual, s[0]->flags_condition());
  }
}


TEST_F(InstructionSelectorTest, Word32NotWithParameter) {
  StreamBuilder m(this, kMachInt32, kMachInt32);
  m.Return(m.Word32Not(m.Parameter(0)));
  Stream s = m.Build();
  ASSERT_EQ(1U, s.size());
  EXPECT_EQ(kArmMvn, s[0]->arch_opcode());
  EXPECT_EQ(kMode_Operand2_R, s[0]->addressing_mode());
  EXPECT_EQ(1U, s[0]->InputCount());
  EXPECT_EQ(1U, s[0]->OutputCount());
}


TEST_F(InstructionSelectorTest, Word32AndWithWord32ShrWithImmediateForARMv7) {
  TRACED_FORRANGE(int32_t, lsb, 0, 31) {
    TRACED_FORRANGE(int32_t, width, 1, 32 - lsb) {
      StreamBuilder m(this, kMachInt32, kMachInt32);
      m.Return(m.Word32And(m.Word32Shr(m.Parameter(0), m.Int32Constant(lsb)),
                           m.Int32Constant(0xffffffffu >> (32 - width))));
      Stream s = m.Build(ARMv7);
      ASSERT_EQ(1U, s.size());
      EXPECT_EQ(kArmUbfx, s[0]->arch_opcode());
      ASSERT_EQ(3U, s[0]->InputCount());
      EXPECT_EQ(lsb, s.ToInt32(s[0]->InputAt(1)));
      EXPECT_EQ(width, s.ToInt32(s[0]->InputAt(2)));
    }
  }
  TRACED_FORRANGE(int32_t, lsb, 0, 31) {
    TRACED_FORRANGE(int32_t, width, 1, 32 - lsb) {
      StreamBuilder m(this, kMachInt32, kMachInt32);
      m.Return(m.Word32And(m.Int32Constant(0xffffffffu >> (32 - width)),
                           m.Word32Shr(m.Parameter(0), m.Int32Constant(lsb))));
      Stream s = m.Build(ARMv7);
      ASSERT_EQ(1U, s.size());
      EXPECT_EQ(kArmUbfx, s[0]->arch_opcode());
      ASSERT_EQ(3U, s[0]->InputCount());
      EXPECT_EQ(lsb, s.ToInt32(s[0]->InputAt(1)));
      EXPECT_EQ(width, s.ToInt32(s[0]->InputAt(2)));
    }
  }
}

}  // namespace compiler
}  // namespace internal
}  // namespace v8
