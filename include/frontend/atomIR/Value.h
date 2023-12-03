#pragma once

#include "Type.h"

namespace ATC {
namespace AtomIR {

class Function;
class Instruction;

class Value {
public:
    Value(Type* type, const std::string& name) : _type(type), _name(name) {}

    void setName(const std::string& name) { _name = name; }

    void setBelong(Function* function);

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
    static ConstantInt* get(int value);

    virtual std::string getValueStr() override;

    virtual bool isConst() override { return true; }

    int getConstValue() { return _constValue; }

private:
    ConstantInt(int value) : Value(Type::getInt32Ty(), ""), _constValue(value) {}
    int _constValue;
};

class ConstantFloat : public Value {
public:
    static ConstantFloat* get(float value);

    virtual std::string getValueStr() override;

    virtual bool isConst() override { return true; }

    float getConstValue() { return _constValue; }

private:
    ConstantFloat(float value) : Value(Type::getFloatTy(), ""), _constValue(value) {}
    float _constValue;
};

class ArrayValue : public Value {
public:
    ArrayValue(Type* type) : Value(type, "") {}

    void addElement(const std::pair<int, std::vector<Value*>>& element) { _elements.push_back(element); }

    virtual std::string toString() override;

private:
    std::vector<std::pair<int, std::vector<Value*>>> _elements;
};

class GloabalVariable : public Value {
public:
    GloabalVariable(Type* type, const std::string& name) : Value(type->getPointerTy(), name) {}

    void setInitialValue(Value* init) { _init = init; }

    virtual std::string getValueStr() override;

    virtual void dump() override;

private:
    Value* _init = nullptr;
};
}  // namespace AtomIR
}  // namespace ATC
