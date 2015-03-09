/* The 'LinkagePass' can be used to set the linkages of all functions
 * in an LLVM module to a given value.
 */

#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"

using namespace llvm;

namespace {
  struct LinkagePass : public ModulePass {
    LinkagePass(GlobalValue::LinkageTypes linkage)
      : ModulePass(ID), Linkage(linkage) {}

    LinkagePass() : LinkagePass(GlobalValue::ExternalLinkage) {}

    bool runOnModule(Module &M) override;

    static char ID;
  private:
    GlobalValue::LinkageTypes Linkage;
  };
}

char LinkagePass::ID = 0;

bool LinkagePass::runOnModule(Module &M) {
  LLVMContext &ctx = M.getContext();

  Module::iterator I = M.begin(), E = M.end();
  while (I != E) {
    auto Next = std::next(I);

    Function *F = &(*I);
    if (!F->isDeclaration())
      F->setLinkage(Linkage);

    I = Next;
  }

  return !M.empty();
}

static RegisterPass<LinkagePass> X("LinkagePass",
                                   "",
                                   false,
                                   false);

Pass *createLinkagePass(GlobalValue::LinkageTypes l) {
  return new LinkagePass(l);
}
