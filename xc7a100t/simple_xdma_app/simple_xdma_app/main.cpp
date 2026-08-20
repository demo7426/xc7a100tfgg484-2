#include "stdafx.h"
#include "simple_xdma_app.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    simple_xdma_app w;
    w.show();
    return a.exec();
}
