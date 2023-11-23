#include "atomIR/Type.h"

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

PointerType* Type::getPointerTy() {
    static std::unordered_map<Type*, PointerType*> ty2PointerTy;
    if (ty2PointerTy.find(this) != ty2PointerTy.end()) {
        return ty2PointerTy[this];
    }
    PointerType* ret = new PointerType(this);
    ty2PointerTy.insert({this, ret});
    return ret;
}

}  // namespace AtomIR
}  // namespace ATC