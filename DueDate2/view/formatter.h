#ifndef FORMATTER_H
#define FORMATTER_H
#include "ui.h"
#include "rx.h"
#include <string>
#include <list>
#include "readable_buffer_interface.h"
#include "cmdgen.h"
#include "session_data_interface.h"

using std::string;

namespace vm {

    class Formatter : public Observer {
        virtual void redraw() = 0;
        public:
            void notify() override { redraw(); }
            virtual ~Formatter() {}
    };

    class CFormatter : public Formatter {
        int visualTopLineOffset;
        UI &view;
        ViewInfo &viewInfo;
        ReadableBufferInterface &buffer;
        SessionDataInterface &sessionData;
        void redraw() override;

        struct Body {
            std::list<string> text;
            int cursorX;
            int cursorY;
        };

        Body makeBody(int width, int height);
        void drawBody(int x, int y, Body body);
        std::pair<string, int> makeStatusBar(size_t width);

        public:
            static const int tabSize;
            CFormatter(UI &v, ViewInfo &vi, ReadableBufferInterface &b,
                    SessionDataInterface &sd) :
                visualTopLineOffset{0}, view{v}, viewInfo{vi},
                buffer{b}, sessionData{sd} {}
    };
}

#endif
