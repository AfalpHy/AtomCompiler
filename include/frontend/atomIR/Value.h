#ifndef ATOM_VALUE_H
#define ATOM_VALUE_H

#include <string>

#include "Type.h"

namespace ATC {
namespace AtomIR {

class Value {
public:
    Value(const std::string& name) : _name(name) {}
    Value(Type* type, const std::string& name) : _type(type), _name(name) {}

    void setName(const std::string& name) { _name = name; }
    void setType(Type* type) { _type = type; }

    const std::string& getName() { return _name; }
    Type* getType() { return _type; }

private:
    std::string _name;
    Type* _type = nullptr;
};
}  // namespace AtomIR
}  // namespace ATC

#endif