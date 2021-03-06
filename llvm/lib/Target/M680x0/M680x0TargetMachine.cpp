//===-- M680x0TargetMachine.cpp - M680x0 target machine ---------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains implementation for M680x0 target machine.
///
//===----------------------------------------------------------------------===//

#include "M680x0TargetMachine.h"
#include "M680x0.h"

#include "M680x0Subtarget.h"
#include "M680x0TargetObjectFile.h"

#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/TargetRegistry.h"
#include <memory>

using namespace llvm;

#define DEBUG_TYPE "m680x0"

extern "C" void LLVMInitializeM680x0Target() {
  RegisterTargetMachine<M680x0TargetMachine> X(TheM680x0Target);
}

namespace {

// FIXME #28 this layout is true for M68000 original cpu, other variants will
// affect DL computation
std::string computeDataLayout(const Triple &TT, StringRef CPU,
                              const TargetOptions &Options) {
  std::string Ret = "";
  // M680x0 is Big Endian
  Ret += "E";

  // FIXME #28 how to wire it with the used object format?
  Ret += "-m:e";

  // M680x0 pointers are always 32 bit wide even for 16 bit cpus
  Ret += "-p:32:32";

  // M680x0 requires i8 to align on 2 byte boundry
  Ret += "-i8:8:8-i16:16:16-i32:32:32";

  // FIXME #29 no floats at the moment

  // The registers can hold 8, 16, 32 bits
  Ret += "-n8:16:32";

  // Aggregates are 32 bit aligned and stack is 16 bit aligned
  Ret += "-a:0:32-S16";

  return Ret;
}

Reloc::Model getEffectiveRelocModel(const Triple &TT,
                                    Optional<Reloc::Model> RM) {
  // If not defined we default to static
  if (!RM.hasValue()) {
    return Reloc::Static;
  }

  return *RM;
}

CodeModel::Model getEffectiveCodeModel(Optional<CodeModel::Model> CM,
                                       bool JIT) {
  if (!CM) {
    return CodeModel::Small;
  } else if (CM == CodeModel::Large) {
    llvm_unreachable("Large code model is not supported");
  } else if (CM == CodeModel::Kernel) {
    // FIXME #31 Kernel afaik is small cm plus some weird binding
    llvm_unreachable("Kernel code model is not supported");
  }
  return CM.getValue();
}
} // end anonymous namespace

M680x0TargetMachine::M680x0TargetMachine(const Target &T, const Triple &TT,
                                         StringRef CPU, StringRef FS,
                                         const TargetOptions &Options,
                                         Optional<Reloc::Model> RM,
                                         Optional<CodeModel::Model> CM,
                                         CodeGenOpt::Level OL, bool JIT)
    : LLVMTargetMachine(T, computeDataLayout(TT, CPU, Options), TT, CPU, FS,
                        Options, getEffectiveRelocModel(TT, RM),
                        ::getEffectiveCodeModel(CM, JIT), OL),
      TLOF(std::make_unique<M680x0ELFTargetObjectFile>()),
      Subtarget(TT, CPU, FS, *this) {
  initAsmInfo();
}

M680x0TargetMachine::~M680x0TargetMachine() {}

const M680x0Subtarget *
M680x0TargetMachine::getSubtargetImpl(const Function &F) const {
  Attribute CPUAttr = F.getFnAttribute("target-cpu");
  Attribute FSAttr = F.getFnAttribute("target-features");

  std::string CPU = !CPUAttr.hasAttribute(Attribute::None)
                        ? CPUAttr.getValueAsString().str()
                        : TargetCPU;
  std::string FS = !FSAttr.hasAttribute(Attribute::None)
                       ? FSAttr.getValueAsString().str()
                       : TargetFS;

  auto &I = SubtargetMap[CPU + FS];
  if (!I) {
    // This needs to be done before we create a new subtarget since any
    // creation will depend on the TM and the code generation flags on the
    // function that reside in TargetOptions.
    resetTargetOptions(F);
    I = std::make_unique<M680x0Subtarget>(TargetTriple, CPU, FS, *this);
  }
  return I.get();
}

//===----------------------------------------------------------------------===//
// Pass Pipeline Configuration
//===----------------------------------------------------------------------===//

namespace {
class M680x0PassConfig : public TargetPassConfig {
public:
  M680x0PassConfig(M680x0TargetMachine &TM, PassManagerBase &PM)
      : TargetPassConfig(TM, PM) {}

  M680x0TargetMachine &getM680x0TargetMachine() const {
    return getTM<M680x0TargetMachine>();
  }

  const M680x0Subtarget &getM680x0Subtarget() const {
    return *getM680x0TargetMachine().getSubtargetImpl();
  }

  bool addInstSelector() override;
  void addPreSched2() override;
  void addPreEmitPass() override;
};
} // namespace

TargetPassConfig *M680x0TargetMachine::createPassConfig(PassManagerBase &PM) {
  return new M680x0PassConfig(*this, PM);
}

bool M680x0PassConfig::addInstSelector() {
  // Install an instruction selector.
  addPass(createM680x0ISelDag(getM680x0TargetMachine()));
  addPass(createM680x0GlobalBaseRegPass());
  return false;
}

void M680x0PassConfig::addPreSched2() {
  addPass(createM680x0ExpandPseudoPass());
}

void M680x0PassConfig::addPreEmitPass() {
  addPass(createM680x0CollapseMOVEMPass());
  // addPass(createM680x0ConvertMOVToMOVMPass());
}
