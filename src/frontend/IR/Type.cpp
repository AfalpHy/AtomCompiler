#include "IR/Type.h"

#include <assert.h>

#include <unordered_map>

namespace ATC {

namespace IR {

Type* Type::getInt32Ty() {
    static Type* ret = new Type(4);
    return ret;
}

Type* Type::getFloatTy() {
    static Type* ret = new Type(4);
    return ret;
}

Type* Type::getVoidTy() {
    static Type* ret = new Type();
    return ret;
}

std::string Type::toString() {
    if (this == getInt32Ty()) {
        return "i32";
    } else if (this == getFloatTy()) {
        return "float";
    } else if (this == getVoidTy()) {
        return "void";
    }
    assert(false && "should not reach here");
    return "";
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

}  // namespace IR
}  // namespace ATC