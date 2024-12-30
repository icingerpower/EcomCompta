#ifndef ORDERIMPORTERCUSTOMEXCEPTION_H
#define ORDERIMPORTERCUSTOMEXCEPTION_H

#include <QtCore/qexception.h>

class OrderImporterCustomException : public QException
{
public:
    void raise() const override;
    OrderImporterCustomException *clone() const override;

    QString error() const;
    void setError(const QString &error);

private:
    QString m_error;

};

#endif // ORDERIMPORTERCUSTOMEXCEPTION_H
