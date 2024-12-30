#ifndef ENTRYPARSERAMAZONPAYMENTS_H
#define ENTRYPARSERAMAZONPAYMENTS_H

#include "EntryParserOrders.h"

class EntryParserAmazonPayments
        : public EntryParserOrders
{
public:
    static QStringList FEES;
    static QString NAME;
    EntryParserAmazonPayments();
    ~EntryParserAmazonPayments() override;
    void clearTransactions() override;
    void recordTransactions(const Shipment *shipmentOrRefund) override;
    AccountingEntries entries(int year) const override;
    Type typeOfEntries() const override;
    QString name() const override;
    QString journal() const override;
    bool isReplacedInfo(
            const QString &entrySetId,
            const QString &colName) const;
    QVariant replacedInfo(
            const QString &entrySetId,
            const QString &colName) const;
    bool isAmountPaidReplaced(const QString &entrySetId) const;
    double amountPaid(const QString &entrySetId) const;
    void replaceInfo(QSharedPointer<AccountingEntrySet> entrySet,
                     const QString &colName,
                     const QVariant &value);
    void loadReplacedInfos();

private:
    QString _findAmazon(const QString &filePath) const;
    bool _isAmazonUE(const QString &amazon, int year) const;
    bool _isSales(const QString &type, const QString &description) const;
    bool _isFees(const QString &type, const QString &description) const;
    bool _isPayableAmazon(const QString &type, const QString &description) const;
    bool _isCurrentReserve(const QString &type, const QString &description) const;
    bool _isPreviousReserve(const QString &type, const QString &description) const;
    QHash<QString, QHash<QString, QVariant>> m_replacedInfo;
    void _saveReplacedInfos();
    void _updateEntrySetFromReplacedInfo(
            QSharedPointer<AccountingEntrySet> entrySet) const;
    QString _settingKey();
};

#endif // ENTRYPARSERAMAZONPAYMENTS_H
