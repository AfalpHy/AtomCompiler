#include <filesystem>
#include <iostream>

#include "AST/CompUnit.h"
#include "AST/Scope.h"
#include "AST/tools/ASTBuilder.h"
#include "AST/tools/DumpASTVisitor.h"
#include "ATCLexer.h"
#include "ATCParser.h"
#include "antlr4-runtime.h"
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
    ATC::ASTBuilder astBuilder;
    astBuilder.setTokenStream(&token);
    context->accept(&astBuilder);
    ATC::CurrentScope->fixupNode();

    ATC::DumpASTVisitor dump;

    for (auto compUnit : ATC::CompUnit::AllCompUnits) {
        compUnit->accept(&dump);
    }

    if (parser.getNumberOfSyntaxErrors() != 0) {
        cerr << argv[1] << endl;
    }
    return 0;
}
