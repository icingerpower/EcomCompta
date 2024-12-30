#include "ExceptionAccountSaleMissing.h"


//----------------------------------------------------------
void ExceptionAccountSaleMissing::raise() const
{
    throw *this;
}
//----------------------------------------------------------
ExceptionAccountSaleMissing *ExceptionAccountSaleMissing::clone() const
{
    return new ExceptionAccountSaleMissing(*this);
}
//----------------------------------------------------------
const QString &ExceptionAccountSaleMissing::accounts() const
{
    return m_accounts;
}
//----------------------------------------------------------
void ExceptionAccountSaleMissing::setAccounts(
        const QString &newAccounts)
{
    m_accounts = newAccounts;
}
//----------------------------------------------------------

