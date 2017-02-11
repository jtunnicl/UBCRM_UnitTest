#ifndef UBCRMWINDOW_H
#define UBCRMWINDOW_H

#include <QMainWindow>

namespace Ui {
class UBCRMWindow;
}

class UBCRMWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit UBCRMWindow(QWidget *parent = 0);
    ~UBCRMWindow();

private:
    Ui::UBCRMWindow *ui;
};

#endif // UBCRMWINDOW_H
