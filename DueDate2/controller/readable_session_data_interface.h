#ifndef READABLE_SESSION_DATA_INTERFACE_H
#define READABLE_SESSION_DATA_INTERFACE_H

#include <string>
#include <memory>

namespace vm {

  class Command;

  class ViewInfo;

  class ReadableSessionDataInterface {
    public:
      enum inputMode { NORMAL, INSERT, REPLACE, REPLACE_ONCE, EX, SEARCH, RSEARCH,
        COMPOSE_DELETE, COMPOSE_YANK, COMPOSE_CHANGE, COMPOSE_FIND, COMPOSE_RFIND,
        SUPERCOMPOSE_FIND, SUPERCOMPOSE_RFIND, COMPOSE_MACRO_REG_CHOICE,
        COMPOSE_RUN_MACRO_REG_CHOICE, SUPERCOMPOSE_SEARCH, SUPERCOMPOSE_RSEARCH};

    private:
      virtual std::string doGetStatusLine() const = 0;
      virtual std::pair<size_t, size_t> doGetIdealCursor() const = 0;
      virtual std::pair<size_t, size_t> doGetCursor() const = 0;
      virtual std::string doGetExLine() const = 0;
      virtual std::string doGetSearchedString() const = 0;
      virtual inputMode doGetMode() const = 0;
      virtual inputMode doGetPreviousMode() const = 0;
      virtual bool doIsRecordingMacro() const = 0;
      virtual char doGetMacroChar() const = 0;
      virtual ViewInfo* doGetViewInfo() const = 0;
      virtual size_t doGetTopLine() const = 0;
      virtual std::unique_ptr<Command> doUndoTop() = 0;
      virtual std::string doGetFileName() const = 0;

    public:
      ReadableSessionDataInterface() {}
      ReadableSessionDataInterface(const ReadableSessionDataInterface&) = default;
      ReadableSessionDataInterface(ReadableSessionDataInterface&&) = default;
      ReadableSessionDataInterface& operator=(const ReadableSessionDataInterface&) = default;
      ReadableSessionDataInterface& operator=(ReadableSessionDataInterface&&) = default;
      virtual ~ReadableSessionDataInterface() = default;

      std::string getStatusLine() { return doGetStatusLine(); }
      std::pair<size_t, size_t> getIdealCursor() { return doGetIdealCursor(); }
      std::pair<size_t, size_t> getCursor() { return doGetCursor(); }
      std::string getExLine() { return doGetExLine(); }
      std::string getSearchedString() { return doGetSearchedString(); }
      inputMode getMode() { return doGetMode(); }
      inputMode getPreviousMode() { return doGetPreviousMode(); }
      bool isRecordingMacro() { return doIsRecordingMacro(); }
      char getMacroChar() { return doGetMacroChar(); }
      ViewInfo* getViewInfo() { return doGetViewInfo(); }
      std::unique_ptr<Command> undoTop() { return doUndoTop(); }
      std::string getFileName() { return doGetFileName(); }
      size_t getTopLine() const { return doGetTopLine(); }

      // Note that these do not use NVI because they are delegated to a class
      // that uses NVI
      virtual std::string getRegister(char) const = 0;
      virtual bool isReadOnly(char) const = 0;
      virtual bool isRegEmpty(char) const = 0;
      virtual std::string getLastUsedRegister() const = 0;
  };
}

#endif
