#ifndef VARIABLE_H
#define VARIABLE_H

#include "DataType.h"
namespace ATC {

class Expression;
class Variable : public TreeNode {
public:
    Variable() = default;
    virtual int getClassId() { return ID_VARIABLE; }

    DataType* getDataType() { return _type; }
    Expression* getInitValue() { return _initValue; }
    bool isConst() { return _isConst; }

    void setDataType(DataType* type) { _type = type; }
    void setInitValue(Expression* value) { _initValue = value; }
    void setIsConst() { _isConst = true; }

    ACCEPT

private:
    DataType* _type;
    Expression* _initValue = nullptr;
    bool _isConst = false;
};

}  // namespace ATC

#endif