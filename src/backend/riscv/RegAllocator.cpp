#include "riscv/RegAllocator.h"

namespace ATC {

namespace RISCV {

void RegAllocator::run() {
    while (true) {
        // coalescing();
        buildInterference();
        if (!coloring()) {
            reset();
            spill();
        } else {
            break;
        }
    }
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
                    if (!dest->isFixed()) {
                        _theFunction->addNeedAllocReg(dest);
                    }
                    alives.erase(dest);
                }

                if (Register* src1 = inst->getSrc1()) {
                    for (auto reg : alives) {
                        if (reg == src1 || reg->isIntReg() != src1->isIntReg()) {
                            continue;
                        }
                        if (!src1->isFixed()) {
                            src1->addInterference(reg);
                        }
                        if (!reg->isFixed()) {
                            reg->addInterference(src1);
                        }
                    }
                    if (!src1->isFixed()) {
                        _theFunction->addNeedAllocReg(src1);
                    }
                    alives.insert(src1);
                }

                if (Register* src2 = inst->getSrc2()) {
                    for (auto reg : alives) {
                        if (reg == src2 || reg->isIntReg() != src2->isIntReg()) {
                            continue;
                        }
                        if (!src2->isFixed()) {
                            src2->addInterference(reg);
                        }
                        if (!reg->isFixed()) {
                            reg->addInterference(src2);
                        }
                    }
                    if (!src2->isFixed()) {
                        _theFunction->addNeedAllocReg(src2);
                    }
                    alives.insert(src2);
                }

                if (inst->getClassId() == ID_FUNCTION_CALL_INST) {
                    for (auto saved : Function::CallerSavedRegs) {
                        for (auto reg : alives) {
                            if (reg == saved || reg->isIntReg() != saved->isIntReg() || reg->isFixed()) {
                                continue;
                            }
                            reg->addInterference(saved);
                        }
                    }
                    FunctionCallInst* call = (FunctionCallInst*)inst;
                    // these reg across the function call
                    for (auto usedReg : call->getUsedRges()) {
                        alives.insert(usedReg);
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

bool RegAllocator::coloring() {
    // clang-format off
    std::vector<std::string> intPhyReg = {
        "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", 
        "t0", "t1", "t2", "t3", "t4", "t5", "t6",
        "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11"
    };
    std::vector<std::string> floatPhyReg = {
        "fa0", "fa1", "fa2", "fa3", "fa4", "fa5", "fa6", "fa7",  
        "ft0", "ft1", "ft2", "ft3", "ft4", "ft5", "ft6", "ft7", "ft8", "ft9", "ft10", "ft11",
        "fs0", "fs1", "fs2", "fs3", "fs4", "fs5", "fs6", "fs7", "fs8", "fs9", "fs10", "fs11"
    };

    std::set<std::string> calleeSavePhyReg = {
        "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", 
        "fs0", "fs1", "fs2", "fs3", "fs4", "fs5", "fs6", "fs7", "fs8", "fs9", "fs10", "fs11"
    };
    // clang-format on

    auto colorOneReg = [&](Register* reg, const std::vector<std::string>& phyRegs) {
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
                if (calleeSavePhyReg.find(phyReg) != calleeSavePhyReg.end()) {
                    for (auto calleeSaveReg : Function::CalleeSavedRegs) {
                        if (calleeSaveReg->getName() == phyReg) {
                            _theFunction->addNeedPushReg(calleeSaveReg);
                            break;
                        }
                    }
                }
                return true;
            }
        }
        return false;
    };

    for (auto reg : _theFunction->getNeedAllocRegs()) {
        bool success;
        if (reg->isIntReg()) {
            success = colorOneReg(reg, intPhyReg);
        } else {
            success = colorOneReg(reg, floatPhyReg);
        }
        if (!success) {
            _needSpill = reg;
            return false;
        }
    }
    return true;
}

void RegAllocator::spill() {
    std::list<Instruction*>::iterator spillPos;
    for (auto bb : _theFunction->getBasicBlocks()) {
        std::vector<std::list<Instruction*>::iterator> reloadPos;
        auto& instList = bb->getMutableInstructionList();
        spillPos = instList.end();
        for (auto begin = instList.begin(); begin != instList.end(); begin++) {
            auto inst = *begin;
            if (inst->getDest() == _needSpill) {
                _currentOffset -= 4;
                spillPos = begin;
            }

            if (inst->getSrc1() == _needSpill || inst->getSrc2() == _needSpill) {
                reloadPos.push_back(begin);
            }
        }
        if (spillPos != instList.end()) {
            instList.insert(++spillPos, new StoreInst(StoreInst::INST_SW, _needSpill, Register::S0, _currentOffset));
        }
        for (auto pos : reloadPos) {
            instList.insert(pos, new LoadInst(LoadInst::INST_LW, _needSpill, Register::S0, _currentOffset));
        }
    }

    _needSpill = nullptr;
}

void RegAllocator::reset() {
    for (auto bb : _theFunction->getBasicBlocks()) {
        bb->reset();
    }

    for (auto reg : _theFunction->getNeedAllocRegs()) {
        reg->reset();
    }
    _theFunction->getNeedAllocRegs().clear();
}

}  // namespace RISCV

}  // namespace ATC