#include "OrderImporterCustomException.h"

//---------------------------------------------------------
void OrderImporterCustomException::raise() const
{
    throw *this;
}
//----------------------------------------------------------
OrderImporterCustomException *OrderImporterCustomException::clone() const
{
    return new OrderImporterCustomException(*this);
}
//----------------------------------------------------------
QString OrderImporterCustomException::error() const
{
    return m_error;
}
//----------------------------------------------------------
void OrderImporterCustomException::setError(const QString &error)
{
    m_error = error;
}
//----------------------------------------------------------
