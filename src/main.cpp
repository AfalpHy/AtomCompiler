#include <stdio.h>

#include <filesystem>
#include <iostream>
#include <string>

#include "AST/ASTBuilder.h"
#include "AST/ASTDumper.h"
#include "AST/CompUnit.h"
#include "AST/Scope.h"
#include "AST/SemanticChecker.h"
#include "ATCLexer.h"
#include "ATCParser.h"
#include "CmdOption.h"
#include "antlr4-runtime.h"
#include "arm/CodeGenerator.h"
#include "atomIR/IRBuilder.h"
#include "llvmIR/IRBuilder.h"
#include "riscv/CodeGenerator.h"

using namespace std;
using namespace antlr4;
using namespace ATC;

int main(int argc, const char *argv[]) {
    llvm::cl::HideUnrelatedOptions({&MyCategory});
    llvm::cl::ParseCommandLineOptions(argc, argv);

    std::ifstream file;
    file.open(SrcPath);

    if (!file.is_open()) {
        cerr << filesystem::absolute(std::string(SrcPath)) << " not exist" << endl;
        return 0;
    }
    ANTLRInputStream input(file);
    input.name = argv[1];
    ATCLexer lexer(&input);
    CommonTokenStream token(&lexer);
    ATCParser parser(&token);
    auto context = parser.compUnit();
    if (parser.getNumberOfSyntaxErrors() != 0) {
        cerr << "There are syntax errors in " << filesystem::absolute(std::string(SrcPath)) << endl;
        return -1;
    }
    ASTBuilder astBuilder;
    astBuilder.setTokenStream(&token);
    context->accept(&astBuilder);

    // SemanticChecker checker;
    // for (auto compUnit : CompUnit::AllCompUnits) {
    //     compUnit->accept(&checker);
    // }

    if (DumpAst) {
        ASTDumper dump;
        for (auto compUnit : CompUnit::AllCompUnits) {
            compUnit->accept(&dump);
        }
    }

    std::filesystem::path filePath = std::string(SrcPath);
    string filename = filePath.stem();
    string outFile = filename + ".out";
    string cmd;
    if (EmitLLVM) {
        LLVMIR::IRBuilder irBuilder;
        // only on module now
        for (auto compUnit : CompUnit::AllCompUnits) {
            compUnit->accept(&irBuilder);
        }

        if (DumpIR) {
            irBuilder.dumpIR(filename + ".ll");
            cmd = "clang " + filename + ".ll";
        } else {
            /// TODO:use llvm API to create the .o file
        }
    } else {
        AtomIR::IRBuilder irBuilder;
        RISCV::CodeGenerator codeGenerator;
        // only on module now
        for (auto compUnit : CompUnit::AllCompUnits) {
            compUnit->accept(&irBuilder);
            codeGenerator.emitModule(irBuilder.getCurrentModule());
        }
        if (DumpIR) {
            irBuilder.dumpIR(filename + ".atom");
        }
        ofstream asmfile(filename + ".s", ios::trunc);
        codeGenerator.dump(asmfile);
        cmd = "clang -target riscv64-linux-gnu " + filename + ".s";
    }
    if (Sy) {
        cmd.append(" ").append(SySrc);
    }
    int ret = WEXITSTATUS(system(cmd.c_str()));
    if (ret) {
        return ret;
    }
    if (RunAfterCompiling) {
        cmd = "./a.out";
        if (!RunInput.empty()) {
            cmd.append(" < ").append(RunInput);
        }
        if (Check) {
            cmd.append(" > ").append(outFile);
            ret = system(cmd.c_str());
            ret = WEXITSTATUS(ret);
            cmd = "echo";
            cmd.append(" ").append(to_string(ret)).append(" >> ").append(outFile);
            system(cmd.c_str());
            cmd = "diff";
            cmd.append(" ").append(outFile).append(" ").append(CompareFile);
            ret = system(cmd.c_str());
            return WEXITSTATUS(ret);
        }
        system(cmd.c_str());
    }
    return 0;
}
