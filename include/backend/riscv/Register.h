#pragma once

#include <set>
#include <string>
namespace ATC {
namespace RISCV {

class Instruction;
class Register {
public:
    Register(Instruction *defined = nullptr, bool b = true);

    void setName(const std::string &name) { _name = name; }
    void addInterference(Register *reg) { _interferences.insert(reg); }
    void setIsFixed(bool b) { _fixed = b; }
    void setSpillOffset(int offset) { _spillOffset = offset; }
    void setSpilled() { _spilled = true; }

    const std::string &getName() { return _name; }
    const std::set<Register *> &getInterferences() { return _interferences; }
    Instruction *getDefined() { return _defined; }
    bool isIntReg() { return _intReg; }
    bool isFixed() { return _fixed; }
    int getSpillOffset() { return _spillOffset; }
    bool isSpilled() { return _spilled; }
    void reset();

    // common regs
    static Register *Ra;
    static Register *S0;
    static Register *Sp;
    static Register *Zero;

    static std::vector<Register *> IntArgReg;
    static std::vector<Register *> FloatArgReg;

private:
    static int Index;
    std::string _name;
    std::set<Register *> _interferences;
    Instruction *_defined;
    bool _intReg;
    bool _fixed = false;
    int _spillOffset;
    bool _spilled = false;
};

}  // namespace RISCV
}  // namespace ATC