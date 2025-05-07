#ifndef CMDGEN_H
#define CMDGEN_H
#include "rx.h"
#include "buffer_interface.h"
#include "command.h"
#include "session_data_interface.h"
#include <memory>
#include <ncurses.h>

using std::unique_ptr;
using std::string;

namespace vm {

    class CmdGenerator {
        SessionDataInterface *sd_i;
        char lastFound;

        // Used for undos on macros
        bool isRunningMacro = false;

        // Used for colon commands
        std::string exLine;
        std::string lastExLine;

        string currentMultiplier;

        // Used for macro recording
        // First two are updated in session_data_manager
        // Last one is only pushed once the recording stops
        bool isRecordingMacro = false;
        char macroChar;
        std::string rawMacro = "";

        ReadableSessionDataInterface::inputMode mode;
        ReadableSessionDataInterface::inputMode previousMode;
        bool lastFoundDir;     // was the last find backwards?
        bool lastSearchDir;     // was the last search backwards?
        string searchedString;

        unique_ptr<Command> multiply(unique_ptr<Command> &&, string &);
        unique_ptr<Command> processRaw(int input);
        unique_ptr<Command> exExecute(const string &s);

        public:
            CmdGenerator(SessionDataInterface *sd_i) : sd_i{sd_i}, lastFound{0},
                exLine{""}, lastExLine{""},
                mode{SessionDataInterface::inputMode::NORMAL},
                previousMode{SessionDataInterface::inputMode::NORMAL},
                lastFoundDir{false}, lastSearchDir{false},
                searchedString{""} {}
            unique_ptr<Command> feed(int input) { return processRaw(input); }
            unique_ptr<Command> genReadInitialFileCmd();
    };
}

#endif
