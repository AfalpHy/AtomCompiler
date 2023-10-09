#ifndef VARIABLE_H
#define VARIABLE_H

#include "TreeNode.h"

namespace ATC {

class Variable : public TreeNode {
public:
    Variable() = default;
    virtual int getClassId() { return ID_VARIABLE; }

    int getDataType() { return _type; }
    bool isConst() { return _isConst; }
    int getIntValue() { return _intValue; }
    float getFloatValue() { return _floatValue; }

    void setDataType(DataType type) { _type = type; }
    void setIsConst(bool b) { _isConst = b; }
    void setIntValue(int value) { _intValue = value; }
    void setFloatValue(float value) { _floatValue = value; }

private:
    DataType _type;
    bool _isConst = false;
    int _intValue = 0;
    float _floatValue = 0;
};

}  // namespace ATC

#endif