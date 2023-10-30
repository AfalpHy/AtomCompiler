#include "AST/Variable.h"

#include "AST/Decl.h"
namespace ATC {
int Variable::getBasicType() { return _dataType->getBasicType(); }
}  // namespace ATC