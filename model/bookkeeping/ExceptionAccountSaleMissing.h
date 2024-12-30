#ifndef EXCEPTIONACCOUNTSALEMISSING_H
#define EXCEPTIONACCOUNTSALEMISSING_H

#include <QtCore/qexception.h>

class ExceptionAccountSaleMissing : public QException
{
public:
    void raise() const override;
    ExceptionAccountSaleMissing *clone() const override;

    const QString &accounts() const;
    void setAccounts(const QString &newAccounts);

private:
    QString m_accounts;

};


#endif // EXCEPTIONACCOUNTSALEMISSING_H
