#include "view.h"

namespace vm {

    void View::addBuffer(ReadableBufferInterface &rb) {
        unique_ptr<Formatter> fmtr =
            make_unique<CFormatter>(CFormatter(*ui, *ui, rb, *sd_i));
        formatters.push_back(std::move(fmtr));
        model->attach(*formatters.back());
    }
}
