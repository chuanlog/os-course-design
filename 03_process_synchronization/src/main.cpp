#include "gui/MainWindow.h"

#include <FL/Fl.H>

int main() {
    MainWindow window(1280, 920, "线程同步与互斥模拟器 - FLTK");
    window.show();
    return Fl::run();
}
