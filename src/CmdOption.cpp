#include "CmdOption.h"

namespace ATC {
llvm::cl::OptionCategory MyCategory("ATC");

llvm::cl::opt<std::string> SrcPath(llvm::cl::Positional, llvm::cl::desc("file"), llvm::cl::cat(MyCategory));

llvm::cl::opt<bool> Sy("sy", llvm::cl::desc("include sy function"), llvm::cl::init(false), llvm::cl::cat(MyCategory));

llvm::cl::opt<std::string> SySrc("sy-src", llvm::cl::desc("sy src which need to be compiled by clang"),
                                 llvm::cl::cat(MyCategory));

llvm::cl::opt<bool> GenerateASM("S", llvm::cl::desc("generate asm only"), llvm::cl::init(false),
                                llvm::cl::cat(MyCategory));

llvm::cl::opt<bool> DumpAst("dump-ast", llvm::cl::desc("dump the ATC Ast"), llvm::cl::init(false),
                            llvm::cl::cat(MyCategory));

llvm::cl::opt<bool> DumpIR("dump-ir", llvm::cl::desc("dump the intermediate representation"), llvm::cl::init(false),
                           llvm::cl::cat(MyCategory));

llvm::cl::opt<bool> RunAfterCompiling("R", llvm::cl::desc("run after compiling"), llvm::cl::init(false),
                                      llvm::cl::cat(MyCategory));

llvm::cl::opt<std::string> Platform("platform", llvm::cl::desc("the platform used to execute the final executable file"),
                                  llvm::cl::cat(MyCategory));

llvm::cl::opt<std::string> RunInput("R-input", llvm::cl::desc("input file for program which will run after compiling"),
                                    llvm::cl::cat(MyCategory));

llvm::cl::opt<bool> Check("check", llvm::cl::desc("check after running"), llvm::cl::init(false),
                          llvm::cl::cat(MyCategory));

llvm::cl::opt<std::string> CompareFile("compare-file",
                                       llvm::cl::desc("right output for program which will run after compiling"),
                                       llvm::cl::cat(MyCategory));
}  // namespace ATC