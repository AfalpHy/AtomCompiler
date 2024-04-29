#pragma once

#include "DataType.h"
namespace ATC {

class Expression;

class Variable : public TreeNode {
public:
    Variable() = default;

    virtual int getClassId() { return ID_VARIABLE; }

    int getBasicType();

    DataType* getDataType() { return _dataType; }
    Expression* getInitValue() { return _initValue; }
    bool isConst() { return _isConst; }
    bool isGlobal() { return _isGlobal; }

    void setDataType(DataType* dataType) { _dataType = dataType; }
    void setInitValue(Expression* value) { _initValue = value; }
    void setIsConst(bool b) { _isConst = b; }
    void setIsGlobal(bool b) { _isGlobal = b; }

    ACCEPT

private:
    DataType* _dataType = nullptr;
    Expression* _initValue = nullptr;
    bool _isConst = false;
    bool _isGlobal = false;
};

}  // namespace ATC
