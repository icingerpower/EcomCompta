#ifndef EXCEPTIONACCOUNTDEPORTEDMISSING_H
#define EXCEPTIONACCOUNTDEPORTEDMISSING_H


#include <QtCore/qexception.h>

class ExceptionAccountDeportedMissing : public QException
{
public:
    void raise() const override;
    ExceptionAccountDeportedMissing *clone() const override;

    const QString &error() const;
    void setCountry(const QString &countryName);

private:
    QString m_error;

};


#endif // EXCEPTIONACCOUNTDEPORTEDMISSING_H
