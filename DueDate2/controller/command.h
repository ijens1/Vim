#ifndef COMMAND_H
#define COMMAND_H

#include <string>
#include <iostream>
#include <vector>
#include <memory>
#include "ui.h"

using std::string;
using std::vector;

namespace vm {

  class SessionDataInterface;
  class BufferInterface;

  /**
   * TODO implement undos
  **/

  class Command {
    protected:
      SessionDataInterface* sd_i;

    public:
      static const int tabSize;
      Command(SessionDataInterface *sd_i) : sd_i{sd_i} {}
      virtual ~Command() {}
      virtual void execute(BufferInterface *) = 0;
  };

  class InsertUndoCommand : public Command {
      size_t l, c;
      std::string str_inserted;
      bool had_unsaved_changes;

    public:
      InsertUndoCommand(SessionDataInterface *sd_i, size_t l, size_t c, std::string str_inserted, bool had_unsaved_changes) : Command{sd_i}, l{l}, c{c}, str_inserted{str_inserted}, had_unsaved_changes{had_unsaved_changes} {}
      ~InsertUndoCommand() {}

      void execute(BufferInterface *) override;
  };

  class InsertCommand : public Command {
      size_t l, c;
      string str_to_insert;

    public:
      InsertCommand(SessionDataInterface *sd_i, size_t l, size_t c, string str_to_insert) : Command{sd_i}, l{l}, c{c}, str_to_insert{str_to_insert} {}
      ~InsertCommand() {}

      void execute(BufferInterface *) override;
  };

  class DeleteUndoCommand : public Command {
      // Note l should be line number of new conjoined line if removed new line
      // otherwise the line that the deletion happened on
      size_t length_of_prev_line, l, c;
      bool did_remove_new_line, had_unsaved_changes;
      std::string str_removed;
    
    public:
      DeleteUndoCommand(SessionDataInterface *sd_i, size_t length_of_prev_line, size_t l, size_t c, bool did_remove_new_line, bool had_unsaved_changes, std::string str_removed) : Command{sd_i}, length_of_prev_line{length_of_prev_line}, l{l}, c{c}, did_remove_new_line{did_remove_new_line}, str_removed{str_removed} {}

      void execute(BufferInterface *) override;
  };

  class DeleteCommand : public Command {
      size_t l, c, len;

    public:
      DeleteCommand(SessionDataInterface *sd_i, size_t l, size_t c, size_t len) : Command{sd_i}, l{l}, c{c}, len{len} {}
      ~DeleteCommand() {}

      void execute(BufferInterface *) override;
  };

  class MoveCursor : public Command {
      size_t x, y;

    public:
      MoveCursor(SessionDataInterface *sd_i, size_t y, size_t x) : Command{sd_i}, x{x}, y{y} {}
      void execute(BufferInterface *) override;
  };

  // Handles all the "ideal position" jazz
  class SmartMoveCursor : public Command {
      int x, y;
      bool restricted;
    public:
      SmartMoveCursor(SessionDataInterface *sd_i, int y, int x, bool r) : Command{sd_i}, x{x}, y{y}, restricted{r} {}
      void execute(BufferInterface *) override;
  };

  class RelativeMoveCursor : public Command {
      int x, y;
      bool restricted;// in insert mode, the cursor can go after the last character
                      // in the line, and is thus "unrestricted"

    public:
      RelativeMoveCursor(SessionDataInterface *sd_i, int y, int x, bool r) : Command{sd_i}, x{x}, y{y}, restricted{r} {}
      void execute(BufferInterface *) override;
  };
  
  class WrapMoveCursor : public Command {
    int x, y;   // Like RelativeMoveCursor, but with line wrap
    bool restricted;

    public:
      WrapMoveCursor(SessionDataInterface *sd_i, int y, int x, bool r) : Command{sd_i}, x{x}, y{y}, restricted{r} {}
      void execute(BufferInterface *) override;
  };

  // This command is given a string, and finds the next/previous character
  // from that string in this file, and moves the cursor there.
  class MoveFind : public Command {
      bool backward;       // Am I searching backwards?
      bool lineOnly;       // Am I restricting myself to one line only?
      string matches;

    public:
      MoveFind(SessionDataInterface *sd_i, string m, bool b) : Command{sd_i}, backward{b}, matches{m} {}
      void execute(BufferInterface *) override;
  };

  class JumpWord : public Command {
      bool backward;
      bool wsOnly;      // are we matching WORDS or just words?
      bool toEnd;       // if true, jump to last char in this word,
                        // else first char of next word. This is relative
                        // to backwards, ie backwards + toEnd goes to the front
                        // of the previous word
   
    public:
      JumpWord(SessionDataInterface *sd_i, bool b, bool w, bool e) : Command{sd_i}, backward{b}, wsOnly{w}, toEnd{e} {}
      void execute(BufferInterface *) override;
  };

  class InsertCursor : public Command {
      string theString;
    
    public:
      InsertCursor(SessionDataInterface *sd_i, string s) : Command{sd_i}, theString{s} {}
      void execute(BufferInterface *) override;
  };

  class Paste : public Command {
    public:
      Paste(SessionDataInterface *sd_i) : Command{sd_i} {}
      void execute(BufferInterface *) override;
  };

  class DeleteCursor : public Command {
      bool behind;
      size_t num;
      bool delN; // will this delete a \n?
      bool restricted;

    public:
      DeleteCursor(SessionDataInterface *sd_i, bool b, size_t n, bool dn, bool r) : Command{sd_i}, behind{b},
        num{n}, delN{dn}, restricted{r} {}
      void execute(BufferInterface *) override;
  };

  class YankCursor : public Command {
      bool behind;
      size_t num;
      bool delN; // will this delete a \n?
      bool restricted;
      bool move;
      char yanked;

    public:
      YankCursor(SessionDataInterface *sd_i, bool b, size_t n, bool dn, bool r, bool m) : Command{sd_i}, behind{b},
        num{n}, delN{dn}, restricted{r}, move{m}, yanked{0} {}
      void execute(BufferInterface *) override;
      char getYanked() { return yanked; }
  };

  class CommandSequence : public Command {
      vector<std::unique_ptr<Command>> cs;

    public:
      CommandSequence(SessionDataInterface *sd_i) : Command{sd_i}, cs{} {}
      void execute(BufferInterface *) override;
      void add(std::unique_ptr<Command> &&c) {
        cs.push_back(std::move(c));
      }
  };

  class JumpLineEnd : public Command {
      bool start;       // end or start?
      bool restricted;

    public:
      JumpLineEnd(SessionDataInterface *sd_i, bool s, bool r) : Command{sd_i}, start{s}, restricted{r} {}
      void execute(BufferInterface *) override;
  };

  class SkipWS : public Command {
    public:
      SkipWS(SessionDataInterface *sd_i) : Command{sd_i} {}
      void execute(BufferInterface *) override;
  };

  class WakeUp : public Command {
    public:
      WakeUp(SessionDataInterface *sd_i) : Command{sd_i} {}
      void execute(BufferInterface *) override {};
  };

  class ReadFile : public Command {
      std::string file_name;
      size_t l, c;
      bool is_initial_file_read;

    public:
      ReadFile(SessionDataInterface *sd_i, std::string file_name, bool is_initial_file_read, size_t l, size_t c) : Command{sd_i}, file_name{file_name}, l{l}, c{c}, is_initial_file_read{is_initial_file_read} {}
      void execute(BufferInterface *) override;
  };

  class ReadFileBelowCursor : public Command {
      std::string file_name;
    public:
      ReadFileBelowCursor(SessionDataInterface *sd_i, std::string file_name) : Command{sd_i}, file_name{file_name} {}
      void execute(BufferInterface *) override;
  };

  class Quit : public Command {
      bool q_override;
    public:
      Quit(SessionDataInterface *sd_i, bool q_override) : Command{sd_i}, q_override{q_override} {}
      void execute(BufferInterface *) override;
  };

  class WriteBuffer : public Command {
      std::string file_name;
    public:
      WriteBuffer(SessionDataInterface *sd_i, std::string file_name) : Command{sd_i}, file_name{file_name} {}
      void execute(BufferInterface *) override;
  };

  class MetaJumpToLine : public Command {
      size_t line_number;
    public:
      MetaJumpToLine(SessionDataInterface *sd_i, size_t line_number) : Command{sd_i}, line_number{line_number} {}
      void execute(BufferInterface *) override;
  };

  class MetaJumpToLastLine : public Command {
    public:
      MetaJumpToLastLine(SessionDataInterface *sd_i) : Command{sd_i} {}
      void execute(BufferInterface *) override;
  };

  class Undefined : public Command {
    public:
      Undefined(SessionDataInterface *sd_i) : Command{sd_i} {}
      void execute(BufferInterface *) override;
  };

  class MotionUsingCommand : public Command {
    public:
      MotionUsingCommand(SessionDataInterface *sd_i) : Command{sd_i} {}
      void execute(BufferInterface *) = 0;
      virtual void executeWithMotion(BufferInterface *,
          std::pair<size_t, size_t> firstCursorPos,
          std::pair<size_t, size_t> secondCursorPos) = 0;
  };

  class MotionBoundCommand : public Command {
      std::unique_ptr<MotionUsingCommand> command;
      std::unique_ptr<Command> motion;
    public:
      MotionBoundCommand(SessionDataInterface *sd_i, std::unique_ptr<MotionUsingCommand> &&command,
          std::unique_ptr<Command> &&motion) : Command{sd_i}, command{std::move(command)}, motion{std::move(motion)} {}
      void execute(BufferInterface *);
  };

  class MultipliedCommand : public Command {
      unsigned int multiplier;
      std::unique_ptr<Command> command;
    public:
      MultipliedCommand(SessionDataInterface *sd_i, unsigned int m, std::unique_ptr<Command> &&command)
          : Command{sd_i}, multiplier{std::move(m)}, command{std::move(command)} {}
      void execute(BufferInterface *);
  };

  class MotionDelete : public MotionUsingCommand {
      bool behind, inclusive, yank;
    public:
      MotionDelete(SessionDataInterface *sd_i, bool b, bool i, bool y) : MotionUsingCommand{sd_i}, behind{b}, inclusive{i}, yank{y} {};
      void execute(BufferInterface *) override;
      virtual void executeWithMotion(BufferInterface *,
          std::pair<size_t, size_t> firstCursorPos,
          std::pair<size_t, size_t> secondCursorPos) override;
  };

  class BkspIfLastLine : public Command {
    public:
      BkspIfLastLine(SessionDataInterface *sd_i) : Command{sd_i} {}
      void execute(BufferInterface *) override;
  };

  class Scroll : public Command {
      int amount;
      bool frame, bound;
    public:
      Scroll(SessionDataInterface *sd_i, int amount, bool frame,
          bool bound) : Command{sd_i}, amount{amount},
          frame{frame}, bound{bound} {}
      void execute(BufferInterface *) override;
  };

  class Search : public Command {
      string searchStr;
      bool dir;
    public:
      Search(SessionDataInterface *sd_i, string s, bool d)
        : Command{sd_i}, searchStr{s}, dir{d} {}
      void execute(BufferInterface *) override;
  };

  class FindMatching : public Command {
    public:
      FindMatching(SessionDataInterface *sd_i) : Command{sd_i} {}
      void execute(BufferInterface *) override;
  };

  class RepeatCommand : public Command {
      unsigned int n;
    public:
      RepeatCommand(SessionDataInterface *sd_i, unsigned int n)
        : Command{sd_i}, n{n} {}
      void execute(BufferInterface *) override {}
      unsigned int numTimes() { return n; }
  };

  class PutFileInfo : public Command {
    public:
      PutFileInfo(SessionDataInterface *sd_i) : Command{sd_i} {}
      void execute(BufferInterface *) override;
  };

}

#endif
