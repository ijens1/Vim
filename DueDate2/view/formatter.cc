#include "formatter.h"

using std::string;
using std::list;
using std::pair;

namespace vm {

    const int CFormatter::tabSize = 8;

    string tabTransform(const string &t, int width, int tabSize) {
        string s;
        int k=0;
        for (size_t i=0; i<t.length(); ++i) {
            if (t[i] == '\t') {
                int j = k % width;
                if (j <= width - (width % tabSize)) {
                    for (int u=0; u < tabSize - (j % tabSize); ++u) {
                        s += ' ';
                        ++k;
                    }
                } else {
                    for (int u=0; u < tabSize - (width % tabSize); ++u) {
                        s += ' ';
                        ++k;
                    }
                }
            } else {
                s += t[i];
                ++k;
            }
        }
        return s;
    }

    CFormatter::Body CFormatter::makeBody(int width, int height) {	

      /* 
         * Overview of this function
         *
         * This function starts with an empty list of strings.
         * It adds the line the cursor's on and the line under that.
         *
         * If that doesn't fill the screen completely:
         *
         *      Add whole lines to the top until you can't fit any more,
         *      then pad the bottom.
         *
         * If it does:
         *      
         *      If this is the same line as last time, use the old
         *      visual top line offset (how far off the screen the top
         *      of the top real line is), and adjust.
         *
         *      Otherwise, start from scratch.
         */
        

      // This is the list of lines to be written to the screen
        list<string> visualLines{};

        size_t bufferLength = buffer.getNumLines();

        pair<size_t, size_t> cursorCoords = sessionData.getCursor();

        size_t cursorLineIndex = cursorCoords.first;
        
      // Add the cursor line and the line after
        
        if (bufferLength > 0) {
            string cursorLine = tabTransform(buffer.getLine(cursorLineIndex),
                    width, tabSize);
            cursorCoords.second = tabTransform(buffer.getLine(cursorLineIndex)
                    .substr(0, cursorCoords.second), width, tabSize).length();
            for (size_t i = 0; i == 0 || i < cursorLine.length(); i += width) {
                visualLines.push_back(cursorLine.substr(i, width));
            }
        }

        int visualCursorLine = cursorCoords.second / width;

        if (cursorLineIndex + 1 <= bufferLength) {
            string secondLine = tabTransform(buffer.getLine(cursorLineIndex + 1),
                   width, tabSize);
            for (size_t i = 0; i == 0 || i < secondLine.length(); i += width) {
                visualLines.push_back(secondLine.substr(i, width));
            }
        }

      // Has this already filled up visualLines?
        
        if (visualLines.size() < static_cast<size_t>(height)) {
          // If not, add lines upward to the top line atomically
            int minTopLine = (cursorLineIndex - 1 < sessionData.getTopLine() && cursorLineIndex > 1)
                ? (cursorLineIndex - 1)
                : sessionData.getTopLine();
            int workingTopLine = cursorLineIndex - 1;

            while (workingTopLine >= minTopLine) {

                string newLine = tabTransform(buffer.getLine(workingTopLine),
                        width, tabSize);
                list<string> newVisualLines{};

              // split the real line into visual lines
                for (size_t i=0; i == 0 || i<newLine.length(); i+=width) {
                    newVisualLines.push_back(newLine.substr(i, width));
                }

              // Append it all at one or not at all
                if (newVisualLines.size() + visualLines.size()
                        <= static_cast<size_t>(height)) {
                    --workingTopLine;
                    visualCursorLine += newVisualLines.size();
                    visualLines.splice(visualLines.begin(), newVisualLines);
                } else {
                    break;
                }
            }

            ++workingTopLine; // last one failed; one below it was last
                              // to succeed

            for (size_t line = cursorLineIndex + 2; line <= bufferLength; ++line) {
              // Now pad the view, line by line, at the bottom

                string newLine = tabTransform(buffer.getLine(line), width, tabSize);

              // split the real line into visual lines
                for (size_t i=0; i == 0 || i<newLine.length(); i+=width) {
                    if (visualLines.size() < static_cast<size_t>(height)) {
                        visualLines.push_back(newLine.substr(i, width));
                    } else {
                        goto padding_break;
                    }
                }
            }

            padding_break:

          // Now we're *really* padding
            while (visualLines.size() < static_cast<size_t>(height)) {
                visualLines.push_back("~");
            }

            sessionData.setTopLine(workingTopLine);
            visualTopLineOffset = 0;

        } else {

          // Our first two lines filled us up

            sessionData.setTopLine(cursorLineIndex);
            
            int newVisualTopLineOffset;

          // If this is the same line we were on last time, keep the
          // same offset as before, accounting for the cursor
            if (cursorLineIndex == sessionData.getTopLine()) {
                newVisualTopLineOffset = 0;
                for (int i = visualTopLineOffset; i > 0 && visualCursorLine > 1
                        && visualLines.size() > static_cast<size_t>(height); --i) {
                    visualLines.pop_front();
                    --visualCursorLine;
                    ++newVisualTopLineOffset;
                }
            } else {
              // otherwise, it's a fresh start
                newVisualTopLineOffset = 0;
            }

          // Then trim what lines you can from the bottom and the rest from the top
            while (visualLines.size() > static_cast<size_t>(height)) {
                if (static_cast<size_t>(visualCursorLine + 2) < visualLines.size()) {
                    visualLines.pop_back();
                } else {
                    visualLines.pop_front();
                    --visualCursorLine;
                    ++newVisualTopLineOffset;
                }
            }

            visualTopLineOffset = newVisualTopLineOffset;
        }

        for (string &each : visualLines) {
            while (each.length() < static_cast<size_t>(width)) {
                each += " ";
            }
        }

        int cursorXPos= cursorCoords.second % width;

        return Body{visualLines, cursorXPos, visualCursorLine};
    }

    void CFormatter::drawBody(int x, int y, Body body) {
        for (string line : body.text) {
            view.writeString(x, y, line, 0);
            ++y;
        }
        view.moveCursor(body.cursorX, body.cursorY);
    }

    std::pair<string, int> CFormatter::makeStatusBar(size_t width) {
        vm::ReadableSessionDataInterface::inputMode mode = sessionData.getMode();
        vm::ReadableSessionDataInterface::inputMode previousMode = sessionData.getPreviousMode();

        //Assume that the macro char is set if isRecordingMacro is true
        bool isRecordingMacro = sessionData.isRecordingMacro();
        char macroChar = sessionData.getMacroChar();

        string insertMode = "";
        string otherMode = "";
        string exLine = "";

        switch (mode) {
          case vm::ReadableSessionDataInterface::inputMode::INSERT:
                insertMode = "-- INSERT -- ";
                sessionData.setStatusLine("");
                if (isRecordingMacro) {
                  insertMode += "recording @";
                  insertMode += macroChar;
                }
                break;
            case vm::ReadableSessionDataInterface::inputMode::REPLACE:
                insertMode = "-- REPLACE -- ";
                sessionData.setStatusLine("");
                if (isRecordingMacro) {
                  insertMode += "recording @";
                  insertMode += macroChar;
                }
                break;
            case vm::ReadableSessionDataInterface::inputMode::REPLACE_ONCE:
                otherMode = "r";
                break;
            case vm::ReadableSessionDataInterface::inputMode::COMPOSE_FIND:
                otherMode = "f";
                break;
            case vm::ReadableSessionDataInterface::inputMode::COMPOSE_RFIND:
                otherMode = "F";
                break;
            case vm::ReadableSessionDataInterface::inputMode::COMPOSE_DELETE:
                otherMode = "d";
                break;
            case vm::ReadableSessionDataInterface::inputMode::COMPOSE_YANK:
                otherMode = "y";
                break;
            case vm::ReadableSessionDataInterface::inputMode::COMPOSE_CHANGE:
                otherMode = "c";
                break;
            case vm::ReadableSessionDataInterface::inputMode::SUPERCOMPOSE_FIND:
            case vm::ReadableSessionDataInterface::inputMode::SUPERCOMPOSE_RFIND:
                switch(previousMode) {
                    case vm::ReadableSessionDataInterface::inputMode::COMPOSE_DELETE:
                        otherMode = "d";
                        break;
                    case vm::ReadableSessionDataInterface::inputMode::COMPOSE_YANK:
                        otherMode = "y";
                        break;
                    case vm::ReadableSessionDataInterface::inputMode::COMPOSE_CHANGE:
                        otherMode = "c";
                        break;
                    default:
                        otherMode = "";
                        break;
                }
                if (mode == vm::ReadableSessionDataInterface::inputMode::SUPERCOMPOSE_FIND)
                    otherMode += "f";
                else
                    otherMode += "F";
                break;
            case vm::ReadableSessionDataInterface::inputMode::EX:
                exLine = ":" + sessionData.getExLine();
                sessionData.setStatusLine("");
                break;
            case vm::ReadableSessionDataInterface::inputMode::SUPERCOMPOSE_RSEARCH:
            case vm::ReadableSessionDataInterface::inputMode::RSEARCH:
                exLine = "?" + sessionData.getSearchedString();
                sessionData.setStatusLine("");
                break;
            case vm::ReadableSessionDataInterface::inputMode::SUPERCOMPOSE_SEARCH:
            case vm::ReadableSessionDataInterface::inputMode::SEARCH:
                exLine = "/" + sessionData.getSearchedString();
                sessionData.setStatusLine("");
                break;
            default:
                if (isRecordingMacro) {
                  sessionData.setStatusLine("");
                  insertMode += "recording @";
                  insertMode += macroChar;
                }
                break;
        }

        
        if (insertMode == "")
            insertMode = sessionData.getStatusLine();

        while (insertMode.length() < 20) insertMode += " ";
        while (otherMode.length() < 8) otherMode = " " + otherMode;

        pair<size_t, size_t> cursorCoords = sessionData.getCursor();
        string coords = std::to_string(cursorCoords.first) + ","
            + std::to_string(cursorCoords.second);

        while (coords.length() < 8) coords = " " + coords;

        if (exLine == "") {
            if (width < insertMode.length()) {
                return std::pair<string, int>("", -1);
            } else if (width < insertMode.length() + 8) {
                return std::pair<string, int>(insertMode, -1);
            } else if (width < insertMode.length() + 16) {
                return std::pair<string, int>(insertMode + coords, -1);
            } else {
                string filler = "";
                for (size_t i=0; i<width-insertMode.length()-16; ++i) filler += " ";
                return std::pair<string, int>(insertMode
                        + filler + otherMode + coords, -1);
            }
        } else {
            string padded = exLine;
            for (size_t i=exLine.length(); i<width; ++i) padded += " ";
            return std::pair<string, int>(padded, exLine.length());
        }
    }

    void CFormatter::redraw() {
        int w = viewInfo.getWidth();
        int h = viewInfo.getHeight();
        Body b = makeBody(w, h-1);
        std::pair<string, int> statusBar = makeStatusBar(w);
        view.writeString(0, h-1, statusBar.first, 0);
        drawBody(0, 0, b);
        if (statusBar.second != -1) {
            view.moveCursor(statusBar.second, h-1);
        }
        view.draw();
    }

}
