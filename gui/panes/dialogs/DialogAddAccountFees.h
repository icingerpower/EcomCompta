#ifndef DIALOGADDACCOUNTFEES_H
#define DIALOGADDACCOUNTFEES_H

#include <QDialog>

namespace Ui {
class DialogAddAccountFees;
}

class DialogAddAccountFees : public QDialog
{
    Q_OBJECT

public:
    explicit DialogAddAccountFees(QWidget *parent = nullptr);
    ~DialogAddAccountFees() override;
    QString getAccountNumber() const;
    QString getAccountLabel() const;

public slots:
    void clear();
    void accept() override;

private:
    Ui::DialogAddAccountFees *ui;
};

#endif // DIALOGADDACCOUNTFEES_H
