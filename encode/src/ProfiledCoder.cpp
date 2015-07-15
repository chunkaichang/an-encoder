
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Value.h"

#include "Coder.h"
#include "UsesVault.h"
#include "CallHandler.h"
#include "ProfileParser.h"
#include "ProfiledCoder.h"


ProfiledCoder::ProfiledCoder (Module *m, ProfileParser *pp, unsigned a)
: M(m), PP(pp) {
	LLVMContext &ctx = M->getContext();

	int64Ty = Type::getInt64Ty(ctx);
	int32Ty = Type::getInt32Ty(ctx);
	voidTy  = Type::getVoidTy(ctx);
	A = ConstantInt::getSigned(int64Ty, a);

	Encode = Intrinsic::getDeclaration(M,
									   Intrinsic::an_encode,
									   int64Ty);
	Decode = Intrinsic::getDeclaration(M,
									   Intrinsic::an_decode,
									   int64Ty);
	Check = Intrinsic::getDeclaration(M,
		   						      Intrinsic::an_check,
									  int64Ty);
	Assert = Intrinsic::getDeclaration(M,
									   Intrinsic::an_assert,
									   int64Ty);
	Blocker  = Intrinsic::getDeclaration(M,
	                                     Intrinsic::x86_movswift,
	                                     int64Ty);

	FunctionType *exitTy = FunctionType::get(voidTy, int32Ty, false);
	Exit = M->getOrInsertFunction("exit", exitTy);



	Builder = new IRBuilder<>(ctx);

	CH = new CallHandler(this);
}

ProfiledCoder::~ProfiledCoder () {
	delete CH;
	delete Builder;
}

IntegerType *ProfiledCoder::getInt64Type() const {
  return this->int64Ty;
}

IntegerType *ProfiledCoder::getInt32Type() const {
  return this->int32Ty;
}

Type *ProfiledCoder::getVoidType() const {
  return this->voidTy;
}

int64_t ProfiledCoder::getA() const {
  return A->getSExtValue();
}

Module* ProfiledCoder::getModule() const {
  return M;
}

// The "extend" and "truncate" methods are simple wrappers around
// the corresponding methods in the 'IRBuilder'. The advantage of
// the methods here is that they do not create any IR instructions
// if the 'Value' V is already of the desired type.
Value *ProfiledCoder::createSExt(Value *V, Type *DestTy, Instruction *I) {
  if (V->getType() == DestTy) return V;

  Builder->SetInsertPoint(I);
  return Builder->CreateCast(Instruction::SExt, V, DestTy);
}

Value *ProfiledCoder::createZExt(Value *V, Type *DestTy, Instruction *I) {
  if (V->getType() == DestTy) return V;

  Builder->SetInsertPoint(I);
  return Builder->CreateCast(Instruction::ZExt, V, DestTy);
}

Value *ProfiledCoder::createTrunc(Value *V, Type *DestTy, Instruction *I) {
  if (V->getType() == DestTy) return V;

  Builder->SetInsertPoint(I);
  return Builder->CreateCast(Instruction::Trunc, V, DestTy);
}

Value *ProfiledCoder::createEncode(Value *V, Instruction *I) {
	bool pointerTy = isPointerType(V);
	if (!isInt64Type(V) && !pointerTy)
		return V;

	if (pointerTy && !PP->hasProfile(ProfileParser::PointerEncoding))
		return V;

	Builder->SetInsertPoint(I);

	Value *result = pointerTy ? Builder->CreatePtrToInt(V, int64Ty) : V;
	result = Builder->CreateCall2(Encode, result, A);
	result = pointerTy ? Builder->CreateIntToPtr(result, V->getType()) : result;
	return result;
}

Value *ProfiledCoder::createDecode(Value *V, Instruction *I) {
	bool pointerTy = isPointerType(V);
	if (!isInt64Type(V) && !pointerTy)
		return V;

	if (pointerTy && !PP->hasProfile(ProfileParser::PointerEncoding))
		return V;

	Builder->SetInsertPoint(I);
	Value *result = pointerTy ? Builder->CreatePtrToInt(V, int64Ty) : V;
	result = Builder->CreateCall2(Decode, result, A);
	result = pointerTy ? Builder->CreateIntToPtr(result, V->getType()) : result;
	return result;
}

Value *ProfiledCoder::createCheck(Value *V, Instruction *I) {
	bool pointerTy = isPointerType(V);
	if (!isInt64Type(V) && !pointerTy)
		return nullptr;

	if (pointerTy && !PP->hasProfile(ProfileParser::PointerEncoding))
		return nullptr;

	Builder->SetInsertPoint(I);
	Value *result = pointerTy ? Builder->CreatePtrToInt(V, int64Ty) : V;
	result = Builder->CreateCall2(Check, result, A);
	return result;
}

Value *ProfiledCoder::createAssert(Value *V, Instruction *I) {
	bool pointerTy = isPointerType(V);
	if (!isInt64Type(V) && !pointerTy)
		return nullptr;

	if (pointerTy && !PP->hasProfile(ProfileParser::PointerEncoding))
		return nullptr;

	Builder->SetInsertPoint(I);
	Value *result = pointerTy ? Builder->CreatePtrToInt(V, int64Ty) : V;
	result = Builder->CreateCall2(Assert, result, A);
	return result;
}

// Returns the value corresponding to the declaration of a function
// that implements an encoded binary operator:
Constant *ProfiledCoder::getEncBinopFunction(StringRef Name) {
  SmallVector<Type*, 2> formals;
  formals.push_back(int64Ty);
  formals.push_back(int64Ty);

  FunctionType *encBinopTy = FunctionType::get(int64Ty,
                                               formals,
                                               false);
  return M->getOrInsertFunction(Name, encBinopTy);
}

// Returns a function call that implements an encoded binary operator.
// The name of the encoded binary operator is passed in 'Name' and there
// must exist a library function of that name in "anlib.c".
Value *ProfiledCoder::createEncBinop(StringRef Name, ArrayRef<Value*> Args, Instruction *I) {
  Builder->SetInsertPoint(I);
  Constant *binop = getEncBinopFunction(Name);
  return Builder->CreateCall(binop, Args);
}

bool ProfiledCoder::insertCheckBefore(Value *v, const BasicBlock::iterator &I, ProfileParser::Operation op) {
	// Do not check constants:
	if (dyn_cast<ConstantInt>(v)) return false;

	if (PP->hasOperationWithPosition(op, ProfileParser::Before)) {
		createAssert(v, &(*I));
		return true;
	}
	return false;
}

bool ProfiledCoder::insertCheckAfter(Value *v, const BasicBlock::iterator &I, ProfileParser::Operation op) {
	// Do not check constants:
	if (dyn_cast<ConstantInt>(v)) return false;

	if (PP->hasOperationWithPosition(op, ProfileParser::After)) {
		createAssert(v, std::next(I));
		return true;
	}
	return false;
}

bool ProfiledCoder::isInt64Type(Value *v) const {
	return v->getType() == getInt64Type();
}

bool ProfiledCoder::isPointerType(Value *v) const {
	return v->getType()->isPointerTy();
}

bool ProfiledCoder::handleBinop(Instruction *I, ProfileParser::Operation op, std::string name) {
	if (!isInt64Type(I))
		return false;

	Value *arg0 = I->getOperand(0);
	Value *arg1 = I->getOperand(1);

	insertCheckBefore(arg0, I, op);
	insertCheckBefore(arg1, I, op);

	SmallVector<Value*, 2> args;
	args.push_back(arg0);
	args.push_back(arg1);

	Value *Res = createEncBinop(name, args, I);

	insertCheckAfter(Res, I, op);

	I->replaceAllUsesWith(Res);
	I->eraseFromParent();
	return true;
}

bool ProfiledCoder::handleArithmetic(Instruction *I) {
	unsigned opcode = I->getOpcode();
	std::string name;

	switch (opcode) {
	case Instruction::Add: name = "add_enc"; break;
	case Instruction::Sub: name = "sub_enc"; break;
	case Instruction::Mul: name = "mul_enc"; break;
	case Instruction::URem: name = "umod_enc"; break;
	case Instruction::SRem: name = "smod_enc"; break;
	// FIXME: Handle division instructions properly. (Requires
	// looking into the signed-ness issue.)
	case Instruction::UDiv: assert(0); break;
	case Instruction::SDiv: assert(0); break;
	default: assert(0);
	}

	return handleBinop(I, ProfileParser::Arithmetic, name);
}

bool ProfiledCoder::handleBitwise(Instruction *I) {
	unsigned opcode = I->getOpcode();
	std::string name;

	switch (opcode) {
	case Instruction::And: name = "and_enc"; break;
	case Instruction::Or: name = "or_enc"; break;
	case Instruction::Shl: name = "shl_enc"; break;
	case Instruction::LShr: name = "shr_enc"; break;
	case Instruction::AShr: name = "ashr_enc"; break;
	case Instruction::Xor: name = "xor_enc"; break;
	default: assert(0);
	}

	return handleBinop(I, ProfileParser::Bitwise, name);
}

bool ProfiledCoder::handleComparison(Instruction *I) {
	assert(I->getOpcode() == Instruction::ICmp);

	bool modified = false;
	ICmpInst *cmp = dyn_cast<ICmpInst>(I);
    assert(cmp->isIntPredicate());

    modified |= insertCheckBefore(I->getOperand(0), I, ProfileParser::Comparison);
    modified |= insertCheckBefore(I->getOperand(1), I, ProfileParser::Comparison);
    return modified;
}

bool ProfiledCoder::handleAlloca(Instruction *I) {
	assert(I->getOpcode() == Instruction::Alloca);
	if (!PP->hasProfile(ProfileParser::PointerEncoding))
		return false;

	UsesVault UV(I->uses());
	BasicBlock::iterator BI(I);
	Value *enc = createEncode(I, std::next(BI));
	UV.replaceWith(enc);
	return true;
}

bool ProfiledCoder::handleLoad(Instruction *I) {
	assert(I->getOpcode() == Instruction::Load);
	bool modified = false;

	if (PP->hasProfile(ProfileParser::PointerEncoding)) {
		Value *ptr = I->getOperand(0);
		if (!dyn_cast<GlobalValue>(ptr->stripPointerCasts())) {
			insertCheckBefore(ptr, I, ProfileParser::Memory);
			I->setOperand(0, createDecode(ptr, I));
			modified |= true;
		}
	}

    modified |= insertCheckAfter(I, I, ProfileParser::Memory);
    return modified;
}

bool ProfiledCoder::handleStore(Instruction *I) {
	assert(I->getOpcode() == Instruction::Store);
	bool modified = false;

    modified |= insertCheckBefore(I->getOperand(0), I, ProfileParser::Memory);

    if (PP->hasProfile(ProfileParser::PointerEncoding)) {
    	Value *ptr = I->getOperand(1);
    	if (!dyn_cast<GlobalValue>(ptr->stripPointerCasts())) {
    		insertCheckBefore(ptr, I, ProfileParser::Memory);
    		I->setOperand(1, createDecode(ptr, I));
    		modified |= true;
    	}
    }

    return modified;
}

bool ProfiledCoder::handleMemory(Instruction *I) {
	unsigned opcode = I->getOpcode();

	switch(opcode) {
	case Instruction::Alloca: return handleAlloca(I);
	case Instruction::Load: return handleLoad(I);
	case Instruction::Store: return handleStore(I);
	default: assert(0);
	}

	return false;
}

bool ProfiledCoder::handlePtrCast(Instruction *I) {
	// If pointers are encoded, then casts between pointers and
	// integers need no special treatment:
	if (PP->hasProfile(ProfileParser::PointerEncoding))
		return false;

	unsigned opcode = I->getOpcode();
	bool modified = false;

	switch(opcode) {
	case Instruction::PtrToInt: {
		// Save all current uses of the 'ptrtoint' instruction to the
		// vault. These are the uses that need to be replaced with the
		// encoded integer value: (The 'createEncRegionEntry' method
		// will generate another use of the 'ptrtoint' instruction's
		// result, which must not be replaced.)
		UsesVault UV(I->uses());

		BasicBlock::iterator BI(I);
		Value *EncInt = createEncode(I, std::next(BI));

		UV.replaceWith(EncInt);
		modified |= true;
		break;
    }
    case Instruction::IntToPtr: {
      Value *Op = I->getOperand(0);
      Value *DecInt = createDecode(Op, I);
      I->setOperand(0, DecInt);
      modified |= true;
      break;
    }
    default: assert(0);
	}

	return modified;
}

bool ProfiledCoder::handleGEP(Instruction *I) {
	assert(I->getOpcode() == Instruction::GetElementPtr);
	GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(I);
	assert(GEP);

	if (PP->hasProfile(ProfileParser::PointerEncoding)) {
		Value *ptr = GEP->getPointerOperand();
     	 if (!dyn_cast<GlobalValue>(ptr->stripPointerCasts())) {
     		 insertCheckBefore(ptr, I, ProfileParser::GEP);
     		 I->setOperand(0, createDecode(ptr, I));
     	 }
	}

    for (unsigned i = 1; i < I->getNumOperands(); i++) {
    	Value *Op = I->getOperand(i);
       	assert(Op->getType()->isIntegerTy());
       	insertCheckBefore(Op, I, ProfileParser::GEP);

       	Value *NewOp = createDecode(Op, I);
       	NewOp = createTrunc(NewOp, getInt32Type(), I);
       	I->setOperand(i, NewOp);
     }

    // If pointers are encoded, we must encode
    // the result of a 'GEP' instruction:
    Value *res = I;
    if (PP->hasProfile(ProfileParser::PointerEncoding)) {
       UsesVault UV(I->uses());
       BasicBlock::iterator BI(I);
       res = createEncode(I, std::next(BI));
       UV.replaceWith(res);
    }

    BasicBlock::iterator BI(dyn_cast<Instruction>(res));
    insertCheckAfter(res, BI, ProfileParser::GEP);
    return true;
}

bool ProfiledCoder::handleCall(Instruction *I) {
	BasicBlock::iterator BI(I);
	return CH->handleCallInst(BI);
}

bool ProfiledCoder::handleExt(Instruction *I) {
	unsigned opcode = I->getOpcode();
	assert(opcode == Instruction::SExt || opcode == Instruction::ZExt);

	UsesVault UV(I->uses());
	BasicBlock::iterator BI(I);
	Value *enc = createEncode(I, std::next(BI));
	if (enc) {
		UV.replaceWith(enc);
		return true;
	}
	return false;
}

bool ProfiledCoder::handleTrunc(Instruction *I) {
	assert(I->getOpcode() == Instruction::Trunc);

	Value *dec = createDecode(I, I);
	if (dec) {
		I->setOperand(0, dec);
		return true;
	}
	return false;
}

Instruction *ProfiledCoder::createExitAtEnd(BasicBlock *BB) {
	Builder->SetInsertPoint(BB);

	ConstantInt *Two = ConstantInt::getSigned(int32Ty, 2);
	Builder->CreateCall(Exit, Two);
	return Builder->CreateUnreachable();
}

Instruction *ProfiledCoder::createCmpZeroAfter(Instruction *I) {
	assert(isInt64Type(I));
	BasicBlock::iterator BI(I);
	Builder->SetInsertPoint(std::next(BI));

	ConstantInt *Zero = ConstantInt::getSigned(int64Ty, 0);
    Value *result = Builder->CreateICmpEQ(I, Zero);
    return dyn_cast<Instruction>(result);
}

BasicBlock *ProfiledCoder::createTrapBlockOnFalse(Instruction *I) {
	assert(I->getOpcode() == Instruction::ICmp);
	BasicBlock *BB = I->getParent();
	BasicBlock::iterator BI(I);
	BasicBlock *splitBB = BB->splitBasicBlock(std::next(BI)),
    	       *trapBB  = BasicBlock::Create(BB->getContext(),
    	                                     "",
    	                                     BB->getParent());
	// Terminating instruction will be a jump to 'splitBB':
    Instruction *term = BB->getTerminator();
    Builder->SetInsertPoint(term);
    Builder->CreateCondBr(I, splitBB, trapBB);
    // The original terminator (inserted by the 'splitBasicBlock'
    // method) is no longer needed):
    term->eraseFromParent();
    return trapBB;
}

static bool isIntrinsic(Instruction *I, Intrinsic::ID id) {
	CallInst *ci = dyn_cast<CallInst>(I);
	Function *callee = ci ? ci->getCalledFunction() : nullptr;

	if (callee && callee->isIntrinsic()) {
		if (callee->getIntrinsicID() == id)
			return true;
	}
	return false;

}

Instruction *ProfiledCoder::expandEncode(Instruction *I) {
	assert(isIntrinsic(I, Intrinsic::an_encode));

    Builder->SetInsertPoint(I);
    CallInst *ci = dyn_cast<CallInst>(I);
    Value *result = Builder->CreateMul(ci->getArgOperand(0), ci->getArgOperand(1));
    I->replaceAllUsesWith(result);
    I->eraseFromParent();
    return dyn_cast<Instruction>(result);
}

Instruction *ProfiledCoder::expandDecode(Instruction *I) {
	assert(isIntrinsic(I, Intrinsic::an_decode));
	BasicBlock *BB = I->getParent();

    Builder->SetInsertPoint(I);
    CallInst *ci = dyn_cast<CallInst>(I);
    Value *x = ci->getArgOperand(0);
    Value *result = Builder->CreateSDiv(x, ci->getArgOperand(1));

    if (PP->hasProfile(ProfileParser::CheckAfterDecode)) {
    	Value *mul = Builder->CreateMul(result, this->A);

    	if (PP->hasProfile(ProfileParser::PinChecks))
    		x = Builder->CreateCall(Blocker, x);

        Value *rem = Builder->CreateSub(x, mul);
    	Instruction *cmp = createCmpZeroAfter(dyn_cast<Instruction>(rem));
    	BasicBlock *trap = createTrapBlockOnFalse(cmp);
    	createExitAtEnd(trap);
    }

    I->replaceAllUsesWith(result);
    I->eraseFromParent();
    return dyn_cast<Instruction>(result);
}

Instruction *ProfiledCoder::expandCheck(Instruction *I) {
	assert(isIntrinsic(I, Intrinsic::an_check));

    Builder->SetInsertPoint(I);
    CallInst *ci = dyn_cast<CallInst>(I);
    Value *x = ci->getArgOperand(0);

    if (PP->hasProfile(ProfileParser::PinChecks))
    	x = Builder->CreateCall(Blocker, x);

    Value *result = Builder->CreateSRem(x, this->A);
    I->replaceAllUsesWith(result);
    I->eraseFromParent();
    return dyn_cast<Instruction>(result);
}

Instruction *ProfiledCoder::expandAssert(Instruction *I) {
	assert(isIntrinsic(I, Intrinsic::an_assert));

	Builder->SetInsertPoint(I);
	CallInst *ci = dyn_cast<CallInst>(I);
	Value *x = ci->getArgOperand(0);

	if (PP->hasProfile(ProfileParser::PinChecks))
		x = Builder->CreateCall(Blocker, x);

	Value *rem = Builder->CreateSRem(x, this->A);
    Instruction *cmp = createCmpZeroAfter(dyn_cast<Instruction>(rem));
    BasicBlock *trap = createTrapBlockOnFalse(cmp);
    Instruction *result = createExitAtEnd(trap);

    I->eraseFromParent();
    return result;
}
