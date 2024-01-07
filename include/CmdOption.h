#pragma once

#include <string>

#include "llvm/Support/CommandLine.h"

namespace ATC {
extern llvm::cl::OptionCategory MyCategory;
extern llvm::cl::opt<std::string> SrcPath;
extern llvm::cl::opt<bool> Sy;
extern llvm::cl::opt<std::string> SySrc;
extern llvm::cl::opt<bool> DumpAst;
extern llvm::cl::opt<bool> DumpIR;
extern llvm::cl::opt<bool> EmitLLVM;
extern llvm::cl::opt<bool> RunAfterCompiling;
extern llvm::cl::opt<std::string> RunInput;
extern llvm::cl::opt<bool> Check;
extern llvm::cl::opt<std::string> CompareFile;

}  // namespace ATC