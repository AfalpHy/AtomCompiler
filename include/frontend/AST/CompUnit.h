#pragma once

#include <vector>

#include "TreeNode.h"
namespace ATC {
class CompUnit : public TreeNode {
public:
    CompUnit() = default;

    virtual int getClassId() override { return ID_COMP_UNIT; }

    const std::vector<TreeNode*>& getElements() { return _elements; }

    void addElement(TreeNode* element) { _elements.push_back(element); }

    ACCEPT

private:
    std::vector<TreeNode*> _elements;
};

}  // namespace ATC
