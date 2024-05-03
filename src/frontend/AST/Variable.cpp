#include "AST/Variable.h"
namespace ATC {
int Variable::getBasicType() { return _dataType->getBasicType(); }
}  // namespace ATC