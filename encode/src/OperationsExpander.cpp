
#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Metadata.h"

#include "llvm/Support/raw_ostream.h"

#include <iostream>

#include "ProfiledCoder.h"

using namespace llvm;

namespace {
  struct OperationsExpander : public FunctionPass {
    OperationsExpander(ProfiledCoder *pc) : FunctionPass(ID), PC(pc) {}

    bool runOnFunction(Function &F) override;

    static char ID;

  private:
    ProfiledCoder *PC;
  };
}

char OperationsExpander::ID = 0;

bool OperationsExpander::runOnFunction(Function &F) {
	LLVMContext &ctx = F.getContext();
	Module *M = F.getParent();

	bool m = true, modified = false;
	// Loop until no more modifications are made to the function:
	// ('m' indicates if the previous pass has made modifications,
	// hence 'm' start out true. Looping is necessary since expanding
	// the assert and signal intrinsics creates new basic blocks.)
	while (m) {
		m = false;
		std::vector<BasicBlock*> BBWorkList;
		for (auto BI = F.begin(), BE = F.end(); BI != BE; BI++) {
			BBWorkList.push_back(&(*BI));
		}

		auto BI = BBWorkList.begin(), BE = BBWorkList.end();
		for (; BI != BE; BI++) {
			std::vector<Instruction*> IWorkList;
			for (auto I = (*BI)->begin(), E = (*BI)->end(); I != E; I++) {
				IWorkList.push_back(&(*I));
			}

			auto I = IWorkList.begin(), E = IWorkList.end();
			for (; I != E; I++) {
				Instruction *i = *I;
				// We are only interested in call instructions, in particular
				// such instructions which call the "AN coding" intrinsics:
				CallInst *ci = dyn_cast<CallInst>(i);
				Function *callee = ci ? ci->getCalledFunction() : nullptr;
				if (!callee || !callee->isIntrinsic())
					continue;

				switch (callee->getIntrinsicID()) {
				default: break;
				case Intrinsic::an_encode: {
					PC->expandEncode(i);
					m |= true;
					break;
				}
				case Intrinsic::an_decode: {
					PC->expandDecode(i);
					m |= true;
					break;
				}
				case Intrinsic::an_encode_value:
				case Intrinsic::an_decode_value: {
					Value *x = PC->createSExt(ci->getArgOperand(0), PC->getInt64Type(), ci);
					Value *result = (callee->getIntrinsicID() == Intrinsic::an_encode_value)
									? PC->createEncode(x, ci)
									: PC->createDecode(x, ci);
					ci->replaceAllUsesWith(result);
					ci->eraseFromParent();
					m = true;
					break;
				}
				case Intrinsic::an_check: {
					PC->expandCheck(i);
					m |= true;
					break;
				}
				case Intrinsic::an_assert: {
					PC->expandAssert(i);
					m |= true;
					break;
				}
				}
				modified |= m;
			}
		}
	}

	return modified;
}

Pass *createOperationsExpander(ProfiledCoder *pc) {
  return new OperationsExpander(pc);
}
