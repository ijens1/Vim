#ifndef READABLE_REGISTER_INTERFACE_H
#define READABLE_REGISTER_INTERFACE_H

#include <string>

namespace vm {

  class ReadableRegisterInterface {
      virtual std::string doGetRegister(char) const = 0;
      virtual bool doIsReadOnly(char) const = 0;
      virtual bool doIsRegEmpty(char) const = 0;
      virtual std::string doGetLastUsedRegister() const = 0;

    public:
      ReadableRegisterInterface() {}
      ReadableRegisterInterface(const ReadableRegisterInterface&) = default;
      ReadableRegisterInterface(ReadableRegisterInterface&&) = default;
      ReadableRegisterInterface& operator=(const ReadableRegisterInterface&) = default;
      ReadableRegisterInterface& operator=(ReadableRegisterInterface&&) = default;
      virtual ~ReadableRegisterInterface() = default;

      std::string getRegister(char reg) const { return doGetRegister(reg); }
      bool isReadOnly(char reg) const { return doIsReadOnly(reg); }
      bool isRegEmpty(char reg) const { return doIsRegEmpty(reg); }
      std::string getLastUsedRegister() const { return doGetLastUsedRegister(); }
  };
}

#endif
