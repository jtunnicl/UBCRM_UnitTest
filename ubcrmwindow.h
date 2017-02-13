#ifndef UBCRMWINDOW_H
#define UBCRMWINDOW_H

#include <QMainWindow>
#include <QtCore>
#include <QWidget>
#include <QChart>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <vector>
#include <cmath>

#include "ubcrm_fcns.h"
#include "ubcrm_engine.h"


namespace Ui {
class UBCRMWindow;
}


class UBCRMWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit UBCRMWindow(QWidget *parent = 0);
    ~UBCRMWindow();

    void setupChart();

private:
    Ui::UBCRMWindow *ui;
    NodeCHObject *ch;
};


#endif // UBCRMWINDOW_H
