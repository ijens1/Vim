#ifndef SESSION_DATA_MANAGER_H
#define SESSION_DATA_MANAGER_H

#include <stack>
#include "session_data_interface.h"
#include "register_manager.h"

namespace vm {

  class SessionDataManagerCopyNotImplemented{};

  class SessionDataManager : public SessionDataInterface {
      std::stack<std::pair<std::unique_ptr<Command>, int>> undo_s;
      std::unique_ptr<vm::RegisterInterface> r_m = std::make_unique<vm::RegisterManager>();
      std::string status_line, ex_line, searchedString;
      std::pair<size_t, size_t> ideal_cursor;
      std::pair<size_t, size_t> cursor;
      inputMode mode;
      inputMode previous_mode;
      bool is_recording_macro;
      char macro_char;
      ViewInfo* v_i;
      size_t frameTopLine;
      int undo_chunk;
      std::string file_name;

      std::string doGetStatusLine() const override { return status_line; }
      std::pair<size_t, size_t> doGetIdealCursor() const override { return ideal_cursor; }
      std::pair<size_t, size_t> doGetCursor() const override { return cursor; }
      std::string doGetExLine() const override { return ex_line; }
      std::string doGetSearchedString() const override { return searchedString; }
      inputMode doGetMode() const override { return mode; }
      inputMode doGetPreviousMode() const override { return previous_mode; }
      bool doIsRecordingMacro() const override { return is_recording_macro; }
      char doGetMacroChar() const override { return macro_char; }
      ViewInfo* doGetViewInfo() const override { return v_i; }
      std::unique_ptr<Command> doUndoTop() override;
      std::string doGetFileName() const override { return file_name; }

      void doSetStatusLine(std::string status_line) override { this->status_line = status_line; }
      void doSetIdealCursor(size_t l, size_t c) override  { ideal_cursor = std::make_pair(l, c); }
      void doSetCursor(size_t l, size_t c) override { cursor = std::make_pair(l, c); }
      void doSetExLine(std::string ex_line) override { this->ex_line = ex_line; }
      void doSetSearchedString(std::string line) override { searchedString = line; }
      void doSetMode(inputMode mode) override { this->mode = mode; }
      void doSetPreviousMode(inputMode previous_mode) override { this->previous_mode = previous_mode; }
      void doSetIsRecordingMacro(bool is_recording_macro) override { this->is_recording_macro = is_recording_macro; }
      void doSetMacroChar(char macro_char) { this->macro_char = macro_char; }
      void doSetViewInfo(ViewInfo* v_i) override { this->v_i = v_i; }

      size_t doGetTopLine() const override { return frameTopLine; }
      void doSetTopLine(size_t line) override { frameTopLine = line; }
      void doStartUndoChunk() override { undo_chunk++; }
      void doPushUndo(std::unique_ptr<Command> c) override { undo_s.push(std::make_pair(std::move(c), undo_chunk)); }
      void doPopUndo() override { undo_chunk--; }
      void doSetFileName(std::string file_name) override { this->file_name = file_name; }

    public:
      SessionDataManager() : ideal_cursor{std::make_pair(1, 0)}, cursor{std::make_pair(1, 0)}, frameTopLine{1}, undo_chunk{0} {}
      SessionDataManager(const SessionDataManager& other) = default;
      SessionDataManager(SessionDataManager&& other) : SessionDataInterface{std::move(other)}, r_m{std::move(other.r_m)}, status_line{std::move(other.status_line)}, ex_line{std::move(other.ex_line)}, ideal_cursor{std::move(other.ideal_cursor)}, cursor{std::move(other.cursor)}, mode{std::move(other.mode)}, previous_mode{std::move(other.previous_mode)}, is_recording_macro{std::move(other.is_recording_macro)}, macro_char{std::move(other.macro_char)}, v_i{std::move(other.v_i)} {}
      SessionDataManager& operator=(const SessionDataManager& other) = default;
      SessionDataManager& operator=(SessionDataManager&& other) {
        SessionDataInterface::operator=(std::move(other));
        r_m = std::move(other.r_m);
        status_line = std::move(other.status_line);
        ex_line = std::move(other.ex_line);
        ideal_cursor = std::move(other.ideal_cursor);
        cursor = std::move(other.cursor);
        mode = std::move(other.mode);
        previous_mode = std::move(other.previous_mode);
        is_recording_macro = std::move(other.is_recording_macro);
        macro_char = std::move(other.macro_char);
        v_i = std::move(other.v_i);
        return *this;
      }
      ~SessionDataManager() {}

      std::string getRegister(char reg) const override { return r_m->getRegister(reg); }
      bool isReadOnly(char reg) const override { return r_m->isReadOnly(reg); }
      bool isRegEmpty(char reg) const override { return r_m->isRegEmpty(reg); }
      std::string getLastUsedRegister() const override { return r_m->getLastUsedRegister(); }
      void setRegister(char reg, std::string str) override { r_m->setRegister(reg, str); }
      void setLastUsedRegister(char reg) override { r_m->setLastUsedRegister(reg); }
  };
}

#endif
