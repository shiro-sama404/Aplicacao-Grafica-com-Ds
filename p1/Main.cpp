#include "graphics/Application.h"
#include "MainWindow.h"

int
main(int argc, char** argv)
{
  puts("Ds template by Paulo Pagliosa (ppagliosa@gmail.com)\n");
  puts("Camera controls keys:\n"
    "(w) move forward  (s) move backward\n"
    "(a) move left     (d) move right\n"
    "(q) move up       (z) move down\n");
  puts("Mouse controls:\n"
    "(scroll wheel)    zoom\n"
    "(middle-click)    pan\n"
    "(Alt+right-click) rotate");
  return cg::Application{new MainWindow{1280, 720}}.run(argc, argv);
}
