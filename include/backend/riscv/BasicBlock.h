#pragma once
#include <string>

namespace ATC {
namespace RISCV_ARCH {

class BasicBlock {
public:
    void setName(const std::string &name) { _name = name; }

    const std::string &getName() { return _name; }

private:
    std::string _name;
};

}  // namespace RISCV_ARCH
}  // namespace ATC
