#pragma once

#include <string>
#include <vector>

namespace ATC {

namespace RISCV_ARCH {

class Function;

class Module {
public:
    const std::string& getContent() { return _content; }

private:
    std::vector<Function*> _functions;
    std::string _content;
};

}  // namespace RISCV_ARCH
}  // namespace ATC