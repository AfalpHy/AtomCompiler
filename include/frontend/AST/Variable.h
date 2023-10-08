#ifndef VARIABLE_H
#define VARIABLE_H

#include "Node.h"

namespace ATC {

class Variable : public Node {
public:
    Variable() = default;

private:
    bool _isConst;
};

}  // namespace ATC

#endif