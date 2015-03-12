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
private:
  Value *createSExt(Value *V, Type *DestTy, Instruction *I);
  Value *createZExt(Value *V, Type *DestTy, Instruction *I);
  Value *createTrunc(Value *V, Type *DestTy, Instruction *I);

  Value *createEncode(Value *V, Instruction *I);
  Value *createDecode(Value *V, Instruction *I);

  Constant *getEncBinopFunction(StringRef Name);
public:
  Value *createEncRegionEntry(Value *V, Instruction *I);
  Value *createEncRegionExit(Value *V, Type *DestTy, Instruction *I);

  Value *preprocessForEncOp(Value *V, Instruction *I);
  Value *postprocessFromEncOp(Value *V, Type *DestTy, Instruction *I);
  Value *createEncBinop(StringRef Name, ArrayRef<Value*> Args, Instruction *I);
private:
  Module *M;
  Constant *A;
  Type *int64Ty;
  Function *Encode, *Decode;
  IRBuilder<> *Builder;
};
#endif /* __CODER_H__ */