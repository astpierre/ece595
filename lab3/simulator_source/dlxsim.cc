//
//	dlxsim.cc
//
//	DLX simulator code.  This is a barebones simulator, but it does
//	include code to handle OS issues such as virtual memory translation.
//	This allows it to be used as a simulator for operating systems
//	classes.
//
//	Copyright (c) 1999 by Ethan L. Miller
//	University of Maryland Baltimore County
//

static char rcsid[] = "$Id: dlxsim.cc,v 1.2 2000/09/20 22:16:34 elm Exp elm $";

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/time.h>
#include "dlx.h"

extern int errno;
char	debug[100];

//----------------------------------------------------------------------
//
//	Cpu::Cpu
//
//	Create a new CPU.
//
//----------------------------------------------------------------------
Cpu::Cpu (int msize)
{
  int		i;
  struct timeval t;

  flags = 0;
  usElapsed = 0.0;
  instrsExecuted = 0.0;
  usPerInst = 1.0;
  for (i = 0; i < 32; i++) {
    sreg[i] = 0;
    ireg[i] = 0;
    freg[i] = 0;
  }
  SetStatusBit (DLX_STATUS_PAGE_TABLE);
  SetStatusBit (DLX_STATUS_SYSMODE);
  EnableInterrupts ();
  timerInterrupt = DLX_TIMER_NOT_ACTIVE;
  memSize = msize;
  memory = new uint32[msize/sizeof(uint32)];
  basicBlockStart = 1;	// basic block can never start at address 1!
  // Initialize the keyboard I/O stuff.
  kbdbufferedchars = 0;
  kbdrpos = kbdwpos = 0;
  kbdcounter = 0;
  SetupRawIo ();
  //Zheng, add (timezone *)
  //gettimeofday (&t, (timezone*)(void *)0);
  gettimeofday (&t, (void *)0);
  realElapsed = (double)t.tv_sec + ((double)t.tv_usec) * 1e-6;
}

//----------------------------------------------------------------------
//
//	TraceFile
//
//	Open the passed file name for tracing.  If the file is NULL,
//	open stdout.
//
//----------------------------------------------------------------------
int
Cpu::TraceFile (char *name)
{
  if ((name == NULL) || (!strcmp (name, "-"))) {
    tracefp = stdout;
    return (1);
  } else if ((tracefp = fopen (name, "w")) == NULL) {
    return (0);
  } else {
    return (1);
  }
}

//----------------------------------------------------------------------
//
//	Instruction parsing functions
//
//	These functions take an instruction and return the relevant fields.
//
//----------------------------------------------------------------------
inline
void
Cpu::GetRFields (uint32 inst, uint32& src1, uint32& src2, uint32& dst)
{
  src1 = (inst >> DLX_RFMT_SRC1_SHIFT) & DLX_REG_MASK;
  src2 = (inst >> DLX_RFMT_SRC2_SHIFT) & DLX_REG_MASK;
  dst = (inst >> DLX_RFMT_DST_SHIFT) & DLX_REG_MASK;
}

inline
void
Cpu::GetIFields (uint32 inst, uint32& src, uint32& imm, uint32& dst)
{
  src = (inst >> DLX_IFMT_SRC_SHIFT) & DLX_REG_MASK;
  dst = (inst >> DLX_IFMT_DST_SHIFT) & DLX_REG_MASK;
  imm = (inst >> DLX_IFMT_IMM_SHIFT) & 0xffff;
}

inline
void
Cpu::GetJFields (uint32 inst, uint32& jaddr)
{
  jaddr = inst & 0x1ffffff;
  if (jaddr & 0x1000000) {
    jaddr |= 0xfe000000;
  }
}

inline
void
Cpu::SignExtend16 (uint32& v)
{
  if (v & 0x8000) {
    v |= 0xffff0000;
  }
}

inline
void
Cpu::SignExtend8 (uint32& v)
{
  if (v & 0x80) {
    v |= 0xffffff00;
  }
}

//----------------------------------------------------------------------
//
//	Cpu::Jump
//
//	Jump to a particular location after dealing with the delay
//	slot.  Checks to ensure that the jump address could be valid
//	(ie, is on a word boundary).  It doesn't check to ensure that
//	the jump destination is valid; that's done on the first instruction
//	fetch to follow.
//
//----------------------------------------------------------------------
inline
int
Cpu::Jump (uint32 jmpDst)
{
  if ((jmpDst & 0x3) == 0) {
    DBPRINTF ('j', "Jump from 0x%x to 0x%x.\n", PC(), jmpDst);
    OutputBasicBlock (jmpDst);
    SetPC (jmpDst);
  } else {
    CauseException (DLX_EXC_ADDRESS);
  }
  return (1);
}


//----------------------------------------------------------------------
//
//	Instruction functions
//
//	These functions implement individual instructions.  They return
//	the number of instructions executed, which will either be 1 if the
//	instruction completed successfully or 0 if the instruction caused
//	an exception.
//
//	Since the instructions are pretty simple (and similar), they won't
//	be documented extensively except for the more complex ones.
//----------------------------------------------------------------------

//----------------------------------------------------------------------
//
//	Generic ALU R-R instructions
//
//----------------------------------------------------------------------
static
int
InstAdd (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;
  uint32	v1, v2, result;

  cpu->GetRFields (inst, src1, src2, dst);
  v1 = cpu->GetIreg(src1);
  v2 = cpu->GetIreg(src2);
  result = v1 + v2;
  cpu->PutIreg (dst, result);
  // Overflow if sign of result isn't the same as sign of operands
  if (((v1 & 0x80000000) == (v2 & 0x80000000)) &&
      ((v1 & 0x80000000) != (result & 0x80000000))) {
    cpu->CauseException (DLX_EXC_OVERFLOW);
  }
  // Instruction isn't redone after exception
  return (1);
}

static
int
InstSub (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;
  uint32	v1, v2, result;

  cpu->GetRFields (inst, src1, src2, dst);
  v1 = cpu->GetIreg(src1);
  v2 = cpu->GetIreg(src2);
  result = v1 - v2;
  cpu->PutIreg (dst, result);
  // Overflow if sign of result isn't the same as sign of first operand,
  // and if second operand has the opposite sign.
  if (((v1 & 0x80000000) != (v2 & 0x80000000)) &&
      ((v1 & 0x80000000) != (result & 0x80000000))) {
    cpu->CauseException (DLX_EXC_OVERFLOW);
  }
  // Instruction isn't redone after exception
  return (1);
}

static
int
InstAddu (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;
  uint32	v1, v2, result;

  cpu->GetRFields (inst, src1, src2, dst);
  v1 = cpu->GetIreg(src1);
  v2 = cpu->GetIreg(src2);
  result = v1 + v2;
  cpu->PutIreg (dst, result);
  return (1);
}

static
int
InstSubu (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;
  uint32	v1, v2, result;

  cpu->GetRFields (inst, src1, src2, dst);
  v1 = cpu->GetIreg(src1);
  v2 = cpu->GetIreg(src2);
  result = v1 - v2;
  cpu->PutIreg (dst, result);
  return (1);
}

//----------------------------------------------------------------------
//
//	Immediate arithmetic functions
//
//----------------------------------------------------------------------
static
int
InstAddi (uint32 inst, Cpu *cpu)
{
  uint32	src1, dst, imm;
  uint32	v1, result;

  cpu->GetIFields (inst, src1, imm, dst);
  cpu->SignExtend16 (imm);
  v1 = cpu->GetIreg(src1);
  result = v1 + imm;
  cpu->PutIreg (dst, result);
  // Overflow if sign of result isn't the same as sign of operands
  if (((v1 & 0x80000000) == (imm & 0x80000000)) &&
      ((v1 & 0x80000000) != (result & 0x80000000))) {
    cpu->CauseException (DLX_EXC_OVERFLOW);
  }
  // Instruction isn't redone after exception
  return (1);
}

static
int
InstSubi (uint32 inst, Cpu *cpu)
{
  uint32	src1, dst, imm;
  uint32	v1, result;

  cpu->GetIFields (inst, src1, imm, dst);
  cpu->SignExtend16 (imm);
  v1 = cpu->GetIreg(src1);
  result = v1 - imm;
  cpu->PutIreg (dst, result);
  // Overflow if sign of result isn't the same as sign of first operand,
  // and if second operand has the opposite sign.
  if (((v1 & 0x80000000) != (imm & 0x80000000)) &&
      ((v1 & 0x80000000) != (result & 0x80000000))) {
    cpu->CauseException (DLX_EXC_OVERFLOW);
  }
  // Instruction isn't redone after exception
  return (1);
}

static
int
InstAddui (uint32 inst, Cpu *cpu)
{
  uint32	src1, imm, dst;
  uint32	v1, result;

  cpu->GetIFields (inst, src1, imm, dst);
  // XXX: do we sign extend values for addui?
  // cpu->SignExtend16 (imm);
  v1 = cpu->GetIreg(src1);
  result = v1 + imm;
  cpu->PutIreg (dst, result);
  return (1);
}

static
int
InstSubui (uint32 inst, Cpu *cpu)
{
  uint32	src1, imm, dst;
  uint32	v1, result;

  cpu->GetIFields (inst, src1, imm, dst);
  // XXX: do we sign extend values for subui?
  // cpu->SignExtend16 (imm);
  v1 = cpu->GetIreg(src1);
  result = v1 - imm;
  cpu->PutIreg (dst, result);
  return (1);
}

//----------------------------------------------------------------------
//
//	R-R logical functions
//
//----------------------------------------------------------------------

static
int
InstAnd (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;

  cpu->GetRFields (inst, src1, src2, dst);
  cpu->PutIreg (dst, cpu->GetIreg(src1) & cpu->GetIreg(src2));
  return (1);
}

static
int
InstOr (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;

  cpu->GetRFields (inst, src1, src2, dst);
  cpu->PutIreg (dst, cpu->GetIreg(src1) | cpu->GetIreg(src2));
  return (1);
}

static
int
InstXor (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;

  cpu->GetRFields (inst, src1, src2, dst);
  cpu->PutIreg (dst, cpu->GetIreg(src1) ^ cpu->GetIreg(src2));
  return (1);
}

static
int
InstSll (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;
  uint32	v1, v2;

  cpu->GetRFields (inst, src1, src2, dst);
  v1 = cpu->GetIreg(src1);
  v2 = cpu->GetIreg(src2) & 0x1f;
  cpu->PutIreg (dst, v1 << v2);
  return (1);
}

static
int
InstSra (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;
  uint32	v1, v2, result;

  cpu->GetRFields (inst, src1, src2, dst);
  v1 = cpu->GetIreg(src1);
  v2 = cpu->GetIreg(src2) & 0x1f;
  result = v1 >> v2;
  // stick in sign bits if operand is negative
  if (v1 & 0x80000000) {
    result |= ((1 << v2) - 1) << (32 - v2);
  }
  cpu->PutIreg (dst, result);
  return (1);
}

static
int
InstSrl (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;
  uint32	v1, v2;

  cpu->GetRFields (inst, src1, src2, dst);
  v1 = cpu->GetIreg(src1);
  v2 = cpu->GetIreg(src2) & 0x1f;
  cpu->PutIreg (dst, v1 >> v2);
  return (1);
}

//----------------------------------------------------------------------
//
//	Immediate logical functions
//
//----------------------------------------------------------------------
static
int
InstAndi (uint32 inst, Cpu *cpu)
{
  uint32	src1, imm, dst;

  cpu->GetIFields (inst, src1, imm, dst);
  cpu->PutIreg (dst, cpu->GetIreg(src1) & imm);
  return (1);
}

static
int
InstOri (uint32 inst, Cpu *cpu)
{
  uint32	src1, imm, dst;

  cpu->GetIFields (inst, src1, imm, dst);
  cpu->PutIreg (dst, cpu->GetIreg(src1) | imm);
  return (1);
}

static
int
InstXori (uint32 inst, Cpu *cpu)
{
  uint32	src1, imm, dst;

  cpu->GetIFields (inst, src1, imm, dst);
  cpu->PutIreg (dst, cpu->GetIreg(src1) ^ imm);
  return (1);
}

static
int
InstSlli (uint32 inst, Cpu *cpu)
{
  uint32	src1, imm, dst;
  uint32	v1;

  cpu->GetIFields (inst, src1, imm, dst);
  v1 = cpu->GetIreg(src1);
  imm &= 0x1f;
  cpu->PutIreg (dst, v1 << imm);
  return (1);
}

static
int
InstSrai (uint32 inst, Cpu *cpu)
{
  uint32	src1, imm, dst;
  uint32	v1, result;

  cpu->GetIFields (inst, src1, imm, dst);
  v1 = cpu->GetIreg(src1);
  imm &= 0x1f;
  result = v1 >> imm;
  // stick in sign bits if operand is negative
  if (v1 & 0x80000000) {
    result |= ((1 << imm) - 1) << (32 - imm);
  }
  cpu->PutIreg (dst, result);
  return (1);
}

static
int
InstSrli (uint32 inst, Cpu *cpu)
{
  uint32	src1, imm, dst;
  uint32	v1;

  cpu->GetIFields (inst, src1, imm, dst);
  v1 = cpu->GetIreg(src1);
  imm &= 0x1f;
  cpu->PutIreg (dst, v1 >> imm);
  return (1);
}

static
int
InstLhi (uint32 inst, Cpu *cpu)
{
  uint32	src, dst, imm;

  cpu->GetIFields (inst, src, imm, dst);
  cpu->PutIreg (dst, imm << 16);
  return (1);
}

//----------------------------------------------------------------------
//
//	R-R set instructions
//
//----------------------------------------------------------------------
static
int
InstSeq (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;

  cpu->GetRFields (inst, src1, src2, dst);
  cpu->PutIreg (dst, (cpu->GetIreg(src1) == cpu->GetIreg(src2)) ? 1 : 0);
  return (1);
}

static
int
InstSne (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;

  cpu->GetRFields (inst, src1, src2, dst);
  cpu->PutIreg (dst, (cpu->GetIreg(src1) != cpu->GetIreg(src2)) ? 1 : 0);
  return (1);
}

static
int
InstSlt (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;

  cpu->GetRFields (inst, src1, src2, dst);
  cpu->PutIreg (dst, ((int)cpu->GetIreg(src1) < (int)cpu->GetIreg(src2))
		? 1 : 0);
  return (1);
}
static
int
InstSgt (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;

  cpu->GetRFields (inst, src1, src2, dst);
  cpu->PutIreg (dst, ((int)cpu->GetIreg(src1) > (int)cpu->GetIreg(src2))
		? 1 : 0);
  return (1);
}
static
int
InstSle (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;

  cpu->GetRFields (inst, src1, src2, dst);
  cpu->PutIreg (dst, ((int)cpu->GetIreg(src1) <= (int)cpu->GetIreg(src2))
		? 1 : 0);
  return (1);
}
static
int
InstSge (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;

  cpu->GetRFields (inst, src1, src2, dst);
  cpu->PutIreg (dst, ((int)cpu->GetIreg(src1) >= (int)cpu->GetIreg(src2))
		? 1 : 0);
  return (1);
}

//----------------------------------------------------------------------
//
//	Immediate set instructions
//
//----------------------------------------------------------------------
static
int
InstSeqi (uint32 inst, Cpu *cpu)
{
  uint32	src1, imm, dst;

  cpu->GetIFields (inst, src1, imm, dst);
  cpu->SignExtend16 (imm);
  cpu->PutIreg (dst, (cpu->GetIreg(src1) == imm) ? 1 : 0);
  return (1);
}

static
int
InstSnei (uint32 inst, Cpu *cpu)
{
  uint32	src1, imm, dst;

  cpu->GetIFields (inst, src1, imm, dst);
  cpu->SignExtend16 (imm);
  cpu->PutIreg (dst, (cpu->GetIreg(src1) != imm) ? 1 : 0);
  return (1);
}

static
int
InstSlti (uint32 inst, Cpu *cpu)
{
  uint32	src1, imm, dst;

  cpu->GetIFields (inst, src1, imm, dst);
  cpu->SignExtend16 (imm);
  cpu->PutIreg (dst, ((int)cpu->GetIreg(src1) < (int)imm) ? 1 : 0);
  return (1);
}

static
int
InstSgti (uint32 inst, Cpu *cpu)
{
  uint32	src1, imm, dst;

  cpu->GetIFields (inst, src1, imm, dst);
  cpu->SignExtend16 (imm);
  cpu->PutIreg (dst, ((int)cpu->GetIreg(src1) > (int)imm) ? 1 : 0);
  return (1);
}

static
int
InstSlei (uint32 inst, Cpu *cpu)
{
  uint32	src1, imm, dst;

  cpu->GetIFields (inst, src1, imm, dst);
  cpu->SignExtend16 (imm);
  cpu->PutIreg (dst, ((int)cpu->GetIreg(src1) <= (int)imm) ? 1 : 0);
  return (1);
}

static
int
InstSgei (uint32 inst, Cpu *cpu)
{
  uint32	src1, imm, dst;

  cpu->GetIFields (inst, src1, imm, dst);
  cpu->SignExtend16 (imm);
  cpu->PutIreg (dst, ((int)cpu->GetIreg(src1) >= (int)imm) ? 1 : 0);
  return (1);
}

//----------------------------------------------------------------------
//
//	Load instructions
//
//----------------------------------------------------------------------
static
int
InstLw (uint32 inst, Cpu *cpu)
{
  uint32	addrReg, offset, dst, addr, val;

  cpu->GetIFields (inst, addrReg, offset, dst);
  addr = cpu->EffectiveAddress (addrReg, offset);
  // If access fails, this instruction isn't considered completed
  if (! cpu->ReadWord (addr, val)) {
    return (0);
  }
  DBPRINTF ('l', "Loading word 0x%08x from location 0x%x.\n", val, addr);
  cpu->TraceAccess("lw", dst, addr, val);
  cpu->PutIreg (dst, val);
  return (1);
}

static
int
InstLh (uint32 inst, Cpu *cpu)
{
  uint32	addrReg, offset, dst, addr, val;
  static int	shiftcount[4] = {16, 16, 0, 0};

  cpu->GetIFields (inst, addrReg, offset, dst);
  addr = cpu->EffectiveAddress (addrReg, offset);
  // Half-word accesses must be 2-byte aligned
  if ((addr & 0x1) == 1) {
    cpu->CauseException (DLX_EXC_ADDRESS);
    return (0);
  }
  // If access fails, this instruction isn't considered completed
  if (! cpu->ReadWord (addr & 0xfffffffc, val)) {
    return (0);
  }
  val >>= shiftcount[addr & 0x3];
  DBPRINTF ('l',"Loading signed half 0x%04x from location 0x%x.\n", val, addr);
  cpu->TraceAccess("lh", dst, addr, val);
  cpu->SignExtend16 (val);
  cpu->PutIreg (dst, val);
  return (1);
}

static
int
InstLhu (uint32 inst, Cpu *cpu)
{
  uint32	addrReg, offset, dst, addr, val;
  static int	shiftcount[4] = {16, 16, 0, 0};

  cpu->GetIFields (inst, addrReg, offset, dst);
  addr = cpu->EffectiveAddress (addrReg, offset);
  // Half-word accesses must be 2-byte aligned
  if ((addr & 0x1) == 1) {
    cpu->CauseException (DLX_EXC_ADDRESS);
    return (0);
  }
  // If access fails, this instruction isn't considered completed
  if (! cpu->ReadWord (addr & 0xfffffffc, val)) {
    return (0);
  }
  val >>= shiftcount[addr & 0x3];
  val &= 0xffff;
  DBPRINTF ('l',"Loading unsigned half 0x%04x from location 0x%x.\n",val,addr);
  cpu->TraceAccess("lhu", dst, addr, val);
  cpu->PutIreg (dst, val);
  return (1);
}

static
int
InstLb (uint32 inst, Cpu *cpu)
{
  uint32	addrReg, offset, dst, addr, val;
  static int	shiftcount[4] = {24, 16, 8, 0};

  cpu->GetIFields (inst, addrReg, offset, dst);
  addr = cpu->EffectiveAddress (addrReg, offset);
  // If access fails, this instruction isn't considered completed
  if (! cpu->ReadWord (addr & 0xfffffffc, val)) {
    return (0);
  }
  val >>= shiftcount[addr & 0x3];
  val &= 0xff;
  DBPRINTF ('l',"Loading signed byte 0x%02x from location 0x%x.\n", val, addr);
  cpu->TraceAccess("lb", dst, addr, val);
  cpu->SignExtend8 (val);
  cpu->PutIreg (dst, val);
  return (1);
}

static
int
InstLbu (uint32 inst, Cpu *cpu)
{
  uint32	addrReg, offset, dst, addr, val;
  static int	shiftcount[4] = {24, 16, 8, 0};

  cpu->GetIFields (inst, addrReg, offset, dst);
  addr = cpu->EffectiveAddress (addrReg, offset);
  // If access fails, this instruction isn't considered completed
  if (! cpu->ReadWord (addr & 0xfffffffc, val)) {
    return (0);
  }
  val >>= shiftcount[addr & 0x3];
  val &= 0xff;
  DBPRINTF ('l',"Loading unsigned byte 0x%02x from location 0x%x.\n",val,addr);
  cpu->TraceAccess("lbu", dst, addr, val);
  cpu->PutIreg (dst, val);
  return (1);
}

//----------------------------------------------------------------------
//
//	Store instructions
//
//----------------------------------------------------------------------
static
int
InstSh (uint32 inst, Cpu *cpu)
{
  uint32	addrReg, offset, dst, addr, val, kval, regval, shiftamt;
  static int	shiftcount[4] = {16, 16, 0, 0};

  cpu->GetIFields (inst, addrReg, offset, dst);
  addr = cpu->EffectiveAddress (addrReg, offset);
  // If access fails, this instruction isn't considered completed
  // ReadWord checks for misaligned accesses.
  if (! cpu->ReadWord (addr & 0xfffffffe, val)) {
    return (0);
  }
  kval = val;
  shiftamt = shiftcount[addr & 0x3];
  regval = (cpu->GetIreg (dst) & 0xffff);
  val &= ~(0xffff << shiftamt);
  val |= regval << shiftamt;
  DBPRINTF ('s',"Storing half 0x%04x (0x%08x -> 0x%08x) to location 0x%x.\n",
	    regval, kval, val, addr);
  // This should never fail (if ReadWord worked), but just in case...
  if (! cpu->WriteWord (addr & 0xfffffffc, val)) {
    return (0);
  }
  cpu->TraceAccess("sh", dst, addr, regval);
  return (1);
}

static
int
InstSb (uint32 inst, Cpu *cpu)
{
  uint32	addrReg, offset, dst, addr, val, kval, regval, shiftamt;
  static int	shiftcount[4] = {24, 16, 8, 0};

  cpu->GetIFields (inst, addrReg, offset, dst);
  addr = cpu->EffectiveAddress (addrReg, offset);
  // If access fails, this instruction isn't considered completed
  if (! cpu->ReadWord (addr & 0xfffffffc, val)) {
    return (0);
  }
  kval = val;
  shiftamt = shiftcount[addr & 0x3];
  regval = (cpu->GetIreg (dst) & 0xff);
  val &= ~(0xff << shiftamt);
  val |= regval << shiftamt;
  DBPRINTF ('s',"Storing byte 0x%02x (0x%08x -> 0x%08x) to location 0x%x.\n",
	    regval, kval, val, addr);
  // This should never fail (if ReadWord worked), but just in case...
  if (! cpu->WriteWord (addr & 0xfffffffc, val)) {
    return (0);
  }
  cpu->TraceAccess("sb", dst, addr, regval);
  return (1);
}

static
int
InstSw (uint32 inst, Cpu *cpu)
{
  uint32	addrReg, offset, dst, addr, val;

  cpu->GetIFields (inst, addrReg, offset, dst);
  addr = cpu->EffectiveAddress (addrReg, offset);
  val = cpu->GetIreg (dst);
  DBPRINTF ('s',"Storing word 0x%08x to location 0x%x.\n", val, addr);
  // If access fails, this instruction isn't considered completed
  if (! cpu->WriteWord (addr, val)) {
    return (0);
  }
  cpu->TraceAccess("sw", dst, addr, val);
  return (1);
}

//----------------------------------------------------------------------
//
//	FP load/store instructions
//
//----------------------------------------------------------------------
static
int
InstLf (uint32 inst, Cpu *cpu)
{
  uint32	addrReg, offset, dst, addr, val;

  cpu->GetIFields (inst, addrReg, offset, dst);
  addr = cpu->EffectiveAddress (addrReg, offset);
  // If access fails, this instruction isn't considered completed
  if (! cpu->ReadWord (addr, val)) {
    return (0);
  }
  cpu->PutFreg (dst, val);
  cpu->TraceAccess("lf", dst, addr, val);
  return (1);
}

static
int
InstLd (uint32 inst, Cpu *cpu)
{
  uint32	addrReg, offset, dst, addr, val1, val2;

  cpu->GetIFields (inst, addrReg, offset, dst);
  if ((dst & 0x1) == 1) {
    cpu->CauseException (DLX_EXC_FORMAT);
    return (1);
  }
  addr = cpu->EffectiveAddress (addrReg, offset);
  // If access fails, this instruction isn't considered completed
  if (! cpu->ReadWord (addr, val1)) {
    return (0);
  }
  if (! cpu->ReadWord (addr+4, val2)) {
    return (0);
  }
#if	(DLX_NATIVE_ENDIAN == DLX_BIG_ENDIAN)
  cpu->PutFreg (dst, val1);
  cpu->PutFreg (dst+1,val2);
#else
  cpu->PutFreg (dst,val2);
  cpu->PutFreg (dst+1, val1);
#endif
  DBPRINTF ('f',"Read double %lf from address %08x\n", cpu->GetFregD (dst),
	    addr);
  cpu->TraceAccess("ld0", dst, addr, val1);
  cpu->TraceAccess("ld1", dst, addr+4, val2);
  return (1);
}

static
int
InstSf (uint32 inst, Cpu *cpu)
{
  uint32	addrReg, offset, dst, addr, val;

  cpu->GetIFields (inst, addrReg, offset, dst);
  addr = cpu->EffectiveAddress (addrReg, offset);
  val = cpu->GetFreg (dst);
  // If access fails, this instruction isn't considered completed
  if (! cpu->WriteWord (addr, val)) {
    return (0);
  }
  cpu->TraceAccess("sf", dst, addr, val);
  return (1);
}

static
int
InstSd (uint32 inst, Cpu *cpu)
{
  uint32	addrReg, offset, dst, addr, val1, val2;

  cpu->GetIFields (inst, addrReg, offset, dst);
  addr = cpu->EffectiveAddress (addrReg, offset);
  // If access fails, this instruction isn't considered completed
  if (! cpu->TestWriteWord (addr+4)) {
    return (0);
  }
#if	(DLX_NATIVE_ENDIAN == DLX_BIG_ENDIAN)
  val1 = cpu->GetFreg (dst);
  val2 = cpu->GetFreg (dst+1);
#else
  val1 = cpu->GetFreg (dst+1);
  val2 = cpu->GetFreg (dst);
#endif
  if (! cpu->WriteWord (addr, val1)) {
    return (0);
  }
  // Should never fail, since write was tested earlier
  if (! cpu->WriteWord (addr+4, val2)) {
    return (0);
  }
  cpu->TraceAccess("sd0", dst, addr, val1);
  cpu->TraceAccess("sd1", dst, addr+4, val2);
  return (1);
}

//----------------------------------------------------------------------
//
//	Miscellaneous FP move instructions
//
//----------------------------------------------------------------------
static
int
InstMovf (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;

  cpu->GetRFields (inst, src1, src2, dst);
  cpu->PutFreg (dst, cpu->GetFreg (src1));
  return (1);
}

static
int
InstMovd (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;

  cpu->GetRFields (inst, src1, src2, dst);
  if ((dst & 0x1) || (src1 & 0x1)) {
    cpu->CauseException (DLX_EXC_FORMAT);
    return (1);
  }
  cpu->PutFreg (dst, cpu->GetFreg (src1));
  cpu->PutFreg (dst+1, cpu->GetFreg (src1+1));
  return (1);
}

static
int
InstMovfp2i (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;

  cpu->GetRFields (inst, src1, src2, dst);
  cpu->PutIreg (dst, cpu->GetFreg (src1));
  return (1);
}

static
int
InstMovi2fp (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;

  cpu->GetRFields (inst, src1, src2, dst);
  cpu->PutFreg (dst, cpu->GetIreg (src1));
  return (1);
}

//----------------------------------------------------------------------
//
//	Unconditional jumps & subroutine calls
//
//----------------------------------------------------------------------
static
int
InstJmp (uint32 inst, Cpu *cpu)
{
  uint32	joffset;

  cpu->GetJFields (inst, joffset);
  return (cpu->Jump (cpu->PC() + (int)joffset));
}

static
int
InstJr (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;

  cpu->GetRFields (inst, src1, src2, dst);
  return (cpu->Jump (cpu->GetIreg(src1)));
}

static
int
InstJal (uint32 inst, Cpu *cpu)
{
  uint32	joffset;

  cpu->GetJFields (inst, joffset);
  cpu->PutIreg (31, cpu->PC());
  DBPRINTF ('j', "jal from 0x%x to 0x%x.\n", cpu->PC(),
	    cpu->PC() + (int)joffset);
  return (cpu->Jump (cpu->PC() + (int)joffset));
}

static
int
InstJalr (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;

  cpu->GetRFields (inst, src1, src2, dst);
  cpu->PutIreg (31, cpu->PC());
  return (cpu->Jump (cpu->GetIreg(src1)));
}

//----------------------------------------------------------------------
//
//	Conditional branches
//
//----------------------------------------------------------------------
static
int
InstBeqz (uint32 inst, Cpu *cpu)
{
  uint32	src, offset, dst;

  cpu->GetIFields (inst, src, offset, dst);
  cpu->SignExtend16 (offset);
  if (cpu->GetIreg (src) == 0) {
    return (cpu->Jump (cpu->PC () + (int)offset));
  } else {
    return (1);
  }
}

static
int
InstBnez (uint32 inst, Cpu *cpu)
{
  uint32	src, offset, dst;

  cpu->GetIFields (inst, src, offset, dst);
  cpu->SignExtend16 (offset);
  if (cpu->GetIreg (src) != 0) {
    return (cpu->Jump (cpu->PC () + (int)offset));
  } else {
    return (1);
  }
}

static
int
InstBfpt (uint32 inst, Cpu *cpu)
{
  uint32	src, offset, dst;

  cpu->GetIFields (inst, src, offset, dst);
  cpu->SignExtend16 (offset);
  if (cpu->StatusBit (DLX_STATUS_FPTRUE)) {
    return (cpu->Jump (cpu->PC () + (int)offset));
  } else {
    return (1);
  }
}

static
int
InstBfpf (uint32 inst, Cpu *cpu)
{
  uint32	src, offset, dst;

  cpu->GetIFields (inst, src, offset, dst);
  cpu->SignExtend16 (offset);
  if (! cpu->StatusBit (DLX_STATUS_FPTRUE)) {
    return (cpu->Jump (cpu->PC () + (int)offset));
  } else {
    return (1);
  }
}

//----------------------------------------------------------------------
//
//	Integer multiply and divide (done by FP unit).  Note that
//	multiplication overflows are NOT caught, and results are
//	unpredictable (may depend on machine on which simulator is
//	run).
//
//----------------------------------------------------------------------
static
int
InstMult (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;
  cpu->GetRFields (inst, src1, src2, dst);
  cpu->PutFreg (dst, (int)cpu->GetFreg (src1) * (int)cpu->GetFreg (src2));
  return (1);
}

static
int
InstMultu (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;
  cpu->GetRFields (inst, src1, src2, dst);
  cpu->PutFreg (dst, cpu->GetFreg (src1) * cpu->GetFreg (src2));
  return (1);
}

static
int
InstDiv (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst, denom;
  cpu->GetRFields (inst, src1, src2, dst);
  denom = cpu->GetFreg (src2);
  if (denom == 0) {
    cpu->CauseException(DLX_EXC_DIV0);
  } else {
    cpu->PutFreg (dst, (int)cpu->GetFreg (src1) / (int)denom);
  }
  return (1);
}

static
int
InstDivu (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst, denom;
  cpu->GetRFields (inst, src1, src2, dst);
  denom = cpu->GetFreg (src2);
  if (denom == 0) {
    cpu->CauseException(DLX_EXC_DIV0);
  } else {
    cpu->PutFreg (dst, cpu->GetFreg (src1) / denom);
  }
  return (1);
}

//----------------------------------------------------------------------
//	trap
//
//	NOTE: different from the description in the DLX handbook.
//
//	The trap in DLX is unvectored; it's up to the interrupt handler
//	to do the vectoring.  However, the cause register is loaded with
//	the trap argument with the top 5 bits set to 00001.  This makes
//	it easy to identify traps and to vector them properly.
//
//	The trap instruction is also a way to access services from the
//	underlying machine.  In particular, we can do such things as
//	file access and printing.  Note that character access is handled
//	separately.
//----------------------------------------------------------------------
static
int
InstTrap (uint32 inst, Cpu *cpu)
{
  uint32	trapVector;

  cpu->GetJFields (inst, trapVector);
  trapVector &= 0x0fffffff;
  DBPRINTF ('t',"Got a trap, inst=%08x, vector=%x\n", inst, trapVector);
  if ((trapVector == DLX_TRAP_EXIT) && ! cpu->IgnoreExit()) {
    cpu->Exit ();
  }
  if (trapVector < 0x2000) {
    // Increment the PC so the address in the IAR is that of the
    // following instruction.
    cpu->SetPC (cpu->PC()+4);
    cpu->CauseException (trapVector | 0x08000000);
  } else {
    cpu->OutputBasicBlock (cpu->PC()+4);
    if (cpu->Flags() & (DLX_TRACE_INSTRUCTIONS | DLX_TRACE_MEMORY)) {
      fprintf (cpu->TraceFp(), "T %x %x\n", trapVector, cpu->PC());
    }
    // Handle simulator services here.  This isn't so performance
    // critical, so we can use a switch statement.
    // Parameters are passed on the stack.
    // Return values go into r1
    switch (trapVector) {
    case DLX_TRAP_EXITSIM:
      cpu->Exit ();
      break;
    case DLX_TRAP_PRINTF:
      cpu->Printf ();
      break;
    case DLX_TRAP_OPEN:
      cpu->Open ();
      break;
    case DLX_TRAP_READ:
      cpu->Read ();
      break;
    case DLX_TRAP_WRITE:
      cpu->Write ();
      break;
    case DLX_TRAP_LSEEK:
      cpu->Seek ();
      break;
    case DLX_TRAP_CLOSE:
      cpu->Close ();
      break;
    case DLX_TRAP_RANDOM:
      cpu->Random ();
      break;
    case DLX_TRAP_SRANDOM:
      cpu->Srandom ();
      break;
    case DLX_TRAP_TIMERGET:
      cpu->Timerget();
      break;
    }
  }
  return (1);
}

//----------------------------------------------------------------------
//	rfe
//
//	When returning from an exception, jump to the IAR.  To avoid
//	illegal addresses, mask off the low two bits.  Also, turn
//	interrupts on.
//----------------------------------------------------------------------
static
int
InstRfe (uint32 inst, Cpu *cpu)
{
  return (cpu->DoRfe (inst));
}

int
Cpu::DoRfe (uint32 inst)
{
  uint32	iar;
  uint32	isr;

  if (UserMode ()) {
    CauseException (DLX_EXC_PRIVILEGE);
    return (1);
  }
  iar = GetSreg (DLX_SREG_IAR) & ~0x3;
  OutputBasicBlock (iar);
  if (flags & (DLX_TRACE_INSTRUCTIONS | DLX_TRACE_MEMORY)) {
    fprintf (tracefp, "R %x %x\n", PC()-4, iar);
  }
  isr = GetSreg (DLX_SREG_ISR);
  PutSreg (DLX_SREG_STATUS, isr);
  SetPC (iar);
  return (1);
}

//----------------------------------------------------------------------
//
//	Miscellaneous instructions
//
//----------------------------------------------------------------------
static
int
InstMovi2s (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;

  if (cpu->UserMode ()) {
    cpu->CauseException (DLX_EXC_PRIVILEGE);
    return (1);
  }
  cpu->GetRFields (inst, src1, src2, dst);
  DBPRINTF ('S',"Moving integer reg %d (0x%x) to special reg %d.\n",
	    src1, cpu->GetIreg(src1), dst);
  cpu->PutSreg (dst, cpu->GetIreg (src1));
  return (1);
}

static
int
InstMovs2i (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;

  if (cpu->UserMode ()) {
    cpu->CauseException (DLX_EXC_PRIVILEGE);
    return (1);
  }
  cpu->GetRFields (inst, src1, src2, dst);
  DBPRINTF ('S',"Moving special reg %d (0x%x) to integer reg %d.\n",
	    src1, cpu->GetSreg(src1), dst);
  cpu->PutIreg (dst, cpu->GetSreg (src1));
  return (1);
}

static
int
InstIllegal (uint32 inst, Cpu *cpu)
{
  cpu->CauseException (DLX_EXC_ILLEGALINST);
  return (1);
}

static
int
InstNop (uint32, Cpu *cpu)
{
  return (1);
}


//----------------------------------------------------------------------
//
//	Single precision FP math instructions.
//
//----------------------------------------------------------------------
static
int
InstAddf (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;
  cpu->GetRFields (inst, src1, src2, dst);
  cpu->PutFregF (dst, cpu->GetFregF (src1) + cpu->GetFregF (src2));
  return (1);
}

static
int
InstSubf (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;
  cpu->GetRFields (inst, src1, src2, dst);
  cpu->PutFregF (dst, cpu->GetFregF (src1) - cpu->GetFregF (src2));
  return (1);
}

static
int
InstMultf (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;
  cpu->GetRFields (inst, src1, src2, dst);
  cpu->PutFregF (dst, cpu->GetFregF (src1) * cpu->GetFregF (src2));
  return (1);
}

static
int
InstDivf (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;
  float	denom;
  cpu->GetRFields (inst, src1, src2, dst);
  denom = cpu->GetFregF (src2);
  if (denom == (float)0.0) {
    cpu->CauseException (DLX_EXC_DIV0);
  } else {
    cpu->PutFregF (dst, cpu->GetFregF (src1) / denom);
  }
  return (1);
}

//----------------------------------------------------------------------
//
//	Single precision FP comparison instructions
//
//----------------------------------------------------------------------
static
int
InstEqf (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;
  cpu->GetRFields (inst, src1, src2, dst);
  if (cpu->GetFregF (src1) == cpu->GetFregF (src2)) {
    cpu->SetStatusBit (DLX_STATUS_FPTRUE);
  } else {
    cpu->ClrStatusBit (DLX_STATUS_FPTRUE);
  }	
  return (1);
}

static
int
InstNef (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;
  cpu->GetRFields (inst, src1, src2, dst);
  if (cpu->GetFregF (src1) != cpu->GetFregF (src2)) {
    cpu->SetStatusBit (DLX_STATUS_FPTRUE);
  } else {
    cpu->ClrStatusBit (DLX_STATUS_FPTRUE);
  }	
  return (1);
}
static
int
InstLtf (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;
  cpu->GetRFields (inst, src1, src2, dst);
  if (cpu->GetFregF (src1) < cpu->GetFregF (src2)) {
    cpu->SetStatusBit (DLX_STATUS_FPTRUE);
  } else {
    cpu->ClrStatusBit (DLX_STATUS_FPTRUE);
  }	
  return (1);
}

static
int
InstGtf (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;
  cpu->GetRFields (inst, src1, src2, dst);
  if (cpu->GetFregF (src1) > cpu->GetFregF (src2)) {
    cpu->SetStatusBit (DLX_STATUS_FPTRUE);
  } else {
    cpu->ClrStatusBit (DLX_STATUS_FPTRUE);
  }	
  return (1);
}

static
int
InstLef (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;
  cpu->GetRFields (inst, src1, src2, dst);
  if (cpu->GetFregF (src1) <= cpu->GetFregF (src2)) {
    cpu->SetStatusBit (DLX_STATUS_FPTRUE);
  } else {
    cpu->ClrStatusBit (DLX_STATUS_FPTRUE);
  }	
  return (1);
}

static
int
InstGef (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;
  cpu->GetRFields (inst, src1, src2, dst);
  if (cpu->GetFregF (src1) >= cpu->GetFregF (src2)) {
    cpu->SetStatusBit (DLX_STATUS_FPTRUE);
  } else {
    cpu->ClrStatusBit (DLX_STATUS_FPTRUE);
  }	
  return (1);
}

//----------------------------------------------------------------------
//
//	Double precision FP math instructions.
//
//----------------------------------------------------------------------
static
int
InstAddd (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;
  cpu->GetRFields (inst, src1, src2, dst);
  cpu->PutFregD (dst, cpu->GetFregD (src1) + cpu->GetFregD (src2));
  return (1);
}

static
int
InstSubd (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;
  cpu->GetRFields (inst, src1, src2, dst);
  cpu->PutFregD (dst, cpu->GetFregD (src1) - cpu->GetFregD (src2));
  return (1);
}

static
int
InstMultd (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;
  cpu->GetRFields (inst, src1, src2, dst);
  cpu->PutFregD (dst, cpu->GetFregD (src1) * cpu->GetFregD (src2));
  return (1);
}

static
int
InstDivd (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;
  double	denom;
  cpu->GetRFields (inst, src1, src2, dst);
  denom = cpu->GetFregD (src2);
  if (denom == (double)0.0) {
    cpu->CauseException (DLX_EXC_DIV0);
  } else {
    cpu->PutFregD (dst, cpu->GetFregD(src1) / denom);
  }
  return (1);
}

//----------------------------------------------------------------------
//
//	Double precision FP comparison instructions
//
//----------------------------------------------------------------------
static
int
InstEqd (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;
  cpu->GetRFields (inst, src1, src2, dst);
  if (cpu->GetFregD (src1) == cpu->GetFregD (src2)) {
    cpu->SetStatusBit (DLX_STATUS_FPTRUE);
  } else {
    cpu->ClrStatusBit (DLX_STATUS_FPTRUE);
  }	
  return (1);
}

static
int
InstNed (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;
  cpu->GetRFields (inst, src1, src2, dst);
  if (cpu->GetFregD (src1) != cpu->GetFregD (src2)) {
    cpu->SetStatusBit (DLX_STATUS_FPTRUE);
  } else {
    cpu->ClrStatusBit (DLX_STATUS_FPTRUE);
  }	
  return (1);
}

static
int
InstLtd (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;
  cpu->GetRFields (inst, src1, src2, dst);
  if (cpu->GetFregD (src1) < cpu->GetFregD (src2)) {
    cpu->SetStatusBit (DLX_STATUS_FPTRUE);
  } else {
    cpu->ClrStatusBit (DLX_STATUS_FPTRUE);
  }	
  return (1);
}

static
int
InstGtd (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;
  cpu->GetRFields (inst, src1, src2, dst);
  if (cpu->GetFregD (src1) > cpu->GetFregD (src2)) {
    cpu->SetStatusBit (DLX_STATUS_FPTRUE);
  } else {
    cpu->ClrStatusBit (DLX_STATUS_FPTRUE);
  }	
  return (1);
}

static
int
InstLed (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;
  cpu->GetRFields (inst, src1, src2, dst);
  if (cpu->GetFregD (src1) <= cpu->GetFregD (src2)) {
    cpu->SetStatusBit (DLX_STATUS_FPTRUE);
  } else {
    cpu->ClrStatusBit (DLX_STATUS_FPTRUE);
  }	
  return (1);
}

static
int
InstGed (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;
  cpu->GetRFields (inst, src1, src2, dst);
  if (cpu->GetFregD (src1) >= cpu->GetFregD (src2)) {
    cpu->SetStatusBit (DLX_STATUS_FPTRUE);
  } else {
    cpu->ClrStatusBit (DLX_STATUS_FPTRUE);
  }	
  return (1);
}

//----------------------------------------------------------------------
//
//	Conversion instructions
//
//----------------------------------------------------------------------
static
int
InstCvtf2d (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;
  cpu->GetRFields (inst, src1, src2, dst);
  cpu->PutFregD (dst, (double)cpu->GetFregF(src1));
  return (1);
}

static
int
InstCvtf2i (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;
  cpu->GetRFields (inst, src1, src2, dst);
  cpu->PutFreg (dst, (int)cpu->GetFregF(src1));
  return (1);
}

static
int
InstCvtd2f (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;
  cpu->GetRFields (inst, src1, src2, dst);
  cpu->PutFregF (dst, (float)cpu->GetFregD(src1));
  return (1);
}

static
int
InstCvtd2i (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;
  cpu->GetRFields (inst, src1, src2, dst);
  cpu->PutFreg (dst, (int)cpu->GetFregD(src1));
  return (1);
}

static
int
InstCvti2f (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;
  cpu->GetRFields (inst, src1, src2, dst);
  cpu->PutFregF (dst, (float)cpu->GetFreg(src1));
  return (1);
}

static
int
InstCvti2d (uint32 inst, Cpu *cpu)
{
  uint32	src1, src2, dst;
  cpu->GetRFields (inst, src1, src2, dst);
  DBPRINTF ('f',"Converting f%d (%08x) to DFP (%lf) in f%d\n",
	    src1, cpu->GetFreg(src1), (double)cpu->GetFreg(src1), dst);
  cpu->PutFregD (dst, (double)cpu->GetFreg(src1));
  return (1);
}

//----------------------------------------------------------------------
//
//	RR Instruction Table
//
//----------------------------------------------------------------------
Instruction Cpu::rrrInstrs[64] = {
  {0x00, DLX_FMT_RFMT, InstIllegal},
  {0x01, DLX_FMT_RFMT, InstIllegal},
  {0x02, DLX_FMT_RFMT, InstIllegal},
  {0x03, DLX_FMT_RFMT, InstIllegal},
  {0x04, DLX_FMT_RFMT, InstSll},
  {0x05, DLX_FMT_RFMT, InstIllegal},
  {0x06, DLX_FMT_RFMT, InstSrl},
  {0x07, DLX_FMT_RFMT, InstSra},
  {0x08, DLX_FMT_RFMT, InstIllegal},
  {0x09, DLX_FMT_RFMT, InstIllegal},
  {0x0a, DLX_FMT_RFMT, InstIllegal},
  {0x0b, DLX_FMT_RFMT, InstIllegal},
  {0x0c, DLX_FMT_RFMT, InstIllegal},		// ALU TRAP instruction
  {0x0d, DLX_FMT_RFMT, InstIllegal},
  {0x0e, DLX_FMT_RFMT, InstIllegal},
  {0x0f, DLX_FMT_RFMT, InstIllegal},
  {0x10, DLX_FMT_RFMT, InstIllegal},
  {0x11, DLX_FMT_RFMT, InstIllegal},
  {0x12, DLX_FMT_RFMT, InstIllegal},
  {0x13, DLX_FMT_RFMT, InstIllegal},
  {0x14, DLX_FMT_RFMT, InstIllegal},
  {0x15, DLX_FMT_RFMT, InstIllegal},
  {0x16, DLX_FMT_RFMT, InstIllegal},
  {0x17, DLX_FMT_RFMT, InstIllegal},
  {0x18, DLX_FMT_RFMT, InstIllegal},
  {0x19, DLX_FMT_RFMT, InstIllegal},
  {0x1a, DLX_FMT_RFMT, InstIllegal},
  {0x1b, DLX_FMT_RFMT, InstIllegal},
  {0x1c, DLX_FMT_RFMT, InstIllegal},
  {0x1d, DLX_FMT_RFMT, InstIllegal},
  {0x1e, DLX_FMT_RFMT, InstIllegal},
  {0x1f, DLX_FMT_RFMT, InstIllegal},
  {0x20, DLX_FMT_RFMT, InstAdd},
  {0x21, DLX_FMT_RFMT, InstAddu},
  {0x22, DLX_FMT_RFMT, InstSub},
  {0x23, DLX_FMT_RFMT, InstSubu},
  {0x24, DLX_FMT_RFMT, InstAnd},
  {0x25, DLX_FMT_RFMT, InstOr},
  {0x26, DLX_FMT_RFMT, InstXor},
  {0x27, DLX_FMT_RFMT, InstIllegal},
  {0x28, DLX_FMT_RFMT, InstSeq},
  {0x29, DLX_FMT_RFMT, InstSne},
  {0x2a, DLX_FMT_RFMT, InstSlt},
  {0x2b, DLX_FMT_RFMT, InstSgt},
  {0x2c, DLX_FMT_RFMT, InstSle},
  {0x2d, DLX_FMT_RFMT, InstSge},
  {0x2e, DLX_FMT_RFMT, InstIllegal},
  {0x2f, DLX_FMT_RFMT, InstIllegal},
  {0x30, DLX_FMT_RFMT, InstMovi2s},
  {0x31, DLX_FMT_RFMT, InstMovs2i},
  {0x32, DLX_FMT_RFMT, InstMovf},
  {0x33, DLX_FMT_RFMT, InstMovd},
  {0x34, DLX_FMT_RFMT, InstMovfp2i},
  {0x35, DLX_FMT_RFMT, InstMovi2fp},
  {0x36, DLX_FMT_RFMT, InstIllegal},
  {0x37, DLX_FMT_RFMT, InstIllegal},
  {0x38, DLX_FMT_RFMT, InstIllegal},
  {0x39, DLX_FMT_RFMT, InstIllegal},
  {0x3a, DLX_FMT_RFMT, InstIllegal},
  {0x3b, DLX_FMT_RFMT, InstIllegal},
  {0x3c, DLX_FMT_RFMT, InstIllegal},
  {0x3d, DLX_FMT_RFMT, InstIllegal},
  {0x3e, DLX_FMT_RFMT, InstIllegal},
  {0x3f, DLX_FMT_RFMT, InstIllegal},
};

//----------------------------------------------------------------------
//
//	Table of "regular" instructions
//
//----------------------------------------------------------------------
Instruction Cpu::regInstrs[64] = {
  {0x00, DLX_FMT_RFMT, InstIllegal},		// R-format instructions
  {0x01, DLX_FMT_RFMT, InstIllegal},		// FP instructions
  {0x02, DLX_FMT_JFMT, InstJmp},
  {0x03, DLX_FMT_JFMT, InstJal},
  {0x04, DLX_FMT_IFMT, InstBeqz},
  {0x05, DLX_FMT_IFMT, InstBnez},
  {0x06, DLX_FMT_IFMT, InstBfpt},
  {0x07, DLX_FMT_IFMT, InstBfpf},
  {0x08, DLX_FMT_IFMT, InstAddi},
  {0x09, DLX_FMT_IFMT, InstAddui},
  {0x0a, DLX_FMT_IFMT, InstSubi},
  {0x0b, DLX_FMT_IFMT, InstSubui},
  {0x0c, DLX_FMT_IFMT, InstAndi},
  {0x0d, DLX_FMT_IFMT, InstOri},
  {0x0e, DLX_FMT_IFMT, InstXori},
  {0x0f, DLX_FMT_IFMT, InstLhi},
  {0x10, DLX_FMT_JFMT, InstRfe},
  {0x11, DLX_FMT_JFMT, InstTrap},
  {0x12, DLX_FMT_JFMT, InstJr},
  {0x13, DLX_FMT_JFMT, InstJalr},
  {0x14, DLX_FMT_IFMT, InstSlli},
  {0x15, DLX_FMT_IFMT, InstNop},
  {0x16, DLX_FMT_IFMT, InstSrli},
  {0x17, DLX_FMT_IFMT, InstSrai},
  {0x18, DLX_FMT_IFMT, InstSeqi},
  {0x19, DLX_FMT_IFMT, InstSnei},
  {0x1a, DLX_FMT_IFMT, InstSlti},
  {0x1b, DLX_FMT_IFMT, InstSgti},
  {0x1c, DLX_FMT_IFMT, InstSlei},
  {0x1d, DLX_FMT_IFMT, InstSgei},
  {0x1e, DLX_FMT_IFMT, InstIllegal},
  {0x1f, DLX_FMT_IFMT, InstIllegal},
  {0x20, DLX_FMT_IFMT, InstLb},
  {0x21, DLX_FMT_IFMT, InstLh},
  {0x22, DLX_FMT_IFMT, InstIllegal},
  {0x23, DLX_FMT_IFMT, InstLw},
  {0x24, DLX_FMT_IFMT, InstLbu},
  {0x25, DLX_FMT_IFMT, InstLhu},
  {0x26, DLX_FMT_IFMT, InstLf},
  {0x27, DLX_FMT_IFMT, InstLd},
  {0x28, DLX_FMT_IFMT, InstSb},
  {0x29, DLX_FMT_IFMT, InstSh},
  {0x2a, DLX_FMT_IFMT, InstIllegal},
  {0x2b, DLX_FMT_IFMT, InstSw},
  {0x2c, DLX_FMT_IFMT, InstIllegal},
  {0x2d, DLX_FMT_IFMT, InstIllegal},
  {0x2e, DLX_FMT_IFMT, InstSf},
  {0x2f, DLX_FMT_IFMT, InstSd},
  {0x30, DLX_FMT_IFMT, InstIllegal},
  {0x31, DLX_FMT_IFMT, InstIllegal},
  {0x32, DLX_FMT_IFMT, InstIllegal},
  {0x33, DLX_FMT_IFMT, InstIllegal},
  {0x34, DLX_FMT_IFMT, InstIllegal},
  {0x35, DLX_FMT_IFMT, InstIllegal},
  {0x36, DLX_FMT_IFMT, InstIllegal},
  {0x37, DLX_FMT_IFMT, InstIllegal},
  {0x38, DLX_FMT_IFMT, InstIllegal},
  {0x39, DLX_FMT_IFMT, InstIllegal},
  {0x3a, DLX_FMT_IFMT, InstIllegal},
  {0x3b, DLX_FMT_IFMT, InstIllegal},
  {0x3c, DLX_FMT_IFMT, InstIllegal},
  {0x3d, DLX_FMT_IFMT, InstIllegal},
  {0x3e, DLX_FMT_IFMT, InstIllegal},
  {0x3f, DLX_FMT_IFMT, InstIllegal},
};

//----------------------------------------------------------------------
//
//	Table of FP instructions
//
//----------------------------------------------------------------------
Instruction Cpu::fpInstrs[32] = {
  {0x00, DLX_FMT_RFMT, InstAddf},
  {0x01, DLX_FMT_RFMT, InstSubf},
  {0x02, DLX_FMT_RFMT, InstMultf},
  {0x03, DLX_FMT_RFMT, InstDivf},
  {0x04, DLX_FMT_RFMT, InstAddd},
  {0x05, DLX_FMT_RFMT, InstSubd},
  {0x06, DLX_FMT_RFMT, InstMultd},
  {0x07, DLX_FMT_RFMT, InstDivd},
  {0x08, DLX_FMT_RFMT, InstCvtf2d},
  {0x09, DLX_FMT_RFMT, InstCvtf2i},
  {0x0a, DLX_FMT_RFMT, InstCvtd2f},
  {0x0b, DLX_FMT_RFMT, InstCvtd2i},
  {0x0c, DLX_FMT_RFMT, InstCvti2f},
  {0x0d, DLX_FMT_RFMT, InstCvti2d},
  {0x0e, DLX_FMT_RFMT, InstMult},
  {0x0f, DLX_FMT_RFMT, InstDiv},
  {0x10, DLX_FMT_RFMT, InstEqf},
  {0x11, DLX_FMT_RFMT, InstNef},
  {0x12, DLX_FMT_RFMT, InstLtf},
  {0x13, DLX_FMT_RFMT, InstGtf},
  {0x14, DLX_FMT_RFMT, InstLef},
  {0x15, DLX_FMT_RFMT, InstGef},
  {0x16, DLX_FMT_RFMT, InstMultu},
  {0x17, DLX_FMT_RFMT, InstDivu},
  {0x18, DLX_FMT_RFMT, InstEqd},
  {0x19, DLX_FMT_RFMT, InstNed},
  {0x1a, DLX_FMT_RFMT, InstLtd},
  {0x1b, DLX_FMT_RFMT, InstGtd},
  {0x1c, DLX_FMT_RFMT, InstLed},
  {0x1d, DLX_FMT_RFMT, InstGed},
  {0x1e, DLX_FMT_RFMT, InstIllegal},
  {0x1f, DLX_FMT_RFMT, InstIllegal},
};

//----------------------------------------------------------------------
//
//	Cpu::CauseException
//
//	Cause an exception.  This loads the CAUSE register with the
//	exception cause, loads the IAR with the proper pointer, and
//	sets things up so that the exception will be taken after the
//	current instruction completes.
//
//----------------------------------------------------------------------
int
Cpu::CauseException (int excType)
{
  uint32	ivec;

  DBPRINTF ('t',"Exception being done (cause=0x%x @ pc=0x%x).\n",excType,
	    PC()-4);
  ivec = GetSreg (DLX_SREG_INTRVEC);
  OutputBasicBlock (ivec);
  if (flags & (DLX_TRACE_INSTRUCTIONS | DLX_TRACE_MEMORY)) {
    fprintf (tracefp, "X %x %x\n",excType, PC()-4);
  }
  PutSreg(DLX_SREG_CAUSE, excType);
  // PC has already been incremented, so decrement it first.  If this
  // is a trap or interrupt, the PC will have already been incremented
  // (if necessary) so the IAR points to the next instruction to
  // execute.
  PutSreg(DLX_SREG_IAR, PC()-4);
  // Save the current status register
  PutSreg(DLX_SREG_ISR, GetSreg (DLX_SREG_STATUS));
  // Save the current value of register 31.  This is necessary to give
  // the interrupt handler a temporary register that can be used to
  // switch to a system stack (rather than user stack)
  PutSreg(DLX_SREG_IR31, GetIreg (31));
  // Set the next instruction to be run to be the interrupt vector.
  SetPC (ivec);
  // Set the status register to be system mode
  PutSreg(DLX_SREG_STATUS, GetSreg (DLX_SREG_STATUS) | DLX_STATUS_SYSMODE);
  // Turn off interrupts
  DisableInterrupts ();
  return (1);
}



//----------------------------------------------------------------------
//
//	Cpu::VaddrToPaddr
//
//	Read a single word from memory.  This involves translating
//	the virtual address to a physical address, checking that the
//	access is allowed, and ensuring that the address itself is
//	valid (ie, aligned).  An exception is caused (and 0 returned)
//	if the access fails for any reason.
//
//----------------------------------------------------------------------
inline
int
Cpu::VaddrToPaddr (uint32 vaddr, uint32& paddr, uint32 op, uint32 pteflags)
{
  uint32	pt1base, pt2base, pt1pagebits, pt2pagebits;
  uint32	pteaddr;
  uint32	offsetinpage, entrynum;
  uint32	pagemask;

  if ((vaddr & 0x3) != 0) {
    CauseException (DLX_EXC_ADDRESS);
    return (0);
  }
  if (StatusBit (DLX_STATUS_PAGE_TABLE)) {
    // Translate if in user mode or if in system mode and the appropriate
    // translation bit is set in the status register.
    if (UserMode () ||
	((op == DLX_MEM_READ) &&
	 (GetSreg (DLX_SREG_STATUS) & DLX_STATUS_XLATE_RD)) ||
	((op == DLX_MEM_WRITE) &&
	 (GetSreg (DLX_SREG_STATUS) & DLX_STATUS_XLATE_WR))) {
      DBPRINTF ('m', "Translating 0x%x\n", vaddr);
      pt1base = GetSreg (DLX_SREG_PGTBL_BASE);
      pt1pagebits = GetSreg (DLX_SREG_PGTBL_BITS);
      pt2pagebits = (pt1pagebits >> 16) & 0xffff;
      pt1pagebits &= 0xffff;
      pagemask = (1 << pt2pagebits) - 1;
      offsetinpage = vaddr & pagemask;
      // Mask off the low bits
      vaddr &= ~pagemask;
      if ((entrynum = (vaddr >> pt1pagebits)) >=
	  GetSreg (DLX_SREG_PGTBL_SIZE)) {
	DBPRINTF ('m', "Out of range (L1 = %db, L2 = %db size=%d entry=%d)\n",
		  pt1pagebits, pt2pagebits, GetSreg(DLX_SREG_PGTBL_SIZE),
		  entrynum);
	CauseException (DLX_EXC_ACCESS);
	return (0);
      }
      pteaddr = pt1base + 4 * entrynum;
      paddr = Memory (pteaddr);
      // If the L2 page size is the same as the L1 page size, there's
      // no L2 page table!
      if (pt1pagebits != pt2pagebits) {
	pt2base = paddr;
	if (pt2base == 0) {
	  DBPRINTF ('m', "No L2 table at entry %d! (base = 0x%x)\n",
		    entrynum, pt1base);
          PutSreg (DLX_SREG_FAULT_ADDR, vaddr);
	  CauseException (DLX_EXC_PAGEFAULT);
	  return (0);
	}
	pteaddr = pt2base + 4 * ((vaddr >> pt2pagebits) &
				 ((1 << (pt1pagebits-pt2pagebits))-1));
	paddr = Memory (pteaddr);
      }
      DBPRINTF ('M', "Using PTE 0x%08x\n", paddr);
      if (!(paddr & DLX_PTE_VALID)) {
	DBPRINTF ('m', "PTE invalid (0x%08x)\n", paddr);
	PutSreg (DLX_SREG_FAULT_ADDR, vaddr);
	CauseException (DLX_EXC_PAGEFAULT);
	return (0);
      }

      //Zheng{
#if USE_ROP
      if ((op == DLX_MEM_WRITE) && (paddr & DLX_PTE_RW)) {
#if USE_ROP_DEBUG
        printf("Cpu: writing a read-only page, vaddr=0x%x\n", vaddr);
#endif
        PutSreg (DLX_SREG_FAULT_ADDR, vaddr);
        CauseException(DLX_ROP_ACCESS);
        return (0);
      }
#endif
      //}Zheng

      //Zheng{
#if USE_ROP
      if (pteflags & DLX_PTE_DIRTY) {
	SetMemory (pteaddr,
		   paddr | (pteflags & DLX_PTE_DIRTY));
      }
#else
      //}Zheng
      if (pteflags & (DLX_PTE_DIRTY | DLX_PTE_REFERENCED)) {
	SetMemory (pteaddr,
		   paddr | (pteflags & (DLX_PTE_DIRTY | DLX_PTE_REFERENCED)));
      }
      //Zheng
#endif

      paddr &= ~(pagemask | DLX_PTE_MASK);
      paddr |= offsetinpage;
      DBPRINTF ('m',
		"0x%x => 0x%x (=%08x) using base1=0x%x/%d, entry %d\n",
		vaddr | offsetinpage, paddr,
		Memory (paddr),
		pt1base, pt1pagebits, entrynum);
      return (1);
    } else {
      // For system references, physical address is the same
      // as virtual address.
      paddr = vaddr;
      if ((vaddr <= memSize) || ((vaddr >= DLX_IO_BASE) &&
				 (vaddr <= (DLX_IO_BASE+DLX_IO_SIZE)))) {
	return (1);
      } else {
	DBPRINTF ('t',"Illegal system address: 0x%x.\n", vaddr);
	CauseException (DLX_EXC_ACCESS);
	return (0);
      }
    }
  } else if (StatusBit (DLX_STATUS_TLB)) {
    // DLX_STATUS_TLB
    return (1);
  } else {
    paddr = vaddr;
    return (1);
  }
}

//----------------------------------------------------------------------
//
//	Cpu::ReadWord
//
//	Read a word from memory.  This can either be a regular memory
//	address or an I/O address.
//
//----------------------------------------------------------------------
int
Cpu::ReadWord (uint32 vaddr, uint32 &val, uint32 op)
{
  uint32	paddr;

  DBPRINTF ('l',"Trying to read virtual address: 0x%x.\n", vaddr);
//Zheng{
#if USE_ROP
  if (!VaddrToPaddr (vaddr, paddr, op, 0)) {
#else
//}Zheng
  if (!VaddrToPaddr (vaddr, paddr, op, DLX_PTE_REFERENCED)) {
#endif
    return (0);
  }
  if (paddr <= memSize) {
    val = Memory(paddr);
  } else {
    DBPRINTF ('l',"Trying to load special address: 0x%x.\n", paddr);
    switch (paddr) {
    case DLX_KBD_NCHARSIN:
      val = KbdNumInChars ();
      break;
    case DLX_KBD_NCHARSOUT:
      val = KbdNumOutChars ();
      break;
    case DLX_KBD_GETCHAR:
      val = KbdGetChar ();
      break;
    case DLX_DISK_STATUS:
      break;
    case DLX_GETMEMSIZE:
      val = memSize;
      break;
    default:
      CauseException (DLX_EXC_ACCESS);
      break;
    }
  }
  return (1);
}

int
Cpu::WriteWord (uint32 vaddr, uint32 val)
{
  uint32	paddr;
  //Zheng{
#if USE_ROP
  if (!VaddrToPaddr (vaddr, paddr, DLX_MEM_WRITE,
		     DLX_PTE_DIRTY)) {
#else
  //}Zheng
  if (!VaddrToPaddr (vaddr, paddr, DLX_MEM_WRITE,
		     DLX_PTE_DIRTY | DLX_PTE_REFERENCED)) {
#endif
    return (0);
  }

  if (paddr <= memSize) {
    SetMemory(paddr, val);
  } else {
    switch (paddr) {
    case DLX_KBD_PUTCHAR:
      KbdPutChar (val);
      break;
    case DLX_KBD_INTR:
      if (val == 0) {
	flags &= ~DLX_FLAG_KBD_INTERRUPT;
      } else {
	flags |= DLX_FLAG_KBD_INTERRUPT;
      }
      break;
    case DLX_TIMER_SETTIMER:
      DBPRINTF ('o',"Setting timer to %d us.\n", val);
      SetTimer (val);
      break;
#if 0
    case DLX_DISK_REQUEST:
      diskReq = val;
      StartDiskIo ();
      break;
    case DLX_DISK_BLOCK:
      diskBlock = val;
      break;
    case DLX_DISK_ADDR:
      diskAddr = val;
      break;
#endif
    default:
      CauseException (DLX_EXC_ACCESS);
      break;
    }
  }
  return (1);
}

int
Cpu::TestWriteWord (uint32 vaddr)
{
  uint32	paddr;
  if (!VaddrToPaddr (vaddr, paddr, DLX_MEM_WRITE)) {
    return (0);
  } else {
    return (1);
  }
}

//----------------------------------------------------------------------
//
//	Cpu::Open
//
//	Open a file in the "real" file system.  All parameters (in
//	particular, the file name) must be passed by physical address in
//	the simulator's address space.
//	accessType = 1 for read, 2 for write, 3 for both.
//
//	The trap is called as:
//	Open (name, accesstype)
//
//----------------------------------------------------------------------
void
Cpu::Open ()
{
  uint32	name;
  char	nameBuf[100];
  char	*tp;
  int		accessType;
  int		i;

  name = GetParam(0);
  accessType = GetParam(1);
  DBPRINTF ('F', "Opening file %s (mode=%d).\n", (char *)memory + name,
	    accessType);
  switch (accessType) {
  case 1:
    tp = "r";
    break;
  case 2:
    tp = "w";
    break;
  case 3:
    tp = "r+";
    break;
  default:
    SetResult (0xffffffff);
    return;
    break;
  }
  for (i = 0; i < DLX_MAX_FILES; i++) {
    if (fp[i] == NULL) {
      if (!CheckAddr (name)) {
	SetResult (0xffffffff);
	return;
      }
      strncpy (nameBuf, (char *)memory + name, 98);
      // If fopen fails, it returns NULL, so it looks like no open
      // was done.
      fp[i] = fopen (nameBuf, tp);
      break;
    }
  }
  if (i >= DLX_MAX_FILES) {
    i = -1;
  } else if (fp[i] == NULL) {
    i = -errno;
  }
  DBPRINTF ('F', "Open returns file descriptor %d\n", i);
  SetResult (i);
}

//----------------------------------------------------------------------
//
//	Cpu::Read
//	Cpu::Write
//
//	Read data from a previously opened file in the real world.
//	Called as:
//	Read (int desc, void* buf, int size)
//	Write (int desc, void* buf, int size)
//	Returns number of bytes read if there were more than 0, 0 on
//	end of file, and -1 otherwise.
//
//----------------------------------------------------------------------
void
Cpu::Read ()
{
  FileIo (DLX_FILE_READ);
}

void
Cpu::Write ()
{
  FileIo (DLX_FILE_WRITE);
}

void
Cpu::FileIo (int kind)
{
  int		fd;
  uint32	buf;
  int		size;
  int		n;

  fd = GetParam (0);
  buf = GetParam (1);
  size = GetParam (2);
  DBPRINTF ('F', "FileIo (%s) on fd %d, size %d, buffer=0x%x\n",
	    (kind == DLX_FILE_WRITE) ? "write" : "read",
	    fd, size, buf);
  if (! CheckAddr (buf) || (! CheckFd (fd))) {
    SetResult (0xffffffff);
    return;
  }
  if (kind == DLX_FILE_WRITE) {
    n = fwrite ((unsigned char *)memory + buf, 1, size, fp[fd]);
  } else {
    n = fread ((unsigned char *)memory + buf, 1, size, fp[fd]);
  }
  if (n > 0) {
    SetResult (n);
  } else if (feof (fp[fd])) {
    SetResult (0);
  } else {
    SetResult (-errno);
  }
}

//----------------------------------------------------------------------
//
//	Cpu::Seek
//
//	Seek to a position in the file.  Call it like:
//	Seek (int fd, int offset, int from)
//	Returns the new file position, or -1 if it fails.
//
//----------------------------------------------------------------------
void
Cpu::Seek ()
{
  int		fd;
  int		offset;
  int		whence;
  int		rv;

  fd = GetParam(0);
  offset = GetParam(1);
  whence = GetParam(2);
  if (! CheckFd (fd)) {
    SetResult (0xffffffff);
    return;
  }
  if ((rv = fseek (fp[fd], offset, whence)) < 0) {
    SetResult (rv);
  } else {
    SetResult (ftell (fp[fd]));
  }
}

//----------------------------------------------------------------------
//
//	Cpu::Close
//
//	Close a file and release its resources.
//
//----------------------------------------------------------------------
void
Cpu::Close ()
{
  int		fd;
  uint32	retval;

  fd = GetParam(0);
  if (!CheckFd (fd)) {
    retval = 0xffffffff;
  } else {
    retval = fclose (fp[fd]);
  }
  DBPRINTF ('F', "Closing file %d.\n", fd);
  fp[fd] = NULL;
  SetResult (retval);
}

//----------------------------------------------------------------------
//
//	Cpu::Random
//
//	Return a random number using the underlying random() function
//	call.
//
//----------------------------------------------------------------------
void
Cpu::Random ()
{
  int		n;

  n = random ();
  SetResult (n);
}

//----------------------------------------------------------------------
//
//	Cpu::Srandom
//
//	Set the random number generator to the number passed.
//
//----------------------------------------------------------------------
void
Cpu::Srandom ()
{
  uint32	seed;

  seed = GetParam (0);
  srandom (seed);
  SetResult (0);
}

//----------------------------------------------------------------------
//
//	Cpu::GetParam
//
//	Parameters are stored on the stack (r29), with the first parameter
//	stored at [r29].  Other parameters are stored at [r29+4], [r29+8],
//	etc.  Parameters are passed on word boundaries.
//
//----------------------------------------------------------------------
uint32
Cpu::GetParam (int p)
{
  uint32	stackPtr;

  stackPtr = GetIreg (29);
  return (Memory(stackPtr + (p << 2)));
}

//----------------------------------------------------------------------
//
//	Cpu::SetResult
//
//	Results are stored in register 1.
//
//----------------------------------------------------------------------
void
Cpu::SetResult (uint32 r)
{
  PutIreg (1, r);
}

//----------------------------------------------------------------------
//
//	Cpu::Printf
//
//	This is the actual printf routine because we have to handle strings
//	differently because of the pointers involved.  Everything else works
//	as would be expected.
//
//	IMPORTANT: floating point numbers aren't supported.
//
//	IMPORTANT: all pointers must be passed as addresses in physical
//	simulated memory, not as virtual addresses.  This basically means
//	that you don't want to call printf from user level because the
//	format string itself is in user space.  Of course, you can always
//	copy it into kernel space, but it's harder to copy the other string
//	arguments.
//
//----------------------------------------------------------------------
void
Cpu::Printf ()
{
  uint32	fmtaddr;
  char	*c;
  uint32	args[10];
  int		nargs = 0;

  fmtaddr = GetParam(0);
  // 
  for (c = fmtaddr + (char *)memory; *c != '\0'; c++) {
    if (*c == '%') {
      // if this is a %%, skip past second %
      if (*(c+1) == '%') {
	c++;
	continue;
      }
      // Get the current argument off the stack
      args[nargs] = GetParam(nargs+1);
      DBPRINTF ('p', "Argument %d at 0x%x is %d (0x%x).\n", nargs,
		args[nargs], args[nargs]);
      while (1) {
	c++;
	if (*c == 's') {
	  // If it's a string, the address is relative to the
	  // start of emulated memory.
	  args[nargs] += (uint32)memory;
	  break;
	} else if (*c == 'l') {
	  continue;
	} else if ((*c == 'f') || (*c == 'g') || (*c == 'e')) {
	  // If it's a floating point number, it'll be passed as
	  // a double, so grab the second word also.
	  nargs += 1;
	  args[nargs] = GetParam(nargs+1);
	  break;
	} else if ((*c >= 'a') && (*c <= 'z')) {
	  // If it's another formatting character, it's not
	  // a string, but we can leave the loop anyway.
	  break;
	}
      }
      nargs += 1;
    }
  }
  printf (fmtaddr + (char *)memory,
	  args[0], args[1], args[2], args[3],
	  args[4], args[5], args[6], args[7]);
  fflush (stdout);
}

//----------------------------------------------------------------------
//
//	Cpu::Exit
//
//	Exit the CPU altogether.  Before doing so, print statistics on
//	instructions executed and time taken.
//
//----------------------------------------------------------------------
void
Cpu::Exit ()
{
  struct timeval	t;

  printf ("Exiting at program request.\n");
  printf ("Instructions executed: %.0lf\n", instrsExecuted);
  printf ("Time simulated: %.03lf secs\n", usElapsed / 1e6);
  //Zheng add timezone*
  //gettimeofday (&t, (timezone*)(void *)0);
  gettimeofday (&t, (void *)0);
  realElapsed = ((double)t.tv_sec + ((double)t.tv_usec)*1e-6) - realElapsed;
  printf ("Real time elapsed: %.03lf secs\n", realElapsed);
  printf ("Execution rate: %.2lfM simulated instructions per real second.\n",
	  instrsExecuted * 1e-6 / realElapsed);
  exit (0);
}

//----------------------------------------------------------------------
//
//	Cpu::ExecOne
//
//	Execute a single CPU instruction in the simulator.
//
//----------------------------------------------------------------------
int
Cpu::ExecOne ()
{
  uint32	curInst;
  uint32	tmpPc;
  uint32	curOp;
  uint32	retval;
  uint32	funcCode;		// subcode for RRR & FP ops

  usElapsed += usPerInst;
  instrsExecuted += 1.0;
  // Increment PC before checking for interrupts because CauseException
  // will subtract 4 off the PC before placing the value into the IAR.
  // By incrementing here, we ensure that the current instruction is
  // the one whose address goes into the IAR.
  SetPC (PC() + 4);
  // Check for an input character.  If we got one and interrupts are
  // enabled, do an interrupt.
  if (kbdcounter++ > DLX_KBD_FREQUENCY) {
    kbdcounter = 0;
    if (GetCharIfAvail () && (IntrLevel () < 8)) {
      DBPRINTF ('t',"Keyboard interrupt at PC=0x%x, t=%.0fus\n",
		PC()-4, usElapsed);
      CauseException (DLX_EXC_KBD);
      return (0);
    }
  }
  if (IntrLevel() < 8) {
    if (timerInterrupt < usElapsed) {
      DBPRINTF ('t', "Timer interrupt at PC=0x%x, t=%.0fus, intr@%.0fus\n",
		PC()-4, usElapsed, timerInterrupt);
      timerInterrupt = DLX_TIMER_NOT_ACTIVE;
      CauseException (DLX_EXC_TIMER);
      return (0);
    }
  }
  if (! ReadWord (PC()-4, curInst, DLX_MEM_INSTR)) {
    DBPRINTF ('I', "Instruction fetch at 0x%x failed!\n", PC()-4);
    return (0);
  }
  curOp = (curInst >> DLX_OPCODE_SHIFT) & DLX_OPCODE_MASK;
  DBPRINTF ('I', "Instr %06d: %08x : %08x (main=%02x, aux=%02x)\n",
	    (int)instrsExecuted % 1000000,
	    curInst, PC() - 4, curOp,
	    (curInst >> DLX_ALU_FUNC_CODE_SHIFT) & DLX_ALU_FUNC_CODE_MASK);
  switch (curOp) {
  case 0x00:		// ALU and other R-R operations
    funcCode = ((curInst >> DLX_ALU_FUNC_CODE_SHIFT) &
		DLX_ALU_FUNC_CODE_MASK);
    retval = (rrrInstrs[funcCode].handler)(curInst, this);
    break;
  case 0x01:		// FP operations
    funcCode = ((curInst >> DLX_FPU_FUNC_CODE_SHIFT) &
		DLX_FPU_FUNC_CODE_MASK);
    retval = (fpInstrs[funcCode].handler)(curInst, this);
    break;
  default:
    retval = (regInstrs[curOp].handler)(curInst, this);
    break;
  }
  return (retval);
}

//----------------------------------------------------------------------
//
//	Cpu::LoadMemory
//
//	Load a file into memory.  The file format consists of a
//	leading address, followed by a colon, followed by the data
//	to go at that address.  If the address is omitted, the data
//	follows that from the previous line of the file.
//
//----------------------------------------------------------------------
static
inline
int
getxvalue (int x)
{
  if ((x >= '0') && (x <= '9')) {
    return (x - '0');
  } else if ((x >= 'a') && (x <= 'f')) {
    return (x + 10 - 'a');
  } else if ((x >= 'A') && (x <= 'F')) {
    return (x + 10 - 'A');
  } else {
    return (0);
  }
}

int
Cpu::LoadMemory (const char *file, uint32& startAt)
{
  FILE	*fp;
  char	buffer[200];
  char	*pos;
  int		nread = 0;
  uint32	addr = 0;
  uint32	val;
  int		count;

  if ((fp = fopen (file, "r")) == NULL) {
    return (0);
  }
  if (fgets (buffer, sizeof (buffer) - 1, fp) == NULL) {
    return (0);
  }
  // Read in the "start" line
  if (strstr (buffer, "start:") == NULL) {
    return (0);
  }
  pos = index (buffer, ':') + 1;
  startAt = strtol (pos, NULL, 16);
  while (1) {
    pos = buffer;
    if (fgets (buffer, sizeof (buffer) - 1, fp) == NULL) {
      return (nread);
    }
    if (index (buffer, ':') == NULL) {
      continue;
    }
    if (*pos != ':') {
      addr = strtol (pos, &pos, 16);
    }
    if (*pos != ':') {
      fprintf (stderr, "Error reading data file near:\n%s\n", buffer);
      return (nread);
    }
    pos++;	// skip past colon
    while (1) {
      while (isspace (*pos)) {
	pos++;
      }
      // Exit loop if at least one digit isn't a hex digit.
      if (!(isxdigit (*pos) && isxdigit (*(pos+1)))) {
	break;
      }
      val = (getxvalue(*pos) * 16) + getxvalue(*(pos+1));
      *(((unsigned char *)memory) + addr) = val;
      pos += 2;
      addr++;
      nread++;
    }
  }
}

//----------------------------------------------------------------------
//
//	Cpu::SetTimer
//
//	Set the CPU timer.  This simply causes an interrupt after the
//	specified time (in microseconds) has elapsed.
//
//----------------------------------------------------------------------
void
Cpu::SetTimer (uint32 usecs)
{
  timerInterrupt = usElapsed + (double)usecs;
}

//----------------------------------------------------------------------
//
//	Cpu::Timerget
//
//	return Simulated time(in milliseconds) 
//
//----------------------------------------------------------------------
uint32 
Cpu::Timerget()
{
   unsigned int result;
   result = (unsigned int)(usElapsed/1e3);
   SetResult (result);
}
//----------------------------------------------------------------------
//
//	Cpu::KbdGetChar
//
//	Get a single character from the keyboard buffer.
//
//----------------------------------------------------------------------
uint32
Cpu::KbdGetChar ()
{
  uint32	v;

  if (kbdbufferedchars == 0) {
    return (0);
  }
  v = kbdbuffer[kbdrpos++];
  kbdrpos %= DLX_KBD_BUFFER_SIZE;
  kbdbufferedchars--;
  return (v);
}

//----------------------------------------------------------------------
//
//	Cpu::IgnoreExit
//
//----------------------------------------------------------------------
void
Cpu::IgnoreExit (int ignore)
{
  flags &= ~DLX_FLAG_IGNORE_EXIT;
  if (ignore) {
    flags |= DLX_FLAG_IGNORE_EXIT;
  }
}

//----------------------------------------------------------------------
//
//	Cpu::OutputBasicBlock
//
//	Print a basic block and reset statistics gathering.
//
//----------------------------------------------------------------------
void
Cpu::OutputBasicBlockActual ()
{
  int		i, ninstrs;

  ninstrs = (PC() - basicBlockStart) >> 2;
  // Print out the basic block information here
  if (flags & DLX_TRACE_INSTRUCTIONS) {
    fprintf (tracefp, "I %x %d\n", basicBlockStart, ninstrs);
  }
  if (flags & DLX_TRACE_MEMORY) {
    for (i = 0; i < naccesses; i++) {
      fprintf (tracefp, "%s r%d %x %x\n", accesses[i].inst, accesses[i].reg,
	       accesses[i].addr,accesses[i].value);
    }
  }
  naccesses = 0;
}
