#ifndef AMAZONREPORTPAYMENTMODEL_H
#define AMAZONREPORTPAYMENTMODEL_H

#include <QtCore/qabstractitemmodel.h>
#include "model/orderimporters/VatOrdersModel.h"

class PaymentAmzNode;
class AmazonReportPaymentModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    static AmazonReportPaymentModel *instance();
    ~AmazonReportPaymentModel() override;
    void clear();

    void compute(OrderManager *orderManager, const QString &paymentId);
    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

public slots:
    void onCustomerSelectedChanged(const QString &customerId);

private:
    explicit AmazonReportPaymentModel(QObject *parent = nullptr);
    PaymentAmzNode *m_rootItem;
};


class PaymentAmzNode {
public:
    PaymentAmzNode(const QString &title,
                   const QString &account,
                   double value,
                   PaymentAmzNode *parent = nullptr);
    virtual ~PaymentAmzNode();
    virtual QVariant data(int column, int role) const;
    PaymentAmzNode *child(int row) const;
    int rowCount() const;
    void removeChild(int row);
    void setTitle(const QString &title);
    QString title() const;
    QString account() const;
    void setAccount(const QString &account);
    double value() const;
    void setValue(double value);
    int row() const;
    PaymentAmzNode *parent() const;
    QList<PaymentAmzNode *> children() const;

private:
    QString m_title;
    QString m_account;
    double m_value;
    void _setRow(int row);
    int m_row;
    PaymentAmzNode *m_parent;
    QList<PaymentAmzNode *> m_children;
};


class PaymentAmzNodeLine : public PaymentAmzNode{
public:
    PaymentAmzNodeLine(const QString &title,
                   const QString &account,
                   double value,
                   PaymentAmzNode *parent = nullptr);
};

class PaymentAmzNodeDetails : public PaymentAmzNode{
public:
    PaymentAmzNodeDetails(const QString &title,
                   const QString &account,
                   double value,
                   PaymentAmzNode *parent = nullptr);
};
#endif // AMAZONREPORTPAYMENTMODEL_H
