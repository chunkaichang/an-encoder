#ifndef __PROFILED_CODER_H__
#define __PROFILED_CODER_H__

#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Metadata.h"

#include "../parser/Profile.h"


using namespace llvm;


class CallHandler;
class ExpandGetElementPtr;
class InterfaceHandler;

class ProfiledCoder {
public:
	ProfiledCoder (Module *m, EncodingProfile *pp, unsigned a=1);
	~ProfiledCoder ();

public:
	Value *createSExt(Value *V, Type *DestTy, Instruction *I);
	Value *createZExt(Value *V, Type *DestTy, Instruction *I);
	Value *createTrunc(Value *V, Type *DestTy, Instruction *I);

public:
	Value *createEncode(Value *V, Instruction *I, bool force=false);
	Value *createDecode(Value *V, Instruction *I, bool force=false);
	Value *createCheck(Value *V, Instruction *I);
	Value *createAssert(Value *V, Instruction *I);
	Value *createExitOnFalse(Value *V, Instruction *I);

private:
	Constant *getEncBinopFunction(StringRef Name);
	Value *createEncBinop(StringRef Name, ArrayRef<Value*> Args, Instruction *I);

public:
	IntegerType *getInt64Type() const;
	IntegerType *getInt32Type() const;
	Type *getVoidType() const;
	int64_t getA() const;
	Module* getModule() const;

public:
	bool isInt64Type(Value *v) const;
	bool isPointerType(Value *v) const;

private:
	bool insertCheckBefore(Value *v, const BasicBlock::iterator &I, EncodingProfile::Operation op);
	bool insertCheckAfter(Value *v, const BasicBlock::iterator &I, EncodingProfile::Operation op);

public:
	bool preEncoding(Module *M);
	bool postEncoding(Module *M);

private:
	bool handleBinop(Instruction *I, EncodingProfile::Operation op, std::string name);
	bool handleAlloca(Instruction *I);
	bool handleLoad(Instruction *I);
	bool handleStore(Instruction *I);

public:
	bool handleArithmetic(Instruction *I);
	bool handleBitwise(Instruction *I);
	bool handleComparison(Instruction *I);
	bool handleMemory(Instruction *I);
	bool handlePtrCast(Instruction *I);
	bool handleGEP(Instruction *I);
	bool handleCall(Instruction *I);
	bool handleExt(Instruction *I);
	bool handleTrunc(Instruction *I);

private:
	void createLoadCmpAssert(Value *ptr, Value *orig, const BasicBlock::iterator &I);
	Instruction *createExitAtEnd(BasicBlock *BB);
	Value *createCmpZero(Value *v, BasicBlock::iterator &I);
	BasicBlock *createTrapBlockOnFalse(Value *v, BasicBlock::iterator &I);

public:
	Value *expandEncode(BasicBlock::iterator &I);
	Value *expandDecode(BasicBlock::iterator &I);
	Value *expandCheck(BasicBlock::iterator &I);
	Instruction *expandAssert(BasicBlock::iterator &I);
	Instruction *expandExitOnFalse(BasicBlock::iterator &I);

private:
	Module *M;

	ConstantInt *A;
	IntegerType *int64Ty, *int32Ty;
	Type *voidTy;
	Function *Encode, *Decode, *Check, *Assert, *Blocker, *ExitOnFalse;
	Constant *Exit;

	EncodingProfile *PP;

	IRBuilder<> *Builder;

	CallHandler *CH;
	ExpandGetElementPtr *GE;
	InterfaceHandler *IH;
};

#endif /* __PROFILED_CODER_H__ */
