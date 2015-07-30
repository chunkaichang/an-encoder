// adapted from llc.cpp


#include "llvm/ADT/Triple.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/PassManager.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Bitcode/BitcodeWriterPass.h"
#include "llvm/Linker/Linker.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/LinkAllPasses.h"

#include <iostream>
#include <fstream>

#include "Coder.h"
#include "coder/ProfiledCoder.h"

#include "parser/Profile.h"
#include "parser/Parser.h"
#include "parser/ProfileLexer.h"


using namespace llvm;

// Passes required for correct encoding:
Pass *createGlobalsEncoder(ProfiledCoder*);
Pass *createConstantsEncoder(ProfiledCoder*);
Pass *createOperationsEncoder(ProfiledCoder*);
Pass *createOperationsExpander(ProfiledCoder*);

// Passes for insertion of checks:
Pass *createBBCheckInserter(Coder*);
Pass *createFunCheckInserter(Coder*);

// Utility passes:
Pass *createLinkagePass(GlobalValue::LinkageTypes);

// (Simple) optimization passes:
Pass *createAccuPromoter(Coder *);

// Command line arguments:
static cl::opt<std::string>
InputFilename(cl::Positional, cl::desc("<input bitcode>"), cl::init("-"));

static cl::opt<std::string>
OutputFilename("o", cl::desc("Output filename"), cl::value_desc("filename"));

static cl::opt<std::string>
ProfileFilename("p", cl::desc("Path to file with encoding profile"), cl::value_desc("filename"));

static cl::opt<bool>
ExpandOnly("expand-only", cl::init(false), cl::desc("Only expand en-/decoding instrinsics"));

static cl::opt<bool>
NoOpts("no-opts", cl::init(false), cl::desc("Disable all optimizations"));

static cl::opt<bool>
NoInlining("no-inlining", cl::init(false), cl::desc("Disable inlining"));

static cl::opt<bool>
NoVerifying("no-verifying", cl::init(false), cl::desc("Disable LLVM verifier"));


static int processModule(char **, LLVMContext &);

static tool_output_file *GetOutputStream() {
  // Open the file.
  std::string error;
  sys::fs::OpenFlags OpenFlags = sys::fs::F_None;
  tool_output_file *FDOut = new tool_output_file(OutputFilename.c_str(), error,
                                                 OpenFlags);
  if (!error.empty()) {
    errs() << error << '\n';
    delete FDOut;
    return nullptr;
  }

  return FDOut;
}

// main - Entry point for the llc compiler.
//
int main(int argc, char **argv) {
  sys::PrintStackTraceOnErrorSignal();
  PrettyStackTraceProgram X(argc, argv);

  // Enable debug stream buffering.
  EnableDebugBuffering = true;

  LLVMContext &Context = getGlobalContext();

  cl::ParseCommandLineOptions(argc, argv, "AN encoder based on llvm\n");

  if (int RetVal = processModule(argv, Context))
    return RetVal;
  return 0;
}

static inline Module* openFileAsModule(const std::string &path,
                                       SMDiagnostic &Err,
                                       LLVMContext &Context) {
  return ParseIRFile(path, Err, Context);
}

static inline bool linkModules(Module *dst,
                               Module *src,
                               std::string *Error) {
  const std::string &TheTriple = dst->getTargetTriple();
  const DataLayout  *TheLayout = dst->getDataLayout();
  src->setTargetTriple(TheTriple);
  src->setDataLayout(TheLayout);

  return Linker::LinkModules(dst,
                             src,
                             Linker::DestroySource,
                             Error);
}

const std::string globalCodeName("A");
const uint64_t    globalCodeValue = 58659;
                                    // '((1 << 19) - 1) == 524287'
                                    //12; //1 << 4;

static int processModule(char **argv, LLVMContext &Context) {
  SMDiagnostic Err;
  std::unique_ptr<Module> M;
  Module *mod, *library;
  PassManager PM, linkagePM;
  std::string linkError;

  mod = openFileAsModule(InputFilename, Err, Context);
  if (mod == nullptr) {
    Err.print(argv[0], errs());
    return 1;
  }
  Coder C(mod, globalCodeValue);

  std::string name(ProfileFilename);
  std::ifstream ifs(name.c_str());
  if (!ifs.good())
	  return 1;

  ProfileLexer Lex(ifs);
  Parser P(Lex);
  EncodingProfile PP = P.parse();
  if (P.error())
    return 1;
  std::cout << "Encoding Profile:\n";
  PP.print();
  std::cout << "-----------------\n";
  ProfiledCoder PC(mod, &PP, globalCodeValue);

  // Figure out where we are going to send the output.
  std::unique_ptr<tool_output_file> Out(GetOutputStream());
  if (!Out) return 1;

  std::string libraryPath(""), idrPath(""), binaryPath(argv[0]);
  int pos = binaryPath.rfind('/');
  if (pos != std::string::npos)
    libraryPath = binaryPath.substr(0, pos+1);
	idrPath = libraryPath;
  // If no '/' is found in 'argv[0]', then this binary is executed from the directory
  // it is in, i.e. the current working directory is this binary's directory. Since the
  // library is expected to reside in the same directory, its relative paths is the same
  // as its file name:
  libraryPath += "anlib.bc";

  // By applying this linkage to library functions (rather than 'ExternalLinkage',
  // which is the default) we achieve that after linking no further copies of the
  // original library functions remain in the module 'mod': (Functions are inlined
  // during linking.)
  linkagePM.add(createLinkagePass(GlobalValue::LinkOnceODRLinkage));

  if (!ExpandOnly) {
    library = openFileAsModule(libraryPath, Err, Context);
    if (library == nullptr) {
      Err.print(argv[0], errs());
      return 1;
    }

    PassManager codePM, postLinkPM;
    if (!NoVerifying) codePM.add(createVerifierPass());

    // Set the global variable "A" from the "anlib.c" source file (which has just been
    // linked in) to the value used for encoding:
    {
      GlobalVariable *globalCode
        = library->getGlobalVariable(globalCodeName,
                                     true);
      Type *globalCodeType = globalCode->getInitializer()->getType();
      globalCode->setInitializer(ConstantInt::get(globalCodeType,
                                                  globalCodeValue));
    }

    codePM.add(createConstantsEncoder(&PC));
    codePM.add(createGlobalsEncoder(&PC));
    codePM.add(createOperationsEncoder(&PC));

    codePM.run(*mod);

    linkagePM.run(*library);
    if (linkModules(mod, library, &linkError)) {
      std::cerr << linkError;
      return 1;
    }

    // Calls to external functions may take constants as arguments. After
    // the 'CallHandler' has run, what used to be a constant previously may
    // no longer be constant - at least not until we have run the
    // 'ConstantPropagationPass':
    postLinkPM.add(createConstantPropagationPass());

    // We run the 'CheckInserters' here for two reasons:
    //  1. Since they will insert new calls to the 'an_assert' intrinsic,
    //     they must be run before the 'OperationsExpander'.
    //  2. Since they assert on the accumulator, they must be run before
    //     optimizations, which may promote the accumulator from a global
    //     variable into a register.
    postLinkPM.add(createOperationsExpander(&PC));
    // Due to previously inserted encode/decode instructions some values
    // that used to be constants may no longer appear to be constant. However,
    // after the decode/encode instructions have been expanded, one should be
    // able to infer that these values are indeed constant - but this requires
    // running the 'ConstantPropagationPass':
    postLinkPM.add(createConstantPropagationPass());
    // Optimization to be run immediately after encoding/decoding operations
    // have been inserted (i.e. after intrinsics for "AN coding" have been
    // expanded:
    if (!NoOpts)
    {
      if (!NoInlining) postLinkPM.add(llvm::createFunctionInliningPass());
      postLinkPM.add(llvm::createDeadCodeEliminationPass());
      postLinkPM.add(createConstantPropagationPass());
    }

    if (!NoVerifying) postLinkPM.add(createVerifierPass());
    if (!NoOpts && !NoInlining)
    {
      //mod->getFunction("___accumulate_enc")->addFnAttr(Attribute::AlwaysInline);
      mod->getFunction("___accumulate0_enc")->addFnAttr(Attribute::AlwaysInline);
      mod->getFunction("___accumulate1_enc")->addFnAttr(Attribute::AlwaysInline);
      postLinkPM.add(llvm::createFunctionInliningPass());
      // Replace the global accumulator with a local variable,
      // one per function:
      postLinkPM.add(createAccuPromoter(&C));
      // Then encourage the keeping of local variables in
      // registers (rather than on ths tack):
    }
    postLinkPM.run(*mod);

    // Add optimization passes (roughly the equivalent of "-O2",
    // code was inspired by 'opt.cpp'):
    if (!NoOpts)
    {
      FunctionPassManager FPM(mod);

      PassManagerBuilder Builder;
      Builder.OptLevel = 2;
      Builder.SizeLevel = 0;
      if (!NoInlining) Builder.Inliner = createAlwaysInlinerPass();
      Builder.DisableUnrollLoops = false;

      Builder.populateFunctionPassManager(FPM);
      Builder.populateModulePassManager(PM);

      for (auto I = mod->begin(), E = mod->end(); I != E; I++)
        FPM.run(*I);

      if (!NoInlining) PM.add(llvm::createFunctionInliningPass());
    }
  } else {
    PM.add(createOperationsExpander(&PC));
  }

  if (!NoInlining) PM.add(llvm::createFunctionInliningPass());
  if (!NoVerifying) PM.add(createVerifierPass());
  // Add pass for writing output:
  PM.add(createBitcodeWriterPass(Out->os()));
  PM.run(*mod);

  // Declare success.
  Out->keep();

  return 0;
}
