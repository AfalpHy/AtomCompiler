#ifndef ATOM_VALUE_H
#define ATOM_VALUE_H

#include "Type.h"

namespace ATC {
namespace AtomIR {

class Function;
class Instruction;

class Value {
public:
    Value(const std::string& name) : _name(name) {}
    Value(Type* type, const std::string& name) : _type(type), _name(name) {}

    void setType(Type* type) { _type = type; }
    void setBelong(Function* function) { _belong = function; }
    void setDefined(Instruction* inst) { _defined = inst; }

    Type* getType() { return _type; }

    const std::string& getName() { return _name; }

    virtual std::string getValueStr();

    virtual bool isConst() { return false; }

    std::string toString();

    virtual void dump();

protected:
    Type* _type = nullptr;
    std::string _name;
    Function* _belong = nullptr;
    Instruction* _defined = nullptr;
};

class ConstantInt : public Value {
public:
    ConstantInt(Type* type, int value) : Value(type, ""), _constValue(value) {}

    virtual std::string getValueStr() override;

    virtual bool isConst() override { return true; }

    int getConstValue() { return _constValue; }

    virtual void dump() override;

private:
    int _constValue;
};

class ConstantFloat : public Value {
public:
    ConstantFloat(Type* type, float value) : Value(type, ""), _constValue(value) {}

    virtual std::string getValueStr() override;

    virtual bool isConst() override { return true; }

    float getConstValue() { return _constValue; }

    virtual void dump() override;

private:
    float _constValue;
};

class GloabalVariable : public Value {
public:
    virtual std::string getValueStr() override;
    virtual void dump() override;
};
}  // namespace AtomIR
}  // namespace ATC

#endif