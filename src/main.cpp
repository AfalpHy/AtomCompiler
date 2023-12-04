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
#include "antlr4-runtime.h"
#include "arm/CodeGenerator.h"
#include "atomIR/IRBuilder.h"
#include "llvmIR/IRBuilder.h"
#include "riscv/CodeGenerator.h"

using namespace std;
using namespace antlr4;

int main(int argc, const char *argv[]) {
    int current = 1;
    string sourceFile;
    string otherSrcFile;
    string runtimeInput;
    string compareFile;
    string compareResult;
    bool useLLVM = false;
    bool runAfterCompile = false;
    bool dumpAst = false;
    bool check = false;

    while (current < argc) {
        if (strcmp(argv[current], "--emit-llvm") == 0) {
            useLLVM = true;
        } else if (strcmp(argv[current], "--other-src") == 0) {
            current++;
            otherSrcFile = argv[current];
        } else if (strcmp(argv[current], "-R") == 0) {
            runAfterCompile = true;
        } else if (strcmp(argv[current], "--R-input") == 0) {
            current++;
            runtimeInput = argv[current];
        } else if (strcmp(argv[current], "--dump-ast") == 0) {
            dumpAst = true;
        } else if (strcmp(argv[current], "--check") == 0) {
            check = true;
        } else if (strcmp(argv[current], "--compare-file") == 0) {
            current++;
            compareFile = argv[current];
        } else if (strcmp(argv[current], "--compare-result") == 0) {
            current++;
            compareResult = argv[current];
        } else {
            sourceFile = argv[current];
        }
        current++;
    }

    std::ifstream file;
    file.open(sourceFile);

    if (!file.is_open()) {
        cerr << filesystem::absolute(sourceFile) << " not exist" << endl;
        return 0;
    }
    ANTLRInputStream input(file);
    input.name = argv[1];
    ATCLexer lexer(&input);
    CommonTokenStream token(&lexer);
    ATCParser parser(&token);
    auto context = parser.compUnit();
    if (parser.getNumberOfSyntaxErrors() != 0) {
        cerr << "There are syntax errors in " << filesystem::absolute(sourceFile) << endl;
        return -1;
    }
    ATC::ASTBuilder astBuilder;
    astBuilder.setTokenStream(&token);
    context->accept(&astBuilder);

    // ATC::SemanticChecker checker;
    // for (auto compUnit : ATC::CompUnit::AllCompUnits) {
    //     compUnit->accept(&checker);
    // }

    if (dumpAst) {
        ATC::ASTDumper dump;
        for (auto compUnit : ATC::CompUnit::AllCompUnits) {
            compUnit->accept(&dump);
        }
    }

    if (useLLVM) {
        ATC::LLVMIR::IRBuilder irBuilder;
        for (auto compUnit : ATC::CompUnit::AllCompUnits) {
            compUnit->accept(&irBuilder);
        }
        std::filesystem::path filePath = sourceFile;
        string filename = filePath.stem();
        string llFile = filename + ".ll";
        string outFile = filename + ".out";
        irBuilder.dumpLL(llFile);
        string cmd = "clang";
        cmd.append(" ").append(llFile).append(" ").append(otherSrcFile);
        system(cmd.c_str());
        if (runAfterCompile) {
            cmd = "./a.out";
            if (!runtimeInput.empty()) {
                cmd.append(" < ").append(runtimeInput);
            }
            if (check) {
                cmd.append(" > ").append(outFile);
                int ret = system(cmd.c_str());
                ret = WEXITSTATUS(ret);
                cmd = "echo";
                cmd.append(" ").append(to_string(ret)).append(" >> ").append(outFile);
                system(cmd.c_str());
                cmd = "diff";
                cmd.append(" ").append(outFile).append(" ").append(compareFile).append(" >> ").append(compareResult);
                system(cmd.c_str());
                cmd = "echo";
                cmd.append(" \"").append(filePath).append("\"").append(" >> ").append(compareResult);
                system(cmd.c_str());
            }
        }
    } else {
        ATC::AtomIR::IRBuilder irBuilder;
        for (auto compUnit : ATC::CompUnit::AllCompUnits) {
            compUnit->accept(&irBuilder);
        }
        ATC::RISCV_ARCH::CodeGenerator codeGenerator;
        codeGenerator.dump(std::cout);
    }

    return 0;
}
