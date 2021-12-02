#include "boardwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);

    BoardWindow window;
    window.show();
    return application.exec();
}
