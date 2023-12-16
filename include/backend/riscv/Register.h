#pragma once

#include <string>
namespace ATC {
namespace RISCV {

class Register {
public:
    Register() { _name = "virtual_reg" + std::to_string(Index++); }

    void setName(const std::string &name) { _name = name; }

    const std::string &getName() { return _name; }

    void setFixed() { _fixed = true; }

private:
    static int Index;

    bool _fixed = false;

    std::string _name;
};

}  // namespace RISCV
}  // namespace ATC