#ifndef VIEW_H
#define VIEW_H
#include "model.h"
#include "readable_buffer_interface.h"
#include "session_data_interface.h"
#include "ui.h"
#include "formatter.h"
#include <memory>
#include <vector>
#include <utility>

using std::unique_ptr;
using std::make_unique;
using std::vector;

namespace vm {
    
    class View {

        unique_ptr<CursesUI> ui;
        vector<unique_ptr<Formatter>> formatters;
        Model *model;
        SessionDataInterface* sd_i;

        public:
            View(Model &model, SessionDataInterface* sd_i) :
                ui{make_unique<CursesUI>(CursesUI())},
                model{&model}, sd_i{sd_i} {
                  sd_i->setViewInfo(ui.get());
                }
            void addBuffer(ReadableBufferInterface &rb);
            ViewInput &getInputSource() { return *ui; }
            ViewInfo &getDisplayInfo() { return *ui; }
    };
}

#endif
