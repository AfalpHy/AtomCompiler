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
    void setName(const std::string& name) { _name = name; }
    void setBelong(Function* function) { _belong = function; }
    void setDefined(Instruction* inst) { _defined = inst; }

    Type* getType() { return _type; }

    const std::string& getName() { return _name; }

    virtual std::string getValueStr();

    virtual std::string toString();

    virtual bool isConst() { return false; }

    virtual void dump();

protected:
    Type* _type = nullptr;
    std::string _name;
    Function* _belong = nullptr;
    Instruction* _defined = nullptr;
};

class ConstantInt : public Value {
public:
    ConstantInt(int value) : Value(Type::getInt32Ty(), ""), _constValue(value) {}

    virtual std::string getValueStr() override;

    virtual bool isConst() override { return true; }

    int getConstValue() { return _constValue; }

    virtual void dump() override;

private:
    int _constValue;
};

class ConstantFloat : public Value {
public:
    ConstantFloat(float value) : Value(Type::getFloatTy(), ""), _constValue(value) {}

    virtual std::string getValueStr() override;

    virtual bool isConst() override { return true; }

    float getConstValue() { return _constValue; }

    virtual void dump() override;

private:
    float _constValue;
};

class ArrayValue : public Value {
public:
    ArrayValue(Type* type) : Value(type, "") {}

    void addElement(const std::pair<int, std::vector<Value*>>& element) { _elements.push_back(element); }

    virtual std::string toString() override;

    virtual void dump() override;

private:
    std::vector<std::pair<int, std::vector<Value*>>> _elements;
};

class GloabalVariable : public Value {
public:
    GloabalVariable(Type* type, const std::string& name) : Value(type->getPointerTy(), name) {}

    void setInitialValue(Value* init) { _init = init; }

    virtual std::string getValueStr() override;

    virtual std::string toString() override;

    virtual void dump() override;

private:
    Value* _init = nullptr;
};
}  // namespace AtomIR
}  // namespace ATC

#endif