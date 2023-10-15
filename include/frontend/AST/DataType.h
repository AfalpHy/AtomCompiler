#ifndef DATA_TYPE_H
#define DATA_TYPE_H

#include "TreeNode.h"

namespace ATC {

class Expression;
class DataType : public TreeNode {
public:
    DataType() = default;
    virtual int getClassId() { return ID_DATA_TYPE; }

    int getBaseType() { return _type; }
    const std::vector<ConstVal*>& getDimensions() { return _dimensions; }

    void setBaseType(BaseType type) { _type = type; }

    void addDimension(ConstVal* dimension) { _dimensions.push_back(dimension); }

    bool isPointer() { return _pointerDataType != nullptr; }

    ACCEPT

private:
    BaseType _type = UNKOWN;
    std::vector<ConstVal*> _dimensions;
    DataType* _pointerDataType;
};

}  // namespace ATC

#endif