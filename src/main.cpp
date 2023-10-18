#include <stdio.h>

#include <filesystem>
#include <iostream>
#include <string>

#include "AST/CompUnit.h"
#include "AST/Scope.h"
#include "AST/tools/ASTBuilder.h"
#include "AST/tools/DumpASTVisitor.h"
#include "AST/tools/SemanticChecker.h"
#include "ATCLexer.h"
#include "ATCParser.h"
#include "antlr4-runtime.h"
#include "llvmIR/IRBuilder.h"
using namespace std;
using namespace antlr4;

int main(int argc, const char *argv[]) {
    std::ifstream file;
    file.open(argv[1]);

    if (!file.is_open()) {
        cerr << filesystem::absolute(argv[1]) << " not exist" << endl;
        return 0;
    }
    ANTLRInputStream input(file);
    input.name = argv[1];
    ATCLexer lexer(&input);
    CommonTokenStream token(&lexer);
    ATCParser parser(&token);
    auto context = parser.compUnit();
    if (parser.getNumberOfSyntaxErrors() != 0) {
        cerr << "There are syntax errors" << endl;
        return 0;
    }
    ATC::ASTBuilder astBuilder;
    astBuilder.setTokenStream(&token);
    context->accept(&astBuilder);

    ATC::SemanticChecker checker;
    for (auto compUnit : ATC::CompUnit::AllCompUnits) {
        compUnit->accept(&checker);
    }

    if (argc > 2 && strcmp(argv[2], "--dump-ast") == 0) {
        ATC::DumpASTVisitor dump;
        for (auto compUnit : ATC::CompUnit::AllCompUnits) {
            compUnit->accept(&dump);
        }
    }

    ATC::IRBuilder irBuilder;
    for (auto compUnit : ATC::CompUnit::AllCompUnits) {
        compUnit->accept(&irBuilder);
    }

    return 0;
}
