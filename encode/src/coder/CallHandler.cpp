
#include "CallHandler.h"

#include "UsesVault.h"
#include "ProfiledCoder.h"


char CallHandler::ID = 0;

CallHandler::CallHandler(ProfiledCoder *pc)
: BasicBlockPass(ID), PC(pc) {
	add_with_overflow = Intrinsic::getDeclaration(PC->getModule(),
	                                              Intrinsic::sadd_with_overflow,
	                                              PC->getInt64Type());
}


bool CallHandler::handleCallInst(BasicBlock::iterator &I) {
	assert(I->getOpcode() == Instruction::Call);
	CallInst *CI = dyn_cast<CallInst>(&(*I));
	assert(CI);
	bool modified = false;

	Function *F = CI->getCalledFunction();
	if (F == add_with_overflow
#if (defined(PRINTF_DEBUG) || defined(LOG_ACCU))
	    || F->getName().startswith("printf")
	    || F->getName().startswith("fprintf")
#endif
	    ) {
	      return false;
	}

	// Only operate on functions that are externally defined,
	// i.e. such functions for which only a declaration exists
	// within the current module:
	if (F->isDeclaration()) {
		// Do not operate on the encoding/decoding intrinsics:
		if (F->isIntrinsic() &&
			(F->getIntrinsicID() == Intrinsic::an_encode ||
			 F->getIntrinsicID() == Intrinsic::an_decode ||
		     F->getIntrinsicID() == Intrinsic::an_encode_value ||
		     F->getIntrinsicID() == Intrinsic::an_decode_value ||
		     F->getIntrinsicID() == Intrinsic::an_check  ||
		     F->getIntrinsicID() == Intrinsic::an_signal ||
		     F->getIntrinsicID() == Intrinsic::an_assert_value ||
		     F->getIntrinsicID() == Intrinsic::an_assert ||
		     F->getIntrinsicID() == Intrinsic::an_placeholder ||
		     F->getIntrinsicID() == Intrinsic::x86_movswift)) {
		return false;
		}
	    // Decode the arguments to external function calls:
		for (unsigned i = 0; i < CI->getNumArgOperands(); i++) {
			Value *Op = CI->getArgOperand(i);
	        if (!PC->isInt64Type(Op) && !PC->isPointerType(Op))
	        	continue;
			if (PC->isPointerType(Op) && dyn_cast<GlobalValue>(Op->stripPointerCasts()))
				continue;

	        Op = PC->createDecode(Op, CI);
	        if (Op) {
	        	if (i < F->getFunctionType()->getNumParams()) {
	        		Type *paramTy = F->getFunctionType()->getParamType(i);
	        	    if (paramTy->isIntegerTy() && paramTy != Op->getType())
	        	            Op = PC->createTrunc(Op, paramTy, CI);
	        	}
	        	CI->setArgOperand(i, Op);
	        	modified |= true;
	        }
		}
	    // Encode the return value from the external function call:
		Value *ret = CI;
	    if (PC->isInt64Type(ret) || PC->isPointerType(ret)) {
	    	// Save all current uses of the return value to the vault. These
	        // are the uses that need to be replaced with the encoded return
	        // value. (The 'createEncRegionEntry' method will give rise to
	        // another use of the return value, which must not be replaced.)
	        UsesVault UV(ret->uses());

	        ret = PC->createEncode(ret, std::next(I));
	        if (ret) {
	        	UV.replaceWith(ret);
	        	modified |= true;
	        }
	    }
	} else {
		// Non-external function call: encode pointers to global variables
		// if they are passed as arguments.
	    for (unsigned i = 0; i < CI->getNumArgOperands(); i++) {
	    	Value *Op = CI->getArgOperand(i);
			if (PC->isPointerType(Op) && dyn_cast<GlobalValue>(Op->stripPointerCasts())) {
				Op = PC->createEncode(Op, CI);
				if (Op) {
					CI->setArgOperand(i, Op);
					modified |= true;
				}
			}
	    }
	}

	return modified;
}

bool CallHandler::runOnBasicBlock(BasicBlock &BB) {
  bool modified = false;
  Module *M = BB.getParent()->getParent();
  LLVMContext &ctx = M->getContext();
  Type *int64Ty = Type::getInt64Ty(ctx);

  auto I = BB.begin(), E = BB.end();
  while (I != E) {
    auto N = std::next(I);

    if(I->getOpcode() == Instruction::Call)
    	modified |= handleCallInst(I);

    I = N;
  }

  return modified;
}

Pass *createCallHandler(ProfiledCoder *pc) {
  return new CallHandler(pc);
}
