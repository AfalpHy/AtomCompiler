#pragma once

#include <iostream>

#include "Function.h"

namespace ATC {
namespace AtomIR {

class Module {
public:
    Module(const std::string& name) { _name = name; }

    void addFunction(Function* function) { _functions.push_back(function); }
    void addGlobalVariable(GloabalVariable* var) { _globalVariables.push_back(var); }

    const std::string& getName() { return _name; }
    const std::vector<Function*>& getFunctions() { return _functions; }
    const std::vector<GloabalVariable*>& getGlobalVariables() { return _globalVariables; }

    void dump(std::ostream& os = std::cout);

private:
    std::string _name;
    std::vector<Function*> _functions;
    std::vector<GloabalVariable*> _globalVariables;
};
}  // namespace AtomIR
}  // namespace ATC
