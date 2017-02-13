#include "ubcrmwindow.h"
#include "ui_ubcrmwindow.h"
#include "ubcrm_engine.h"

UBCRMWindow::UBCRMWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::UBCRMWindow),
    ch(new NodeCHObject)
{
    ui->setupUi(this);
    setupChart();
    setWindowTitle("UBCRM Regime Solver");
}



void UBCRMWindow::setupChart()
{

}


void UBCRMWindow::regimeCalc()
{

}



UBCRMWindow::~UBCRMWindow()
{
    delete ui;
}
