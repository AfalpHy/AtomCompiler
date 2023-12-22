#include "riscv/RegAllocator.h"

namespace ATC {

namespace RISCV {

void RegAllocator::run() {
    buildInterference();
    coloring();
}

void RegAllocator::buildInterference() {
    for (auto bb : _theFunction->getBasicBlocks()) {
        analyzeRegAlive(bb);
    }
}

void RegAllocator::coloring() {
    std::set<std::string> intPhyReg = {"a1", "a2", "a3", "a4", "a5"};
    for (auto reg : Function::AllRegInFunction) {
        for (auto phyReg : intPhyReg) {
            bool conflict = false;
            for (auto interference : reg->getInterferences()) {
                if (interference->getName() == phyReg) {
                    conflict = true;
                    break;
                }
            }
            if (!conflict) {
                reg->setName(phyReg);
                break;
            }
        }
    }
}

void RegAllocator::analyzeRegAlive(BasicBlock* bb) {
    if (bb->isAnalyzed()) {
        return;
    }
    std::set<Register*> alive;
    for (auto succ : bb->getSuccessors()) {
        analyzeRegAlive(succ);
        alive.insert(succ->getAlives().begin(), succ->getAlives().end());
    }

    for (auto rbegin = bb->getInstructionList().rbegin(); rbegin != bb->getInstructionList().rend(); rbegin++) {
        auto inst = *rbegin;
        if (Register* dest = inst->getDest()) {
            alive.erase(dest);
        }

        if (Register* src1 = inst->getSrc1()) {
            for (auto reg : alive) {
                src1->addInterference(reg);
                reg->addInterference(src1);
            }
            alive.insert(src1);
        }

        if (Register* src2 = inst->getSrc2()) {
            for (auto reg : alive) {
                src2->addInterference(reg);
                reg->addInterference(src2);
            }
            alive.insert(src2);
        }
    }

    bb->setAlive(alive);
    bb->setAnalyzed(true);
}
}  // namespace RISCV

}  // namespace ATC