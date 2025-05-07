#ifndef REGISTER_MANAGER_H
#define REGISTER_MANAGER_H
#include "register_interface.h"
#include <map>

namespace vm {

  class RegisterManager : public RegisterInterface {
      std::map<char, std::string> regs;
      char last_used_reg;

      std::string doGetRegister(char) const override;
      bool doIsReadOnly(char) const override;
      bool doIsRegEmpty(char) const override;
      std::string doGetLastUsedRegister() const override;
      void doSetRegister(char, std::string) override;
      void doSetLastUsedRegister(char) override;

    public:

      RegisterManager() {}
      RegisterManager(const RegisterManager& other) : RegisterInterface{other}, regs{other.regs} {}
      RegisterManager(RegisterManager&& other) : RegisterInterface{std::move(other)}, regs{std::move(other.regs)} {}
      RegisterManager& operator=(const RegisterManager& other) {
        RegisterInterface::operator=(other);
        regs = other.regs;
        return *this;
      }
      RegisterManager& operator=(const RegisterManager&& other) {
        RegisterInterface::operator=(std::move(other));
        regs = std::move(other.regs);
        return *this;
      }
      ~RegisterManager() {}
  };

}

#endif
