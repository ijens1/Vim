#include "register_manager.h"
#include "command.h"

std::string vm::RegisterManager::doGetRegister(char reg) const {
  return (isRegEmpty(reg)) ? "" : regs.at(reg);
}

bool vm::RegisterManager::doIsReadOnly(char reg) const {
  return false;
}

bool vm::RegisterManager::doIsRegEmpty(char reg) const {
  if (reg == '@') return regs.count(last_used_reg) == 0;
  return regs.count(reg) == 0;
}

std::string vm::RegisterManager::doGetLastUsedRegister() const {
  return (isRegEmpty(last_used_reg)) ? "" : regs.at(last_used_reg);
}

void vm::RegisterManager::doSetRegister(char reg, std::string str) {
  regs[reg] = str;
}

void vm::RegisterManager::doSetLastUsedRegister(char reg) {
  last_used_reg = reg;
}
