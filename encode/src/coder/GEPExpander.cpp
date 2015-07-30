//
// Taken from Google's NaCl project:

// https://chromium.googlesource.com/native_client/pnacl-llvm/+/mseaborn/merge-34-squashed/lib/Transforms/NaCl/ExpandGetElementPtr.cpp
//
//===- ExpandGetElementPtr.cpp - Expand GetElementPtr into arithmetic------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This pass expands out GetElementPtr instructions into ptrtoint,
// inttoptr and arithmetic instructions.
//
// This simplifies the language so that the PNaCl translator does not
// need to handle GetElementPtr and struct types as part of a stable
// wire format for PNaCl.
//
// Note that we drop the "inbounds" attribute of GetElementPtr.
//
//===----------------------------------------------------------------------===//

#include "GEPExpander.h"
#include "ProfiledCoder.h"

#include <vector>


char ExpandGetElementPtr::ID = 0;

static Value *CastToPtrSize(Value *Val, Instruction *InsertPt,
                            const DebugLoc &Debug, Type *PtrType) {
    unsigned ValSize = Val->getType()->getIntegerBitWidth();
    unsigned PtrSize = PtrType->getIntegerBitWidth();
    if (ValSize == PtrSize)
        return Val;
    Instruction *Inst;
    if (ValSize > PtrSize) {
        Inst = new TruncInst(Val, PtrType, "gep_trunc", InsertPt);
    } else {
        // GEP indexes must be sign-extended.
        Inst = new SExtInst(Val, PtrType, "gep_sext", InsertPt);
    }
    Inst->setDebugLoc(Debug);
    return Inst;
}
void ExpandGetElementPtr::FlushOffset(Instruction **Ptr, int64_t *CurrentOffset,
                        Instruction *InsertPt, const DebugLoc &Debug,
                        Type *PtrType, std::vector<Instruction*> &worklist) {
    if (*CurrentOffset) {
        *Ptr = BinaryOperator::Create(Instruction::Add, *Ptr,
                                      ConstantInt::get(PtrType, *CurrentOffset),
                                      "gep", InsertPt);
        worklist.push_back(*Ptr);
        (*Ptr)->setDebugLoc(Debug);
        *CurrentOffset = 0;
    }
}
Instruction *ExpandGetElementPtr::ExpandGEP(GetElementPtrInst *GEP, DataLayout *DL, Type *PtrType) {
  std::vector<Instruction*> worklist;

    const DebugLoc &Debug = GEP->getDebugLoc();
    Instruction *Ptr = new PtrToIntInst(GEP->getPointerOperand(), PtrType,
                                        "gep_int", GEP);
		if (dyn_cast<GlobalValue>(GEP->getPointerOperand()->stripPointerCasts()))
			Ptr = dyn_cast<Instruction>(PC->createEncode(Ptr, GEP));

    Ptr->setDebugLoc(Debug);
    Type *CurrentTy = GEP->getPointerOperand()->getType();
    // We do some limited constant folding ourselves.  An alternative
    // would be to generate verbose, unfolded output (e.g. multiple
    // adds; adds of zero constants) and use a later pass such as
    // "-instcombine" to clean that up.  However, "-instcombine" can
    // reintroduce GetElementPtr instructions.
    int64_t CurrentOffset = 0;
    for (GetElementPtrInst::op_iterator Op = GEP->op_begin() + 1;
         Op != GEP->op_end();
         ++Op) {
        Value *Index = *Op;
        if (StructType *StTy = dyn_cast<StructType>(CurrentTy)) {
            uint64_t Field = cast<ConstantInt>(Op)->getZExtValue();
            CurrentTy = StTy->getElementType(Field);
            CurrentOffset += DL->getStructLayout(StTy)->getElementOffset(Field) * PC->getA();
        } else {
            CurrentTy = cast<SequentialType>(CurrentTy)->getElementType();
            uint64_t ElementSize = DL->getTypeAllocSize(CurrentTy) * PC->getA();

            FlushOffset(&Ptr, &CurrentOffset, GEP, Debug, PtrType, worklist);
            Index = CastToPtrSize(Index, GEP, Debug, PtrType);

            Index = BinaryOperator::Create(Instruction::Mul, Index,
                                           ConstantInt::get(PtrType, ElementSize),
                                           "gep_array", GEP);
            worklist.push_back(dyn_cast<Instruction>(Index));

            Ptr = BinaryOperator::Create(Instruction::Add, Ptr,
                                         Index, "gep", GEP);
            worklist.push_back(Ptr);
            Ptr->setDebugLoc(Debug);
        }
    }
    FlushOffset(&Ptr, &CurrentOffset, GEP, Debug, PtrType, worklist);
    assert(CurrentTy == GEP->getType()->getElementType());
    Instruction *Result = new IntToPtrInst(Ptr, GEP->getType(), "", GEP);
    Result->setDebugLoc(Debug);
    Result->takeName(GEP);
    GEP->replaceAllUsesWith(Result);
    GEP->eraseFromParent();

    for (auto i = worklist.begin(); i != worklist.end(); i++)
      PC->handleArithmetic(*i);

    return Result;
}

bool ExpandGetElementPtr::runOnBasicBlock(BasicBlock &BB) {
    bool Modified = false;
    DataLayout DL(BB.getParent()->getParent());
    Type *PtrType = DL.getIntPtrType(BB.getContext());
    for (BasicBlock::InstListType::iterator Iter = BB.begin();
         Iter != BB.end(); ) {
        Instruction *Inst = Iter++;
        if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(Inst)) {
            Modified = true;
            ExpandGEP(GEP, &DL, PtrType);
        }
    }
    return Modified;
}

BasicBlockPass *createExpandGetElementPtrPass(ProfiledCoder *pc) {
    return new ExpandGetElementPtr(pc);
}
