//===-- M680x0Subtarget.h - Define Subtarget for the M680x0 -----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file declares the M680x0 specific subclass of TargetSubtargetInfo.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CPU0_M680X0SUBTARGET_H
#define LLVM_LIB_TARGET_CPU0_M680X0SUBTARGET_H

#include "M680x0FrameLowering.h"
#include "M680x0ISelLowering.h"
#include "M680x0InstrInfo.h"

#include "llvm/CodeGen/SelectionDAGTargetInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/MC/MCInstrItineraries.h"
#include "llvm/Support/Alignment.h"

#include <string>

#define GET_SUBTARGETINFO_HEADER
#include "M680x0GenSubtargetInfo.inc"

extern bool M680x0ReserveGP;
extern bool M680x0NoCpload;

namespace llvm {
class StringRef;

class M680x0TargetMachine;

class M680x0Subtarget : public M680x0GenSubtargetInfo {
  virtual void anchor();

protected:
  // These define which ISA is supported. Since each Motorola M680x0 ISA is
  // built on top of the previous one whenever an ISA is selected the previous
  // selected as well.
  bool IsM68000 = false;
  bool IsM68010 = false;
  bool IsM68020 = false;
  bool IsM68030 = false;
  bool IsM68040 = false;

  InstrItineraryData InstrItins;

  /// Small section is used.
  bool UseSmallSection = true;

  const M680x0TargetMachine &TM;

  SelectionDAGTargetInfo TSInfo;
  M680x0InstrInfo InstrInfo;
  M680x0FrameLowering FrameLowering;
  M680x0TargetLowering TLInfo;

  /// The minimum alignment known to hold of the stack frame on
  /// entry to the function and which must be maintained by every function.
  unsigned stackAlignment = 8;

  Triple TargetTriple;

public:
  /// This constructor initializes the data members to match that
  /// of the specified triple.
  M680x0Subtarget(const Triple &TT, StringRef CPU, StringRef FS,
                  const M680x0TargetMachine &_TM);

  /// Parses features string setting specified subtarget options.  Definition
  /// of function is auto generated by tblgen.
  void ParseSubtargetFeatures(StringRef CPU, StringRef TuneCPU, StringRef FS);

  bool isM68000() const { return IsM68000; }
  bool isM68010() const { return IsM68010; }
  bool isM68020() const { return IsM68020; }
  bool isM68030() const { return IsM68030; }
  bool isM68040() const { return IsM68040; }

  bool useSmallSection() const { return UseSmallSection; }

  bool abiUsesSoftFloat() const;

  const Triple &getTargetTriple() const { return TargetTriple; }

  bool isTargetELF() const { return TargetTriple.isOSBinFormatELF(); }

  /// Return true if the subtarget allows calls to immediate address.
  bool isLegalToCallImmediateAddr() const;

  bool isPositionIndependent() const;

  /// Classify a global variable reference for the current subtarget according
  /// to how we should reference it in a non-pcrel context.
  unsigned char classifyLocalReference(const GlobalValue *GV) const;

  /// Classify a global variable reference for the current subtarget according
  /// to how we should reference it in a non-pcrel context.
  unsigned char classifyGlobalReference(const GlobalValue *GV,
                                        const Module &M) const;
  unsigned char classifyGlobalReference(const GlobalValue *GV) const;

  /// Classify a external variable reference for the current subtarget according
  /// to how we should reference it in a non-pcrel context.
  unsigned char classifyExternalReference(const Module &M) const;

  /// Classify a global function reference for the current subtarget.
  unsigned char classifyGlobalFunctionReference(const GlobalValue *GV,
                                                const Module &M) const;
  unsigned char classifyGlobalFunctionReference(const GlobalValue *GV) const;

  /// Classify a blockaddress reference for the current subtarget according to
  /// how we should reference it in a non-pcrel context.
  unsigned char classifyBlockAddressReference() const;

  unsigned getJumpTableEncoding() const;

  /// TODO this must be controlled by options like -malign-int and -mshort
  Align getStackAlignment() const { return Align(stackAlignment); }

  /// getSlotSize - Stack slot size in bytes.
  unsigned getSlotSize() const { return 4; }

  M680x0Subtarget &
  initializeSubtargetDependencies(StringRef CPU, Triple TT, StringRef FS,
                                  const M680x0TargetMachine &TM);

  const SelectionDAGTargetInfo *getSelectionDAGInfo() const override {
    return &TSInfo;
  }

  const M680x0InstrInfo *getInstrInfo() const override { return &InstrInfo; }

  const M680x0FrameLowering *getFrameLowering() const override {
    return &FrameLowering;
  }

  const M680x0RegisterInfo *getRegisterInfo() const override {
    return &InstrInfo.getRegisterInfo();
  }

  const M680x0TargetLowering *getTargetLowering() const override {
    return &TLInfo;
  }

  const InstrItineraryData *getInstrItineraryData() const override {
    return &InstrItins;
  }
};
} // namespace llvm

#endif
