#include <QObject>

#include "ExceptionAccountDeportedMissing.h"


//----------------------------------------------------------
void ExceptionAccountDeportedMissing::raise() const
{
    throw *this;
}
//----------------------------------------------------------
ExceptionAccountDeportedMissing *ExceptionAccountDeportedMissing::clone() const
{
    return new ExceptionAccountDeportedMissing(*this);
}
//----------------------------------------------------------
const QString &ExceptionAccountDeportedMissing::error() const
{
    return m_error;
}
//----------------------------------------------------------
void ExceptionAccountDeportedMissing::setCountry(
        const QString &countryName)
{
    m_error = QObject::tr("Les comptes de stock déportés sont manquants pour les pays suivants:");
    m_error += " ";
    m_error += countryName;
}
//----------------------------------------------------------
