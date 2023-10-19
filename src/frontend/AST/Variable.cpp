#include "AST/Variable.h"

#include "AST/Decl.h"
namespace ATC {

DataType* Variable::getDataType() { return static_cast<Decl*>(getParent())->getDataType(); }
}  // namespace ATC