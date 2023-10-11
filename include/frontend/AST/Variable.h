#ifndef VARIABLE_H
#define VARIABLE_H

#include "TreeNode.h"

namespace ATC {

class Expression;
class Variable : public TreeNode {
public:
    Variable() = default;
    virtual int getClassId() { return ID_VARIABLE; }

    int getDataType() { return _type; }
    Expression* getInitValue() { return _initValue; }
    bool isConst();

    void setDataType(DataType type) { _type = type; }
    void setInitValue(Expression* value) { _initValue = value; }

    ACCEPT

private:
    DataType _type;
    Expression* _initValue = nullptr;
};

}  // namespace ATC

#endif