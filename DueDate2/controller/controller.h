#ifndef CONTROLLER_H
#define CONTROLLER_H
#include "ui.h"
#include "rx.h"
#include "command.h"
#include "cmdgen.h"
#include "model.h"
#include <string>
#include <memory>

using std::string;
using std::unique_ptr;
using std::make_unique;

namespace vm {
    
    class SessionDataManager;

    class Controller : public Subject {
          std::unique_ptr<SessionDataInterface> sd_i = nullptr;
          bool running;
          int currentInput;
          ViewInput *vi;
          unique_ptr<CmdGenerator> cmdGen;
          Model *model;

          int takeInput();
          void cStop() { running = false; }
          int cGetInput() { return currentInput; }
          unique_ptr<Command> lastCommand;

        public:
          Controller() : sd_i{std::make_unique<SessionDataManager>()}, running{true}, currentInput{0},
              cmdGen{make_unique<CmdGenerator>(CmdGenerator(sd_i.get()))},
              model{nullptr}, lastCommand{nullptr} {}
          ~Controller() {};
          void start();
          SessionDataInterface* getSessionDataInterface() { return sd_i.get(); }
          void setFileName(std::string fileName) { sd_i->setFileName(fileName); }
          void setInputSource(ViewInput &j) { vi = &j; }
          void setModel(Model &m) { model = &m; }
    };
}

#endif
