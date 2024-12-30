#ifndef TABLEENTRYASSOCIATIONS_H
#define TABLEENTRYASSOCIATIONS_H

#include <QObject>
#include <QtCore/qhash.h>
#include <QtCore/qset.h>
#include "model/UpdateToCustomer.h"

class AccountingEntrySet;

class TableEntryAssociations : public QObject, public UpdateToCustomer
{
    Q_OBJECT
public:
    static TableEntryAssociations *instance();
    void onCustomerSelectedChanged(const QString &customerId) override;
    QString uniqueId() const override;
    /// Need to be called for each couple (one way only)
    void loadAssociations(int year);
    void removeAssociation(
            int yearFrom,
            AccountingEntrySet *entrySetFrom);
    void removeAssociation(
            int yearFrom,
            int yearTo,
            AccountingEntrySet *entrySetFrom,
            AccountingEntrySet *entrySetTo);
    /// Need to be called for each couple (one way only)
    void addAssociation(
            int yearFrom,
            int yearTo,
            AccountingEntrySet *entrySetFrom,
            AccountingEntrySet *entrySetTo,
            bool save = true);

    void addSelfAssociation(AccountingEntrySet *entrySet,
            const QString &idSelfEntry, const QString &selfEntryAccount, const QString &selfEntryTitle, bool save = true);
    void removeAllSelfAssociations(
            const QString &idSelfEntry);
    void removeSelfAssociation(AccountingEntrySet *entrySet);

    void selectAssociation(const QString &id);
    void unselectAssociation(const QString &id);
    bool isSelfAssociated(const QString &id) const;
    bool isAssociated(const QString &id) const;
    bool isAssociatedAndSelected(const QString &id) const;
    void recordEntrySet(QSharedPointer<AccountingEntrySet> entrySet);
    void removeEntrySet(QSharedPointer<AccountingEntrySet> entrySet);

signals:
    void idsSelected(const QSet<QString> &ids);
    void idsUnselected(const QSet<QString> &ids);
    void idsAssociated(const QSet<QString> &ids);
    void idsDissociated(const QSet<QString> &ids);

    void idSelfAssociated(const QString &id);
    void idSelfDissociated(const QString &id);

protected:
    ///  year      identryset  idself
    QMap<int, QHash<QString, QString>> m_selfAssociations;

    QMap<int, QMap<int, QHash<QString, QSet<QString>>>> m_associations;
    QHash<QString, AccountingEntrySet*> m_entrySets;
    QHash<QString, QSet<QString>> m_associationsNoYear;
    QSet<QString> m_selected;
    TableEntryAssociations(QObject *parent = nullptr);
    void _saveAssociations();

};

#endif // TABLEENTRYASSOCIATIONS_H
