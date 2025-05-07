#ifndef SESSION_DATA_INTERFACE_H
#define SESSION_DATA_INTERFACE_H

#include "readable_session_data_interface.h"
#include "ui.h"

namespace vm {

  class SessionDataInterface : public ReadableSessionDataInterface {
      virtual void doSetStatusLine(std::string) = 0;
      virtual void doSetIdealCursor(size_t, size_t) = 0;
      virtual void doSetCursor(size_t, size_t) = 0;
      virtual void doSetExLine(std::string) = 0;
      virtual void doSetSearchedString(std::string) = 0;
      virtual void doSetMode(inputMode) = 0;
      virtual void doSetPreviousMode(inputMode) = 0;
      virtual void doSetIsRecordingMacro(bool) = 0;
      virtual void doSetMacroChar(char) = 0;
      virtual void doSetViewInfo(ViewInfo*) = 0;
      virtual void doSetTopLine(size_t line) = 0;
      virtual void doStartUndoChunk() = 0;
      virtual void doPushUndo(std::unique_ptr<Command>) = 0;
      virtual void doPopUndo() = 0;
      virtual void doSetFileName(std::string) = 0;

    public:
      SessionDataInterface() {}
      SessionDataInterface(const SessionDataInterface& other) : ReadableSessionDataInterface{other} {}
      SessionDataInterface(SessionDataInterface&& other) : ReadableSessionDataInterface{std::move(other)} {}
      SessionDataInterface& operator=(const SessionDataInterface& other) {
        ReadableSessionDataInterface::operator=(other);
        return *this;
      }
      SessionDataInterface& operator=(SessionDataInterface&& other) {
        ReadableSessionDataInterface::operator=(std::move(other));
        return *this;
      }
      virtual ~SessionDataInterface() = default;

      void setStatusLine(std::string status_line) { doSetStatusLine(status_line); }
      void setIdealCursor(size_t l, size_t c) { doSetIdealCursor(l, c); }
      void setCursor(size_t l, size_t c) { doSetCursor(l, c); }
      void setExLine(std::string ex_line) { doSetExLine(ex_line); }
      void setSearchedString(std::string line) { doSetSearchedString(line); }
      void setMode(inputMode input_mode) { doSetMode(input_mode); }
      void setPreviousMode(inputMode previous_input_mode) { doSetPreviousMode(previous_input_mode); }
      void setIsRecordingMacro(bool is_recording_macro) { doSetIsRecordingMacro(is_recording_macro); }
      void setMacroChar(char macro_char) { doSetMacroChar(macro_char); }
      void setViewInfo(ViewInfo* v_i) { doSetViewInfo(v_i); }
      void startUndoChunk() { doStartUndoChunk(); }
      void pushUndo(std::unique_ptr<Command> c) { doPushUndo(std::move(c)); }
      void popUndo() { doPopUndo(); }
      void setFileName(std::string file_name) { doSetFileName(file_name); }
      void setTopLine(size_t line) { doSetTopLine(line); }

      // Again no NVI because the class which this method is delegated to
      // has NVI
      virtual void setRegister(char, std::string) = 0;
      virtual void setLastUsedRegister(char) = 0;
  };
}

#endif
