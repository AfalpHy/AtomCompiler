#ifndef DTATA_TYPE_H
#define DATA_TYPE_H

#include "TreeNode.h"

namespace ATC {

class Expression;
class DataType : public TreeNode {
public:
    DataType() = default;
    virtual int getClassId() { return ID_DATA_TYPE; }

    int getBaseDataType() { return _type; }
    const std::vector<ConstVal*>& getDimensions() { return _dimensions; }

    void setBaseDataType(BaseDataType type) { _type = type; }

    void addDimension(ConstVal* dimension) { _dimensions.push_back(dimension); }

    ACCEPT

private:
    BaseDataType _type;
    std::vector<ConstVal*> _dimensions;
};

}  // namespace ATC

#endif