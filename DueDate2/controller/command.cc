#include "command.h"
#include "buffer_interface.h"
#include "session_data_interface.h"
#include <iostream>
#include <fstream>

namespace vm {

  void InsertUndoCommand::execute(BufferInterface *b_i) {
    int num_new_lines_in_string = 0;
    size_t eol_index = 0;
    std::string str_copy = str_inserted;
    eol_index = str_copy.find(BufferInterface::end_of_line_expr);
    while (eol_index != std::string::npos) {
      num_new_lines_in_string++;
      str_copy = str_copy.substr(eol_index + 1);
      eol_index = str_copy.find(BufferInterface::end_of_line_expr);
    }
    int num_intermediate_lines_to_remove = num_new_lines_in_string - 1, removed_count = 0;
    for (int i = 0; i < num_intermediate_lines_to_remove; ++i) {
      removed_count += b_i->getLine(l + 1).length();
      b_i->deleteText(l + 1, 1, std::string::npos);
      removed_count++;
      b_i->deleteText(l + 1, 0, -1);
    }
    if (str_inserted.length() - removed_count > b_i->getLine(l).length() - (c - 1)) {
      b_i->deleteText(l + 1, 1, str_inserted.length() - removed_count - 1);
      b_i->deleteText(l, c, std::string::npos);
      b_i->deleteText(l + 1, 0, -1);
    } else {
      b_i->deleteText(l, c, str_inserted.length() - removed_count);
    }
    sd_i->setCursor(l, c-1);
    b_i->setHasUnsavedChanges(had_unsaved_changes);
  }

  void InsertCommand::execute(BufferInterface *b_i) {
    sd_i->pushUndo(std::make_unique<InsertUndoCommand>(sd_i, l, c, str_to_insert, b_i->hasUnsavedChanges()));
    b_i->insertText(l, c, str_to_insert);
    b_i->setHasUnsavedChanges(true);
  }

  void DeleteUndoCommand::execute(BufferInterface *b_i) {
    if (did_remove_new_line) {
      b_i->insertText(l, length_of_prev_line + 1, BufferInterface::end_of_line_expr);
      sd_i->setCursor(l + 1, 0);
    } else {
      b_i->insertText(l, c, str_removed);
      sd_i->setCursor(l, c);
    }
    b_i->setHasUnsavedChanges(had_unsaved_changes);
  }

  void DeleteCommand::execute(BufferInterface *b_i) {
    // Note it doesn't matter what str_removed is if c == 0 in the
    // DeleteUndoCommand.execute
    std::string str_removed = (c == 0) ? "" : b_i->getLine(l).substr(c - 1, len);
    size_t l_undo = (c == 0) ? l - 1 : l;
    sd_i->pushUndo(std::make_unique<DeleteUndoCommand>(sd_i, b_i->getLine(l - 1).length(), l_undo, c, (c == 0), b_i->hasUnsavedChanges(), str_removed));
    b_i->deleteText(l, c, len);
    b_i->setHasUnsavedChanges(true);
  }

  void MoveCursor::execute(BufferInterface *b_i) {
    if (y >= 1 && y <= b_i->getNumLines()
        && x >= 0 && x <= b_i->getLine(y).length()) {
        sd_i->setCursor(y, x);
    }
  }

  int tabCount(const string &t) {
      int c = 0;
      for (auto &i : t) {
          if (i == '\t') {
              ++c;
          }
      }
      return c;
  }

  const int Command::tabSize = 8;

  int getVisualXFromReal(int p, const string &t, int width, int tabSize) {
    int k=0;
    for (size_t i=0; i<t.length(); ++i) {
      if (p <= 0) break;
      if (t[i] == '\t') {
        int j = k % width;
        if (j <= width - (width % tabSize)) {
          k += tabSize - (j % tabSize);
        } else {
          k += tabSize - (width % tabSize);
        }
      } else {
        ++k;
      }
      --p;
    }
    return k;
  }

  int getRealXFromVisual(int p, const string &t, int width, int tabSize) {
    int k=0;
    int q=0;
    for (size_t i=0; i<t.length(); ++i) {
      if (k >= p) break;
      if (t[i] == '\t') {
        int j = k % width;
        if (j <= width - (width  % tabSize)) {
          k += tabSize - (j % tabSize);
        } else {
          k += tabSize - (width % tabSize);
        }
      } else {
        ++k;
      }
      ++q;
    }
    return q;
  }

  void SmartMoveCursor::execute(BufferInterface *b_i) {
    int r = (restricted) ? 1 : 0;
    std::pair<size_t, size_t> yx = sd_i->getCursor();
    std::pair<size_t, size_t> iyx = sd_i->getIdealCursor();
    int iX = iyx.second;  // "ideal" x -- lets you go from a long line to
                          // a short line and then to a long line and not lose
                          // your posn
    int width = sd_i->getViewInfo()->getWidth();

    // Finds the correct position for the cursor, depending on:
    //    - Whether or not you're in insert mode and can go after
    //      the end of the line
    //    - The cached cursor position the last time the x position was
    //      modified
    //    - The current position of the cursor
    //    - The input

    int oX = yx.second;         // Original x & y
                                //-- so you know whether you're moving or not.
    int oY = yx.first;          // iX is reset depending on these
    string line1 = b_i->getLine(oY);
    string line2 = b_i->getLine(y);

    int voX = getVisualXFromReal(x, line1, width, tabSize);
    int rX = getRealXFromVisual(voX, line2, width, tabSize);
    int riX = getRealXFromVisual(iX, line2, width, tabSize);


    size_t l;
    if (line2.length() > 0) {
      l = b_i->getLine(y).length()-r;
    } else {
      l = 0;
    }

    if (oX != x && oY == y) {
      iX = voX;
    } else {
      rX = (static_cast<size_t>(riX) < l) ? riX : l;
    }

    sd_i->setIdealCursor(y, iX);
    MoveCursor(sd_i, y, rX).execute(b_i);
  }

  void RelativeMoveCursor::execute(BufferInterface *b_i) {
    int r = (restricted) ? 1 : 0;
    std::pair<size_t, size_t> yx = sd_i->getCursor();
    int cX = yx.second;   // current x/y
    int cY = yx.first;

    cY += y;
    if (cY < 1)
      cY = 1;
    else if (static_cast<size_t>(cY) > b_i->getNumLines())
      cY = b_i->getNumLines();

    size_t l = b_i->getLine(cY).length()-r;
    cX += x;
    if (cX < 0)
      cX = 0;
    if (static_cast<size_t>(cX) > l) {
      cX = l;
    }

    SmartMoveCursor(sd_i, cY, cX, restricted).execute(b_i);
  }

  void WrapMoveCursor::execute(BufferInterface *b_i) {
    int r = (restricted) ? 1 : 0;
    std::pair<size_t, size_t> yx = sd_i->getCursor();
    int cX = yx.second;   // current x/y
    int cY = yx.first;

    cY += y;
    if (cY < 1)
      cY = 1;
    else if (static_cast<size_t>(cY) > b_i->getNumLines())
      cY = b_i->getNumLines();

    int l = b_i->getLine(cY).length()-r;
    if (l < 0) l = 0;
    if (cX < 0)
      cX = 0;
    if (cX > l) {
      cX = l;
    }

    while (x < 0) {
      --cX;
      ++x;
      if (cX < 0) {
        --cY;
        if (cY < 1) {
          cY = 1;
          cX = 0;
        } else {
          l = b_i->getLine(cY).length()-r;
          if (l < 0) l = 0;
          cX = l;
        }
      }
    }

    while (x > 0) {
      ++cX;
      --x;
      if (cX > l) {
        ++cY;
        if (static_cast<size_t>(cY) > b_i->getNumLines()) {
          cY = b_i->getNumLines();
          l = b_i->getLine(cY).length()-r;
          if (l < 0) l = 0;
          cX = l;
        } else {
          l = b_i->getLine(cY).length()-r;
          if (l < 0) l = 0;
          cX = 0;
        }
      }
    }

    sd_i->setIdealCursor(cY, cX);
    MoveCursor(sd_i, cY, cX).execute(b_i);
  }

  void MoveFind::execute(BufferInterface *b_i) {
    std::pair<size_t, size_t> yx = sd_i->getCursor();
    int lineNum = yx.first;
    int charNum = yx.second;
    if (!backward) {
      string line = b_i->getLine(lineNum);
      size_t p = line.substr(charNum+1, string::npos).find_first_of(matches);
      if (p != string::npos) {
        SmartMoveCursor(sd_i, lineNum, charNum+1+p, true).execute(b_i);
      }
    } else {
      string line = b_i->getLine(lineNum);
      size_t p = line.substr(0, charNum).find_last_of(matches);
      if (p != string::npos) {
        SmartMoveCursor(sd_i, lineNum, p, true).execute(b_i);
      }
    }
  }

  void JumpWord::execute(BufferInterface *b_i) {
    int increment = (backward) ? -1 : 1;
    bool startSpace = false;
    bool startWord = false;
    bool startPunc = false;
    bool space = false;
    bool word = false;
    bool punc = false;
    string line;
    std::pair<size_t, size_t> yx = sd_i->getCursor();

    if (toEnd) {
      WrapMoveCursor(sd_i, 0, increment, true).execute(b_i);
      yx = sd_i->getCursor();
      line = b_i->getLine(yx.first);
      if (line.length() == 0) return;
    }


    yx = sd_i->getCursor();
    line = b_i->getLine(yx.first);
    char start = line[yx.second];
    if (('0' <= start && start <= '9') || ('A' <= start && start <= 'Z')
        || ('a' <= start && start <= 'z') || start == '_') {
      startWord = true;
      word = true;
    } else if (32 == start || 9 == start) {
      startSpace = true;
      space = true;
    } else if (line.length() == 0) {
      startSpace = true;
    } else {
      startPunc = true;
      punc = true;
    }
    char current;
    std::pair<size_t, size_t> oldYX;

    while (true) {
      oldYX = yx;
      WrapMoveCursor(sd_i, 0, increment, true).execute(b_i);
      yx = sd_i->getCursor();
      line = b_i->getLine(yx.first);
      
      current = line[yx.second];

      if (oldYX.first != yx.first) {
        space = true;
        if (!toEnd) {
          if (startWord) word = false;
          if (startPunc) punc = false;
        }
        if (startWord && space && word) break;
        if (startWord && punc && !wsOnly) break;
        if (startPunc && space && punc) break;
        if (startPunc && word && !wsOnly) break;
        if (startSpace && space && word) break;
        if (startSpace && space && punc) break;
        if (wsOnly && space && punc) break;
        if (wsOnly && space && word) break;
      } else if (oldYX.second == yx.second) {
        break;
      }

      if (('0' <= current && current <= '9') || ('A' <= current && current <= 'Z')
          || ('a' <= current && current <= 'z') || current == '_') {
        word = true;
        if (toEnd) {
          if (startSpace) {
            startSpace = false;
            startWord = true;
          }
          space = false;
        }
      } else if (32 == current || 9 == current) {
        space = true;
        if (!toEnd) {
          if (startWord) word = false;
          if (startPunc) punc = false;
          if (wsOnly) {
            word = false;
            punc = false;
          }
        }
      } else if (line.length() == 0) {
        space = true;
        punc = true;
        word = true;
      } else {
        punc = true;
        if (toEnd) {
          if (startSpace) {
            startSpace = false;
            startPunc = true;
          }
          space = false;
        }
      }

      if (startWord && space && word) break;
      if (startWord && punc && !wsOnly) break;
      if (startPunc && space && punc) break;
      if (startPunc && word && !wsOnly) break;
      if (startSpace && space && word) break;
      if (startSpace && space && punc) break;
      if (wsOnly && space && punc) break;
      if (wsOnly && space && word) break;
    }

    if (toEnd && oldYX != yx) WrapMoveCursor(sd_i, 0, -increment, true).execute(b_i);
  }

  void ReadFile::execute(BufferInterface *b_i) {
    std::string lines;

    std::ifstream fin{file_name};
    std::string line;
    while (std::getline(fin, line)) {
      if (lines.empty()) lines = line;
      else lines += BufferInterface::end_of_line_expr + line;
    }
    if (is_initial_file_read) {
      b_i->insertText(l, c, lines);
      sd_i->setStatusLine("\"" + sd_i->getFileName() + "\"");
    }
    else {
      InsertCommand(sd_i, l, c, lines).execute(b_i);
    }
    b_i->setHasUnsavedChanges(!is_initial_file_read);
  }

  void ReadFileBelowCursor::execute(BufferInterface *b_i) {
    std::string lines;

    std::ifstream fin{file_name};
    std::string line;
    while (std::getline(fin, line)) {
      lines += BufferInterface::end_of_line_expr + line;
    }
    std::pair<size_t, size_t> cursor_pos = sd_i->getCursor();
    size_t c_insert_pos = b_i->getLine(cursor_pos.first).length() + 1;
    InsertCommand(sd_i, cursor_pos.first, c_insert_pos, lines).execute(b_i);
  }

  void Quit::execute(BufferInterface *b_i) {
    if (!b_i->hasUnsavedChanges() || q_override) {
      b_i->setIsOpen(false);
    } else {
      sd_i->setStatusLine("There are unsaved changes! Write them with :w");
    }
  }

  void InsertCursor::execute(BufferInterface *b_i) {
    std::pair<size_t, size_t> yx = sd_i->getCursor();
    InsertCommand(sd_i, yx.first, yx.second+1, theString).execute(b_i);
    WrapMoveCursor(sd_i, 0, theString.length(), false).execute(b_i);
  }

  void DeleteCursor::execute(BufferInterface *b_i) {
    for (; num>0; --num) {
      int relativePos = (behind)?1:0;
      std::pair<size_t, size_t> yx = sd_i->getCursor();
      if (yx.second+1-relativePos == 0) {
        if (!delN) return;
        else {
          if (yx.first > 1) {
            size_t l = b_i->getLine(yx.first-1).length();
            DeleteCommand(sd_i, yx.first, yx.second+1-relativePos, 1).execute(b_i);
            MoveCursor(sd_i, yx.first-1, l-(restricted?1:0))
              .execute(b_i);
            yx = sd_i->getCursor();
            sd_i->setIdealCursor(yx.first, yx.second);
          }
          return;
        }
      } else {
        size_t l = b_i->getLine(yx.first).length();
        if (yx.second+1-relativePos <= l && !behind) {
          DeleteCommand(sd_i, yx.first, yx.second+1-relativePos, 1).execute(b_i);
          RelativeMoveCursor(sd_i, 0, 0, restricted).execute(b_i);
        } else if (yx.second+1-relativePos > l && !behind) {
          if (delN) {
            std::unique_ptr<CommandSequence> cs = std::make_unique<CommandSequence>(sd_i);
            cs->add(std::make_unique<WrapMoveCursor>(sd_i, 0, 1, false));
            cs->add(std::make_unique<DeleteCursor>(sd_i, true, 1, true, false));
            cs->add(std::make_unique<RelativeMoveCursor>(sd_i, 0, 0, restricted));
            cs->execute(b_i);
          }
        } else {
          DeleteCommand(sd_i, yx.first, yx.second+1-relativePos, 1).execute(b_i);
          RelativeMoveCursor(sd_i, 0, -1, restricted).execute(b_i);
        }
      }
    }
  }

  void YankCursor::execute(BufferInterface *b_i) {
    for (; num>0; --num) {
      int relativePos = (behind)?1:0;
      std::pair<size_t, size_t> yx = sd_i->getCursor();
      if (yx.second+1-relativePos == 0) {
        if (!delN) return;
        else {
          if (yx.first > 1) {
            size_t l = b_i->getLine(yx.first-1).length();
            yanked = '\n';
            if (move) {
              MoveCursor(sd_i, yx.first-1, l-(restricted?1:0))
                .execute(b_i);
              yx = sd_i->getCursor();
              sd_i->setIdealCursor(yx.first, yx.second);
            }
          }
          return;
        }
      } else {
        size_t l = b_i->getLine(yx.first).length();
        if (yx.second+1-relativePos <= l && !behind) {
          string line = b_i->getLine(yx.first);
          yanked = line[yx.second+1-relativePos-1];
          if (move) RelativeMoveCursor(sd_i, 0, 0, restricted).execute(b_i);
        } else if (yx.second+1-relativePos > l && !behind) {
          if (delN) {
            yanked = '\n';
            if (move) {
              WrapMoveCursor(sd_i, 0, 1, false).execute(b_i);
              RelativeMoveCursor(sd_i, 0, 0, restricted).execute(b_i);
            }
          }
        } else {
          string line = b_i->getLine(yx.first);
          yanked = line[yx.second+1-relativePos-1];
          if (move) RelativeMoveCursor(sd_i, 0, -1, restricted).execute(b_i);
        }
      }
    }
  }

  void JumpLineEnd::execute(BufferInterface *b_i) {
    std::pair<size_t, size_t> yx = sd_i->getCursor();
    if (start) SmartMoveCursor(sd_i, yx.first, 0, restricted).execute(b_i);
    else SmartMoveCursor(sd_i, yx.first, b_i->getLine(yx.first).length()-(restricted?1:0), restricted).execute(b_i);
  }

  void SkipWS::execute(BufferInterface *b_i) {
    while (true) {
      std::pair<size_t, size_t> yx = sd_i->getCursor();
      const string &l = b_i->getLine(yx.first);
      if (yx.second < l.length() && (l[yx.second] == ' ' || l[yx.second] == '\t'))
        RelativeMoveCursor(sd_i, 0, 1, false).execute(b_i);
      else break;
    }
  }

  void CommandSequence::execute(BufferInterface *b_i) {
    for (auto &c : cs) {
      c->execute(b_i);
    }
  }

  void WriteBuffer::execute(BufferInterface *b_i) {
    std::ofstream fout{file_name};
    b_i->writeToStream(fout);
    sd_i->setStatusLine("\"" + sd_i->getFileName() + "\"");
    b_i->setHasUnsavedChanges(false);
  }


  void MetaJumpToLine::execute(BufferInterface *b_i) {
    size_t max_line = b_i->getNumLines();

    if (line_number > max_line) line_number = max_line;
    if (line_number < 1) line_number = 1;
    MoveCursor(sd_i, line_number, 0).execute(b_i);

    sd_i->setIdealCursor(line_number, 0);
  }

  void MetaJumpToLastLine::execute(BufferInterface *b_i) {
    size_t max_line = b_i->getNumLines();

    MoveCursor(sd_i, max_line, 0).execute(b_i);

    sd_i->setIdealCursor(max_line, 0);
  }

  void Undefined::execute(BufferInterface *b_i) {
    sd_i->setStatusLine("That editor command is not defined.");
  }

  void MotionBoundCommand::execute(BufferInterface *b_i) {
    
    std::pair<size_t, size_t> oldPos = sd_i->getCursor();
    motion->execute(b_i);
    std::pair<size_t, size_t> newPos = sd_i->getCursor();
    //std::cerr << oldPos.first << "," << oldPos.second << ","
      //<< newPos.first << "," << newPos.second;
    command->executeWithMotion(b_i, oldPos, newPos);
  }

  void MultipliedCommand::execute(BufferInterface *b_i) {
    for (unsigned int i=0; i<multiplier; ++i) {
      command->execute(b_i);
    }
  }

  void MotionDelete::execute(BufferInterface *b_i) {
    std::pair<size_t, size_t> pos = sd_i->getCursor();
    if (pos.first == b_i->getNumLines())
      executeWithMotion(b_i, {pos.first, 0}, {pos.first,
         b_i->getLine(pos.first).length()});
    else
      executeWithMotion(b_i, {pos.first, 0}, {pos.first+1, 0});
  }

  void MotionDelete::executeWithMotion(BufferInterface *b_i, std::pair<size_t, size_t> firstCursorPos,
      std::pair<size_t, size_t> secondCursorPos) {
    string recorded = "";
    std::pair<size_t, size_t> f;
    std::pair<size_t, size_t> s;
    if (firstCursorPos.first < secondCursorPos.first) {
      f = firstCursorPos, s = secondCursorPos;
    } else if (firstCursorPos.first == secondCursorPos.first
        && firstCursorPos.second < secondCursorPos.second) {
      f = firstCursorPos, s = secondCursorPos;
    } else {
      s = firstCursorPos, f = secondCursorPos;
    }

    MoveCursor(sd_i, s.first, s.second).execute(b_i);
    while (sd_i->getCursor() != f) {
      std::pair<size_t, size_t> c = sd_i->getCursor();
      string line = b_i->getLine(c.first);
      YankCursor yc = YankCursor(sd_i, behind, 1, true, false, yank);
      yc.execute(b_i);
      if (yc.getYanked()) recorded = yc.getYanked() + recorded;
      if (!yank) DeleteCursor(sd_i, behind, 1, true, false).execute(b_i);
      if (!behind) WrapMoveCursor(sd_i, 0, -1, false).execute(b_i);
    }
    if (inclusive && !yank) {
      YankCursor yc = YankCursor(sd_i, !behind, 1, false, false, false);
      yc.execute(b_i);
      if (yc.getYanked()) recorded = yc.getYanked() + recorded;
      DeleteCursor(sd_i, !behind, 1, false, false).execute(b_i);
    }
    sd_i->setRegister(0, recorded);
  }

  void Paste::execute(BufferInterface *b_i) {
    string p = sd_i->getRegister(0);
    RelativeMoveCursor(sd_i, 0, 1, false).execute(b_i);
    for (auto i : p)
      InsertCursor(sd_i, string()+i).execute(b_i);
    RelativeMoveCursor(sd_i, 0, 0, true).execute(b_i);
  };

  void BkspIfLastLine::execute(BufferInterface *b_i) {
    std::pair<size_t, size_t> c = sd_i->getCursor();
    size_t l = b_i->getNumLines();
    if (c.first == l) {
      DeleteCursor(sd_i, true, 1, true, false).execute(b_i);
      RelativeMoveCursor(sd_i, 0, 0, true).execute(b_i);
    }
  }

  void Scroll::execute(BufferInterface *b_i) {
    RelativeMoveCursor(sd_i, amount, 0, true).execute(b_i);
    long int f = sd_i->getTopLine() + amount;
    if (frame) {
      if (f > 1 && bound && static_cast<size_t>(f) + sd_i->getViewInfo()->getHeight()
          > b_i->getNumLines()) {
        f = b_i->getNumLines() - sd_i->getViewInfo()->getHeight();
      } else if (f > 1 && !bound && static_cast<size_t>(f) > b_i->getNumLines()) {
        f = b_i->getNumLines();
      }
      if (f < 1) f = 1;
      sd_i->setTopLine(f);
    }
  }

  void Search::execute(BufferInterface *b_i) {
    int inc = (dir) ? -1 : 1;
    std::pair<size_t, size_t> oldCursor = sd_i->getCursor();
    while (true) {
      std::pair<size_t, size_t> c = sd_i->getCursor();
      WrapMoveCursor(sd_i, 0, inc, true).execute(b_i);
      std::pair<size_t, size_t> c2 = sd_i->getCursor();
      if (b_i->getLine(c2.first).substr(c2.second, searchStr.length()) == searchStr)
        break;
      else if (c == c2) {
        MoveCursor(sd_i, oldCursor.first, oldCursor.second).execute(b_i);
        sd_i->setIdealCursor(oldCursor.first, oldCursor.second);
        sd_i->setStatusLine("Pattern '" + searchStr + "' not found!");
        break;
      }
    }
  }

  void FindMatching::execute(BufferInterface *b_i) {
    RelativeMoveCursor(sd_i, 0, 0, true).execute(b_i);
    std::pair<size_t, size_t> oldCursor = sd_i->getCursor();
    string l = b_i->getLine(oldCursor.first);
    if (l.length() == 0) return;

    char paren = l[oldCursor.second];
    int inc;
    if (paren == '(' || paren == '[' || paren == '{')
      inc = 1;
    else if (paren == ')' || paren == ']' || paren == '}')
      inc = -1;
    else return;

    int count = 0;

    while (true) {
      std::pair<size_t, size_t> c = sd_i->getCursor();
      WrapMoveCursor(sd_i, 0, inc, true).execute(b_i);
      std::pair<size_t, size_t> c2 = sd_i->getCursor();

      if (c == c2) {
        MoveCursor(sd_i, oldCursor.first, oldCursor.second).execute(b_i);
        sd_i->setIdealCursor(oldCursor.first, oldCursor.second);
        break;
      }

      l = b_i->getLine(c2.first);
      if (l.length() != 0) {
        if ((l[c2.second] == '(' && paren == ')')
            || (l[c2.second] == ')' && paren == '(')
            || (l[c2.second] == '[' && paren == ']')
            || (l[c2.second] == ']' && paren == '[')
            || (l[c2.second] == '{' && paren == '}')
            || (l[c2.second] == '}' && paren == '{')) {
          if (count == 0) break;
          else --count;
        }
        else if ((l[c2.second] == '(' && paren == '(')
            || (l[c2.second] == ')' && paren == ')')
            || (l[c2.second] == '[' && paren == '[')
            || (l[c2.second] == ']' && paren == ']')
            || (l[c2.second] == '{' && paren == '{')
            || (l[c2.second] == '}' && paren == '}')) {
          ++count;
        }
      }
    }
  }

  void PutFileInfo::execute(BufferInterface *b_i) {
    size_t b_num_lines = b_i->getNumLines();
    std::string modified = (b_i->hasUnsavedChanges()) ? "[Modified] " : "";
    sd_i->setStatusLine("\"" + sd_i->getFileName() + "\" " + modified + std::to_string(b_num_lines) + " lines --" + std::to_string(int((double(sd_i->getCursor().first) / b_num_lines) * 100)) + "\%--");
  }
}
