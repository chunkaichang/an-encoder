
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Metadata.h"

#include "coder/ProfiledCoder.h"

using namespace llvm;

namespace {
  struct OperationsExpander : public ModulePass {
    OperationsExpander(ProfiledCoder *pc) : ModulePass(ID), PC(pc) {}

    bool runOnModule(Module &M) override;

    bool handleFunction(Function &F);

    static char ID;

  private:
    ProfiledCoder *PC;
  };
}

char OperationsExpander::ID = 0;

bool OperationsExpander::runOnModule(Module &M) {
  bool modified = false;

  PC->preExpansion(&M);

  for (auto F = M.begin(); F != M.end(); F++) {
      modified |= handleFunction(*F);
  }

  PC->postExpansion(&M);

  return modified;
}

static void populateWorkList(std::vector<BasicBlock::iterator> &worklist, Function &F) {
	for (auto BI = F.begin(), BE = F.end(); BI != BE; BI++) {
		for (auto I = BI->begin(), E = BI->end(); I != E; I++) {
			worklist.push_back(I);
		}
	}
}

bool OperationsExpander::handleFunction(Function &F) {
	LLVMContext &ctx = F.getContext();
	Module *M = F.getParent();

	bool m = true, modified = false;
	// Loop until no more modifications are made to the function:
	// ('m' indicates if the previous pass has made modifications,
	// hence 'm' start out true. Looping is necessary since expanding
	// the assert and signal intrinsics creates new basic blocks.)
	while (m) {
		m = false;
		std::vector<BasicBlock::iterator> worklist;
		populateWorkList(worklist, F);

		for (auto it = worklist.begin(); it != worklist.end(); it++) {
			BasicBlock::iterator &i = *it;
			// We are only interested in call instructions, in particular
			// such instructions which call the "AN coding" intrinsics:
			CallInst *ci = dyn_cast<CallInst>(i);
			Function *callee = ci ? ci->getCalledFunction() : nullptr;
			if (!callee)
				continue;

			llvm::StringRef name = callee->getName();
			if (name.equals("an_encode")) {
				PC->expandEncode(i);
				m |= true;
				break;
			} else if (name.equals("an_decode")) {
				PC->expandDecode(i);
				m |= true;
				break;
			} else if (name.equals("an_encode_value") || name.equals("an_decode_value")) {
				Value *x = PC->createSExt(ci->getArgOperand(0), PC->getInt64Type(), ci);
				Value *result = name.equals("an_encode_value")
								? PC->createEncode(x, ci)
								: PC->createDecode(x, ci);
				ci->replaceAllUsesWith(result);
				ci->eraseFromParent();
				m = true;
				break;
			} else if (name.equals("an_check")) {
				PC->expandCheck(i);
				m |= true;
				break;
			} else if (name.equals("an_assert")) {
				PC->expandAssert(i);
				m |= true;
				break;
			} else if (name.equals("an_assert_value")) {
			  Value *x = PC->createSExt(ci->getArgOperand(0), PC->getInt64Type(), ci);
			  Value *result = PC->createAssert(x, ci);
        ci->replaceAllUsesWith(result);
        ci->eraseFromParent();
        m = true;
        break;
      } else if (name.equals("an_exit_on_false")) {
			  PC->expandExitOnFalse(i);
			  m |= true;
			  break;
			} else if (name.equals("an_move")) {
        PC->expandBlocker(i);
        m |= true;
        break;
      }

			modified |= m;
		}
	}

	return modified;
}

Pass *createOperationsExpander(ProfiledCoder *pc) {
  return new OperationsExpander(pc);
}
