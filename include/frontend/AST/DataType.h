#pragma once

#include "TreeNode.h"

namespace ATC {

class Expression;
class DataType : public TreeNode {
public:
    DataType() = default;
    DataType(TreeNode* parent) : TreeNode(parent) {}

    virtual int getBasicType() = 0;
    virtual DataType* getBaseDataType() { return nullptr; }
};

class BasicType : public DataType {
public:
    BasicType() = default;
    BasicType(TreeNode* parent) : DataType(parent) {}

    virtual int getClassId() override { return ID_BASIC_TYPE; }

    enum Type { UNKOWN, VOID, BOOL, INT, FLOAT };
    void setType(Type type) { _type = type; }

    virtual int getBasicType() { return _type; }

    ACCEPT

private:
    Type _type = UNKOWN;
};

class ArrayType : public DataType {
public:
    ArrayType() = default;
    ArrayType(TreeNode* parent) : DataType(parent) {}

    virtual int getClassId() override { return ID_ARRAY_TYPE; }

    const std::vector<Expression*>& getDimensionExprs() { return _dimensionExprs; }
    const std::vector<int>& getDimensions() { return _dimensions; }
    const std::vector<int>& getElementSize() { return _elementSize; }
    int getTotalSize() { return _totalSize; }
    virtual DataType* getBaseDataType() override { return _baseDataType; }

    void addDimensionExpr(Expression* dimension) { _dimensionExprs.push_back(dimension); }
    void addDimension(int dimension) { _dimensions.push_back(dimension); }
    void setElementSize(const std::vector<int>& elementSize) { _elementSize = elementSize; }
    void setTotalSize(int size) { _totalSize = size; }
    void setBaseDataType(DataType* dataType) { _baseDataType = dataType; }

    virtual int getBasicType() { return _baseDataType->getBasicType(); }

    ACCEPT

private:
    std::vector<Expression*> _dimensionExprs;
    std::vector<int> _dimensions;
    // Save the number of elements represented by each dimension
    std::vector<int> _elementSize;
    int _totalSize = 0;
    DataType* _baseDataType = nullptr;
};

class PointerType : public DataType {
public:
    PointerType() = default;
    PointerType(TreeNode* parent) : DataType(parent) {}

    virtual int getClassId() override { return ID_POINTER_TYPE; }

    virtual DataType* getBaseDataType() override { return _baseDataType; }

    void setBaseDataType(DataType* dataType) { _baseDataType = dataType; }

    virtual int getBasicType() { return _baseDataType->getBasicType(); }

    ACCEPT

private:
    DataType* _baseDataType = nullptr;
};

}  // namespace ATC
