#include "applauncher.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    AppLauncher l;
    l.show();

    return a.exec();
}
