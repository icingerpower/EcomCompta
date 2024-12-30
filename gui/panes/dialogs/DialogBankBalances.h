#ifndef DIALOGBANKBALANCES_H
#define DIALOGBANKBALANCES_H

#include <QDialog>

namespace Ui {
class DialogBankBalances;
}

class DialogBankBalances : public QDialog
{
    Q_OBJECT

public:
    explicit DialogBankBalances(const QString &year, QWidget *parent = nullptr);
    ~DialogBankBalances();

protected slots:
    void onSelectedRow(int row);

private:
    Ui::DialogBankBalances *ui;
    QList<QList<QStringList>> m_listOfTable;
    QString m_year;
    void _compute();
};

#endif // DIALOGBANKBALANCES_H
