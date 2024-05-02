#pragma once

#include <string>

namespace ATC {

namespace IR {

class PointerType;

class Type {
public:
    static Type* getInt32Ty();
    static Type* getFloatTy();
    static Type* getVoidTy();

    PointerType* getPointerTy();

    bool isIntType() { return this->isPointerType() || this == getInt32Ty(); }

    virtual std::string toString();

    virtual Type* getBaseType() { return nullptr; }

    virtual bool isPointerType() { return 0; }
    virtual bool isArrayType() { return 0; }

    virtual int getByteLen() { return _byteLen; }

protected:
    Type(int byteLen = 0) : _byteLen(byteLen) {}
    int _byteLen;
};

class ArrayType : public Type {
public:
    static ArrayType* get(Type* baseType, int size);

    virtual Type* getBaseType() { return _baseType; }

    virtual std::string toString() override;

    virtual bool isArrayType() override { return 1; }

    virtual int getByteLen() override { return _size * _baseType->getByteLen(); }

private:
    ArrayType(Type* baseType, int size) : _baseType(baseType), _size(size) {}
    Type* _baseType;
    int _size;
};

class PointerType : public Type {
public:
    static PointerType* get(Type* baseType);

    virtual Type* getBaseType() { return _baseType; }

    virtual std::string toString() override;

    virtual bool isPointerType() override { return 1; }

    virtual int getByteLen() override { return 8; }

private:
    PointerType(Type* baseType) : _baseType(baseType) {}
    Type* _baseType;
};
}  // namespace IR
}  // namespace ATC
