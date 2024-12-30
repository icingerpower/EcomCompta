#include <QtGui/qcolor.h>
#include <QtGui/qbrush.h>
#include <QtCore/qsettings.h>

#include "RefundManager.h"
#include "model/CustomerManager.h"
#include "OrderManager.h"

//----------------------------------------------------------
QString RefundManager::COL_VAT_REFUND = QObject::tr("Remboursement TVA");
QString RefundManager::COL_VAT_REFUND_CONV = QObject::tr("Remboursement TVA converti");
//----------------------------------------------------------
RefundManager::RefundManager(OrderManager *orderManager, QObject *parent)
    : QAbstractItemModel(parent)
{
    m_rootItem = new RefundManagerNode("");
    m_orderManager = orderManager;
    setOrderManager(orderManager);
    *allRefundManagers() << this;
    QString selectedCustomerId = CustomerManager::instance()->getSelectedCustomerId();
    if (!selectedCustomerId.isEmpty()) {
        onCustomerSelectedChanged(selectedCustomerId);
    }
    connect(CustomerManager::instance(),
            &CustomerManager::selectedCustomerChanged,
            this,
            &RefundManager::onCustomerSelectedChanged);
}
//----------------------------------------------------------
QList<RefundManager *> *RefundManager::allRefundManagers()
{
    static QList<RefundManager *> managers;
    return &managers;
}
//----------------------------------------------------------
const QMap<QString, OrdersMapping>
*RefundManager::getRefundsManualByChannel() const
{
    return &m_refundsManualByChannel;
}
//----------------------------------------------------------
const OrderManager *RefundManager::getOrderManager() const
{
    return m_orderManager;
}
//----------------------------------------------------------
void RefundManager::setOrderManager(
        OrderManager *orderManager)
{
    if (m_orderManager != nullptr) {
        disconnect(m_orderManager);
    }
    m_orderManager = orderManager;
    connect(m_orderManager,
            &OrderManager::ordersRecorded,
            this,
            &RefundManager::retriveRefunds);
    retriveRefunds();
}
//----------------------------------------------------------
const QList<RefundManager::ColInfo> *RefundManager::colInfos() const
{
    static QList<RefundManager::ColInfo> colInfos
            = {
        {tr("Identifiant"), [](const Refund *refund, const RefundManager *) ->QVariant{
             return refund->getId();
         }},
        {tr("Commande"), [](const Refund *refund, const RefundManager *) ->QVariant{
             return refund->orderId();
         }},
        {tr("Date remboursement"), [](const Refund *refund, const RefundManager *) ->QVariant{
             return refund->getDateTime().toString("yyyy-MM-dd hh:mm:ss");
         }},
        {OrderManager::COL_VAT_REGIME, [](const Refund *refund, const RefundManager *) ->QVariant{
             return refund->getRegimeVat();
         }},
        {OrderManager::COL_COUNTRY_VAT, [](const Refund *refund, const RefundManager *) ->QVariant{
             return refund->getCountryCodeVat();
         }},
        {OrderManager::COL_COUNTRY_VAT_DECL, [](const Refund *refund, const RefundManager *) ->QVariant{
             return refund->getCountrySaleDeclaration();
         }},
        {tr("Remboursement TTC converti"), [](const Refund *refund, const RefundManager *) ->QVariant{
             return QString::number(refund->getTotalPriceTaxedConverted(), 'f', 2);
         }},
        {tr("Remboursement TTC"), [](const Refund *refund, const RefundManager *) ->QVariant{
             return QString::number(refund->getTotalPriceTaxed(), 'f', 2);
         }},
        {COL_VAT_REFUND_CONV, [](const Refund *refund, const RefundManager *) ->QVariant{
             return QString::number(refund->getTotalPriceTaxes(), 'f', 2);
         }},
        {COL_VAT_REFUND, [](const Refund *refund, const RefundManager *) ->QVariant{
             return QString::number(refund->getTotalPriceTaxesConverted(), 'f', 2);
         }},
        {tr("Monnaie"), [](const Refund *refund, const RefundManager *) ->QVariant{
             return refund->getCurrency();
         }},
        {tr("Pays Expédition"), [](const Refund *refund, const RefundManager *) ->QVariant{
             if (refund->getShipments().size() > 0) {
                 return refund->getShipments()[0]->countryCodeFrom();
             }
             return "";
         }},
        {tr("Pays Arrivé"), [](const Refund *refund, const RefundManager *) ->QVariant{
             if (refund->getShipments().size() > 0) {
                 return refund->getShipments()[0]->countryCodeTo();
             }
             return "";
         }},
        {tr("Total commande TTC"), [](const Refund *refund, const RefundManager *) ->QVariant{
             if (refund->getOrder() == nullptr) {
                 return "";
             }
             return QString::number(refund->getOrder()->getTotalPriceTaxed(), 'f', 2);
         }},
        {tr("Commande TVA"), [](const Refund *refund, const RefundManager *) ->QVariant{
             if (refund->getOrder() == nullptr) {
                 return "";
             }
             return QString::number(refund->getOrder()->getTotalPriceTaxes(), 'f', 2);
         }},
        {tr("Date commande"), [](const Refund *refund, const RefundManager *) ->QVariant{
             if (refund->getOrder() == nullptr) {
                 return "";
             }
             return refund->getOrder()->getDateTime().toString("yyyy-MM-dd hh:mm:ss");
         }}
    };
    return &colInfos;
}
//----------------------------------------------------------
void RefundManager::_clear()
{
    if (rowCount() > 0) {
        beginRemoveRows(QModelIndex(), 0, rowCount() - 1);
        m_allRefundsUnitedByChannel.clear();
        endRemoveRows();
    }
}
//----------------------------------------------------------
void RefundManager::onCustomerSelectedChanged(const QString &customerId)
{
    if (customerId.isEmpty()) {
        m_settingKey = "";
        _clear();
    } else {
        m_settingKey = "RefundManager__onCustomerSelectedChanged-" + customerId;
        // TODO instead of create refund from here, I should only load refund of given year when asked
        loadFromSettings();
    }
}
//----------------------------------------------------------
RefundManager::~RefundManager()
{
    disconnect(m_orderManager);
    disconnect(CustomerManager::instance());
    disconnect(); //TODO check it is disconnect  all
    allRefundManagers()->removeOne(this);
    delete m_rootItem;
}
//----------------------------------------------------------
/*
RefundManager *RefundManager::instance()
{
    static RefundManager instance;
    return &instance;
}
//*/
//----------------------------------------------------------
RefundState RefundManager::isOrderRefunded(
        const QString &channel, const QString &orderId) const
{
    auto order = m_orderManager->getOrder(channel, orderId);
    return isOrderRefunded(order);
}
//----------------------------------------------------------
RefundState RefundManager::isOrderRefunded(const Order *order) const
{
    RefundState refundState = RefundState::NotRefunded;
    QString channel = order->getChannel();
    QString orderId = order->getId();
    double refundTotal = getTotalRefunds(order);
    if (qAbs(order->getTotalPriceTaxed() - refundTotal) < 0.01) {
        refundState = RefundState::Refunded;
    } else if (refundTotal >= 0.01) {
        refundState = RefundState::PartiallyRefunded;
    }
    return refundState;
}
//----------------------------------------------------------
double RefundManager::getTotalRefunds(
        const QString &channel, const QString &orderId) const
{
    auto order = m_orderManager->getOrder(channel, orderId);
    return getTotalRefunds(order);
}
//----------------------------------------------------------
double RefundManager::getTotalRefunds(const Order *order) const
{
    double refundTotal = 0.;
    QString channel = order->getChannel();
    QString orderId = order->getId();
    if (m_allRefundsUnitedByChannel.contains(channel)
            && m_allRefundsUnitedByChannel[channel].refundByOrderId.contains(orderId)) {
        for (auto refund : m_allRefundsUnitedByChannel[channel].refundByOrderId.values(orderId)) {
            refundTotal += refund->getTotalPriceTaxed();
        }
        Q_ASSERT(order->getTotalPriceTaxed() >= refundTotal - 0.0001);
    }
    return refundTotal;
}
//----------------------------------------------------------
bool RefundManager::canRefund(const Order *order, double amount) const
{
    double diff = order->getTotalPriceTaxed() - getTotalRefunds(order) - amount;
    bool can = diff > -0.001;
    return can;
}
//----------------------------------------------------------
void RefundManager::saveInSettings() const
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (!m_refundsManualByChannel.isEmpty()) {
        QStringList lines;
        for (auto itChannel = m_refundsManualByChannel.begin();
             itChannel != m_refundsManualByChannel.end();
             ++itChannel) {
            for (auto refund : qAsConst(itChannel.value().refundById)) {
                lines << itChannel.key() + SettingManager::SEP_COL + refund->toString();
            }
        }
        settings.setValue(m_settingKey, lines.join(SettingManager::SEP_LINES));
    } else {
        settings.remove(m_settingKey);
    }
}
//----------------------------------------------------------
void RefundManager::loadFromSettings()
{
    _clear();
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (settings.contains(m_settingKey)) {
        QStringList lines = settings.value(m_settingKey).toString().split(SettingManager::SEP_LINES);
        for (auto line : lines) {
            QStringList elements = line.split(SettingManager::SEP_COL);
            QString channel = elements[0];
            QSharedPointer<Refund> refund(Refund::fromString(elements[1]));
            auto order = m_orderManager->getOrderNotConst(refund->channel(), refund->orderId());
            if (order == nullptr) {
                m_refundsWithoutOrderToRecordLater << refund;
            } else {
                refund->init(order);
                _addRefund(channel, refund);
            }
        }
        /*
        retreiveRefunds();
        if (m_refundsManualByChannel.size() > 0) {
            beginInsertRows(QModelIndex(), 0, m_refundsManualByChannel.size()-1);
         _generateTree();
         endInsertRows();
     }
     //*/
    }
    _unitRefunds();
}
//----------------------------------------------------------
bool RefundManager::contains(
          const QString &channel, const QString &id) const
{
     bool contains = (m_orderManager->m_ordersByChannel.contains(channel)
                && m_orderManager->m_ordersByChannel[channel].refundById.contains(id))
               || (m_refundsManualByChannel.contains(channel)
                && m_refundsManualByChannel[channel].refundById.contains(id));
     return contains;
}
//----------------------------------------------------------
QList<int> RefundManager::vatColIndexes()
{
    static QList<int> indexes = [this]() -> QList<int> {
        QList<int> indexes;
        auto _colInfos = *colInfos();
        QStringList colNames = {
            OrderManager::COL_VAT
            , OrderManager::COL_VAT_REGIME
            , OrderManager::COL_COUNTRY_VAT
            , OrderManager::COL_COUNTRY_VAT_DECL
            , COL_VAT_REFUND
            , COL_VAT_REFUND_CONV
            };
        int i=0;
        for (auto it = _colInfos.begin();
        it != _colInfos.end();++it) {
            if (colNames.contains(it->name)) {
                indexes << i;
            }
            ++i;
        }
        return indexes;
    }();
    return indexes;

}
//----------------------------------------------------------
void RefundManager::removeRefund(const QModelIndex &index)
{
    if (index.isValid()) {
        RefundManagerNode *item
                = static_cast<RefundManagerNode *>(index.internalPointer());
        RefundManagerNodeRefund *itemRefund
                = dynamic_cast<RefundManagerNodeRefund *>(
                    item);
        if (itemRefund != nullptr) {
            QString channel = itemRefund->channel();
            if (m_refundsManualByChannel.contains(channel)) {
                QString refundId = itemRefund->value();
                if (m_refundsManualByChannel[channel].refundById.contains(refundId)){
                    beginRemoveRows(index.parent(), itemRefund->row(), itemRefund->row());
                    auto parentItemRefund = itemRefund->parent();
                    parentItemRefund->removeChild(itemRefund->row());
                    //auto refund = m_refundsManualByChannel[channel].refundById[refundId];
                    //m_refundsManualByChannel[channel].removeRefund(refund);
                    //m_refundsManualByChannel.remove(channel);

                    //old m_allRefundsUnitedByChannel[channel].removeRefund(refund);
                    for (auto instance : *allRefundManagers()) {
                        if (instance->m_refundsManualByChannel.contains(channel)
                                && instance->m_refundsManualByChannel[channel].refundById.contains(refundId)) {
                            auto refundInInstance = instance->m_refundsManualByChannel[channel].refundById[refundId];
                            instance->m_refundsManualByChannel[channel].removeRefund(refundInInstance);
                            if (instance->m_refundsManualByChannel[channel].refundById.isEmpty()) {
                                instance->m_refundsManualByChannel.remove(channel);
                                if (instance == this) {
                                    parentItemRefund->parent()->removeChild(parentItemRefund->row());
                                    if (parentItemRefund->parent()->rowCount() == 0) {
                                        parentItemRefund->parent()->parent()->removeChild(0);
                                    }
                                }
                            }
                            if (instance != this) {
                                instance->_generateTree();
                            }
                        }
                    }
                    endRemoveRows();
                    saveInSettings();
                } else {
                    RefundIdException exeption;
                    exeption.setError(tr("Le remboursement n'est pas un remboursement ajouté manuellement."));
                    exeption.raise();
                }
            } else {
                RefundIdException exeption;
                exeption.setError(tr("Le remboursement n'est pas un remboursement ajouté manuellement."));
                exeption.raise();
            }
        }
    }
}
//----------------------------------------------------------
void RefundManager::_addRefund(
          const QString &channel, QSharedPointer<Refund> refund)
{
     QString orderId = refund->orderId();
     if (!m_refundsManualByChannel.contains(channel)) {
          m_refundsManualByChannel[channel] = OrdersMapping();
     }
     /*
    if (!m_allRefundsUnitedByChannel.contains(channel)) {
     m_allRefundsUnitedByChannel[channel] = OrdersMapping();
    }
    //*/
    QString refundId = refund->getId();
 //if (!m_refundsManualByChannel[channel].refundById.contains(refund->getId())) {
    m_refundsManualByChannel[channel].refundById[refundId] = refund;
    int year = refund->getDateTime().date().year();
    if (!m_refundsManualByChannel[channel].refundByDate.contains(year)) {
        m_refundsManualByChannel[channel].refundByDate[year]
                = QMultiMap<QDateTime, QSharedPointer<Refund>>();
    }
    m_refundsManualByChannel[channel].refundByDate[year].insert(
                         refund->getDateTime(), refund);
     //}
     //m_allRefundsUnitedByChannel[channel].orderById
     //[refund->orderId()] = refund->getOrder();
}
//----------------------------------------------------------
void RefundManager::addRefundAndUniteAll(
          const QString &channel, QSharedPointer<Refund> refund)
{
    _addRefund(channel, refund); //TODO beginsertrow
    QString orderId = refund->orderId();
    for (auto instance : *allRefundManagers()) {
        if (instance != this
                && instance->m_orderManager->m_ordersByChannel[channel].orderById
                .contains(orderId)) {
            QSharedPointer<Refund> refundCopy
                    (Refund::fromString(refund->toString()));
            refundCopy->init(instance->m_orderManager->getOrderNotConst(channel, orderId));
            _addRefund(channel, refund);
        }
    }
    for (auto instance : *allRefundManagers()) {
         if (instance == this) {
              int TEMP=10;++TEMP;
         }
         if (instance->m_orderManager->m_ordersByChannel.contains(channel)
                   && instance->m_orderManager->m_ordersByChannel[channel].orderById.contains(
                        refund->orderId())) { ///If orders exists in the current order manager, we ask tree to be updated
              instance->_unitRefunds(channel);
         }
    }
    saveInSettings();
}
//----------------------------------------------------------
void RefundManager::_unitRefunds()
{
     //_clear(); //TODO no right because it delete manual refund
     // TODO manual refund not added
     for (auto itChannel = m_orderManager->m_ordersByChannel.begin();
          itChannel != m_orderManager->m_ordersByChannel.end();
          ++itChannel) {
          m_allRefundsUnitedByChannel[itChannel.key()] = OrdersMapping();
          m_allRefundsUnitedByChannel[itChannel.key()].uniteRefunds(itChannel.value());
     }
     _generateTree();
}
//----------------------------------------------------------
void RefundManager::_unitRefunds(const QString &channel)
{
     bool containsChannel = m_allRefundsUnitedByChannel.contains(channel);
     /*
    QModelIndex indexModelChannel;
    if (containsChannel) {
     int indexChannel = m_allRefundsUnitedByChannel.keys().indexOf(channel);
     indexModelChannel = index(indexChannel, 0, QModelIndex());
     beginRemoveRows(indexModelChannel, 0, rowCount()-1);
     //*/
     if (containsChannel) {
          m_allRefundsUnitedByChannel.remove(channel);
          m_allRefundsUnitedByChannel[channel] = OrdersMapping();
          //endRemoveRows();
     }
     m_allRefundsUnitedByChannel[channel].uniteRefunds(
                    m_orderManager->m_ordersByChannel[channel]);
     /*
    if (containsChannel) {
     beginInsertRows(indexModelChannel, 0, m_allRefundsUnitedByChannel[channel].size()-1);
    } else {
     beginInsertRows(QModelIndex(), 0, rowCount()-1);
    }
    //*/
     _generateTree();
}
//----------------------------------------------------------
void RefundManager::_generateTree()
{
     // TODO only in order ids contain refund->orderId()
     // TODO here I should add the manual refunds
     beginRemoveRows(QModelIndex(), 0, rowCount()-1);
     if (m_rootItem != nullptr) {
          delete m_rootItem;
     }
     m_rootItem = new RefundManagerNode("");
     endRemoveRows();
     _addInTree(m_refundsManualByChannel, tr("Ajouté", "refund added manually") + " ");
     _addInTree(m_allRefundsUnitedByChannel);
    beginInsertRows(QModelIndex(), 0, rowCount()-1);
    endInsertRows();
}
//----------------------------------------------------------
void RefundManager::_addInTree(
        const QMap<QString, OrdersMapping> &ordersMappingByChannel,
        const QString &prefixYear)
{
    QList<int> years;
    for (auto channel : ordersMappingByChannel) {
        for (auto itDate = channel.refundByDate.begin();
             itDate != channel.refundByDate.end();
             ++itDate) {
            int year = itDate.key();
            if (!years.contains(year)) {
                years << itDate.key();
            }
        }
    }
    std::sort(years.begin(), years.end(), [](int left, int right) -> bool {
        return left > right;
    });
    for (auto year : years) {
        auto itemYear = new RefundManagerNodeYear(
                    prefixYear + QString::number(year), m_rootItem);
        for (auto itChannel = ordersMappingByChannel.begin();
             itChannel != ordersMappingByChannel.end();
             ++itChannel) {
            auto itemChannel = new RefundManagerNodeChannel(
                        itChannel.key(), itemYear);
            for (auto itRefund = itChannel.value().refundByDate[year].begin();
                 itRefund != itChannel.value().refundByDate[year].end();
                 ++itRefund) {
                auto itemRefund = new RefundManagerNodeRefund(
                            itChannel.key(), itRefund.value()->getId(), this, itemChannel);
                (void)itemRefund;
            }
        }
    }

}
//----------------------------------------------------------
QVariant RefundManager::headerData(int section, Qt::Orientation orientation, int role) const
{
    QVariant value;
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        value = (*colInfos())[section].name;
    }
    return value;
}
//----------------------------------------------------------
QModelIndex RefundManager::index(int row, int column, const QModelIndex &parent) const
{
    QModelIndex index;
    if (hasIndex(row, column, parent)) {
        RefundManagerNode *item = nullptr;
        if (parent.isValid()) {
            RefundManagerNode *itemParent
                    = static_cast<RefundManagerNode *>(
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
QModelIndex RefundManager::parent(const QModelIndex &index) const
{
    QModelIndex parentIndex;
    if (index.isValid()) {
        RefundManagerNode *item
                = static_cast<RefundManagerNode *>(
                    index.internalPointer());
        if (item->parent() != nullptr) {
            parentIndex = createIndex(item->parent()->row(), 0, item->parent());
        }
    }
    return parentIndex;
}
//----------------------------------------------------------
int RefundManager::rowCount(const QModelIndex &parent) const
{
    RefundManagerNode *itemParent = nullptr;
    if (parent.isValid()) {
        itemParent = static_cast<RefundManagerNode *>(
                    parent.internalPointer());
    } else {
        itemParent = m_rootItem;
    }
    int count = 0;
    if (itemParent != nullptr) {
        count = itemParent->rowCount();
    }
    return count;
}
//----------------------------------------------------------
int RefundManager::columnCount(const QModelIndex &) const
{
    return (*colInfos()).size();
}
//----------------------------------------------------------
QVariant RefundManager::data(const QModelIndex &index, int role) const
{
    QVariant value;
    if ((role == Qt::DisplayRole || role == Qt::EditRole) && index.isValid()) {
        RefundManagerNode *item
                = static_cast<RefundManagerNode *>(
                    index.internalPointer());
        value = item->data(index.column(), role);
    } else if (role == Qt::BackgroundRole) {
        RefundManagerNode *item
                = static_cast<RefundManagerNode *>(
                    index.internalPointer());
        if (item->row() % 2 == 0) {
            value = QBrush(QColor("#eaf2ff"));
        }
    }
    return value;
}
//----------------------------------------------------------
void RefundManager::retriveRefunds()
{
    for (int i=m_refundsWithoutOrderToRecordLater.size()-1; i>-1; i--) {
        auto refund = m_refundsWithoutOrderToRecordLater[i];
        QString channel = refund->channel();
        QString orderId = refund->orderId();
        auto order = m_orderManager->getOrderNotConst(channel, orderId);
        if (order != nullptr) {
            m_refundsWithoutOrderToRecordLater.removeAt(i);
            refund->init(order);
            _addRefund(channel, refund);
        }
    }
    _unitRefunds();
}
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
RefundManagerNode::RefundManagerNode(
        const QString &value, RefundManagerNode *parent)
{
    m_value = value;
    m_parent = parent;
    m_row = 0;
    if (parent != nullptr) {
        m_row = parent->m_children.size();
        parent->m_children << this;
    }
}
//----------------------------------------------------------
//----------------------------------------------------------
RefundManagerNode::~RefundManagerNode()
{
    qDeleteAll(m_children);
}
//----------------------------------------------------------
//----------------------------------------------------------
QVariant RefundManagerNode::data(int column, int role) const
{
    QVariant value;
    if (role == Qt::DisplayRole && column == 0) {
        value = m_value;
    }
    return value;
}
//----------------------------------------------------------
//----------------------------------------------------------
QString RefundManagerNode::value() const
{
    return m_value;
}
//----------------------------------------------------------
//----------------------------------------------------------
void RefundManagerNode::setValue(const QString &value)
{
    m_value = value;
}
//----------------------------------------------------------
//----------------------------------------------------------
RefundManagerNode *RefundManagerNode::parent() const
{
    return m_parent;
}
//----------------------------------------------------------
//----------------------------------------------------------
RefundManagerNode *RefundManagerNode::child(int row) const
{
    return m_children[row];
}
//----------------------------------------------------------
//----------------------------------------------------------
int RefundManagerNode::rowCount() const
{
    return m_children.size();
}
//----------------------------------------------------------
//----------------------------------------------------------
int RefundManagerNode::row() const
{
    return m_row;
}
//----------------------------------------------------------
//----------------------------------------------------------
void RefundManagerNode::removeChild(int row)
{
    auto child = m_children.takeAt(row);
    if (m_children.size() == 0) {
        //removeRecursively();
    } else {
        for (int i=0; i<m_children.size(); ++i) {
            m_children[i]->m_row = i;
        }
    }
    delete child;
}
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
RefundManagerNodeYear::RefundManagerNodeYear(
        const QString &value, RefundManagerNode *parent)
    : RefundManagerNode (value, parent)
{
}
//----------------------------------------------------------
//----------------------------------------------------------
RefundManagerNodeChannel::RefundManagerNodeChannel(
        const QString &value, RefundManagerNode *parent)
    : RefundManagerNode (value, parent)
{
}
//----------------------------------------------------------
//----------------------------------------------------------
RefundManagerNodeSubchannl::RefundManagerNodeSubchannl(
        const QString &value, RefundManagerNode *parent)
    : RefundManagerNode (value, parent)
{
}
//----------------------------------------------------------
//----------------------------------------------------------
RefundManagerNodeRefund::RefundManagerNodeRefund(
        const QString &channel,
        const QString &value,
        const RefundManager *manager,
        RefundManagerNode *parent)
    : RefundManagerNode (value, parent)
{
    m_channel = channel;
    m_refundManager = manager;
}
//----------------------------------------------------------
//----------------------------------------------------------
QVariant RefundManagerNodeRefund::data(int column, int role) const
{
    QVariant data;
    if (role == Qt::DisplayRole) {
        //if (m_refundManager->m_allRefundsUnitedByChannel.contains(channel())
                //&& m_refundManager->m_allRefundsUnitedByChannel[channel()]
                //.refundById.contains(value())) {
            QSharedPointer<Refund> refund(nullptr);
            QString refundId = value();

            if (m_refundManager->m_allRefundsUnitedByChannel.contains(channel())) {
                refund = m_refundManager->m_allRefundsUnitedByChannel[channel()].refundById.value(
                            refundId, QSharedPointer<Refund>(nullptr));
            }
            if (refund.isNull()) {
                refund = m_refundManager->m_refundsManualByChannel[channel()].refundById[refundId];
            }
            data = (*m_refundManager->colInfos())[column].getValue(refund.data(), m_refundManager);
        //}
    }
    return data;
}
//----------------------------------------------------------
QString RefundManagerNodeRefund::channel() const
{
    return m_channel;
}
//----------------------------------------------------------
//----------------------------------------------------------
void RefundIdException::raise() const
{
    throw *this;
}
//----------------------------------------------------------
//----------------------------------------------------------
RefundIdException *RefundIdException::clone() const
{
    return new RefundIdException(*this);
}
//----------------------------------------------------------
//----------------------------------------------------------
QString RefundIdException::error() const
{
    return m_error;
}
//----------------------------------------------------------
//----------------------------------------------------------
void RefundIdException::setError(const QString &error)
{
    m_error = error;
}
//----------------------------------------------------------
//----------------------------------------------------------
