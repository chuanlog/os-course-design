#include "gui/MainWindow.h"

#include <FL/Fl.H>

int main() {
    MainWindow window(1280, 980, "处理机调度模拟器 - FLTK");
    window.show();
    return Fl::run();
}
