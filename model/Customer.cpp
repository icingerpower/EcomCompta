#include <QtCore/qdatetime.h>
#include <QtCore/qstringlist.h>

#include "Customer.h"

//----------------------------------------------------------
int Customer::maxId = 0;
//----------------------------------------------------------
Customer::Customer()
{
}
//----------------------------------------------------------
Customer::Customer(
        const QString &name,
        const QString &notes,
        const QString &siret,
        const QString &phoneNumber,
        const QString &email,
        const QString &currency)
{
    Customer::maxId++;
    m_internalId
            = "Customer_" + QString::number(Customer::maxId)
            + QDateTime::currentDateTime().toString("_yyyy-MM-dd-hh-mm-ss_")
            + m_name;
    m_name = name;
    m_currency = currency;
    m_notes = notes;
    m_siret = siret;
    m_phoneNumber = phoneNumber;
    m_email = email;
}
//----------------------------------------------------------
Customer::Customer(const QString &string)
{
    loadFromString(string);
}
//----------------------------------------------------------
Customer &Customer::operator=(const Customer &customer)
{
    m_name = customer.m_name;
    m_notes = customer.m_notes;
    m_siret = customer.m_siret;
    m_phoneNumber = customer.m_phoneNumber;
    m_email = customer.m_email;
    m_internalId = customer.m_internalId;
    m_currency = customer.m_currency;
    return *this;
}
//----------------------------------------------------------
void Customer::copyFromOther(const Customer &address)
{
    QString internalIdTemp = m_internalId;
    *this = address;
    m_internalId = internalIdTemp;
}
//----------------------------------------------------------
bool Customer::operator!=(const Customer &customer2) const
{
    bool different
            = m_name != customer2.m_name
            || m_notes != customer2.m_notes
            || m_siret != customer2.m_siret
            || m_currency != customer2.m_currency
            || m_phoneNumber != customer2.m_phoneNumber
            || m_email != customer2.m_email;
    return different;
}
//----------------------------------------------------------
bool Customer::isEmpty() const
{
    bool is = m_name.isEmpty()
            && m_notes.isEmpty()
            && m_siret.isEmpty()
            && m_phoneNumber.isEmpty()
            && m_currency.isEmpty()
            && m_email.isEmpty();
    return is;
}
//----------------------------------------------------------
QString Customer::toString() const
{
    QStringList elements;
    elements << m_internalId;
    elements << m_name;
    elements << m_notes;
    elements << m_siret;
    elements << m_phoneNumber;
    elements << m_email;
    elements << m_currency;
    QString string = elements.join(";;;");
    return string;
}
//----------------------------------------------------------
void Customer::loadFromString(const QString &string)
{
    QStringList elements = string.split(";;;");
    m_internalId = elements.takeFirst();
    m_name = elements.takeFirst();
    m_notes = elements.takeFirst();
    m_siret = elements.takeFirst();
    m_phoneNumber = elements.takeFirst();
    m_email = elements.takeFirst();
    if (elements.size() > 0) {
        m_currency = elements.takeFirst(); // for previously saved files
    } else {
        m_currency = "EUR";
    }
}
//----------------------------------------------------------
QString Customer::name() const
{
    return m_name;
}
//----------------------------------------------------------
void Customer::setName(const QString &name)
{
    m_name = name;
}
//----------------------------------------------------------
QString Customer::notes() const
{
    return m_notes;
}
//----------------------------------------------------------
void Customer::setNotes(const QString &notes)
{
    m_notes = notes;
}
//----------------------------------------------------------
QString Customer::siret() const
{
    return m_siret;
}
//----------------------------------------------------------
void Customer::setSiret(const QString &siret)
{
    m_siret = siret;
}
//----------------------------------------------------------
QString Customer::phoneNumber() const
{
    return m_phoneNumber;
}
//----------------------------------------------------------
void Customer::setPhoneNumber(const QString &phoneNumber)
{
    m_phoneNumber = phoneNumber;
}
//----------------------------------------------------------
QString Customer::email() const
{
    return m_email;
}
//----------------------------------------------------------
void Customer::setEmail(const QString &email)
{
    m_email = email;
}
//----------------------------------------------------------
QString Customer::internalId() const
{
    return m_internalId;
}

QString Customer::currency() const
{
    return m_currency;
}

void Customer::setCurrency(const QString &currency)
{
    m_currency = currency;
}
//----------------------------------------------------------
