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
    const std::vector<Expression*>& getDimensions() { return _dimensions; }
    bool isPointer() { return _baseDataType != nullptr; }
    DataType* getBaseDataType() { return _baseDataType; }

    void setBaseType(BaseType type) { _type = type; }
    void addDimension(Expression* dimension) { _dimensions.push_back(dimension); }
    void setBaseDataType(DataType* dataType) { _baseDataType = dataType; }

    ACCEPT

private:
    BaseType _type = UNKOWN;
    std::vector<Expression*> _dimensions;
    DataType* _baseDataType;
};

}  // namespace ATC

#endif