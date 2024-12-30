#include <QtCore/qlist.h>

#include "AmazonReportPaymentModel.h"
#include "model/SettingManager.h"
#include "model/CustomerManager.h"
#include "model/orderimporters/Shipment.h"
#include "model/orderimporters/OrderManager.h"
#include "model/orderimporters/OrderImporterAmazonUE.h"

//----------------------------------------------------------
AmazonReportPaymentModel::AmazonReportPaymentModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    m_rootItem = new PaymentAmzNode("", "", 0.);
    QString selectedCustomerId
            = CustomerManager::instance()->getSelectedCustomerId();
    if (!selectedCustomerId.isEmpty()) {
        onCustomerSelectedChanged(selectedCustomerId);
    }
    connect(CustomerManager::instance(),
            &CustomerManager::selectedCustomerChanged,
            this,
            &AmazonReportPaymentModel::onCustomerSelectedChanged);
}
//----------------------------------------------------------
void AmazonReportPaymentModel::onCustomerSelectedChanged(
        const QString &)
{
    clear();
}
//----------------------------------------------------------
AmazonReportPaymentModel *AmazonReportPaymentModel::instance()
{
    static AmazonReportPaymentModel instance;
    return &instance;
}
//----------------------------------------------------------
AmazonReportPaymentModel::~AmazonReportPaymentModel()
{
}
//----------------------------------------------------------
void AmazonReportPaymentModel::clear()
{
    beginRemoveRows(QModelIndex(), 0, rowCount()-1);
    delete m_rootItem;
    endRemoveRows();
    m_rootItem = new PaymentAmzNode("", "", 0.);
}
//----------------------------------------------------------
void AmazonReportPaymentModel::compute(
        OrderManager *orderManager, const QString &paymentId)
{
    clear();
    QString channel = OrderImporterAmazonUE().name();
    auto shipmentsAndRefunds = orderManager->getShipmentsAndRefunds(
                [paymentId](const Shipment *shipment) -> bool {
        return shipment->getPaymentId() == paymentId;
    });
    double totalTaxes = 0.;
    double totalTaxed = 0.;
    double totalUntaxed = 0.;
    double totalTaxesRefund = 0.;
    double totalTaxedRefund = 0.;
    double totalUntaxedRefund = 0.;
    double totalTaxesSale = 0.;
    double totalTaxedSale = 0.;
    double totalUntaxedSale = 0.;
    QSet<int> years;
    for (auto shipmentOrRefund : shipmentsAndRefunds) {
        double taxes = shipmentOrRefund->getTotalPriceTaxesConverted();
        double taxed = shipmentOrRefund->getTotalPriceTaxedConverted();
        double untaxed = shipmentOrRefund->getTotalPriceUntaxedConverted();
        totalTaxes += taxes;
        totalTaxed += taxed;
        totalUntaxed += untaxed;
        years << shipmentOrRefund->getDateTime().date().year();
        if (taxed > 0.) {
            totalTaxesSale += taxes;
            totalTaxedSale += taxed;
            totalUntaxedSale += untaxed;
        } else {
            totalTaxesRefund += taxes;
            totalTaxedRefund += taxed;
            totalUntaxedRefund += untaxed;
        }
    }

    new PaymentAmzNodeLine(
                tr("Ventes et remboursements TTC"), "", totalTaxed, m_rootItem);
    new PaymentAmzNodeLine(
                tr("Ventes et remboursements HT"), "7xxxxx", totalUntaxed, m_rootItem);
    new PaymentAmzNodeLine(
                tr("Ventes et remboursements TVA"), "4xxxxx", totalTaxes, m_rootItem);

    new PaymentAmzNodeLine(
                tr("Ventes TTC"), "", totalTaxedSale, m_rootItem);
    new PaymentAmzNodeLine(
                tr("Ventes HT"), "7xxxxx", totalUntaxedSale, m_rootItem);
    new PaymentAmzNodeLine(
                tr("Ventes TVA"), "4xxxxx", totalTaxesSale, m_rootItem);

    new PaymentAmzNodeLine(
                tr("Remboursements TTC"), "", totalTaxedRefund, m_rootItem);
    new PaymentAmzNodeLine(
                tr("Remboursements HT"), "7xxxxx", totalUntaxedRefund, m_rootItem);
    new PaymentAmzNodeLine(
                tr("Remboursements TVA"), "4xxxxx", totalTaxesRefund, m_rootItem);
    /*
    for (auto itRegime = vat.begin();
         itRegime != vat.end();
         ++itRegime) {
        QString regime = itRegime.key();
        for (auto itCountry = itRegime.value().begin();
             itCountry != itRegime.value().end();
             ++itCountry) {
            QString countryCode = itCountry.key();
            QString countryName = SettingManager::instance()->countryName(countryCode);
            auto itemRegime = itRegime.value();
            auto itemCountryValue = itCountry.value();
            auto valuesTaxes = itemCountryValue.value(VatTableModelUE::titleTaxes);
            auto valuesTotalTaxed = itCountry.value().value(VatTableModelUE::titleTotalTaxed);
            auto valuesTotalUntaxed = itCountry.value().value(VatTableModelUE::titleTotalUntaxed);
            double countryTotalTaxes = 0.;
            double countryTotalTaxed = 0.;
            double countryTotalUntaxed = 0.;
            for (int i=0; i<12; ++i) {
                countryTotalTaxes += valuesTaxes[i];
                countryTotalTaxed += valuesTotalTaxed[i];
                countryTotalUntaxed += valuesTotalUntaxed[i];
            }
            if (countryTotalTaxed > 0.001) {
                auto detailsItemTaxed = new PaymentAmzNodeDetails(
                            regime + " - " + VatTableModelUE::titleTotalTaxed + " " + countryName,
                            "", countryTotalTaxed, itemTaxes);
                auto detailsItemUntaxed = new PaymentAmzNodeDetails(
                            regime + " - " + VatTableModelUE::titleTotalUntaxed + " " + countryName,
                            "7xxxxx", countryTotalUntaxed, itemTaxes);
                auto detailsItemTaxes = new PaymentAmzNodeDetails(
                            regime + " - " + VatTableModelUE::titleTaxes + " " + countryName,
                            "4xxxxx", countryTotalTaxes, itemTaxes);
                (void)detailsItemTaxed;
                (void)detailsItemUntaxed;
                (void)detailsItemTaxes;
            }
        }
    }
    //*/
    QMultiMap<QString, double> fees;
    for (auto year : years) {
        fees.unite(orderManager->getNonOrderFees(
                channel,
                year,
                paymentId));
    }
    QMap<QString, double> feesSummed;
    for (auto itFee = fees.begin();
         itFee != fees.end();
         ++itFee) {
        if (feesSummed.contains(itFee.key())) {
            feesSummed[itFee.key()] += itFee.value();
        } else {
            feesSummed[itFee.key()] = itFee.value();
        }
    }
    double amountCharge = 0.;
    double amountRefund = 0.;
    for (auto fee : feesSummed) {
        if (fee < 0.) {
            amountCharge += fee;
        } else {
            amountRefund += fee;
        }
    }
    auto itemCharge = new PaymentAmzNodeLine(
                tr("Charges amazon"), "6xxxxxx", amountCharge, m_rootItem);
    auto itemRefund = new PaymentAmzNodeLine(
                tr("Remboursement frais amazon"), "6xxxxxx", amountRefund, m_rootItem);
    for (auto itFee = feesSummed.begin();
         itFee != feesSummed.end();
         ++itFee) {
        if (itFee.value() < 0.) {
            auto subItemCharge = new PaymentAmzNodeDetails(
                        itFee.key(),
                        "6xxxxxx",
                        itFee.value(),
                        itemCharge);
            (void) subItemCharge;
        } else {
            auto subItemRefund = new PaymentAmzNodeDetails(
                        itFee.key(),
                        "6xxxxxx",
                        itFee.value(),
                        itemRefund);
            (void) subItemRefund;
        }
    }
    double balancePrevious = 0.;
    double balanceNow = 0.;
    double totalPayment = totalTaxed + amountCharge + amountRefund + balancePrevious + balanceNow;
    if (balancePrevious > 0.001) {
        auto itemBalancePrevious = new PaymentAmzNodeLine(
                tr("Réserve précédente", "How much money amazon kept at last payment"), "500000", balancePrevious, m_rootItem);
        (void)itemBalancePrevious;
    }
    if (balanceNow < -0.001) {
        auto itemBalanceNow = new PaymentAmzNodeLine(
                tr("Réserve actuelle", "How much money amazon will keep for a while"), "500000", balancePrevious, m_rootItem);
        (void)itemBalanceNow;
    }
    auto itemTotal = new PaymentAmzNodeLine(
                tr("Virement amazon", "bank to bank wire from amazon"), "500000", totalPayment, m_rootItem);
    (void)itemTotal;
    //*/
}
//----------------------------------------------------------
QVariant AmazonReportPaymentModel::headerData(
        int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        static QList<QVariant> values = {tr("Nom", "Accounting entry name"),
                                         tr("Compte", "Accounting entry account"),
                                         tr("Montant")};
        return values[section];
    }
    return QVariant();
}
//----------------------------------------------------------
QModelIndex AmazonReportPaymentModel::index(
        int row, int column, const QModelIndex &parent) const
{
    QModelIndex index;
    if (hasIndex(row, column, parent)) {
        PaymentAmzNode *item = nullptr;
        if (parent.isValid()) {
            PaymentAmzNode *itemParent
                    = static_cast<PaymentAmzNode *>(
                        parent.internalPointer());
            item = itemParent->child(row);;
        } else {
            item = m_rootItem->child(row);
        }
        index = createIndex(row, column, item);
    }
    return index;

}
//----------------------------------------------------------
QModelIndex AmazonReportPaymentModel::parent(const QModelIndex &index) const
{
    QModelIndex parentIndex;
    if (index.isValid()) {
        PaymentAmzNode *item
                = static_cast<PaymentAmzNode *>(
                    index.internalPointer());
        if (item->parent() != nullptr) {
            parentIndex = createIndex(item->parent()->row(), 0, item->parent());
        }
    }
    return parentIndex;
}
//----------------------------------------------------------
int AmazonReportPaymentModel::rowCount(const QModelIndex &parent) const
{
    PaymentAmzNode *itemParent = nullptr;
    if (parent.isValid()) {
        itemParent = static_cast<PaymentAmzNode *>(
                    parent.internalPointer());
    } else {
        itemParent = m_rootItem;
    }
    int count = itemParent->rowCount();
    return count;
}
//----------------------------------------------------------
int AmazonReportPaymentModel::columnCount(const QModelIndex &) const
{
    return 3;
}
//----------------------------------------------------------
QVariant AmazonReportPaymentModel::data(const QModelIndex &index, int role) const
{
    QVariant value;
    if ((role == Qt::DisplayRole || role == Qt::EditRole) && index.isValid()) {
        PaymentAmzNode *item
                = static_cast<PaymentAmzNode *>(
                    index.internalPointer());
        value = item->data(index.column(), role);
    }
    return value;
}
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
PaymentAmzNode::PaymentAmzNode(
        const QString &title,
        const QString &account,
        double value, PaymentAmzNode *parent)
{
    m_title = title;
    m_account = account;
    m_value = value;
    m_row = 0;
    m_parent = parent;
    if (parent != nullptr) {
        m_row = parent->m_children.size();
        parent->m_children << this;
    }
}
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
PaymentAmzNode::~PaymentAmzNode()
{
    qDeleteAll(m_children);
}
//----------------------------------------------------------
//----------------------------------------------------------
QVariant PaymentAmzNode::data(int column, int role) const
{
    if (role == Qt::DisplayRole) {
        if (column == 0) {
            return m_title;
        } else if (column == 1) {
            return m_account;
        } else if (column == 2) {
            return QString::number(m_value, 'f', 2);
        }
        /*
        static QList<std::function<QVariant()>> functionValues = {
                [this]() -> QVariant {
            return m_title;
        },
                [this]() -> QVariant {
            return m_account;
        },
                [this]() -> QVariant {
            return m_value;
        }
        };
        return functionValues[column]();
        //*/
    }
    return QVariant();
}
//----------------------------------------------------------
//----------------------------------------------------------
PaymentAmzNode *PaymentAmzNode::child(int row) const
{
    return m_children[row];
}
//----------------------------------------------------------
//----------------------------------------------------------
int PaymentAmzNode::rowCount() const
{
    return m_children.size();
}
//----------------------------------------------------------
//----------------------------------------------------------
void PaymentAmzNode::removeChild(int row)
{
    auto child = m_children.takeAt(row);
    for (int i=row; i<m_children.size(); ++i) {
        m_children[i]->_setRow(i);
    }
    delete child;
}
//----------------------------------------------------------
//----------------------------------------------------------
QString PaymentAmzNode::title() const
{
    return m_title;
}
//----------------------------------------------------------
//----------------------------------------------------------
QString PaymentAmzNode::account() const
{
    return m_account;
}
//----------------------------------------------------------
//----------------------------------------------------------
void PaymentAmzNode::setAccount(const QString &account)
{
    m_account = account;
}
//----------------------------------------------------------
//----------------------------------------------------------
double PaymentAmzNode::value() const
{
    return m_value;
}
//----------------------------------------------------------
//----------------------------------------------------------
void PaymentAmzNode::setValue(double value)
{
    m_value = value;
}
//----------------------------------------------------------
//----------------------------------------------------------
int PaymentAmzNode::row() const
{
    return m_row;
}
//----------------------------------------------------------
//----------------------------------------------------------
void PaymentAmzNode::_setRow(int row)
{
    m_row = row;
}
//----------------------------------------------------------
//----------------------------------------------------------
QList<PaymentAmzNode *> PaymentAmzNode::children() const
{
    return m_children;
}
//----------------------------------------------------------
//----------------------------------------------------------
PaymentAmzNode *PaymentAmzNode::parent() const
{
    return m_parent;
}
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
PaymentAmzNodeLine::PaymentAmzNodeLine(
        const QString &title,
        const QString &account,
        double value,
        PaymentAmzNode *parent)
    : PaymentAmzNode (title, account, value, parent)
{

}
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
PaymentAmzNodeDetails::PaymentAmzNodeDetails(
        const QString &title,
        const QString &account,
        double value,
        PaymentAmzNode *parent)
    : PaymentAmzNode (title, account, value, parent)
{

}
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
