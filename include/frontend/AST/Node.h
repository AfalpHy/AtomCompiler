#ifndef NODE_H
#define NODE_H

#include <string>

#include "../../Common.h"

namespace ATC {
typedef struct {
    std::string _fileName;
    int _leftLine;
    int _leftColumn;
    int _rightLine;
    int _rightColumn;
} Position;

class Node {
public:
    Node() = default;

    virtual int getClassId() = 0;

    Node* getParent() { return _parent; }
    std::string getName() { return _name; }
    Position getPosition() { return _position; }

    void setName(std::string name) { _name = name; }
    void setPosition(Position position) { _position = position; }

private:
    Node* _parent = nullptr;
    std::string _name;
    Position _position;
};

}  // namespace ATC

#endif