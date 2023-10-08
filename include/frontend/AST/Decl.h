#ifndef DECL_H
#define DECL_H

#include <vector>

#include "Node.h"
#include "Variable.h"
namespace ATC {
class Decl : public Node {
public:
    Decl(/* args */) = default;
    ~Decl() = default;

private:
    Variable* _var;
};

}  // namespace ATC

#endif