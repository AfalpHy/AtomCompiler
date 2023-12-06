#pragma once

#include <string>
#include <vector>

namespace ATC {

namespace RISCV_ARCH {

class BasicBlock;
class Module;

class Function {
public:
    const std::string& getContent() { return _content; }

private:
    std::vector<BasicBlock*> _basicBlocks;
    std::string _content;
};

}  // namespace RISCV_ARCH
}  // namespace ATC