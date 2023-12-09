#pragma once
#include <list>
#include <string>

namespace ATC {
namespace RISCV_ARCH {

class Instruction;

class BasicBlock {
public:
    void setName(const std::string &name) { _name = name; }

    const std::string &getName() { return _name; }

    void addInstruction(Instruction *inst);

    const std::list<Instruction *> &getInstructionList() { return _instructions; }

private:
    std::string _name;
    std::list<Instruction *> _instructions;
};

}  // namespace RISCV_ARCH
}  // namespace ATC
