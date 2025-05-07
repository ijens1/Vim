#include <fstream>
#include <string>
#include <iostream>
#include <cctype>
#include "cmdgen.h"
#include "controller.h"

namespace vm {

    unique_ptr<Command> CmdGenerator::multiply(unique_ptr<Command> &&c, string &m) {
        if (m == "") return std::move(c);
        else {
            int i;
            try {
                i = std::stoi(m);
            } catch (...) {
                m = "";
                return std::move(c);
            }
            m = "";
            return make_unique<MultipliedCommand>(sd_i, i, std::move(c));
        }
    }

    unique_ptr<Command> CmdGenerator::processRaw(int input) {
        if (isRecordingMacro) {
          if (input == 'q') {
            isRecordingMacro = false;
            sd_i->setIsRecordingMacro(isRecordingMacro);
            if (std::isupper(macroChar)) {
              sd_i->setRegister(std::tolower(macroChar), sd_i->getRegister(std::tolower(macroChar)) + rawMacro);
            } else {
              sd_i->setRegister(macroChar, rawMacro);
            }
            rawMacro.clear();
            return make_unique<WakeUp>(sd_i);
          } else if (input != 7) {
            rawMacro += input;
          }
        }
        if (mode == vm::ReadableSessionDataInterface::inputMode::NORMAL) {
            if (('1' <= input && input <= '9')
                    || (input == '0' && currentMultiplier != "")) {
                currentMultiplier += static_cast<char>(input);
                return make_unique<WakeUp>(sd_i);
            }
            switch (input) {
                case 4: //^d
                    return make_unique<Scroll>(sd_i, 25, true, true);
                    break;
                case 21:        // ^u
                    return make_unique<Scroll>(sd_i, -25, true, true);
                    break;
                case 6:         //^f
                    return make_unique<Scroll>(sd_i, 50, true, false);
                    break;
                case 2:        // ^b
                    return make_unique<Scroll>(sd_i, -50, true, false);
                    break;
                case 7:         // ^g
                    return make_unique<PutFileInfo>(sd_i);
                    break;
                case 'p': {
                    sd_i->startUndoChunk();
                    unique_ptr<CommandSequence> cs = make_unique<CommandSequence>(sd_i);
                    cs->add(make_unique<RelativeMoveCursor>(sd_i, 0, -1, true));
                    cs->add(make_unique<Paste>(sd_i));
                    return std::move(cs);
                    break;
                }
                case 'P': {
                    sd_i->startUndoChunk();
                    unique_ptr<CommandSequence> cs = make_unique<CommandSequence>(sd_i);
                    cs->add(make_unique<JumpLineEnd>(sd_i, true, false));
                    cs->add(make_unique<InsertCursor>(sd_i, "\n"));
                    cs->add(make_unique<WrapMoveCursor>(sd_i, 0, -1, false));
                    cs->add(make_unique<Paste>(sd_i));
                    return std::move(cs);
                    break;
                }
                case '/':
                    mode = vm::ReadableSessionDataInterface::inputMode::SEARCH;
                    sd_i->setMode(mode);
                    searchedString = "";
                    lastSearchDir = false;
                    sd_i->setSearchedString(searchedString);
                    break;
                case '?':
                    mode = vm::ReadableSessionDataInterface::inputMode::RSEARCH;
                    sd_i->setMode(mode);
                    searchedString = "";
                    lastSearchDir = true;
                    sd_i->setSearchedString(searchedString);
                    break;
                case ':':
                    mode = vm::ReadableSessionDataInterface::inputMode::EX;
                    sd_i->setMode(mode);
                    exLine = "";
                    sd_i->setExLine(exLine);
                    break;
                case 'h':
                case 127:
                case KEY_BACKSPACE:
                case KEY_LEFT: {       // key_left
                    return multiply(
                                make_unique<RelativeMoveCursor>(sd_i, 0, -1, true),
                                currentMultiplier);
                    break;
                }
                case ' ':
                case 'l':
                case KEY_RIGHT: {       // key_right
                    return multiply(
                                make_unique<RelativeMoveCursor>(sd_i, 0, 1, true),
                                currentMultiplier);
                    break;
                }
                case 'k':
                case KEY_UP: {       // key_up
                    return multiply(
                                make_unique<RelativeMoveCursor>(sd_i, -1, 0, true),
                                currentMultiplier);
                    break;
                }
                case 'j':
                case KEY_DOWN: {       // key_down
                    return multiply(
                                make_unique<RelativeMoveCursor>(sd_i, 1, 0, true),
                                currentMultiplier);
                    break;
                }
                case 'J': {
                    unique_ptr<CommandSequence> cs = make_unique<CommandSequence>(sd_i);
                    cs->add(make_unique<JumpLineEnd>(sd_i, false, false));
                    cs->add(make_unique<InsertCursor>(sd_i, " "));
                    cs->add(make_unique<WrapMoveCursor>(sd_i, 0, 1, false));
                    cs->add(make_unique<DeleteCursor>(sd_i, true, 1, true, false));
                    cs->add(make_unique<RelativeMoveCursor>(sd_i, 0, 0, true));
                    return std::move(cs);
                    break;
                }
                case 'r':
                    mode = vm::ReadableSessionDataInterface::inputMode::REPLACE_ONCE;
                    sd_i->setMode(mode);
                    break;
                case 'R':
                    mode = vm::ReadableSessionDataInterface::inputMode::REPLACE;
                    sd_i->setMode(mode);
                    break;
                case 'i':
                    if (!isRunningMacro) sd_i->startUndoChunk();
                    mode = vm::ReadableSessionDataInterface::inputMode::INSERT;
                    sd_i->setMode(mode);
                    break;
                case 's': {
                    mode = vm::ReadableSessionDataInterface::inputMode::INSERT;
                    sd_i->setMode(mode);
                    if (currentMultiplier == "")
                        return make_unique<DeleteCursor>(sd_i, false, 1, false, true);
                    else {
                        int i;
                        try {
                            i = std::stoi(currentMultiplier);
                        } catch (...) {
                            currentMultiplier = "";
                            return make_unique<DeleteCursor>(sd_i, false, 1, false, true);
                        }
                        currentMultiplier = "";
                        return make_unique<DeleteCursor>(sd_i, false, i, false, true);
                    }
                    break;
                }
                case 'a':
                    if (!isRunningMacro) sd_i->startUndoChunk();
                    mode = vm::ReadableSessionDataInterface::inputMode::INSERT;
                    sd_i->setMode(mode);
                    return make_unique<RelativeMoveCursor>(sd_i, 0, 1, false);
                    break;
                case 'f':
                    mode = vm::ReadableSessionDataInterface::inputMode::COMPOSE_FIND;
                    sd_i->setMode(mode);
                    lastFoundDir = false;
                    break;
                case 'F':
                    mode = vm::ReadableSessionDataInterface::inputMode::COMPOSE_RFIND;
                    sd_i->setMode(mode);
                    lastFoundDir = true;
                    break;
                case ';': {
                    return multiply(
                                make_unique<MoveFind>(sd_i, string()+static_cast<char>(lastFound), lastFoundDir),
                                currentMultiplier);
                    break;
                }
                case ',': {
                    return multiply(
                                make_unique<MoveFind>(sd_i, string()+static_cast<char>(lastFound), !lastFoundDir),
                                currentMultiplier);
                    break;
                }
                case 'w': {
                    return multiply(
                                make_unique<JumpWord>(sd_i, false, false, false),
                                currentMultiplier);
                    break;
                }
                case 'W': {
                    return multiply(
                                make_unique<JumpWord>(sd_i, false, true, false),
                                currentMultiplier);
                    break;
                }
                case 'b': {
                    return multiply(
                                make_unique<JumpWord>(sd_i, true, false, true),
                                currentMultiplier);
                    break;
                }
                case 'B': {
                    return multiply(
                                make_unique<JumpWord>(sd_i, true, true, true),
                                currentMultiplier);
                    break;
                }
                case 'e': {
                    return multiply(
                                make_unique<JumpWord>(sd_i, false, false, true),
                                currentMultiplier);
                    break;
                }
                case 'E': {
                    return multiply(
                                make_unique<JumpWord>(sd_i, false, true, true),
                                currentMultiplier);
                    break;
                }
                case '.': {
                    if (currentMultiplier == "")
                        return make_unique<RepeatCommand>(sd_i, 1);
                    else {
                        int i;
                        try {
                            i = std::stoi(currentMultiplier);
                        } catch (...) {
                            currentMultiplier = "";
                            return make_unique<RepeatCommand>(sd_i, 1);
                        }
                        currentMultiplier = "";
                        return make_unique<RepeatCommand>(sd_i, i);
                    }
                    break;
                }
                case KEY_DC:
                case 'x': {
                    if (!isRunningMacro) sd_i->startUndoChunk();
                    if (currentMultiplier == "")
                        return make_unique<DeleteCursor>(sd_i, false, 1, false, true);
                    else {
                        int i;
                        try {
                            i = std::stoi(currentMultiplier);
                        } catch (...) {
                            currentMultiplier = "";
                            return make_unique<DeleteCursor>(sd_i, false, 1, false, true);
                        }
                        currentMultiplier = "";
                        return make_unique<DeleteCursor>(sd_i, false, i, false, true);
                    }
                    break;
                }
                case 'X': {
                    if (!isRunningMacro) sd_i->startUndoChunk();
                    if (currentMultiplier == "")
                        return make_unique<DeleteCursor>(sd_i, true, 1, false, true);
                    else {
                        int i;
                        try {
                            i = std::stoi(currentMultiplier);
                        } catch (...) {
                            currentMultiplier = "";
                            return make_unique<DeleteCursor>(sd_i, true, 1, false, true);
                        }
                        currentMultiplier = "";
                        return make_unique<DeleteCursor>(sd_i, true, i, false, true);
                    }
                    return multiply(
                                make_unique<DeleteCursor>(sd_i, true, 1, false, true),
                                currentMultiplier);
                    break;
                }
                case 'A':
                    if (!isRunningMacro) sd_i->startUndoChunk();
                    mode = vm::ReadableSessionDataInterface::inputMode::INSERT;
                    sd_i->setMode(mode);
                    return make_unique<JumpLineEnd>(sd_i, false, false);
                    break;
                case 'S': {
                    mode = vm::ReadableSessionDataInterface::inputMode::INSERT;
                    sd_i->setMode(mode);
                    unique_ptr<CommandSequence> cs2 = make_unique<CommandSequence>(sd_i);
                    unique_ptr<CommandSequence> cs = make_unique<CommandSequence>(sd_i);
                    cs->add(make_unique<JumpLineEnd>(sd_i, true, true));
                    cs->add(make_unique<SkipWS>(sd_i));
                    cs->add(make_unique<MotionBoundCommand>(
                                    sd_i,
                                    make_unique<MotionDelete>(sd_i, true, true, false),
                                    make_unique<JumpLineEnd>(sd_i, false, true)));
                    cs->add(make_unique<RelativeMoveCursor>(sd_i, 0, 1, false));
                    return std::move(cs);
                    break;
                    }
                case '$':
                    return make_unique<JumpLineEnd>(sd_i, false, true);
                    break;
                case '0':
                    return make_unique<JumpLineEnd>(sd_i, true, true);
                    break;
                case 'I':
                    if (!isRunningMacro) sd_i->startUndoChunk();
                    mode = vm::ReadableSessionDataInterface::inputMode::INSERT;
                    sd_i->setMode(mode);
                case '^':
                case '_': {
                    unique_ptr<CommandSequence> cs = make_unique<CommandSequence>(sd_i);
                    cs->add(make_unique<JumpLineEnd>(sd_i, true, true));
                    cs->add(make_unique<SkipWS>(sd_i));
                    return std::move(cs);
                    break;
                }
                case KEY_ENTER:
                case '\n': {
                    unique_ptr<CommandSequence> cs = make_unique<CommandSequence>(sd_i);
                    cs->add(make_unique<RelativeMoveCursor>(sd_i, 1, 0, true));
                    cs->add(make_unique<JumpLineEnd>(sd_i, true, true));
                    cs->add(make_unique<SkipWS>(sd_i));
                    return std::move(cs);
                    break;
                }
                case 'o': {
                    if (!isRunningMacro) sd_i->startUndoChunk();
                    mode = vm::ReadableSessionDataInterface::inputMode::INSERT;
                    sd_i->setMode(mode);
                    unique_ptr<CommandSequence> cs = make_unique<CommandSequence>(sd_i);
                    cs->add(make_unique<JumpLineEnd>(sd_i, false, false));
                    cs->add(make_unique<InsertCursor>(sd_i, "\n"));
                    return std::move(cs);
                    break;
                }
                case 'O': {
                    if (!isRunningMacro) sd_i->startUndoChunk();
                    mode = vm::ReadableSessionDataInterface::inputMode::INSERT;
                    sd_i->setMode(mode);
                    unique_ptr<CommandSequence> cs = make_unique<CommandSequence>(sd_i);
                    cs->add(make_unique<JumpLineEnd>(sd_i, true, false));
                    cs->add(make_unique<InsertCursor>(sd_i, "\n"));
                    cs->add(make_unique<WrapMoveCursor>(sd_i, 0, -1, false));
                    return std::move(cs);
                    break;
                }
                case 'd':
                    if (!isRunningMacro) sd_i->startUndoChunk();
                    mode = vm::ReadableSessionDataInterface::inputMode::COMPOSE_DELETE;
                    sd_i->setMode(mode);
                    break;
                case 'c':
                    if (!isRunningMacro) sd_i->startUndoChunk();
                    mode = vm::ReadableSessionDataInterface::inputMode::COMPOSE_CHANGE;
                    sd_i->setMode(mode);
                    break;
                case 'y':
                    mode = vm::ReadableSessionDataInterface::inputMode::COMPOSE_YANK;
                    sd_i->setMode(mode);
                    break;
                case 'q':
                    mode = vm::ReadableSessionDataInterface::inputMode::COMPOSE_MACRO_REG_CHOICE;
                    sd_i->setMode(mode);
                    break;
                case '@':
                    mode = vm::ReadableSessionDataInterface::inputMode::COMPOSE_RUN_MACRO_REG_CHOICE;
                    sd_i->setMode(mode);
                    break;
                case 'n':
                    if (lastSearchDir) {
                        sd_i->setStatusLine("?"+searchedString);
                    } else {
                        sd_i->setStatusLine("/"+searchedString);
                    }
                    return multiply(
                            make_unique<Search>(sd_i, searchedString, lastSearchDir),
                            currentMultiplier);
                    break;
                case 'N':
                    if (lastSearchDir) {
                        sd_i->setStatusLine("/"+searchedString);
                    } else {
                        sd_i->setStatusLine("?"+searchedString);
                    }
                    return multiply(
                            make_unique<Search>(sd_i, searchedString, !lastSearchDir),
                            currentMultiplier);
                    break;

                case '%':
                    return make_unique<FindMatching>(sd_i);
                    break;
                case 'u':
                    std::unique_ptr<Command> undo_cmd = sd_i->undoTop();
                    if (undo_cmd == nullptr) {
                      sd_i->setStatusLine("Already at oldest change");
                    }
                    else {
                      sd_i->popUndo();
                      return std::move(undo_cmd);
                    }
            }
        } else if (mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_FIND || mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_RFIND) {
            bool r;
            if (mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_FIND) r = false;
            else r = true;
            mode = vm::ReadableSessionDataInterface::inputMode::NORMAL;
            sd_i->setMode(mode);
            if ((32 <= input && input <= 126) || input == 9) {
                lastFound = input;
                return make_unique<MoveFind>(sd_i, string()+static_cast<char>(input),
                        r?true:false);
            }
        } else if (mode == vm::ReadableSessionDataInterface::inputMode::SUPERCOMPOSE_FIND
                || mode == vm::ReadableSessionDataInterface::inputMode::SUPERCOMPOSE_RFIND) {
            bool r;
            if (mode == vm::ReadableSessionDataInterface::inputMode::SUPERCOMPOSE_FIND) r = false;
            else r = true;

            if (previousMode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_CHANGE)
                mode = vm::ReadableSessionDataInterface::inputMode::INSERT;
            else
                mode = vm::ReadableSessionDataInterface::inputMode::NORMAL;
            sd_i->setMode(mode);

            if ((32 <= input && input <= 126) || input == 9) {
                lastFound = input;

                bool yank = (previousMode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_YANK) ? true : false;

                unique_ptr<CommandSequence> cs = make_unique<CommandSequence>(sd_i);
                cs->add(multiply(
                            make_unique<MotionBoundCommand>(
                                sd_i,
                                make_unique<MotionDelete>(sd_i, r?true:false, false, yank),
                                make_unique<MoveFind>(sd_i, string()
                                    +static_cast<char>(input), r?true:false)),
                            currentMultiplier));
                cs->add(make_unique<DeleteCursor>(sd_i, false, 1, false, false));
                cs->add(make_unique<RelativeMoveCursor>(sd_i, 0, 0, true));
                return std::move(cs);
            }
        } else if (mode == ReadableSessionDataInterface::inputMode::REPLACE
                || mode == ReadableSessionDataInterface::inputMode::REPLACE_ONCE) {
            bool once = (mode == ReadableSessionDataInterface::inputMode::REPLACE)
                ? false : true;
            switch (input) {
                case 27: {
                    mode = vm::ReadableSessionDataInterface::inputMode::NORMAL;
                    sd_i->setMode(mode);
                    if (!once)
                        return make_unique<RelativeMoveCursor>(sd_i, 0, -1, true);
                    break;
                }
                case KEY_BACKSPACE:
                case 127:
                case KEY_LEFT:
                    if (once) {
                        mode = vm::ReadableSessionDataInterface::inputMode::NORMAL;
                        sd_i->setMode(mode);
                        break;
                    }
                    return make_unique<RelativeMoveCursor>(sd_i, 0, -1, false);
                    break;
                case KEY_RIGHT:
                    if (once) {
                        mode = vm::ReadableSessionDataInterface::inputMode::NORMAL;
                        sd_i->setMode(mode);
                        break;
                    }
                    return make_unique<RelativeMoveCursor>(sd_i, 0, 1, false);
                    break;
                case KEY_UP:
                    if (once) {
                        mode = vm::ReadableSessionDataInterface::inputMode::NORMAL;
                        sd_i->setMode(mode);
                        break;
                    }
                    return make_unique<RelativeMoveCursor>(sd_i, -1, 0, false);
                    break;
                case KEY_DOWN:
                    if (once) {
                        mode = vm::ReadableSessionDataInterface::inputMode::NORMAL;
                        sd_i->setMode(mode);
                        break;
                    }
                    return make_unique<RelativeMoveCursor>(sd_i, 1, 0, false);
                    break;
                case KEY_DC:
                    return make_unique<DeleteCursor>(sd_i, false, 1, true, false);
                    break;
                default:
                    if ((32 <= input && input <= 126) || input == 9 || input == 10) {
                        if (once) {
                            mode = vm::ReadableSessionDataInterface::inputMode::NORMAL;
                            sd_i->setMode(mode);
                        }
                        unique_ptr<CommandSequence> cs =
                            make_unique<CommandSequence>(sd_i);
                        cs->add(make_unique<DeleteCursor>(sd_i, false, 1, false, false));
                        cs->add(make_unique<InsertCursor>(sd_i, string()+static_cast<char>(input)));
                        if (once)
                            cs->add(make_unique<RelativeMoveCursor>(sd_i, 0, -1, true));
                        return std::move(cs);
                    }
                    break;
            }
        } else if (mode == vm::ReadableSessionDataInterface::inputMode::INSERT) {
            switch (input) {
                case 27:
                    mode = vm::ReadableSessionDataInterface::inputMode::NORMAL;
                    sd_i->setMode(mode);
                    return make_unique<RelativeMoveCursor>(sd_i, 0, -1, true);
                case KEY_LEFT:
                    return make_unique<RelativeMoveCursor>(sd_i, 0, -1, false);
                    break;
                case KEY_RIGHT:
                    return make_unique<RelativeMoveCursor>(sd_i, 0, 1, false);
                    break;
                case KEY_UP:
                    return make_unique<RelativeMoveCursor>(sd_i, -1, 0, false);
                    break;
                case KEY_DOWN:
                    return make_unique<RelativeMoveCursor>(sd_i, 1, 0, false);
                    break;
                case KEY_DC:
                    return make_unique<DeleteCursor>(sd_i, false, 1, true, false);
                    break;
                case 127:
                case KEY_BACKSPACE:
                    return make_unique<DeleteCursor>(sd_i, true, 1, true, false);
                    break;
                default:
                    if ((32 <= input && input <= 126) || input == 9 || input == 10) {
                        //std::cerr << "HELLO";
                        return make_unique<InsertCursor>(sd_i, string()+static_cast<char>(input));
                    }
                    break;
            }
        } else if (mode == vm::ReadableSessionDataInterface::inputMode::EX) {
            if (input == '\n' || input == KEY_ENTER) {
                mode = vm::ReadableSessionDataInterface::inputMode::NORMAL;
                sd_i->setMode(mode);
                lastExLine = exLine;
                return exExecute(exLine);
            } else if (input == 27) {
                mode = vm::ReadableSessionDataInterface::inputMode::NORMAL;
                sd_i->setMode(mode);
            } else if (input == KEY_BACKSPACE || input == 127) {
                if (!exLine.empty()) {
                    exLine = exLine.substr(0, exLine.length()-1);
                    sd_i->setExLine(exLine);
                }
            } else {
                exLine += input;
                sd_i->setExLine(exLine);
            }
        } else if (mode == vm::ReadableSessionDataInterface::inputMode::SEARCH
                || mode == vm::ReadableSessionDataInterface::inputMode::RSEARCH
                || mode == vm::ReadableSessionDataInterface::inputMode::SUPERCOMPOSE_SEARCH
                || mode == vm::ReadableSessionDataInterface::inputMode::SUPERCOMPOSE_RSEARCH) {
            if (input == '\n' || input == KEY_ENTER) {
                if (mode == vm::ReadableSessionDataInterface::inputMode::SEARCH
                    || mode == ReadableSessionDataInterface::inputMode::SUPERCOMPOSE_SEARCH) {
                    lastSearchDir = false;
                    sd_i->setStatusLine("/"+searchedString);
                } else {
                    lastSearchDir = true;
                    sd_i->setStatusLine("?"+searchedString);
                }

                if (mode == vm::ReadableSessionDataInterface::inputMode::SUPERCOMPOSE_SEARCH
                    || mode == vm::ReadableSessionDataInterface::inputMode::SUPERCOMPOSE_RSEARCH) {

                    if (previousMode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_CHANGE)
                        mode = vm::ReadableSessionDataInterface::inputMode::INSERT;
                    else mode = vm::ReadableSessionDataInterface::inputMode::NORMAL;
                    sd_i->setMode(mode);

                    bool yank = (previousMode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_YANK) ? true : false;

                    return make_unique<MotionBoundCommand>(
                                    sd_i,
                                    make_unique<MotionDelete>(sd_i, true, false, yank),
                                    make_unique<Search>(sd_i, searchedString,
                                        lastSearchDir));
                } else {
                    mode = vm::ReadableSessionDataInterface::inputMode::NORMAL;
                    sd_i->setMode(mode);
                    return make_unique<Search>(sd_i, searchedString, lastSearchDir);
                }
            } else if (input == 27) {
                mode = vm::ReadableSessionDataInterface::inputMode::NORMAL;
                sd_i->setMode(mode);
            } else if (input == KEY_BACKSPACE || input == 127) {
                if (!searchedString.empty()) {
                    searchedString = searchedString.substr(0, searchedString.length()-1);
                    sd_i->setSearchedString(searchedString);
                }
            } else {
                searchedString += input;
                sd_i->setSearchedString(searchedString);
            }
        } else if (mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_DELETE || mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_YANK
                || mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_CHANGE) {
            if (('1' <= input && input <= '9')
                    || (input == '0' && currentMultiplier != "")) {
                currentMultiplier += static_cast<char>(input);
                return make_unique<WakeUp>(sd_i);
            }
            bool yank = (mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_YANK) ? true : false;
            switch (input) {
                case 'd':
                    if (mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_DELETE) {
                        mode = vm::ReadableSessionDataInterface::inputMode::NORMAL;
                        sd_i->setMode(mode);
                        unique_ptr<CommandSequence> cs2 = make_unique<CommandSequence>(sd_i);
                        cs2->add(make_unique<JumpLineEnd>(sd_i, false, false));
                        cs2->add(make_unique<WrapMoveCursor>(sd_i, 0, 1, false));

                        unique_ptr<CommandSequence> cs = make_unique<CommandSequence>(sd_i);
                        cs->add(make_unique<JumpLineEnd>(sd_i, true, true));
                        cs->add(multiply(
                                    make_unique<MotionBoundCommand>(
                                        sd_i,
                                        make_unique<MotionDelete>(sd_i, true, false, false),
                                        std::move(cs2)),
                                    currentMultiplier));
                        cs->add(make_unique<RelativeMoveCursor>(sd_i, 0, 0, true));
                        cs->add(make_unique<BkspIfLastLine>(sd_i));
                        return std::move(cs);
                    }
                    break;
                case 'c':
                    if (mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_CHANGE) {
                        mode = vm::ReadableSessionDataInterface::inputMode::INSERT;
                        sd_i->setMode(mode);
                        unique_ptr<CommandSequence> cs = make_unique<CommandSequence>(sd_i);
                        cs->add(make_unique<JumpLineEnd>(sd_i, true, true));
                        cs->add(multiply(
                                    make_unique<MotionBoundCommand>(
                                        sd_i,
                                        make_unique<MotionDelete>(sd_i, true, false, false),
                                        make_unique<JumpLineEnd>(sd_i, false, false)),
                                    currentMultiplier));
                        cs->add(make_unique<RelativeMoveCursor>(sd_i, 0, 0, true));
                        return std::move(cs);
                    }
                    break;
                case 'y':
                    if (mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_YANK) {
                        mode = vm::ReadableSessionDataInterface::inputMode::NORMAL;
                        sd_i->setMode(mode);
                        unique_ptr<CommandSequence> cs = make_unique<CommandSequence>(sd_i);
                        cs->add(make_unique<JumpLineEnd>(sd_i, true, true));
                        cs->add(multiply(
                                    make_unique<MotionBoundCommand>(
                                        sd_i,
                                        make_unique<MotionDelete>(sd_i, true, false, true),
                                        make_unique<JumpLineEnd>(sd_i, false, false)),
                                    currentMultiplier));
                        cs->add(make_unique<RelativeMoveCursor>(sd_i, 0, 0, true));
                        return std::move(cs);
                    }
                    break;
                case KEY_LEFT:
                case 'h': {
                    if (mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_DELETE || mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_YANK)
                        mode = vm::ReadableSessionDataInterface::inputMode::NORMAL;
                    else if (mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_CHANGE)
                        mode = vm::ReadableSessionDataInterface::inputMode::INSERT;
                    sd_i->setMode(mode);

                    unique_ptr<CommandSequence> cs = make_unique<CommandSequence>(sd_i);
                    cs->add(multiply(
                                make_unique<MotionBoundCommand>(
                                    sd_i,
                                    make_unique<MotionDelete>(sd_i, true, false, yank),
                                    make_unique<RelativeMoveCursor>(sd_i, 0, -1, false)),
                                currentMultiplier));
                    cs->add(make_unique<RelativeMoveCursor>(sd_i, 0, 0, true));
                    return std::move(cs);
                    break;
                }

                case KEY_DOWN:
                case 'j': {
                    if (mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_DELETE || mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_YANK)
                        mode = vm::ReadableSessionDataInterface::inputMode::NORMAL;
                    else if (mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_CHANGE)
                        mode = vm::ReadableSessionDataInterface::inputMode::INSERT;
                    sd_i->setMode(mode);

                    unique_ptr<CommandSequence> cs2 = make_unique<CommandSequence>(sd_i);
                    cs2->add(make_unique<RelativeMoveCursor>(sd_i, 1, 0, true));
                    cs2->add(make_unique<JumpLineEnd>(sd_i, false, false));
                    cs2->add(make_unique<WrapMoveCursor>(sd_i, 0, 1, false));

                    unique_ptr<CommandSequence> cs = make_unique<CommandSequence>(sd_i);
                    cs->add(make_unique<JumpLineEnd>(sd_i, true, true));
                    cs->add(multiply(
                                make_unique<MotionBoundCommand>(
                                    sd_i,
                                    make_unique<MotionDelete>(sd_i, true, false, yank),
                                    std::move(cs2)),
                                currentMultiplier));
                    cs->add(make_unique<RelativeMoveCursor>(sd_i, 0, 0, true));
                    return std::move(cs);
                    break;
                }

                case KEY_UP:
                case 'k': {
                    if (mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_DELETE || mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_YANK)
                        mode = vm::ReadableSessionDataInterface::inputMode::NORMAL;
                    else if (mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_CHANGE)
                        mode = vm::ReadableSessionDataInterface::inputMode::INSERT;
                    sd_i->setMode(mode);

                    unique_ptr<CommandSequence> cs2 = make_unique<CommandSequence>(sd_i);
                    cs2->add(make_unique<RelativeMoveCursor>(sd_i, -1, 0, true));
                    cs2->add(make_unique<JumpLineEnd>(sd_i, true, true));
                    cs2->add(make_unique<WrapMoveCursor>(sd_i, 0, -1, false));

                    unique_ptr<CommandSequence> cs = make_unique<CommandSequence>(sd_i);
                    cs->add(make_unique<JumpLineEnd>(sd_i, false, false));
                    cs->add(multiply(
                                make_unique<MotionBoundCommand>(
                                    sd_i,
                                    make_unique<MotionDelete>(sd_i, true, false, yank),
                                    std::move(cs2)),
                                currentMultiplier));
                    cs->add(make_unique<RelativeMoveCursor>(sd_i, 0, 0, true));
                    return std::move(cs);
                    break;
                }

                case KEY_RIGHT:
                case 'l': {
                    if (mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_DELETE || mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_YANK)
                        mode = vm::ReadableSessionDataInterface::inputMode::NORMAL;
                    else if (mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_CHANGE)
                        mode = vm::ReadableSessionDataInterface::inputMode::INSERT;
                    sd_i->setMode(mode);

                    unique_ptr<CommandSequence> cs = make_unique<CommandSequence>(sd_i);
                    cs->add(multiply(
                                make_unique<MotionBoundCommand>(
                                    sd_i,
                                    make_unique<MotionDelete>(sd_i, true, false, yank),
                                    make_unique<RelativeMoveCursor>(sd_i, 0, 1, false)),
                                currentMultiplier));
                    cs->add(make_unique<RelativeMoveCursor>(sd_i, 0, 0, true));
                    return std::move(cs);
                    break;
                }

                case 'f':
                    previousMode = mode;
                    mode = vm::ReadableSessionDataInterface::inputMode::SUPERCOMPOSE_FIND;
                    sd_i->setMode(mode);
                    sd_i->setPreviousMode(previousMode);
                    lastFoundDir = false;
                    break;

                case 'F':
                    previousMode = mode;
                    mode = vm::ReadableSessionDataInterface::inputMode::SUPERCOMPOSE_RFIND;
                    sd_i->setMode(mode);
                    sd_i->setPreviousMode(previousMode);
                    lastFoundDir = true;
                    break;

                case '/':
                    previousMode = mode;
                    mode = vm::ReadableSessionDataInterface::inputMode::SUPERCOMPOSE_SEARCH;
                    sd_i->setMode(mode);
                    searchedString = "";
                    lastSearchDir = false;
                    sd_i->setSearchedString(searchedString);
                    break;
                case '?':
                    previousMode = mode;
                    mode = vm::ReadableSessionDataInterface::inputMode::SUPERCOMPOSE_RSEARCH;
                    sd_i->setMode(mode);
                    searchedString = "";
                    lastSearchDir = true;
                    sd_i->setSearchedString(searchedString);
                    break;

                case KEY_HOME:
                case '0': {
                    if (mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_DELETE || mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_YANK)
                        mode = vm::ReadableSessionDataInterface::inputMode::NORMAL;
                    else if (mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_CHANGE)
                        mode = vm::ReadableSessionDataInterface::inputMode::INSERT;
                    return multiply(
                                make_unique<MotionBoundCommand>(
                                    sd_i,
                                    make_unique<MotionDelete>(sd_i, true, false, yank),
                                    make_unique<JumpLineEnd>(sd_i, true, false)),
                                currentMultiplier);
                    break;
                }

                case KEY_END:
                case '$': {
                    if (mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_DELETE || mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_YANK)
                        mode = vm::ReadableSessionDataInterface::inputMode::NORMAL;
                    else if (mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_CHANGE)
                        mode = vm::ReadableSessionDataInterface::inputMode::INSERT;
                    sd_i->setMode(mode);

                    unique_ptr<CommandSequence> cs = make_unique<CommandSequence>(sd_i);
                    cs->add(multiply(
                                make_unique<MotionBoundCommand>(
                                    sd_i,
                                    make_unique<MotionDelete>(sd_i, true, false, yank),
                                    make_unique<JumpLineEnd>(sd_i, false, false)),
                                currentMultiplier));
                    cs->add(make_unique<RelativeMoveCursor>(sd_i, 0, 0, true));
                    return std::move(cs);
                    break;
                }

                case '^':
                case '_': {
                    if (mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_DELETE || mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_YANK)
                        mode = vm::ReadableSessionDataInterface::inputMode::NORMAL;
                    else if (mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_CHANGE)
                        mode = vm::ReadableSessionDataInterface::inputMode::INSERT;
                    sd_i->setMode(mode);

                    unique_ptr<CommandSequence> cs = make_unique<CommandSequence>(sd_i);
                    cs->add(make_unique<JumpLineEnd>(sd_i, true, true));
                    cs->add(make_unique<SkipWS>(sd_i));
                    return multiply(
                                make_unique<MotionBoundCommand>(
                                    sd_i,
                                    make_unique<MotionDelete>(sd_i, true, false, yank),
                                    std::move(cs)),
                                currentMultiplier);
                }

                case 'w': {
                    if (mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_DELETE || mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_YANK)
                        mode = vm::ReadableSessionDataInterface::inputMode::NORMAL;
                    else if (mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_CHANGE)
                        mode = vm::ReadableSessionDataInterface::inputMode::INSERT;
                    sd_i->setMode(mode);

                    return multiply(
                                make_unique<MotionBoundCommand>(
                                    sd_i,
                                    make_unique<MotionDelete>(sd_i, true, false, yank),
                                    make_unique<JumpWord>(sd_i, false, false, false)),
                                currentMultiplier);
                    break;
                }

                case '%': {
                    if (mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_DELETE || mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_YANK)
                        mode = vm::ReadableSessionDataInterface::inputMode::NORMAL;
                    else if (mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_CHANGE)
                        mode = vm::ReadableSessionDataInterface::inputMode::INSERT;
                    sd_i->setMode(mode);

                    return make_unique<MotionBoundCommand>(
                                    sd_i,
                                    make_unique<MotionDelete>(sd_i, true, true, yank),
                                    make_unique<FindMatching>(sd_i));
                    break;
                }

                case 'n': {
                    if (lastSearchDir) {
                        sd_i->setStatusLine("?"+searchedString);
                    } else {
                        sd_i->setStatusLine("/"+searchedString);
                    }
                    if (mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_DELETE || mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_YANK)
                        mode = vm::ReadableSessionDataInterface::inputMode::NORMAL;
                    else if (mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_CHANGE)
                        mode = vm::ReadableSessionDataInterface::inputMode::INSERT;
                    sd_i->setMode(mode);

                    return multiply(make_unique<MotionBoundCommand>(
                                    sd_i,
                                    make_unique<MotionDelete>(sd_i, true, false, yank),
                                    make_unique<Search>(sd_i, searchedString,
                                        lastSearchDir)),
                                    currentMultiplier);
                    break;
                }

                case 'N': {
                    if (lastSearchDir) {
                        sd_i->setStatusLine("/"+searchedString);
                    } else {
                        sd_i->setStatusLine("?"+searchedString);
                    }
                    if (mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_DELETE || mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_YANK)
                        mode = vm::ReadableSessionDataInterface::inputMode::NORMAL;
                    else if (mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_CHANGE)
                        mode = vm::ReadableSessionDataInterface::inputMode::INSERT;
                    sd_i->setMode(mode);

                    return multiply(make_unique<MotionBoundCommand>(
                                    sd_i,
                                    make_unique<MotionDelete>(sd_i, true, false, yank),
                                    make_unique<Search>(sd_i, searchedString,
                                        !lastSearchDir)),
                                    currentMultiplier);
                    break;
                }

                case 'W':{
                    if (mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_DELETE || mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_YANK)
                        mode = vm::ReadableSessionDataInterface::inputMode::NORMAL;
                    else if (mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_CHANGE)
                        mode = vm::ReadableSessionDataInterface::inputMode::INSERT;
                    sd_i->setMode(mode);

                    return multiply(
                                make_unique<MotionBoundCommand>(
                                    sd_i,
                                    make_unique<MotionDelete>(sd_i, true, false, yank),
                                    make_unique<JumpWord>(sd_i, false, true, false)),
                                currentMultiplier);
                    break;
                }

                case 'e':{
                    if (mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_DELETE || mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_YANK)
                        mode = vm::ReadableSessionDataInterface::inputMode::NORMAL;
                    else if (mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_CHANGE)
                        mode = vm::ReadableSessionDataInterface::inputMode::INSERT;
                    sd_i->setMode(mode);

                    unique_ptr<CommandSequence> cs = make_unique<CommandSequence>(sd_i);
                    cs->add(make_unique<JumpWord>(sd_i, false, false, true));
                    cs->add(make_unique<RelativeMoveCursor>(sd_i, 0, 1, false));
                    return multiply(
                                make_unique<MotionBoundCommand>(
                                    sd_i,
                                    make_unique<MotionDelete>(sd_i, true, false, yank),
                                    std::move(cs)),
                                currentMultiplier);
                    break;
                }

                case 'E':{
                    if (mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_DELETE || mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_YANK)
                        mode = vm::ReadableSessionDataInterface::inputMode::NORMAL;
                    else if (mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_CHANGE)
                        mode = vm::ReadableSessionDataInterface::inputMode::INSERT;
                    sd_i->setMode(mode);

                    unique_ptr<CommandSequence> cs = make_unique<CommandSequence>(sd_i);
                    cs->add(make_unique<JumpWord>(sd_i, false, true, true));
                    cs->add(make_unique<RelativeMoveCursor>(sd_i, 0, 1, false));
                    return multiply(
                                make_unique<MotionBoundCommand>(
                                    sd_i,
                                    make_unique<MotionDelete>(sd_i, true, false, yank),
                                    std::move(cs)),
                                currentMultiplier);
                    break;
                }

                case 'b':{
                    if (mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_DELETE || mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_YANK)
                        mode = vm::ReadableSessionDataInterface::inputMode::NORMAL;
                    else if (mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_CHANGE)
                        mode = vm::ReadableSessionDataInterface::inputMode::INSERT;
                    sd_i->setMode(mode);

                    return multiply(
                                make_unique<MotionBoundCommand>(
                                    sd_i,
                                    make_unique<MotionDelete>(sd_i, true, false, yank),
                                    make_unique<JumpWord>(sd_i, true, false, true)),
                                currentMultiplier);
                    break;
                }

                case 'B':{
                    if (mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_DELETE || mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_YANK)
                        mode = vm::ReadableSessionDataInterface::inputMode::NORMAL;
                    else if (mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_CHANGE)
                        mode = vm::ReadableSessionDataInterface::inputMode::INSERT;
                    sd_i->setMode(mode);

                    return multiply(
                                make_unique<MotionBoundCommand>(
                                    sd_i,
                                    make_unique<MotionDelete>(sd_i, true, false, yank),
                                    make_unique<JumpWord>(sd_i, true, true, true)),
                                currentMultiplier);
                    break;
                }

                default:
                    if (mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_DELETE || mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_YANK)
                        mode = vm::ReadableSessionDataInterface::inputMode::NORMAL;
                    else if (mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_CHANGE)
                        mode = vm::ReadableSessionDataInterface::inputMode::INSERT;
                    sd_i->setMode(mode);
                    break;
            }
        } else if (mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_MACRO_REG_CHOICE) {
          if (std::isalnum(input)) {
            mode = vm::ReadableSessionDataInterface::inputMode::NORMAL;
            isRecordingMacro = true;
            macroChar = char(input);
            sd_i->setMode(mode);
            sd_i->setIsRecordingMacro(isRecordingMacro);
            sd_i->setMacroChar(macroChar);
          }
        } else if (mode == vm::ReadableSessionDataInterface::inputMode::COMPOSE_RUN_MACRO_REG_CHOICE) {
          if ((std::isalnum(input) && !std::isupper(input)) || input == '@') {
              mode = vm::ReadableSessionDataInterface::inputMode::NORMAL;
              if (!sd_i->isRegEmpty(input)) {
                std::string raw_reg = (input == '@') ? sd_i->getLastUsedRegister() : sd_i->getRegister(char(input));
                std::unique_ptr<CommandSequence> return_cmd = std::make_unique<CommandSequence>(sd_i);
                isRunningMacro = true;
                sd_i->startUndoChunk();
                for (size_t i = 0; i < raw_reg.length(); ++i) {
                  return_cmd->add(processRaw(raw_reg[i]));
                }
                isRunningMacro = false;
                if (input != '@') sd_i->setLastUsedRegister(input);
                return std::move(return_cmd);
              }
          }
        }
        return make_unique<WakeUp>(sd_i);
    }

    unique_ptr<Command> CmdGenerator::exExecute(const string &s) {
        if (s == "q") {
            return make_unique<Quit>(sd_i, false);
        } else if (s == "q!") {
            return make_unique<Quit>(sd_i, true);
        } else if (s == "w") {
            return make_unique<WriteBuffer>(sd_i, sd_i->getFileName());
        } else if (s == "wq") {
            unique_ptr<CommandSequence> cmdSeq = make_unique<CommandSequence>(sd_i);
            cmdSeq->add(make_unique<WriteBuffer>(sd_i, sd_i->getFileName()));
            cmdSeq->add(make_unique<Quit>(sd_i, false));
            return std::move(cmdSeq);
        } else if (s.substr(0, 2) == "r ") {
            if (!isRunningMacro) sd_i->startUndoChunk();
            return make_unique<ReadFileBelowCursor>(sd_i, s.substr(2, string::npos));
        } else if (s == "$") {
            return make_unique<MetaJumpToLastLine>(sd_i);
        } else {
            int r = 0;
            bool w;
            try {
                r = std::stoi(s);
                w = true;
            } catch (...) {
                w = false;
            }
            if (w) {
                return make_unique<MetaJumpToLine>(sd_i, r);
            } else {
                return make_unique<Undefined>(sd_i);
            }
        }
    }

    unique_ptr<Command> CmdGenerator::genReadInitialFileCmd() {
      unique_ptr<ReadFile> readFileCmd = make_unique<ReadFile>(sd_i, sd_i->getFileName(), 1, 1, true);
      return std::move(readFileCmd);
    }

}
