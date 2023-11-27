#ifndef ATOM_VALUE_H
#define ATOM_VALUE_H

#include "Type.h"

namespace ATC {
namespace AtomIR {

class Function;

class Value {
public:
    Value(const std::string& name) : _originName(name) {}
    Value(Type* type, const std::string& name) : _type(type), _originName(name) {}

    void setCurrentName(const std::string& name) { _currentName = name; }
    void setType(Type* type) { _type = type; }
    void setBelongFunction(Function* function) { _belongFunction = function; }

    const std::string& getOriginName() { return _originName; }
    const std::string& getCurrentName() { return _currentName; }
    Type* getType() { return _type; }

    void dump();

private:
    std::string _originName;
    std::string _currentName;
    Function* _belongFunction = nullptr;
    Type* _type = nullptr;
};
}  // namespace AtomIR
}  // namespace ATC

#endif