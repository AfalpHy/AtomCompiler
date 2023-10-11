#include "AST/Variable.h"

#include "AST/Expression.h"
namespace ATC {

bool Variable::isConst() {
    if (_initValue) return _initValue->isConst();
    return false;
}
}  // namespace ATC