#ifndef ADDRESS_H
#define ADDRESS_H

#include <QtCore/qstring.h>

class Address
{
public:
    Address();
    Address(const QString &fullName,
            const QString &street1,
            const QString &street2,
            const QString &street3,
            const QString &city,
            const QString &postalCode,
            const QString &countryCode,
            const QString &state,
            const QString &phoneNumber);
    Address &operator=(const Address &address);
    void copyFromOther(const Address &address);
    bool isEmpty() const;
    void merge(const Address &other);
    bool operator!=(const Address &other) const;
    bool operator<(const Address &other) const;
    QStringList toStringList() const;
    QString toString(const QString &sep = ";;;") const;
    static Address fromString(const QString &string,
                              const QString &sep = ";;;");
    static Address fromFbaInfos(
            const QString &code,
            const QString &countryCode,
            const QString &city,
            const QString &postalCode);
    QString guessVatNumberFromStreet() const;

    QString fullName() const;
    void setFullName(const QString &fullName);

    QString street1() const;
    void setStreet1(const QString &street1);

    QString street2() const;
    void setStreet2(const QString &street2);

    QString street3() const;
    void setStreet3(const QString &street3);

    QString state() const;
    void setState(const QString &state);

    QString postalCode() const;
    void setPostalCode(const QString &postalCode);

    QString countryName() const;
    QString countryCode() const;
    void setCountryCode(const QString &countryCode);

    QString phoneNumber() const;
    void setPhoneNumber(const QString &phoneNumber);

    QString city() const;
    void setCity(const QString &city);

    QString internalId() const;

    QString label() const;
    void setLabel(const QString &label);

private:
    QString m_internalId;
    static int maxId;
    QString m_label;
    QString m_fullName;
    QString m_street1;
    QString m_street2;
    QString m_street3;
    QString m_city;
    QString m_state;
    QString m_postalCode;
    QString m_countryCode;
    QString m_phoneNumber;
};

#endif // ADDRESS_H
