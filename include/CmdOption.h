#pragma once

#include <string>

#include "llvm/Support/CommandLine.h"

namespace ATC {
extern llvm::cl::OptionCategory MyCategory;
extern llvm::cl::opt<std::string> SrcPath;
extern llvm::cl::opt<bool> EmitLLVM;
extern llvm::cl::opt<std::string> OtherSrc;
extern llvm::cl::opt<bool> RunAfterCompiling;
extern llvm::cl::opt<std::string> RunInput;
extern llvm::cl::opt<bool> DumpAst;
extern llvm::cl::opt<bool> Check;
extern llvm::cl::opt<std::string> CompareFile;
extern llvm::cl::opt<std::string> CompareResult;

}  // namespace ATC