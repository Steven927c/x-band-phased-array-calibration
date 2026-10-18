#include "demo_window.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    radarcal::DemoWindow window;
    window.show();
    return application.exec();
}

