#ifndef UI_H
#define UI_H
#include <ncurses.h>
#include <string>
#include "rx.h"

using std::string;

namespace vm {

    class UI: public Subject {
        virtual void cWriteString(int x, int y, string text, int colour) = 0;
        virtual void cMoveCursor(int x, int y) = 0;
        virtual void cSetColour(int id, int fg, int bg) = 0;
        virtual void cDraw() = 0;
        public:
            void writeString(int x, int y, string text, int colour) { cWriteString(x, y, text, colour); }
            void moveCursor(int x, int y) { cMoveCursor(x, y); }
            void setColour(int id, int fg, int bg) { cSetColour(id, fg, bg); }
            void draw() { cDraw(); }
            virtual ~UI() {}
    };

    class ViewInput {
        virtual int cGetChar() = 0;
        public:
            int getChar() { return cGetChar(); }
    };
    
    class ViewInfo {
        virtual int cGetHeight() = 0;
        virtual int cGetWidth() = 0;
        public:
            int getHeight() { return cGetHeight(); }
            int getWidth() { return cGetWidth(); }
    };

    class CursesSession {      // RAII on the display mode
        public:
            CursesSession();  // resize tolerance not yet implemented
            ~CursesSession();
    };

    class CursesUI : public UI, public ViewInput, public ViewInfo {
        CursesSession cs;
        void cWriteString(int x, int y, string text, int colour) override;
        void cMoveCursor(int x, int y) override;
        void cSetColour(int id, int fg, int bg) override;
        void cDraw() override;
        int cGetChar() override;
        int cGetHeight() override;
        int cGetWidth() override;
        public:
            CursesUI();
            ~CursesUI();
    };
}

#endif
