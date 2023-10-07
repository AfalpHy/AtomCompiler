#ifndef COMP_UNIT_H
#define COMP_UNIT_H

#include <vector>

#include "Node.h"
namespace ATC {
class CompUnit : public Node {
public:
    CompUnit() = default;
    
    virtual int getClassId() override { return ID_COMP_UNIT; }

    const std::vector<Node*>& getElements() { return _elements; }

    void addElement(Node* element) { _elements.push_back(element); }

private:
    std::vector<Node*> _elements;
};

}  // namespace ATC
#endif