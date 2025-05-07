#include "ui.h"

namespace vm {
 
    CursesSession::CursesSession() {
        initscr();
        start_color();
        raw();
        noecho();
        keypad(stdscr, TRUE);
        set_escdelay(1);
    }

    CursesSession::~CursesSession() {
        endwin();
    }

    CursesUI::CursesUI() {}

    CursesUI::~CursesUI() {}

    void CursesUI::cWriteString(int x, int y, string text, int colour) {
        attron(COLOR_PAIR(colour));
        mvprintw(y, x, "%s", text.c_str());
        attroff(COLOR_PAIR(colour));
    }

    void CursesUI::cMoveCursor(int x, int y) {
        move(y, x);
    }

    void CursesUI::cSetColour(int id, int fg, int bg) {
        init_pair(id, fg, bg);
    }

    void CursesUI::cDraw() {
        refresh();
    }

    int CursesUI::cGetChar() {
        return getch();
    }

    int CursesUI::cGetHeight() {
        return getmaxy(stdscr);
    }

    int CursesUI::cGetWidth() {
        return getmaxx(stdscr);
    }

}
