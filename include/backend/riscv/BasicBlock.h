#pragma once
#include <list>

#include "Instruction.h"

namespace ATC {
namespace RISCV {

class BasicBlock {
public:
    BasicBlock(const std::string &name) : _name(name) {}

    const std::string &getName() { return _name; }

    void addInstruction(Instruction *inst) { _instructions.push_back(inst); }

    const std::list<Instruction *> &getInstructionList() { return _instructions; }

    std::string toString() {
        std::string str;
        if (!_name.empty()) {
            str.append(_name + ":\n");
        }
        for (auto inst : _instructions) {
            str.append("\t").append(inst->toString()).append("\n");
        }
        return str;
    }

    static std::string getNewBBName() { return ".LBB" + std::to_string(Index++); }

private:
    static int Index;
    std::string _name;
    std::list<Instruction *> _instructions;
};

}  // namespace RISCV
}  // namespace ATC
