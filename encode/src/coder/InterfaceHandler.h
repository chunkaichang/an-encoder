#ifndef __INTERFACE_HANDLER_H__
#define __INTERFACE_HANDLER_H__

#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Constants.h"


using namespace llvm;


class ProfiledCoder;


struct InterfaceHandler: public FunctionPass {
  InterfaceHandler(ProfiledCoder *pc) : FunctionPass(ID), PC(pc) {}

  bool runOnFunction(Function &F) override;

  bool handleFunction(Function &F);

  static char ID;

private:
  ProfiledCoder *PC;
};

Pass *createInterfaceHandler(ProfiledCoder *pc);

#endif /* __INTERFACE_HANDLER_H__ */
