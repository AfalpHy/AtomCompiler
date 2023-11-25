#ifndef ATOM_TYPE_H
#define ATOM_TYPE_H

#include <string>

namespace ATC {

namespace AtomIR {
enum TypeEnum { INT1_TY, INT32_TY, FLOAT_TY, VOID_TY, POINTER_TY };

class PointerType;

class Type {
public:
    static Type* getInt1Ty();
    static Type* getInt32Ty();
    static Type* getFloatTy();
    static Type* getVoidTy();

    PointerType* getPointerTy();

    int getTypeEnum() { return _type; }

    virtual std::string toString();

    virtual bool isPointerType() { return 0; }

protected:
    Type(TypeEnum type) : _type(type) {}
    TypeEnum _type;
};

class PointerType : public Type {
public:
    static PointerType* get(Type* baseType);

    Type* getBaseType() { return _baseType; }

    virtual std::string toString() override;

    virtual bool isPointerType() override { return 1; }

private:
    PointerType(Type* baseType) : Type(POINTER_TY), _baseType(baseType) {}
    Type* _baseType;
};
}  // namespace AtomIR
}  // namespace ATC

#endif