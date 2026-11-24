#include <QApplication>
#include "clockwidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    ClockWidget clock;
    clock.show();

    return a.exec();
}
