#include "gui/MainWindow.h"

#include <FL/Fl.H>

int main() {
    MainWindow window(1280, 930, "内存管理模拟器 - FLTK");
    window.show();
    return Fl::run();
}
