#pragma once

#include <string>
namespace ATC {
namespace RISCV_ARCH {

class Register {
public:
    Register(const std::string &name) : _name(name) {}

    void setName(const std::string &name) { _name = name; }

    const std::string &getName() { return _name; }

private:
    std::string _name;
};

}  // namespace RISCV_ARCH
}  // namespace ATC