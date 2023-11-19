#ifndef COMP_UNIT_H
#define COMP_UNIT_H

#include <vector>

#include "TreeNode.h"
namespace ATC {
class CompUnit : public TreeNode {
public:
    CompUnit() = default;
    CompUnit(TreeNode* parent) : TreeNode(parent) {}

    virtual int getClassId() override { return ID_COMP_UNIT; }

    const std::vector<TreeNode*>& getElements() { return _elements; }

    void addElement(TreeNode* element) { _elements.push_back(element); }

    static std::vector<CompUnit*> AllCompUnits;

    ACCEPT

private:
    std::vector<TreeNode*> _elements;
};

}  // namespace ATC
#endif