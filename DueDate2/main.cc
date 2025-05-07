#include "ui.h"
#include "formatter.h"
#include "controller.h"
#include "cmdgen.h"
#include "model.h"
#include "dll_buffer.h"
#include "session_data_manager.h"
#include "command.h"
#include "view.h"
#include <string>
#include <vector>
#include <memory>
#include <iostream>

void initModel(vm::Model&);

int main(int argc, char** argv) {

    if (argc != 2) {
      std::cerr << "VM requires a file name. Please call './vm yourfilename'" << std::endl;
      return 1;
    }

    vm::Model model;
    vm::Controller controller;

    model.addBuffer(std::make_unique<vm::DllBuffer>(1));

    vm::View view(model, controller.getSessionDataInterface());

    view.addBuffer(*model.getBuffer());

    controller.setInputSource(view.getInputSource());
    controller.setModel(model);
    controller.setFileName(argv[1]);

    controller.start();

    return 0;
}
