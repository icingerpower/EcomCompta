#ifndef DIALOGADDBANKACCOUNT_H
#define DIALOGADDBANKACCOUNT_H

#include <QDialog>

namespace Ui {
class DialogAddBankAccount;
}
class EntryParserBankTable;

class DialogAddBankAccount : public QDialog
{
    Q_OBJECT

public:
    explicit DialogAddBankAccount(QWidget *parent = nullptr);
    ~DialogAddBankAccount() override;
    QString getBankFilePath() const;
    QString getBankFilePathDisplay() const;
    QString getBank() const;
    QDate getDate() const;
    void clear();

    bool wasAccepted() const;

public slots:
    void accept() override;
    void browseFilePath();
    void browseFilePathDisplay();
    void setBank(const QString &bankName);
    void load();

private:
    Ui::DialogAddBankAccount *ui;
    EntryParserBankTable *m_bankTable;
    bool m_wasAccepted;
};

#endif // DIALOGADDBANKACCOUNT_H
