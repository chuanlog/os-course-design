#include "gui/MainWindow.h"

#include <FL/Fl.H>

int main() {
    MainWindow window(1280, 910, "文件系统模拟器 - FLTK");
    window.show();
    return Fl::run();
}
