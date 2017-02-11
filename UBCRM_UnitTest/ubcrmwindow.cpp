#include "ubcrmwindow.h"
#include "ui_ubcrmwindow.h"

UBCRMWindow::UBCRMWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::UBCRMWindow)
{
    ui->setupUi(this);
}

UBCRMWindow::~UBCRMWindow()
{
    delete ui;
}
