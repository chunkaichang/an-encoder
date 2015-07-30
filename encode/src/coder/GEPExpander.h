#ifndef __GEP_EXPANDER_H__
#define __GEP_EXPANDER_H__

#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Metadata.h"


using namespace llvm;

class ProfiledCoder;

class ExpandGetElementPtr : public BasicBlockPass {
public:
    ExpandGetElementPtr(ProfiledCoder *pc) : BasicBlockPass(ID), PC(pc) {}

    virtual bool runOnBasicBlock(BasicBlock &BB) override;

    Instruction *ExpandGEP(GetElementPtrInst *GEP, DataLayout *DL, Type *PtrType);

    static char ID;

private:
    void FlushOffset(Instruction **Ptr, int64_t *CurrentOffset,
                     Instruction *InsertPt, const DebugLoc &Debug,
                     Type *PtrType, std::vector<Instruction*> &worklist);

private:
    ProfiledCoder *PC;
};

Pass *createGEPExpander(ProfiledCoder *pc);

#endif /* __GEP_EXPANDER_H__ */
