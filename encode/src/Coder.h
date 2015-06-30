#ifndef __CODER_H__
#define __CODER_H__

#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Metadata.h"


using namespace llvm;

struct Coder {
  Coder(Module *m, unsigned a = 1);
  ~Coder();
public:
  Value *createSExt(Value *V, Type *DestTy, Instruction *I);
  Value *createZExt(Value *V, Type *DestTy, Instruction *I);
  Value *createTrunc(Value *V, Type *DestTy, Instruction *I);

  Value *createEncode(Value *V, Instruction *I, bool noA = false);
  Value *createDecode(Value *V, Instruction *I, bool noA = false);
  Value *createAssert(Value *V, Instruction *I);
  
  Value *createLoadAccu(Instruction *I, unsigned i);
  Value *createAssertOnAccu(Instruction *I);
  Value *createAccumulate(Value *V, Instruction *I);

  Constant *getEncBinopFunction(StringRef Name);
public:
  IntegerType *getInt64Type();
  IntegerType *getInt32Type();
  Type *getVoidType();
  int64_t getA();

  Value *createEncRegionEntry(Value *V, Instruction *I);
  Value *createEncRegionExit(Value *V, Type *DestTy, Instruction *I);

  Value *preprocessForEncOp(Value *V, Instruction *I);
  Value *postprocessFromEncOp(Value *V, Type *DestTy, Instruction *I);
  Value *createEncBinop(StringRef Name, ArrayRef<Value*> Args, Instruction *I);

private:
  Module *M;
  ConstantInt *A;
  IntegerType *int64Ty, *int32Ty;
  Type *voidTy;
  Function *Encode, *Decode, *Assert;
  Function *EncodeValue, *DecodeValue;
  //Constant *Accumulate;
  Constant *Accumulate0, *Accumulate1;
public:
  Constant *idr, *ce;
private:
  IRBuilder<> *Builder;
  unsigned toggle;
};
#endif /* __CODER_H__ */
