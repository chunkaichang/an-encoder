
#include "Coder.h"

using namespace llvm;

Coder::Coder(Module *m, unsigned a) : M(m) {
  LLVMContext &ctx = M->getContext();

  this->int64Ty = Type::getInt64Ty(ctx);
  this->int32Ty = Type::getInt32Ty(ctx);
  this->A = ConstantInt::getSigned(int64Ty, a);
  this->Encode =
    Intrinsic::getDeclaration(M,
                              Intrinsic::an_encode,
                              int64Ty);
  this->Decode =
    Intrinsic::getDeclaration(M,
                              Intrinsic::an_decode,
                              int64Ty);
  Builder = new IRBuilder<>(ctx);
}

Coder::~Coder() {
  delete Builder;
}

IntegerType *Coder::getInt64Type() {
  return this->int64Ty;
}

IntegerType *Coder::getInt32Type() {
  return this->int32Ty;
}

int64_t Coder::getA() {
  return A->getSExtValue();
}

// The "extend" and "truncate" methods are simple wrappers around
// the corresponding methods in the 'IRBuilder'. The advantage of
// the methods here is that they do not create any IR instructions
// if the 'Value' V is already of the desired type.
Value *Coder::createSExt(Value *V, Type *DestTy, Instruction *I) {
  if (V->getType() == DestTy) return V;

  Builder->SetInsertPoint(I);
  return Builder->CreateCast(Instruction::SExt, V, DestTy);
}

Value *Coder::createZExt(Value *V, Type *DestTy, Instruction *I) {
  if (V->getType() == DestTy) return V;

  Builder->SetInsertPoint(I);
  return Builder->CreateCast(Instruction::ZExt, V, DestTy);
}

Value *Coder::createTrunc(Value *V, Type *DestTy, Instruction *I) {
  if (V->getType() == DestTy) return V;

  Builder->SetInsertPoint(I);
  return Builder->CreateCast(Instruction::Trunc, V, DestTy);
}

// NOTE: The "encode" and "decode" methods only handle values that
// are 64bit integers.
Value *Coder::createEncode(Value *V, Instruction *I) {
  if (V->getType() != int64Ty)
    return NULL;

  Builder->SetInsertPoint(I);
  return Builder->CreateCall2(Encode, V, A);
}

Value *Coder::createDecode(Value *V, Instruction *I) {
  if (V->getType() != int64Ty)
    return NULL;

  Builder->SetInsertPoint(I);
  return Builder->CreateCall2(Decode, V, A);
}

// This method should be called on values that enter an encoded region,
// e.g. return values from an external function call:
Value *Coder::createEncRegionEntry(Value *V, Instruction *I) {
  if (!V->getType()->isIntegerTy())
    return NULL;

  Type *origTy = V->getType();
  unsigned w = origTy->getIntegerBitWidth();
  if (w < 64) V = createSExt(V, int64Ty, I);
  V = createEncode(V, I);
  //Handle values of bitwidth < 64 properly:
  return postprocessFromEncOp(V, origTy, I);
}

// This method should be called on calues that leave an encoded region,
// e.g. arguments to external function calls:
Value *Coder::createEncRegionExit(Value *V, Type *DestTy, Instruction *I) {
  if (!DestTy->isIntegerTy())
    return NULL;

  //Handle values of bitwidth < 64 properly:
  V = preprocessForEncOp(V, I);
  assert(V->getType() == int64Ty);
  V = createDecode(V, I);

  unsigned w = DestTy->getIntegerBitWidth();
  if (w < 64) V = createTrunc(V, DestTy, I);
  return V;
}

// The 'preprocessForEncOp' and 'postprocessFromEncOp' essentially handle
// extension/truncation of values to/from 64bit integers. It is asumed that
// the 'Value' argument 'V' is an encoded value. (Since this assumption is
// meaningless for values of type 'i1', such values receive special
// treatment.)
Value *Coder::preprocessForEncOp(Value *V, Instruction *I) {
  if (!V->getType()->isIntegerTy())
    return NULL;

  unsigned w = V->getType()->getIntegerBitWidth();
  if (w < 64) {
    V = createSExt(V, int64Ty, I);
    if (w == 1) V = createEncode(V, I);
  }
  return V;
}

Value *Coder::postprocessFromEncOp(Value *V, Type *DestTy, Instruction *I) {
  if (V->getType() != int64Ty ||
      !DestTy->isIntegerTy())
    return NULL;

  unsigned w = DestTy->getIntegerBitWidth();
  if (w < 64) {
    if (w == 1) V = createDecode(V, I);
    V = createTrunc(V, DestTy, I);
  }
  return V;
}

// Returns the value corresponding to the declaration of a function
// that implements an encoded binary operator:
Constant *Coder::getEncBinopFunction(StringRef Name) {
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
Value *Coder::createEncBinop(StringRef Name, ArrayRef<Value*> Args, Instruction *I) {
  Builder->SetInsertPoint(I);
  Constant *binop = getEncBinopFunction(Name);
  return Builder->CreateCall(binop, Args);
}
