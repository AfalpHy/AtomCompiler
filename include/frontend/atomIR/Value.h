#pragma once

#include <iostream>

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

    Instruction* getDefined() { return _defined; }

    virtual std::string getValueStr();

    virtual std::string toString();

    virtual bool isConst() { return false; }

    virtual bool isGlobal() { return false; }

    virtual void dump();

protected:
    Type* _type = nullptr;
    std::string _name;
    Function* _belong = nullptr;
    Instruction* _defined = nullptr;
};

class Constant : public Value {
public:
    Constant(Type* type) : Value(type, "") {}

    virtual bool isConst() override { return true; }

    virtual std::string getLiteralStr() = 0;

    virtual bool isInt() = 0;
};

class ConstantInt : public Constant {
public:
    static ConstantInt* get(int value);

    virtual std::string getValueStr() override;

    virtual std::string getLiteralStr() override { return std::to_string(_constValue); }

    virtual bool isInt() { return true; }

    int getConstValue() { return _constValue; }

private:
    ConstantInt(int value) : Constant(Type::getInt32Ty()), _constValue(value) {}
    int _constValue;
};

class ConstantFloat : public Constant {
public:
    static ConstantFloat* get(float value);

    virtual std::string getValueStr() override;

    virtual std::string getLiteralStr() override {
        int* valuePtr = (int*)&_constValue;
        return std::to_string(*valuePtr);
    }

    virtual bool isInt() { return false; }

    float getConstValue() { return _constValue; }

private:
    ConstantFloat(float value) : Constant(Type::getFloatTy()), _constValue(value) {}
    float _constValue;
};

class ArrayValue : public Value {
public:
    ArrayValue(Type* type) : Value(type, "") {}

    void addElement(const std::pair<int, std::vector<Value*>>& element) { _elements.push_back(element); }

    virtual std::string toString() override;

    const std::vector<std::pair<int, std::vector<Value*>>>& getElements() { return _elements; }

private:
    std::vector<std::pair<int, std::vector<Value*>>> _elements;
};

class GloabalVariable : public Value {
public:
    GloabalVariable(Type* type, const std::string& name) : Value(type->getPointerTy(), name) {}

    void setInitialValue(Value* init) { _init = init; }

    Value* getInitialValue() { return _init; }

    virtual std::string getValueStr() override;

    virtual bool isGlobal() { return true; }

    virtual std::string toString() override;

private:
    Value* _init = nullptr;
};
}  // namespace AtomIR
}  // namespace ATC
