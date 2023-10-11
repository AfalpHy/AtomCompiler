#include <iostream>

#include "AST/CompUnit.h"
#include "AST/tools/ASTBuilder.h"
#include "AST/tools/DumpASTVisitor.h"
#include "ATCLexer.h"
#include "ATCParser.h"
#include "antlr4-runtime.h"
using namespace std;
using namespace antlr4;

int main(int argc, const char *argv[]) {
    std::ifstream stream;
    stream.open(argv[1]);

    ANTLRInputStream input(stream);
    input.name = argv[1];
    ATCLexer lexer(&input);
    CommonTokenStream token(&lexer);
    ATCParser parser(&token);
    auto context = parser.compUnit();
    ATC::ASTBuilder astBuilder;
    astBuilder.setTokenStream(&token);
    context->accept(&astBuilder);

    ATC::DumpASTVisitor dump;

    for (auto compUnit : ATC::CompUnit::AllCompUnits) {
        compUnit->accept(&dump);
    }

    if (parser.getNumberOfSyntaxErrors() != 0) {
        cerr << argv[1] << endl;
    }
    return 0;
}
