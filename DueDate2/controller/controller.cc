#include "controller.h"

namespace vm {

    void Controller::start() {
        currentInput = 0;
        unique_ptr<Command> wakeUp = cmdGen->feed(currentInput);
        model->execute(*wakeUp);

        // This line assumes the controller setFileName method has been called
        unique_ptr<Command> putFileCmd = cmdGen->genReadInitialFileCmd();
        model->execute(*putFileCmd);
        while (running && model->isRunning()) {
            currentInput = takeInput();
            unique_ptr<Command> cmd = cmdGen->feed(currentInput);
            RepeatCommand *rc = dynamic_cast<RepeatCommand*>(&*cmd);
            if (rc) {
                model->execute(*lastCommand);
            } else {
                model->execute(*cmd);
                if (!dynamic_cast<RelativeMoveCursor*>(&*cmd))
                    lastCommand = std::move(cmd);
            }
        }
    }

    int Controller::takeInput() {
        return vi->getChar();
    }

}
