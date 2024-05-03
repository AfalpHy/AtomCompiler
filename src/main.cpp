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
#include "IR/IRBuilder.h"
#include "antlr4-runtime.h"
#include "arm/CodeGenerator.h"
#include "riscv/CodeGenerator.h"

using namespace std;
using namespace antlr4;
using namespace ATC;

int main(int argc, const char *argv[]) {
    llvm::cl::HideUnrelatedOptions({&MyCategory});
    llvm::cl::ParseCommandLineOptions(argc, argv);

    vector<string> objFiles;
    for (auto SrcPath : SrcPathList) {
        std::ifstream file;
        file.open(SrcPath);
        if (!file.is_open()) {
            cerr << filesystem::absolute(std::string(SrcPath)) << " not exist" << endl;
            return -1;
        }

        ANTLRInputStream input(file);
        ATCLexer lexer(&input);
        CommonTokenStream token(&lexer);
        ATCParser parser(&token);
        auto context = parser.compUnit();

        if (parser.getNumberOfSyntaxErrors() != 0) {
            cerr << "There are syntax errors in " << filesystem::absolute(std::string(SrcPath)) << endl;
            return -1;
        }
        ASTBuilder astBuilder(&token);
        CompUnit *compUnit = context->accept(&astBuilder);

        // SemanticChecker checker;
        // for (auto compUnit : CompUnit::AllCompUnits) {
        //     compUnit->accept(&checker);
        // }

        if (DumpAst) {
            ASTDumper dump;
            compUnit->accept(&dump);
        }

        std::filesystem::path filePath = SrcPath;
        string filename = filePath.stem();

        IR::IRBuilder irBuilder;
        compUnit->accept(&irBuilder);
        if (DumpIR) {
            irBuilder.dumpIR(filename + ".atom");
        }
        RISCV::CodeGenerator codeGenerator;
        codeGenerator.emitModule(irBuilder.getCurrentModule());

        ofstream asmfile(filename + ".s", ios::trunc);
        codeGenerator.print(asmfile);
        if (GenerateASM) {
            continue;
        }
        string cmd = "riscv64-linux-gnu-gcc -c " + filename + ".s -o " + filename + ".o";
        int ret = WEXITSTATUS(system(cmd.c_str()));
        if (ret) {
            return ret;
        }
        objFiles.push_back(filename + ".o");
    }
    if (GenerateASM) {
        return 0;
    }
    string cmd = "riscv64-linux-gnu-gcc -static";
    if (!SyLibPath.empty()) {
        cmd += " " + SyLibPath;
    }
    for (auto obj : objFiles) {
        cmd += " " + obj;
    }

    int ret = 0;
    ret = WEXITSTATUS(system(cmd.c_str()));
    if (ret) {
        return ret;
    }

    if (RunAfterCompiling) {
        if (Platform == "riscv") {
            cmd = "qemu-riscv64 ./a.out";
        } else if (Platform == "arm") {
            // TODO:
        }
        if (!RunInput.empty()) {
            cmd.append(" < ").append(RunInput);
        }
        if (Check) {
            string resultFile = "result.txt";
            cmd.append(" > ").append(resultFile);
            ret = system(cmd.c_str());
            ret = WEXITSTATUS(ret);
            cmd = "echo";
            cmd.append(" ").append(to_string(ret)).append(" >> ").append(resultFile);
            system(cmd.c_str());
            cmd = "diff";
            cmd.append(" ").append(resultFile).append(" ").append(CompareFile);
            ret = system(cmd.c_str());
            return WEXITSTATUS(ret);
        }
        system(cmd.c_str());
    }
    return 0;
}
