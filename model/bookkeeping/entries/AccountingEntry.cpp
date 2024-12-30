#include "../common/currencies/CurrencyRateManager.h"

#include "model/CustomerManager.h"


#include "AccountingEntry.h"

//----------------------------------------------------------
AccountingEntry::AccountingEntry()
{
    m_main = false;
    m_currency = CustomerManager::instance()->getSelectedCustomerCurrency();
    m_rateForConversion = 1.;
}
//----------------------------------------------------------
QString AccountingEntry::journal() const
{
    return m_journal;
}
//----------------------------------------------------------
void AccountingEntry::setJournal(const QString &journal)
{
    m_journal = journal;
}
//----------------------------------------------------------
QDate AccountingEntry::date() const
{
    return m_date;
}
//----------------------------------------------------------
void AccountingEntry::setDate(const QDate &date)
{
    m_date = date;
}
//----------------------------------------------------------
QString AccountingEntry::account() const
{
    return m_account;
}
//----------------------------------------------------------
void AccountingEntry::setAccount(const QString &account)
{
    m_account = account;
}
//----------------------------------------------------------
QString AccountingEntry::fileRelWorkingDir() const
{
    return m_file;
}
//----------------------------------------------------------
void AccountingEntry::setFileRelWorkingDir(const QString &file)
{
    m_file = file;
}
//----------------------------------------------------------
QString AccountingEntry::label() const
{
    return m_label;
}
//----------------------------------------------------------
void AccountingEntry::setLabel(const QString &label)
{
    m_label = label;
}
//----------------------------------------------------------
double AccountingEntry::debitOrigDouble() const
{
    return m_debitOrig.toDouble();
}
//----------------------------------------------------------
QString AccountingEntry::debitOrig() const
{
    return m_debitOrig;
}
//----------------------------------------------------------
void AccountingEntry::setDebitOrig(const QString &debit)
{
    m_debitOrig = debit;
}
//----------------------------------------------------------
void AccountingEntry::setDebitOrig(double debit)
{
    m_debitOrig = QString::number(debit, 'f', 2);
}
//----------------------------------------------------------
double AccountingEntry::creditOrigDouble() const
{
    return m_creditOrig.toDouble();
}
//----------------------------------------------------------
QString AccountingEntry::creditOrig() const
{
    return m_creditOrig;
}
//----------------------------------------------------------
void AccountingEntry::setCreditOrig(const QString &credit)
{
    m_creditOrig = credit;
}
//----------------------------------------------------------
void AccountingEntry::setCreditOrig(double credit)
{
    m_creditOrig = QString::number(credit, 'f', 2);
}
//----------------------------------------------------------
/*
double AccountingEntry::amountDoubleConv() const
{
    return m_debitOrig.toDouble() - m_creditOrig.toDouble();
}
//----------------------------------------------------------
QString AccountingEntry::amountCounv() const
{
    return QString::number(amountDoubleConv(), 'f', 2);
}
//*/
//----------------------------------------------------------
bool AccountingEntry::isMain() const
{
    return m_main;
}
//----------------------------------------------------------
void AccountingEntry::setMain(bool main)
{
    m_main = main;
}
/*
//----------------------------------------------------------
double AccountingEntry::amountOrigCurrencyDouble() const
{
    double amount = amountDoubleConv()/qAbs(amountDoubleConv())
            * qAbs(m_amountOrigCurrency);
    return amount;
}
//----------------------------------------------------------
QString AccountingEntry::amountOrigCurrency() const
{
    return QString::number(amountOrigCurrencyDouble(), 'f', 2);
}
//----------------------------------------------------------
void AccountingEntry::setAmountOrigCurrency(
        double amountOrigCurrency)
{
    m_amountOrigCurrency = amountOrigCurrency;
}
//----------------------------------------------------------
void AccountingEntry::setAmountOrigCurrency(
        const QString &amountOrigCurrency)
{
    m_amountOrigCurrency = amountOrigCurrency.toDouble();
}
//*/
//----------------------------------------------------------
QString AccountingEntry::currency() const
{
    return m_currency;
}
//----------------------------------------------------------
void AccountingEntry::setCurrency(const QString &currency)
{
    m_currency = currency;
}
//----------------------------------------------------------
double AccountingEntry::rateForConversion() const
{
    return m_rateForConversion;
}
//----------------------------------------------------------
void AccountingEntry::setRateForConversion(double rateForConversion)
{
    m_rateForConversion = rateForConversion;
}

const QString &AccountingEntry::type() const
{
    return m_type;
}
//----------------------------------------------------------
void AccountingEntry::setType(
        const QString &newType)
{
    m_type = newType;
}
//----------------------------------------------------------
QString AccountingEntry::accountReplaced() const
{
    if (m_accountReplaced.isEmpty()) {
        return m_account;
    }
    return m_accountReplaced;
}
//----------------------------------------------------------
void AccountingEntry::setAccountReplaced(const QString &accountReplaced)
{
    m_accountReplaced = accountReplaced;
}
//----------------------------------------------------------
QString AccountingEntry::labelReplaced() const
{
    if (m_labelReplaced.isEmpty()) {
        return m_label;
    }
    return m_labelReplaced;
}
//----------------------------------------------------------
void AccountingEntry::setLabelReplaced(const QString &labelReplaced)
{
    m_labelReplaced = labelReplaced;
}
//----------------------------------------------------------
QString AccountingEntry::amountOrig() const
{
    return QString::number(amountOrigDouble(), 'f', 2);
}
//----------------------------------------------------------
double AccountingEntry::amountOrigDouble() const
{
    return debitOrigDouble() - creditOrigDouble();
}
//----------------------------------------------------------
QString AccountingEntry::amountConv() const
{
    return QString::number(amountConvDouble(), 'f', 2);
}
//----------------------------------------------------------
double AccountingEntry::amountConvDouble() const
{
    return creditConvDouble() - debitConvDouble();
}
//----------------------------------------------------------
QString AccountingEntry::debitConv() const
{
    return QString::number(debitConvDouble(), 'f', 2);
}
//----------------------------------------------------------
double AccountingEntry::debitConvDouble() const
{
    if (m_currency != CustomerManager::instance()->getSelectedCustomerCurrency()) {
        double rate = m_rateForConversion;
        if (qAbs(rate - 1) < 0.00001) {
            rate = CurrencyRateManager::instance()->rate(
                        m_currency,
                        CustomerManager::instance()->getSelectedCustomerCurrency(),
                        m_date);
        }
        return m_debitOrig.toDouble() * rate;
    }
    return m_debitOrig.toDouble();
}
//----------------------------------------------------------
QString AccountingEntry::creditConv() const
{
    return QString::number(creditConvDouble(), 'f', 2);
}
//----------------------------------------------------------
double AccountingEntry::creditConvDouble() const
{
    if (m_currency != CustomerManager::instance()->getSelectedCustomerCurrency()) {
        double rate = m_rateForConversion;
        if (qAbs(rate - 1) < 0.00001) {
            rate = CurrencyRateManager::instance()->rate(
                        m_currency,
                        CustomerManager::instance()->getSelectedCustomerCurrency(),
                        m_date);
        }
        return m_creditOrig.toDouble() * rate;
    }
    return m_creditOrig.toDouble();
}
//----------------------------------------------------------
