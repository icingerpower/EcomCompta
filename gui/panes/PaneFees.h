#ifndef PANEFEES_H
#define PANEFEES_H

#include <QWidget>

namespace Ui {
class PaneFees;
}

class DialogAddAccountFees;

class PaneFees : public QWidget
{
    Q_OBJECT

public:
    explicit PaneFees(QWidget *parent = nullptr);
    ~PaneFees();

public slots:
    void addAccount();
    void deleteSelectedAccount();

private:
    Ui::PaneFees *ui;
    void _connectSlots();
    DialogAddAccountFees *m_dialogAddAccount;
};

#endif // PANEFEES_H
