#include "riscv/RegAllocator.h"

namespace ATC {

namespace RISCV {

void RegAllocator::run() {
    buildInterference();
    coloring();
}

void RegAllocator::buildInterference() {
    bool update;
    do {
        update = false;
        for (auto bb : _theFunction->getBasicBlocks()) {
            std::set<Register*> alives;
            for (auto succ : bb->getSuccessors()) {
                alives.insert(succ->getAlives().begin(), succ->getAlives().end());
            }

            for (auto rbegin = bb->getInstructionList().rbegin(); rbegin != bb->getInstructionList().rend(); rbegin++) {
                auto inst = *rbegin;
                if (Register* dest = inst->getDest()) {
                    alives.erase(dest);
                }

                if (Register* src1 = inst->getSrc1()) {
                    for (auto reg : alives) {
                        if (reg == src1 || reg->isIntReg() != src1->isIntReg()) {
                            continue;
                        }
                        src1->addInterference(reg);
                        reg->addInterference(src1);
                    }
                    alives.insert(src1);
                }

                if (Register* src2 = inst->getSrc2()) {
                    for (auto reg : alives) {
                        if (reg == src2 || reg->isIntReg() != src2->isIntReg()) {
                            continue;
                        }
                        src2->addInterference(reg);
                        reg->addInterference(src2);
                    }
                    alives.insert(src2);
                }

                if (inst->getClassId() == ID_FUNCTION_CALL_INST) {
                    for (auto saved : Function::CallerSavedRegs) {
                        for (auto reg : alives) {
                            if (reg == saved || reg->isIntReg() != saved->isIntReg()) {
                                continue;
                            }
                            reg->addInterference(saved);
                        }
                    }
                }
            }
            if (bb->getAlives() != alives) {
                update = true;
                bb->setAlives(alives);
            }
        }
    } while (update);
}

void RegAllocator::coloring() {
    std::set<std::string> intPhyReg = {"a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7"};
    std::set<std::string> floatPhyReg = {"fa0", "fa1", "fa2", "fa3", "fa4", "fa5", "fa6", "fa7"};

    auto colorOneReg = [](Register* reg, const std::set<std::string>& phyRegs) {
        for (auto phyReg : phyRegs) {
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
    };

    for (auto reg : Function::AllRegInFunction) {
        if (reg->isFixed()) {
            continue;
        }
        if (reg->isIntReg()) {
            colorOneReg(reg, intPhyReg);
        } else {
            colorOneReg(reg, floatPhyReg);
        }
    }
}

}  // namespace RISCV

}  // namespace ATC