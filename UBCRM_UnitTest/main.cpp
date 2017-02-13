#include "ubcrmwindow.h"
#include <QApplication>
#include <QWidget>
#include "ubcrmwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    UBCRMWindow w;
    w.show();

    return a.exec();
}
