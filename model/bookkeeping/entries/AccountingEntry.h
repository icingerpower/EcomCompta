#ifndef ACCOUNTINGENTRY_H
#define ACCOUNTINGENTRY_H

#include <QtCore/qstringlist.h>
#include <QtCore/qvariant.h>
#include <QtCore/qdatetime.h>

class AccountingEntry
{
public:
    AccountingEntry();

    QString journal() const;
    void setJournal(const QString &journal);

    QDate date() const;
    void setDate(const QDate &date);

    QString account() const;
    void setAccount(const QString &account);

    QString accountReplaced() const;
    void setAccountReplaced(const QString &accountReplaced);

    QString fileRelWorkingDir() const;
    void setFileRelWorkingDir(const QString &fileRelWorkingDir);

    //QString reference() const;
    //void setReference(const QString &reference);

    QString label() const;
    void setLabel(const QString &label);

    QString labelReplaced() const;
    void setLabelReplaced(const QString &labelReplaced);

    /*
    double amountDoubleConv() const;
    QString amountCounv() const;
    //*/


    QString amountOrig() const;
    double amountOrigDouble() const;
    QString amountConv() const;
    double amountConvDouble() const;

    QString debitConv() const;
    double debitConvDouble() const;
    QString creditConv() const;
    double creditConvDouble() const;

    double debitOrigDouble() const;
    QString debitOrig() const;
    void setDebitOrig(const QString &debitOrig);
    void setDebitOrig(double debitOrig);

    double creditOrigDouble() const;
    QString creditOrig() const;
    void setCreditOrig(const QString &creditOrig);
    void setCreditOrig(double creditOrig);

    bool isMain() const;
    void setMain(bool isMain);

    /*
    double amountOrigCurrencyDouble() const;
    QString amountOrigCurrency() const;
    void setAmountOrigCurrency(const QString &amountOrigCurrency);
    void setAmountOrigCurrency(double amountOrigCurrency);
    //*/

    QString currency() const;
    void setCurrency(const QString &currency);

    double rateForConversion() const;
    void setRateForConversion(double rateForConversion);

    const QString &type() const;
    void setType(const QString &newType);

private:
    QString m_journal;
    QDate m_date;
    QString m_account;
    QString m_accountReplaced;
    QString m_file;
    QString m_label;
    QString m_labelReplaced;
    QString m_debitOrig;
    QString m_creditOrig;
    QString m_currency;
    QString m_type;
    bool m_main;
    double m_rateForConversion;

};

#endif // ACCOUNTINGENTRY_H
