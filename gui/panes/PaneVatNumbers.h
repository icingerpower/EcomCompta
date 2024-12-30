#ifndef PANEVATNUMBERS_H
#define PANEVATNUMBERS_H

#include <QWidget>

class DialogAddVatnumber;

namespace Ui {
class PaneVatNumbers;
}

class PaneVatNumbers : public QWidget
{
    Q_OBJECT

public:
    explicit PaneVatNumbers(QWidget *parent = nullptr);
    ~PaneVatNumbers();
    void addVatNumber();
    void removeVatNumberSelected();

private:
    Ui::PaneVatNumbers *ui;
    DialogAddVatnumber *m_dialogAddNumber;
    void _connectSlots();
};

#endif // PANEVATNUMBERS_H
