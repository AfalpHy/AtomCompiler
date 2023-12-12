#include "atomIR/Type.h"

#include <assert.h>

#include <unordered_map>

namespace ATC {

namespace AtomIR {

Type* Type::getInt32Ty() {
    static Type* ret = new Type(INT32_TY);
    return ret;
}

Type* Type::getFloatTy() {
    static Type* ret = new Type(FLOAT_TY);
    return ret;
}

Type* Type::getVoidTy() {
    static Type* ret = new Type(VOID_TY);
    return ret;
}

std::string Type::toString() {
    switch (_type) {
        case INT32_TY:
            return "i32";
        case FLOAT_TY:
            return "float";
        case VOID_TY:
            return "void";
        default:
            assert(false && "should not reach here");
            break;
    }
}

PointerType* Type::getPointerTy() { return PointerType::get(this); }

ArrayType* ArrayType::get(Type* baseType, int size) {
    static std::unordered_map<Type*, std::unordered_map<int, ArrayType*>> ty2ArrayTy;
    if (ty2ArrayTy.find(baseType) != ty2ArrayTy.end() &&
        ty2ArrayTy[baseType].find(size) != ty2ArrayTy[baseType].end()) {
        return ty2ArrayTy[baseType][size];
    }
    ArrayType* ret = new ArrayType(baseType, size);
    ty2ArrayTy[baseType].insert({size, ret});
    return ret;
}

std::string ArrayType::toString() {
    std::string str;
    str.append("[").append(std::to_string(_size)).append(" x ").append(_baseType->toString()).append("]");
    return str;
}

PointerType* PointerType::get(Type* baseType) {
    static std::unordered_map<Type*, PointerType*> ty2PointerTy;
    if (ty2PointerTy.find(baseType) != ty2PointerTy.end()) {
        return ty2PointerTy[baseType];
    }
    PointerType* ret = new PointerType(baseType);
    ty2PointerTy.insert({baseType, ret});
    return ret;
}

std::string PointerType::toString() { return _baseType->toString() + "*"; }

}  // namespace AtomIR
}  // namespace ATC