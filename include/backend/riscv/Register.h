#pragma once

#include <set>
#include <string>
namespace ATC {
namespace RISCV {

class Register {
public:
    Register();

    void setName(const std::string &name) { _name = name; }
    void addInterference(Register *reg) { _interferences.insert(reg); }
    void setIsFixed(bool b) { _fixed = b; }

    const std::string &getName() { return _name; }
    const std::set<Register *> &getInterferences() { return _interferences; }
    bool isFixed() { return _fixed; }

private:
    static int Index;
    std::string _name;
    std::set<Register *> _interferences;
    bool _fixed = false;
};

}  // namespace RISCV
}  // namespace ATC