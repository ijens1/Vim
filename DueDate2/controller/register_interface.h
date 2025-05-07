#ifndef REGISTER_INTERFACE_H
#define REGISTER_INTERFACE_H
#include "readable_register_interface.h"

namespace vm {

  class RegisterInterface : public ReadableRegisterInterface {
      virtual void doSetRegister(char, std::string) = 0;
      virtual void doSetLastUsedRegister(char) = 0;

    public:
      RegisterInterface() {}
      RegisterInterface(const RegisterInterface& other) : ReadableRegisterInterface{other} {}
      RegisterInterface(RegisterInterface&& other) : ReadableRegisterInterface{std::move(other)} {}
      RegisterInterface& operator=(const RegisterInterface& other) {
        ReadableRegisterInterface::operator=(other);
        return *this;
      }
      RegisterInterface& operator=(RegisterInterface&& other) {
        ReadableRegisterInterface::operator=(std::move(other));
        return *this;
      }
      virtual ~RegisterInterface() = default;

      void setRegister(char reg, std::string str) { doSetRegister(reg, str); }
      void setLastUsedRegister(char reg) { doSetLastUsedRegister(reg); }
  };

}


#endif
