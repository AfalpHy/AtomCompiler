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

    const std::string &getName() { return _name; }
    const std::set<Register *> &getInterferences() { return _interferences; }

private:
    static int Index;

    std::string _name;

    std::set<Register *> _interferences;
};

}  // namespace RISCV
}  // namespace ATC