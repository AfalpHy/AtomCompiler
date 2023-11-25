#include "atomIR/Type.h"

#include <assert.h>

#include <unordered_map>
namespace ATC {

namespace AtomIR {

Type* Type::getInt1Ty() {
    static Type* ret = new Type(INT1_TY);
    return ret;
}

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
        case INT1_TY:
            return "i1";
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

std::string PointerType::toString() { return _baseType->toString() + "*"; }

PointerType* PointerType::get(Type* baseType) {
    static std::unordered_map<Type*, PointerType*> ty2PointerTy;
    if (ty2PointerTy.find(baseType) != ty2PointerTy.end()) {
        return ty2PointerTy[baseType];
    }
    PointerType* ret = nullptr;
    if (baseType->isPointerType()) {
        ret = get(static_cast<PointerType*>(baseType)->getBaseType());
    } else {
        ret = new PointerType(baseType);
    }
    ty2PointerTy.insert({baseType, ret});
    return ret;
}

}  // namespace AtomIR
}  // namespace ATC