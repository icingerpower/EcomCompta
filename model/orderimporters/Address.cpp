#include <QtCore/qhash.h>
#include <QtCore/qdatetime.h>

#include "../common/countries/CountryManager.h"

#include "Address.h"

//----------------------------------------------------------
int Address::maxId = 0;
//==========================================================
Address::Address()
{
    m_internalId
            = "Address_" + QString::number(Address::maxId)
            + QDateTime::currentDateTime().toString("_yyyy-MM-dd-hh-mm-ss");
}
//==========================================================
Address::Address(
        const QString &fullName,
        const QString &street1,
        const QString &street2,
        const QString &street3,
        const QString &city,
        const QString &postalCode,
        const QString &countryCode,
        const QString &state,
        const QString &phoneNumber)
{
    m_fullName = fullName;
    m_internalId
            = "Address_" + QString::number(Address::maxId)
            + QDateTime::currentDateTime().toString("_yyyy-MM-dd-hh-mm-ss");
    m_street1 = street1;
    m_street2 = street2;
    m_street3 = street3;
    m_city = city;
    m_state = state;
    m_postalCode = postalCode;
    m_countryCode = countryCode;
    Q_ASSERT(countryCode.size() == 2
             || countryCode == "GB-NIR");
    m_phoneNumber = phoneNumber;
}
//==========================================================
Address &Address::operator=(const Address &address)
{
    m_label = address.m_label;
    m_internalId = address.internalId();
    m_fullName = address.m_fullName;
    m_street1 = address.m_street1;
    m_street2 = address.m_street2;
    m_street3 = address.m_street3;
    m_city = address.m_city;
    m_state = address.m_state;
    m_postalCode = address.m_postalCode;
    m_countryCode = address.m_countryCode;
    m_state = address.m_state;
    m_phoneNumber = address.m_phoneNumber;
    return *this;
}
//==========================================================
void Address::copyFromOther(const Address &address)
{
    QString previousInternalId = m_internalId;
    *this = address;
    m_internalId = previousInternalId;
}
//==========================================================
bool Address::isEmpty() const
{
    return m_countryCode == "";
}
//==========================================================
void Address::merge(const Address &other)
{
    if (m_fullName.isEmpty()) {
        m_fullName = other.m_fullName;
    }
    if (m_street1.isEmpty()) {
        m_street1 = other.m_street1;
    }
    if (m_street2.isEmpty()) {
        m_street2 = other.m_street2;
    }
    if (m_street3.isEmpty()) {
        m_street3 = other.m_street3;
    }
    if (m_state.isEmpty()) {
        m_state = other.m_state;
    }
    if (m_postalCode.isEmpty()) {
        m_postalCode = other.m_postalCode;
    }
    if (m_countryCode.isEmpty()
            || (m_countryCode.contains("-")
                && m_countryCode != "GB-NIR")) {
        m_countryCode = other.m_countryCode;
    }
    if (m_countryCode == "GB"
            && other.m_countryCode == "GB-NIR") {
        m_countryCode = other.m_countryCode; // TODO WORKAROUND => I should instead analyse each postal code to see if from Irelande or not
    }
    if (m_phoneNumber.isEmpty()) {
        m_phoneNumber = other.m_phoneNumber;
    }
}
//==========================================================
bool Address::operator!=(const Address &other) const
{
    bool different
            = m_city != other.m_city
            || m_label != other.m_label
            || m_state != other.m_state
            || m_street1 != other.m_street1
            || m_street2 != other.m_street2
            || m_street3 != other.m_street3
            || m_fullName != other.m_fullName
            || m_postalCode != other.m_postalCode
            || m_countryCode != other.m_countryCode;
    return different;
}
//==========================================================
bool Address::operator<(const Address &other) const
{
    static QHash<QString, int> priorities = {{"FR",1}, {"DE",2}, {"IT",3}, {"ES", 4}, {"PL", 5}, {"CZ", 6}, {"NL", 7}, {"SE", 8}};
    if (priorities.contains(m_countryCode)){
        if (!priorities.contains(other.m_countryCode) ) {
            return true;
        } else {
            return priorities[m_countryCode] < priorities[other.m_countryCode];
        }
    } else {
        if (priorities.contains(other.m_countryCode) ) {
            return false;
        } else {
            return m_countryCode < other.m_countryCode;
        }
    }
}
//==========================================================
QStringList Address::toStringList() const
{
    QStringList elements;
    elements << m_fullName;
    elements << m_street1;
    elements << m_street2;
    elements << m_street3;
    elements << m_postalCode + " " + m_city;
    elements << m_state;
    for (int i=elements.size()-1; i>=0; --i) {
        if (elements[i].size() < 3) {
            elements.removeAt(i);
        }
    }
    QString country = this->countryName();
    if (country.isEmpty()) {
        country = m_countryCode;
    }
    elements << country;
    return elements;
}
//==========================================================
QString Address::toString(const QString &sep) const
{
    QStringList elements;
    elements << m_fullName;
    elements << m_street1;
    elements << m_street2;
    elements << m_street3;
    elements << m_city;
    elements << m_state;
    elements << m_postalCode;
    elements << m_countryCode;
    elements << m_phoneNumber;
    elements << m_label;
    elements << m_internalId;
    return elements.join(sep);
}
//==========================================================
Address Address::fromString(
        const QString &string, const QString &sep)
{
    QStringList elements = string.split(sep);
    Address address;
    address.m_fullName = elements.takeFirst();
    address.m_street1 = elements.takeFirst();
    address.m_street2 = elements.takeFirst();
    address.m_street3 = elements.takeFirst();
    address.m_city = elements.takeFirst();
    address.m_state = elements.takeFirst();
    address.m_postalCode = elements.takeFirst();
    address.m_countryCode = elements.takeFirst();
    address.m_phoneNumber = elements.takeFirst();
    if (elements.size() > 0) { //TODO delete and recreate addresses
        address.m_label = elements.takeFirst();
        address.m_internalId = elements.takeFirst();
    }
    return address;
}
//==========================================================
Address Address::fromFbaInfos(
        const QString &code,
        const QString &countryCode,
        const QString &city,
        const QString &postalCode)
{
    Address address;
    address.m_fullName = code;
    address.m_countryCode = countryCode;
    address.m_city = city;
    address.m_postalCode = postalCode;
    return address;
}
//==========================================================
QString Address::guessVatNumberFromStreet() const
{
    QStringList list = {m_street1, m_street2, m_street3};
    for (auto it = list.begin(); // TODO add VAT in Address
         it != list.end(); ++it) {
        if (it->contains("TVA:")) {
            QStringList elements = it->split(" ");
            return elements[1];
        }
    }
    return QString();
}
//==========================================================
QString Address::fullName() const
{
    return m_fullName;
}
//==========================================================
void Address::setFullName(const QString &fullName)
{
    m_fullName = fullName;
}
//==========================================================
QString Address::street1() const
{
    return m_street1;
}
//==========================================================
void Address::setStreet1(const QString &street1)
{
    m_street1 = street1;
}
//==========================================================
QString Address::street2() const
{
    return m_street2;
}
//==========================================================
void Address::setStreet2(const QString &street2)
{
    m_street2 = street2;
}
//==========================================================
QString Address::street3() const
{
    return m_street3;
}
//==========================================================
void Address::setStreet3(const QString &street3)
{
    m_street3 = street3;
}
//==========================================================
QString Address::state() const
{
    return m_state;
}
//==========================================================
void Address::setState(const QString &state)
{
    m_state = state;
}
//==========================================================
QString Address::postalCode() const
{
    return m_postalCode;
}
//==========================================================
void Address::setPostalCode(const QString &postalCode)
{
    m_postalCode = postalCode;
}
//==========================================================
QString Address::countryName() const
{
    return CountryManager::instance()->countryName(m_countryCode);
}
//==========================================================
QString Address::countryCode() const
{
    return m_countryCode;
}
//==========================================================
void Address::setCountryCode(const QString &countryCode)
{
    Q_ASSERT(countryCode.size() == 2);
    m_countryCode = countryCode;
}
//==========================================================
QString Address::phoneNumber() const
{
    return m_phoneNumber;
}
//==========================================================
void Address::setPhoneNumber(const QString &phoneNumber)
{
    m_phoneNumber = phoneNumber;
}
//==========================================================
QString Address::city() const
{
    return m_city;
}
//==========================================================
void Address::setCity(const QString &city)
{
    m_city = city;
}
//==========================================================
QString Address::internalId() const
{
    return m_internalId;
}
//==========================================================
QString Address::label() const
{
    return m_label;
}
//==========================================================
void Address::setLabel(const QString &label)
{
    m_label = label;
}
//==========================================================

