#ifndef CUSTOMER_H
#define CUSTOMER_H

#include <QtCore/qstring.h>

class Customer
{
public:
    Customer();
    Customer(const QString &name,
             const QString &notes,
             const QString &siret,
             const QString &phoneNumber,
             const QString &email,
             const QString &currency);
    Customer(const QString &string);

    Customer &operator=(const Customer &address);
    void copyFromOther(const Customer &address);
    bool operator!=(const Customer &customer2) const;
    bool isEmpty() const;

    /// TO UPDATE EACH PARAMETERS UPDATE
    QString toString() const;
    void loadFromString(const QString &string);

    QString name() const;
    void setName(const QString &name);

    QString notes() const;
    void setNotes(const QString &notes);

    QString siret() const;
    void setSiret(const QString &siret);

    QString phoneNumber() const;
    void setPhoneNumber(const QString &phoneNumber);

    QString email() const;
    void setEmail(const QString &email);

    QString internalId() const;

    QString currency() const;
    void setCurrency(const QString &currency);

private:
    QString m_internalId;
    QString m_name;
    QString m_currency;
    QString m_notes;
    QString m_siret;
    QString m_phoneNumber;
    QString m_email;
    static int maxId;
};

#endif // CUSTOMER_H
