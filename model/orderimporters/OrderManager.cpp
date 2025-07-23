#include <QtCore/qsettings.h>
#include <QtCore/qfile.h>
#include <QtCore/qtextstream.h>
#include <QtGui/qcolor.h>
#include <QtGui/qbrush.h>

#include "../common/countries/CountryManager.h"

#include "OrderManager.h"
#include "Shipment.h"
#include "Refund.h"
#include "AbstractOrderImporter.h"
#include "OrderManagerNode.h"
#include "model/CustomerManager.h"
#include "model/SettingManager.h"
#include "model/orderimporters/ShippingAddressesManager.h"

//----------------------------------------------------------
QString OrderManager::COL_VAT = tr("TVA");
QString OrderManager::COL_VAT_REGIME = tr("Régime TVA");
QString OrderManager::COL_COUNTRY_VAT = tr("Pays TVA");
QString OrderManager::COL_COUNTRY_VAT_DECL = tr("Pays Déclaration");
//----------------------------------------------------------
OrderManager *OrderManager::instance()
{
    static OrderManager instance;
    return &instance;
}
//----------------------------------------------------------
OrderManager *OrderManager::copyFilter(
        std::function<bool (const Order *)> filterIsOrderOk) const
{
    OrderManager *copy = copyEmpty();
    for (auto itChannel = m_ordersByChannel.begin();
         itChannel != m_ordersByChannel.end();
         ++itChannel) {
        QString channel = itChannel.key();
        copy->m_ordersByChannel[channel].refundById
                = m_ordersByChannel[channel].refundById;
        copy->m_ordersByChannel[channel].nonOrderFees
                = m_ordersByChannel[channel].nonOrderFees;
        copy->m_ordersByChannel[channel].minDate
                = m_ordersByChannel[channel].minDate;
        copy->m_ordersByChannel[channel].maxDate
                = m_ordersByChannel[channel].maxDate;
        copy->m_ordersByChannel[channel]
                = OrdersMapping();
        for (auto itOrder = itChannel.value().orderById.begin();
             itOrder != itChannel.value().orderById.end();
             ++itOrder) {
            if (filterIsOrderOk(itOrder.value().data())) {
                copy->m_ordersByChannel[channel]
                        .orderById[itOrder.value()->getId()] = itOrder.value();
                auto dateOrder = itOrder.value()->getDateTime();
                int year = dateOrder.date().year();
                if (!copy->m_ordersByChannel[channel].orderByDate.contains(
                            year)) {
                    copy->m_ordersByChannel[channel].orderByDate[year]
                            = QMultiMap<QDateTime, QSharedPointer<Order>>();
                    copy->m_ordersByChannel[channel].ordersQuantityByDate[year]
                            = QHash<QString, QMap<QDate, int>>();
                }
                copy->m_ordersByChannel[channel].orderByDate[year]
                        .insert(dateOrder, itOrder.value());
                QString subChannel = itOrder.value()->getSubchannel();
                if (!copy->m_ordersByChannel[channel].ordersQuantityByDate[year]
                        .contains(subChannel)) {
                    copy->m_ordersByChannel[channel].ordersQuantityByDate[year][subChannel]
                            = QMap<QDate, int>();
                }
                if (!copy->m_ordersByChannel[channel].ordersQuantityByDate[year][subChannel]
                        .contains(dateOrder.date())) {
                    copy->m_ordersByChannel[channel].ordersQuantityByDate[year][subChannel][dateOrder.date()] = 0;
                } else {
                    ++copy->m_ordersByChannel[channel].ordersQuantityByDate[year][subChannel][dateOrder.date()];
                }
            }
        }
        for (auto itShipment = itChannel.value().shipmentById.begin();
             itShipment != itChannel.value().shipmentById.end();
             ++itShipment) {
            auto shipment = itShipment.value();
            if (copy->m_ordersByChannel[channel].orderById.contains(
                        shipment->getOrder()->getId())) {
                if (!copy->m_ordersByChannel[channel].shipmentById.contains(itShipment.key())) {
                    copy->m_ordersByChannel[channel].shipmentById[itShipment.key()]
                            = itShipment.value();
                    int year = itShipment.value()->getDateTime().date().year();
                    if (!copy->m_ordersByChannel[channel].shipmentByDate.contains(year)) {
                        copy->m_ordersByChannel[channel].shipmentByDate[year]
                                = QMultiMap<QDateTime, QSharedPointer<Shipment>>();
                    }
                    copy->m_ordersByChannel[channel].shipmentByDate[year]
                            .insert(itShipment.value()->getDateTime(), itShipment.value());
                }
            }
        }
    }
    QStringList keysToRemove;
    for (auto itChannel = copy->m_ordersByChannel.begin();
         itChannel != copy->m_ordersByChannel.end();
         ++itChannel) {
        if (itChannel.value().nOrders() == 0) {
            keysToRemove << itChannel.key();
        }
    }
    for (auto key : keysToRemove) {
        copy->m_ordersByChannel.remove(key);
    }
    copy->_generateTree();
    return copy;
}
//----------------------------------------------------------
OrderManager *OrderManager::copyFilter(
        std::function<bool (const Order *)> isOrderOk,
        std::function<bool (const Refund *)> isRefundOk) const
{
    OrderManager *copy = copyEmpty();
    for (auto itChannel = m_ordersByChannel.begin();
         itChannel != m_ordersByChannel.end();
         ++itChannel) {
        QString channel = itChannel.key();
        //copy->m_ordersByChannel[channel].refundById
                //= m_ordersByChannel[channel].refundById;
        copy->m_ordersByChannel[channel].nonOrderFees
                = m_ordersByChannel[channel].nonOrderFees;
        copy->m_ordersByChannel[channel].minDate
                = m_ordersByChannel[channel].minDate;
        copy->m_ordersByChannel[channel].maxDate
                = m_ordersByChannel[channel].maxDate;
        copy->m_ordersByChannel[channel]
                = OrdersMapping();
        for (auto itOrder = itChannel.value().orderById.begin();
             itOrder != itChannel.value().orderById.end();
             ++itOrder) {
            Q_ASSERT(itOrder.value().data() != nullptr);
            if (isOrderOk(itOrder.value().data())) {
                copy->m_ordersByChannel[channel]
                        .orderById[itOrder.value()->getId()] = itOrder.value();
                auto dateOrder = itOrder.value()->getDateTime();
                int year = dateOrder.date().year();
                if (!copy->m_ordersByChannel[channel].orderByDate.contains(
                            year)) {
                    copy->m_ordersByChannel[channel].orderByDate[year]
                            = QMultiMap<QDateTime, QSharedPointer<Order>>();
                    copy->m_ordersByChannel[channel].ordersQuantityByDate[year]
                            = QHash<QString, QMap<QDate, int>>();
                }
                copy->m_ordersByChannel[channel].orderByDate[year]
                        .insert(dateOrder, itOrder.value());
                QString subChannel = itOrder.value()->getSubchannel();
                if (!copy->m_ordersByChannel[channel].ordersQuantityByDate[year]
                        .contains(subChannel)) {
                    copy->m_ordersByChannel[channel].ordersQuantityByDate[year][subChannel]
                            = QMap<QDate, int>();
                }
                if (!copy->m_ordersByChannel[channel].ordersQuantityByDate[year][subChannel]
                        .contains(dateOrder.date())) {
                    copy->m_ordersByChannel[channel].ordersQuantityByDate[year][subChannel][dateOrder.date()] = 0;
                } else {
                    ++copy->m_ordersByChannel[channel].ordersQuantityByDate[year][subChannel][dateOrder.date()];
                }
            }
        }
        for (auto itShipment = itChannel.value().shipmentById.begin();
             itShipment != itChannel.value().shipmentById.end();
             ++itShipment) {
            auto shipment = itShipment.value();
            if (shipment->getOrder() != nullptr && copy->m_ordersByChannel[channel].orderById.contains(
                        shipment->getOrder()->getId())) {
                if (!copy->m_ordersByChannel[channel].shipmentById.contains(itShipment.key())) {
                    copy->m_ordersByChannel[channel].shipmentById[itShipment.key()]
                            = itShipment.value();
                    int year = itShipment.value()->getDateTime().date().year();
                    if (!copy->m_ordersByChannel[channel].shipmentByDate.contains(year)) {
                        copy->m_ordersByChannel[channel].shipmentByDate[year]
                                = QMultiMap<QDateTime, QSharedPointer<Shipment>>();
                    }
                    copy->m_ordersByChannel[channel].shipmentByDate[year]
                            .insert(itShipment.value()->getDateTime(), itShipment.value());
                }
            }
        }
        for (auto itRefund = itChannel.value().refundById.begin();
             itRefund != itChannel.value().refundById.end();
             ++itRefund) {
            auto refund = itRefund.value();
            if(isRefundOk(refund.data())) {
                copy->m_ordersByChannel[channel]
                        .refundById[refund->getId()] = refund;
                auto dateTime = refund->getDateTime();
                int year = dateTime.date().year();
                if (!copy->m_ordersByChannel[channel].refundByDate.contains(
                            year)) {
                    copy->m_ordersByChannel[channel].refundByDate[year]
                            = QMultiMap<QDateTime, QSharedPointer<Refund>>();
                }
                copy->m_ordersByChannel[channel].refundByDate[year]
                        .insert(dateTime, refund);
                copy->m_ordersByChannel[channel].refundByOrderId
                        .insert(refund->orderId(), refund);
            }
        }
    }
    QStringList keysToRemove;
    for (auto itChannel = copy->m_ordersByChannel.begin();
         itChannel != copy->m_ordersByChannel.end();
         ++itChannel) {
        if (itChannel.value().nOrders() == 0) {
            keysToRemove << itChannel.key();
        }
    }
    for (auto key : keysToRemove) {
        copy->m_ordersByChannel.remove(key);
    }
    copy->_generateTree();
    return copy;
}
//----------------------------------------------------------
OrderManager *OrderManager::copyEmpty() const
{
    OrderManager *copy = new OrderManager();
    copy->m_updatedAddressByShipment = m_updatedAddressByShipment;
    copy->m_originalAddressByShipment = m_originalAddressByShipment;
    copy->m_updatedDateByShipmentRefund = m_updatedDateByShipmentRefund;
    copy->m_originalDateByShipmentRefund = m_originalDateByShipmentRefund;
    connect(instance(),
            &OrderManager::addressChanged,
            [copy](OrderManager *emeter, const QString &channel, const QString &shipmentId){
        if (copy != emeter) {
            if (copy->m_ordersByChannel.contains(channel)
                    && copy->m_ordersByChannel[channel].shipmentById.contains(shipmentId)) {
                copy->m_originalAddressByShipment[shipmentId] = emeter->m_originalAddressByShipment[shipmentId];
                copy->m_updatedAddressByShipment[shipmentId] = emeter->m_updatedAddressByShipment[shipmentId];
                copy->m_ordersByChannel[channel].shipmentById[shipmentId]->setAddressFrom(
                            emeter->m_ordersByChannel[channel].shipmentById[shipmentId]->getAddressFrom());
                emit copy->dataChanged(copy->index(0,0), copy->index(copy->rowCount()-1, copy->columnCount()-1));
            }
        }
    });
    connect(instance(),
            &OrderManager::addressChangeCancelled,
            [copy](OrderManager *emeter, const QString &channel, const QString &shipmentId){
        if (copy != emeter) {
            if (copy->m_ordersByChannel.contains(channel)
                    && copy->m_ordersByChannel[channel].shipmentById.contains(shipmentId)) {
                copy->m_ordersByChannel[channel].shipmentById[shipmentId]->setAddressFrom(
                            emeter->m_originalAddressByShipment[shipmentId]);
                copy->m_originalAddressByShipment.remove(shipmentId);
                copy->m_updatedAddressByShipment.remove(shipmentId);
                emit copy->dataChanged(copy->index(0,0), copy->index(copy->rowCount()-1, copy->columnCount()-1));
            }
        }
    });
    connect(instance(),
            &OrderManager::dateTimeChanged,
            [copy](OrderManager *emeter, const QString &channel, const QString &shipmentId){
        if (copy != emeter) {
            bool containsShipment = false;
            bool containsRefund = copy->m_ordersByChannel.contains(channel)
                    && copy->m_ordersByChannel[channel].refundById.contains(shipmentId);
            if (!containsRefund) {
                containsShipment = copy->m_ordersByChannel.contains(channel)
                        && copy->m_ordersByChannel[channel].shipmentById.contains(shipmentId);
            }
            bool contains = containsShipment | containsRefund;
            if (contains) {
                copy->m_originalDateByShipmentRefund[shipmentId] = emeter->m_originalDateByShipmentRefund[shipmentId];
                copy->m_updatedDateByShipmentRefund[shipmentId] = emeter->m_updatedDateByShipmentRefund[shipmentId];
                if (containsShipment) {
                    copy->m_ordersByChannel[channel].shipmentById[shipmentId]->setDateTime(
                                emeter->m_ordersByChannel[channel].shipmentById[shipmentId]->getDateTime());
                } else {
                    copy->m_ordersByChannel[channel].refundById[shipmentId]->setDateTime(
                                emeter->m_ordersByChannel[channel].refundById[shipmentId]->getDateTime());
                }
                emit copy->dataChanged(copy->index(0,0), copy->index(copy->rowCount()-1, copy->columnCount()-1));
            }
        }
    });
    connect(instance(),
            &OrderManager::dateChangedCancelled,
            [copy](OrderManager *emeter, const QString &channel, const QString &shipmentId){
        if (copy != emeter) {
            bool contains = copy->m_ordersByChannel.contains(channel)
                    && (copy->m_ordersByChannel[channel].refundById.contains(shipmentId)
                        || copy->m_ordersByChannel[channel].shipmentById.contains(shipmentId));
            if (contains) {
                copy->m_ordersByChannel[channel].shipmentById[shipmentId]->setDateTime(
                            emeter->m_originalDateByShipmentRefund[shipmentId]);
                copy->m_originalDateByShipmentRefund.remove(shipmentId);
                copy->m_updatedDateByShipmentRefund.remove(shipmentId);
                emit copy->dataChanged(copy->index(0,0), copy->index(copy->rowCount()-1, copy->columnCount()-1));
            }
        }
    });

    return copy;
}
//----------------------------------------------------------
OrderManager::OrderManager(QObject *parent)
    : QAbstractItemModel(parent)
{
    m_rootItem = new OrderManagerNodeRoot(this);
    QString selectedCustomerId = CustomerManager::instance()->getSelectedCustomerId();
    if (!selectedCustomerId.isEmpty()) {
        onCustomerSelectedChanged(selectedCustomerId);
    }
    connect(CustomerManager::instance(),
            &CustomerManager::selectedCustomerChanged,
            this,
            &OrderManager::onCustomerSelectedChanged);
    connect(ShippingAddressesManager::instance(),
            &ShippingAddressesManager::adressAboutToBeRemoved,
            this,
            &OrderManager::onAddressBeingRemoved);
}
//----------------------------------------------------------
OrderManager::~OrderManager()
{
    disconnect(CustomerManager::instance());
    disconnect(ShippingAddressesManager::instance());
    disconnect(instance());
}
//----------------------------------------------------------
void OrderManager::onCustomerSelectedChanged(
        const QString &customerId)
{
    m_settingKey = "OrderManager_" + customerId;
    loadFromSettings();
}
//----------------------------------------------------------
void OrderManager::onAddressBeingRemoved(const QString &internalId)
{
    QSet<QString> toDelete;
    for (auto it = m_updatedAddressByShipment.begin();
         it != m_updatedAddressByShipment.end();
         ++it) {
        if (it.value() == internalId) {
            toDelete << it.key();
        }
    }
    bool addressesChanged = false;
    for (auto shipmentId : toDelete) {
        m_updatedAddressByShipment.remove(shipmentId);
        Address originalAddress = m_originalAddressByShipment[shipmentId];
        m_originalAddressByShipment.remove(shipmentId);
        for (auto mapping : m_ordersByChannel) {
            if (mapping.shipmentById.contains(shipmentId)) {
                auto shipment = mapping.shipmentById[shipmentId];
                shipment->setAddressFrom(originalAddress);
                addressesChanged = true;
            }
        }
    }
    if (addressesChanged) {
        saveInSettings();
        emit dataChanged(index(0,0), index(rowCount()-1, columnCount()-1));
    }
}
//----------------------------------------------------------
void OrderManager::_clear()
{
    beginRemoveRows(QModelIndex(), 0, rowCount()-1);
    m_updatedAddressByShipment.clear();
    m_originalAddressByShipment.clear();
    m_updatedDateByShipmentRefund.clear();
    m_originalDateByShipmentRefund.clear();
    m_ordersByChannel.clear();
    if (m_rootItem != nullptr) {
        delete m_rootItem;
    }
    endRemoveRows();
    m_rootItem = new OrderManagerNodeRoot(this);
}
//----------------------------------------------------------
void OrderManager::clearOrders()
{
    if (rowCount() > 0) {
        beginRemoveRows(QModelIndex(), 0, rowCount()-1);
        m_ordersByChannel.clear();
        if (m_rootItem != nullptr) {
            delete m_rootItem;
        }
        endRemoveRows();
        m_rootItem = new OrderManagerNodeRoot(this);
    }
}
//----------------------------------------------------------
void OrderManager::saveInSettings() const
{
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (!m_updatedAddressByShipment.isEmpty()) {
        QStringList elements;
        for (auto it = m_updatedAddressByShipment.begin();
             it != m_updatedAddressByShipment.end();
             ++it) {
            elements << it.key() + ":::" + it.value();
        }
        settings.setValue(m_settingKey, elements.join(";;;"));
    } else if (settings.contains(m_settingKey)) {
        settings.remove(m_settingKey);
    }
    if (!m_updatedDateByShipmentRefund.isEmpty()) {
        QStringList elements;
        for (auto it = m_updatedDateByShipmentRefund.begin();
             it != m_updatedDateByShipmentRefund.end();
             ++it) {
            elements << it.key() + ":::" + it.value().toString(
                            SettingManager::DATE_TIME_FORMAT_ORDER);
        }
        settings.setValue(m_settingKey, elements.join(";;;"));
    } else if (settings.contains(_settingKeyUpdatedDate())) {
        settings.remove(_settingKeyUpdatedDate());
    }
}
//----------------------------------------------------------
void OrderManager::loadFromSettings()
{
    _clear();
    QSettings settings(SettingManager::instance()->settingsFilePath(),
                       QSettings::IniFormat);
    if (settings.contains(m_settingKey)) {
        QStringList elements = settings.value(m_settingKey).toString().split(";;;");
        for (auto element : qAsConst(elements)) {
            QStringList keyValue = element.split(":::");
            m_updatedAddressByShipment[keyValue[0]] = keyValue[1];
        }
    }
    if (settings.contains(_settingKeyUpdatedDate())) {
        QStringList elements = settings.value(_settingKeyUpdatedDate()).toString().split(";;;");
        for (auto element : qAsConst(elements)) {
            QStringList keyValue = element.split(":::");
            m_updatedDateByShipmentRefund[keyValue[0]] = QDateTime::fromString(
                        keyValue[1],
                    SettingManager::DATE_TIME_FORMAT_ORDER);
        }
    }
}
//----------------------------------------------------------
void OrderManager::updateShippingAddress(
        const QModelIndex &index, const Address &address)
{
    if (isIndexOfShipmentOrOrderOneShip(index)) {
        OrderManagerNode *item
                = static_cast<OrderManagerNode *>(
                    index.internalPointer());
        OrderManagerNode *parent = item->parent();
        while (dynamic_cast<OrderManagerNodeImporter*>(parent) == nullptr) {
            parent = parent->parent();
        }
        QString channel = parent->value().toString();
        QString shipmentId;
        QSharedPointer<Shipment> shipment;
        if (dynamic_cast<OrderManagerNodeOrder*>(item) != nullptr) {
            auto orderItem = static_cast<OrderManagerNodeOrder*>(item);
            QString orderId = orderItem->value().toString();
            auto order = getOrder(channel, orderItem->value().toString());
            shipment = order->getShipments().values()[0];
            shipmentId = shipment->getId();
        } else {
            auto shipmentItem = static_cast<OrderManagerNodeShipment*>(item);
            QString shipmentId = shipmentItem->value().toString();
            shipment = m_ordersByChannel[channel].shipmentById[shipmentId];
        }
        m_updatedAddressByShipment[shipmentId] = address.internalId();
        m_originalAddressByShipment[shipmentId] = shipment->getAddressFrom();
        shipment->setAddressFrom(address);
        saveInSettings();
        auto indexLeft = this->index(index.row(), 0, index.parent());
        auto indexRight = this->index(index.row(), columnCount(index.parent()), index.parent());
        emit dataChanged(indexLeft, indexRight);
        emit instance()->addressChanged(this, channel, shipmentId);
    }
}
//----------------------------------------------------------
void OrderManager::cancelUpdatedShippingAddress(
        const QModelIndex &index)
{
    if (isIndexOfShipmentOrOrderOneShip(index)) {
        OrderManagerNode *item
                = static_cast<OrderManagerNode *>(
                    index.internalPointer());
        OrderManagerNode *parent = item->parent();
        while (dynamic_cast<OrderManagerNodeImporter*>(parent) == nullptr) {
            parent = parent->parent();
        }
        QString channel = parent->value().toString();
        QString shipmentId;
        QSharedPointer<Shipment> shipment;
        if (dynamic_cast<OrderManagerNodeOrder*>(item) != nullptr) {
            auto orderItem = static_cast<OrderManagerNodeOrder*>(item);
            QString orderId = orderItem->value().toString();
            auto order = getOrder(channel, orderItem->value().toString());
            shipment = order->getShipments().values()[0];
            shipmentId = shipment->getId();
        } else {
            auto shipmentItem = static_cast<OrderManagerNodeShipment*>(item);
            QString shipmentId = shipmentItem->value().toString();
            shipment = m_ordersByChannel[channel].shipmentById[shipmentId];
        }
        m_updatedAddressByShipment.remove(shipmentId);
        shipment->setAddressFrom(m_originalAddressByShipment[shipmentId]);
        m_originalAddressByShipment.remove(shipmentId);
        saveInSettings();
        auto indexLeft = this->index(index.row(), 0, index.parent());
        auto indexRight = this->index(index.row(), columnCount(index.parent()), index.parent());
        emit dataChanged(indexLeft, indexRight);
        emit instance()->addressChangeCancelled(this, channel, shipmentId);
    }

}
//----------------------------------------------------------
void OrderManager::updateDateTime(
        const QModelIndex &index, const QDateTime &dateTime)
{
    if (isIndexOfShipmentOrOrderOneShip(index)) {
        OrderManagerNode *item
                = static_cast<OrderManagerNode *>(
                    index.internalPointer());
        OrderManagerNode *parent = item->parent();
        while (dynamic_cast<OrderManagerNodeImporter*>(parent) == nullptr) {
            parent = parent->parent();
        }
        QString channel = parent->value().toString();
        QString shipmentId;
        QSharedPointer<Shipment> shipment;
        if (dynamic_cast<OrderManagerNodeOrder*>(item) != nullptr) {
            auto orderItem = static_cast<OrderManagerNodeOrder*>(item);
            QString orderId = orderItem->value().toString();
            auto order = getOrder(channel, orderItem->value().toString());
            shipment = order->getShipments().values()[0];
            shipmentId = shipment->getId();
        } else {
            auto shipmentItem = static_cast<OrderManagerNodeShipment*>(item);
            QString shipmentId = shipmentItem->value().toString();
            shipment = m_ordersByChannel[channel].shipmentById[shipmentId];
        }
        m_updatedDateByShipmentRefund[shipmentId] = dateTime;
        m_originalAddressByShipment[shipmentId] = shipment->getAddressFrom();
        shipment->setDateTime(dateTime);
        saveInSettings();
        auto indexLeft = this->index(index.row(), 0, index.parent());
        auto indexRight = this->index(index.row(), columnCount(index.parent()), index.parent());
        emit dataChanged(indexLeft, indexRight);
        emit instance()->dateTimeChanged(this, channel, shipmentId);
    }
}
//----------------------------------------------------------
void OrderManager::cancelUpdatedDateTime(const QModelIndex &index)
{
    if (isIndexOfShipmentOrOrderOneShip(index)) {
        OrderManagerNode *item
                = static_cast<OrderManagerNode *>(
                    index.internalPointer());
        OrderManagerNode *parent = item->parent();
        while (dynamic_cast<OrderManagerNodeImporter*>(parent) == nullptr) {
            parent = parent->parent();
        }
        QString channel = parent->value().toString();
        QString shipmentId;
        QSharedPointer<Shipment> shipment;
        if (dynamic_cast<OrderManagerNodeOrder*>(item) != nullptr) {
            auto orderItem = static_cast<OrderManagerNodeOrder*>(item);
            QString orderId = orderItem->value().toString();
            auto order = getOrder(channel, orderItem->value().toString());
            shipment = order->getShipments().values()[0];
            shipmentId = shipment->getId();
        } else {
            auto shipmentItem = static_cast<OrderManagerNodeShipment*>(item);
            QString shipmentId = shipmentItem->value().toString();
            shipment = m_ordersByChannel[channel].shipmentById[shipmentId];
        }
        m_updatedDateByShipmentRefund.remove(shipmentId);
        shipment->setDateTime(m_originalDateByShipmentRefund[shipmentId]);
        m_originalDateByShipmentRefund.remove(shipmentId);
        saveInSettings();
        auto indexLeft = this->index(index.row(), 0, index.parent());
        auto indexRight = this->index(index.row(), columnCount(index.parent()), index.parent());
        emit dataChanged(indexLeft, indexRight);
        emit instance()->dateChangedCancelled(this, channel, shipmentId);
    }
}
//----------------------------------------------------------
QMultiMap<QString, double> OrderManager::getNonOrderFees(
        const QString &channel, int year, const QString &paymentId) const
{
    QMultiMap<QString, double> allFees;
    for (auto fees : m_ordersByChannel[channel].nonOrderFees[year][paymentId]) {
        allFees.unite(fees);
    }
    for (auto shipment : m_ordersByChannel[channel].shipmentByDate[year]) {
        if (shipment->getPaymentId() == paymentId) {
            auto shipmentFees = shipment->getChargedFeesBySKU();
            for (auto shipmentFee : shipmentFees) {
                for (auto itFee = shipmentFee.begin();
                     itFee != shipmentFee.end();
                     ++itFee) {
                    if (itFee.value().amountTotal < 0.) {
                        allFees.insert(itFee.key(),
                                       itFee.value().amountTotal);
                    }
                }
            }
        }
    }
    return allFees;
}
//----------------------------------------------------------
QMultiMap<QDateTime, QSharedPointer<Shipment> > OrderManager::getShipmentsOfPayment(
        const QString &channel, int year, const QString &paymentId) const
{
    QMultiMap<QDateTime, QSharedPointer<Shipment> > shipments;
    for (auto shipment : m_ordersByChannel[channel].shipmentByDate[year]) {
        if (shipment->getPaymentId() == paymentId) {
            shipments.insert(shipment->getDateTime(), shipment);
        }
    }
    return shipments;
}
//----------------------------------------------------------
QMultiMap<QDateTime, Shipment *> OrderManager::getShipmentsAndRefunds(
        const QDate &begin, const QDate &end) const
{
    QMultiMap<QDateTime, Shipment *> shipmentsAndRefunds;
    for (auto it = m_ordersByChannel.begin();
         it != m_ordersByChannel.end();
         ++it) {
        for (auto itYear = it.value().shipmentByDate.begin();
             itYear != it.value().shipmentByDate.end();
             ++itYear) {
            for (auto shipment : itYear.value()) {
                auto date = shipment->getDateTime().date();
                if (date >= begin && date <= end) {
                    shipmentsAndRefunds.insert(shipment->getDateTime(), shipment.data());
                }
            }
        }
        for (auto itYear = it.value().refundByDate.begin();
             itYear != it.value().refundByDate.end();
             ++itYear) {
            for (auto refund : itYear.value()) {
                auto date = refund->getDateTime().date();
                if (date >= begin && date <= end) {
                    shipmentsAndRefunds.insert(refund->getDateTime(), refund.data());
                }
            }
        }
    }
    return shipmentsAndRefunds;
}
//----------------------------------------------------------
QMultiMap<QDateTime, Shipment *> OrderManager::getShipmentsAndRefunds(
        std::function<bool (const Shipment *)> filterIsOk) const
{
    QMultiMap<QDateTime, Shipment *> shipmentsAndRefunds;
    for (auto it = m_ordersByChannel.begin();
         it != m_ordersByChannel.end();
         ++it) {
        for (auto itYear = it.value().shipmentByDate.begin();
             itYear != it.value().shipmentByDate.end();
             ++itYear) {
            for (auto shipment : itYear.value()) {
                if (filterIsOk(shipment.data())) {
                    shipmentsAndRefunds.insert(shipment->getDateTime(), shipment.data());
                }
            }
        }
        for (auto itYear = it.value().refundByDate.begin();
             itYear != it.value().refundByDate.end();
             ++itYear) {
            for (auto refund : itYear.value()) {
                if (filterIsOk(refund.data())) {
                    auto totalTaxed = refund->getTotalPriceTaxed();
                    shipmentsAndRefunds.insert(refund->getDateTime(), refund.data());
                }
            }
        }
    }
    return shipmentsAndRefunds;
}
//----------------------------------------------------------
QList<int> OrderManager::vatColIndexes()
{
    static QList<int> indexes = [this]() -> QList<int> {
        QList<int> indexes;
        auto _colInfos = colInfos();
        QStringList colNames = {COL_VAT, COL_VAT_REGIME, COL_COUNTRY_VAT, COL_COUNTRY_VAT_DECL};
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
const QMap<QString, OrdersMapping> &OrderManager::getOrdersByChannel() const
{
    return m_ordersByChannel;
}
//----------------------------------------------------------
void OrderManager::_generateTree()
{
    for (auto itChannel = m_ordersByChannel.begin();
         itChannel != m_ordersByChannel.end();
         ++itChannel) {
        OrderManagerNode *itemChannel
                = new OrderManagerNodeImporter(
                    itChannel.key(), this, m_rootItem);
        QMap<int, QMultiMap<QDateTime, QSharedPointer<Order>>> *p_orderByDate;
        QMap<int, QMultiMap<QDateTime, QSharedPointer<Order>>> orderByDateCopy;
        if (itChannel.value().orderByDate.size() == 0
                && itChannel.value().orderById.size() > 0) { ///Means we don't have order date
            QSet<QString> orderIdsAdded;
            for (auto shipment : itChannel.value().shipmentById) {
                QDateTime dateTime = shipment->getDateTime();
                int year = dateTime.date().year();
                if (!orderByDateCopy.contains(year)) {
                    orderByDateCopy[year] = QMultiMap<QDateTime, QSharedPointer<Order>>();
                }
                auto orderId = shipment->getOrder()->getId();
                if(!orderIdsAdded.contains(orderId)) {
                    orderIdsAdded << orderId;
                    auto order = itChannel.value().orderById[orderId];
                    orderByDateCopy[year].insert(dateTime, order);
                }
            }
            p_orderByDate = &orderByDateCopy;
        } else {
            p_orderByDate = &itChannel.value().orderByDate;
        }
        for (auto itYear = p_orderByDate->begin();
             itYear != p_orderByDate->end();
             ++itYear) {
            OrderManagerNode *itemYear
                    = new OrderManagerNodeYear(
                        itYear.key(), this, itemChannel);
            //if (itemYear->value() == "0") {
                //int TEMP=0;++TEMP;
            //}
            for (auto itDate = itYear.value().begin();
                 itDate != itYear.value().end();
                 ++itDate) {
                OrderManagerNode *itemOrder
                    = new OrderManagerNodeOrder(
                        itDate.value()->getId(), this, itemYear);
                if (itDate.value()->getShipmentCount() > 1) {
                    auto shipments = itDate.value()->getShipments();
                    for (auto itShipment = shipments.begin();
                         itShipment != shipments.end();
                         ++itShipment) {
                        OrderManagerNode *itemShipment
                                = new OrderManagerNodeShipment(
                                    itShipment.value()->getId(), this, itemOrder);
                        auto articles = itShipment.value()->getArticlesShipped();
                        for (auto itArticle = articles.begin();
                             itArticle != articles.end();
                             ++itArticle) {
                            OrderManagerNode *itemArticle
                                    = new OrderManagerNodeArticle(
                                        itArticle.value()->getSku(), this, itemShipment);
                            (void) itemArticle; /// To avoid unused warning as we use it for debuging
                        }
                    }
                } else {
                    int rowArticle = 0;
                    auto order = itDate.value();
                    if (order->getShipmentCount() > 0) {
                        auto articles = order->getShipmentFirst()->getArticlesShipped();
                        for (auto itArticle = articles.begin();
                             itArticle != articles.end();
                             ++itArticle) {
                            OrderManagerNode *itemArticle
                                    = new OrderManagerNodeArticle(
                                        itArticle.value()->getSku(), this, itemOrder);
                            ++rowArticle;
                            (void) itemArticle; /// To avoid unused warning as we use it for debuging
                        }
                    }
                }
            }
        }
    }
    beginInsertRows(QModelIndex(), 0, m_ordersByChannel.size()-1);
    endInsertRows();
}
//----------------------------------------------------------
void OrderManager::recordOrders(
        const QString &channel, const OrdersMapping &ordersMapping)
{
    //Q_ASSERT(!m_ordersByChannel[channel].orderByDate.contains(0));
    //Q_ASSERT(!ordersMapping.orderByDate.contains(0));
    bool containsFirst = false;
    if (m_ordersByChannel.contains(channel)) {
        m_ordersByChannel[channel].unite(ordersMapping); //TODO unit
        //Q_ASSERT(!m_ordersByChannel[channel].orderByDate.contains(0));
    } else {
        m_ordersByChannel[channel] = ordersMapping;
    }
    beginRemoveRows(QModelIndex(), 0, rowCount()-1);
    if (m_rootItem != nullptr) {
        delete m_rootItem;
    }
    m_rootItem = new OrderManagerNodeRoot(this);
    for (auto shipment : m_ordersByChannel[channel].shipmentById) {
        if (m_updatedAddressByShipment.contains(shipment->getId())) {
            auto updatedAddressId = m_updatedAddressByShipment[shipment->getId()];
            if (updatedAddressId != shipment->getAddressFrom().internalId()) {
                m_originalAddressByShipment[shipment->getId()] = shipment->getAddressFrom();
                shipment->setAddressFrom(
                            ShippingAddressesManager::instance()->getAddress(
                                updatedAddressId));
            } else {
                m_updatedAddressByShipment.remove(shipment->getId());
            }
        }
        if (m_updatedDateByShipmentRefund.contains(shipment->getId())) {
            auto updatedDateTime = m_updatedDateByShipmentRefund[shipment->getId()];
            if (updatedDateTime != shipment->getDateTime()) {
                m_originalDateByShipmentRefund[shipment->getId()] = shipment->getDateTime();
                shipment->setDateTime(updatedDateTime);
            } else {
                m_updatedDateByShipmentRefund.remove(shipment->getId());
            }
        }
    }
    for (auto refund : m_ordersByChannel[channel].refundById) {
        QString refundId = refund->getId();
        if (m_updatedDateByShipmentRefund.contains(refundId)) {
            auto updatedDateTime = m_updatedDateByShipmentRefund[refundId];
            if (updatedDateTime != refund->getDateTime()) {
                m_originalDateByShipmentRefund[refundId] = refund->getDateTime();
                refund->setDateTime(updatedDateTime);
            } else {
                m_updatedDateByShipmentRefund.remove(refundId);
            }
        }
    }
    endRemoveRows();
    _generateTree(); //TODO Optimise and only generate for channel;
    emit ordersRecorded();
}
//----------------------------------------------------------
void OrderManager::recordOrders(const OrderManager *otherManager)
{
    for (auto it = otherManager->m_ordersByChannel.begin();
         it != otherManager->m_ordersByChannel.end();
         ++it) {
        recordOrders(it.key(), it.value());
    }
}
//----------------------------------------------------------
void OrderManager::createRefundsFromUncomplete()
{
    for (auto it = m_ordersByChannel.begin(); it != m_ordersByChannel.end(); ++it) {
        it.value().createRefundsFromUncomplete();
    }
}
//----------------------------------------------------------
QStringList OrderManager::channels() const
{
    return m_ordersByChannel.keys();
}
//----------------------------------------------------------
bool OrderManager::isIndexOfOrder(const QModelIndex &index) const
{
    bool is = false;
    if (index.isValid()) {
        OrderManagerNode *itemParent
                = static_cast<OrderManagerNode *>(
                    index.internalPointer());
        is = dynamic_cast<OrderManagerNodeOrder*>(itemParent) != nullptr;
    }
    return is;
}
//----------------------------------------------------------
bool OrderManager::isIndexOfShipmentOrOrderOneShip(
        const QModelIndex &index) const
{
    bool is = false;
    if (index.isValid()) {
        OrderManagerNode *item
                = static_cast<OrderManagerNode *>(
                    index.internalPointer());
        is = dynamic_cast<OrderManagerNodeShipment*>(item) != nullptr;
        if (!is) {
            auto itemOrder = dynamic_cast<OrderManagerNodeOrder*>(item);
            is = itemOrder != nullptr /// It means it is an order with one shipment
                    && itemOrder->rowCount() > 0 ///In case it is an order not shipped yet
                    && dynamic_cast<OrderManagerNodeShipment*>(itemOrder->child(0)) == nullptr;
        }
    }
    return is;
}
//----------------------------------------------------------
bool OrderManager::isIndexInOrder(QModelIndex index) const
{
    // TODO delete because it seems it can only return false
    while (index.isValid()) {
        if (isIndexOfOrder(index)) {
            /*
            OrderManagerNode *item
                    = static_cast<OrderManagerNodeOrder *>(
                        index.internalPointer());
            return item->row() % 2 == 0;
            //*/
            return true;
        }
        index = parent(index);
    }
    return false;
}
//----------------------------------------------------------
const Order *OrderManager::getOrder(
        const QString &channel, const QString &orderId) const
{
    if (m_ordersByChannel.contains(channel)
            && m_ordersByChannel[channel].orderById.contains(orderId)) {
        return m_ordersByChannel[channel].orderById[orderId].data();
    }
    return nullptr;
}
//----------------------------------------------------------
Order *OrderManager::getOrderNotConst(
        const QString &channel, const QString &orderId)
{
    Order *order = nullptr;
    if (m_ordersByChannel.contains(channel)
            && m_ordersByChannel[channel].orderById.contains(orderId)) {
        order = m_ordersByChannel[channel].orderById[orderId].data();
    }
    return order;
}
//----------------------------------------------------------
QList<QSharedPointer<Order> > OrderManager::getOrders(
        const QString &channel, std::function<bool (const Order *)> filterIsOrderOk) const
{
    QList<QSharedPointer<Order> > orders;
    for (auto order : m_ordersByChannel[channel].orderById) {
        if (filterIsOrderOk(order.data())) {
            orders << order;
        }
    }
    return orders;
}
//----------------------------------------------------------
bool OrderManager::containsOrder(const QString &channel, const QString &orderId)
{
    return m_ordersByChannel.contains(channel)
            && m_ordersByChannel[channel].orderById.contains(orderId);
}
//----------------------------------------------------------
void OrderManager::exportCsv(const QString filePath) const
{
    static QList<OrderManager::ColCsvInfo> colCsvInfos
            = {
        {tr("id-commande"), [](const Shipment *shipment) -> QString{
             return shipment->getOrder()->getId();}}
        , {tr("id-expedition"), [](const Shipment *shipment) -> QString{
             return shipment->getId();}}
        , {tr("date-expedition"), [](const Shipment *shipment) -> QString{
             return shipment->getDateTime().toString(("yyyy-MM-dd hh:mm:ss"));}}
        , {tr("channel"), [](const Shipment *shipment) -> QString{
             return shipment->getOrder()->getChannel();}}
        , {tr("sous-channel"), [](const Shipment *shipment) -> QString{
             return shipment->getOrder()->getSubchannel();}}
        , {tr("total-ttc"), [](const Shipment *shipment) -> QString{
             return QString::number(shipment->getTotalPriceTaxed(), 'f', 2);}}
        , {tr("total-ht"), [](const Shipment *shipment) -> QString{
             return QString::number(shipment->getTotalPriceUntaxed(), 'f', 2);}}
        , {tr("total-tva"), [](const Shipment *shipment) -> QString{
             return QString::number(shipment->getTotalPriceTaxes(), 'f', 2);}}
        , {tr("commande-pro"), [](const Shipment *shipment) -> QString{
             return shipment->getOrder()->isBusinessCustomer()
               ? tr("Oui") : tr("Non");}}
        , {tr("acheteur-numero-tva"), [](const Shipment *shipment) -> QString{
             return shipment->getOrder()->getVatNumber();}}
        , {tr("nom-adresse-expedition"), [](const Shipment *shipment) -> QString{
             return shipment->getAddressFrom().fullName();}}
        , {tr("pays-expedition"), [](const Shipment *shipment) -> QString{
             return shipment->getAddressFrom().countryCode();}}
        , {tr("pays-destination"), [](const Shipment *shipment) -> QString{
             return shipment->getOrder()->getAddressTo().countryCode();}}
        , {tr("pays-tva"), [](const Shipment *shipment) -> QString{
             return shipment->getCountryCodeVat();}}
        , {tr("regime-tva"), [](const Shipment *shipment) -> QString{
             return shipment->getRegimeVat();}}
        , {tr("date-commande"), [](const Shipment *shipment) -> QString{
             return shipment->getOrder()->getDateTime().toString(("yyyy-MM-dd hh:mm:ss"));}}
        , {tr("acheteur-adresse-1"), [](const Shipment *shipment) -> QString{
             return shipment->getOrder()->getAddressTo().street1();}}
        , {tr("acheteur-adresse-2"), [](const Shipment *shipment) -> QString{
             return shipment->getOrder()->getAddressTo().street2();}}
        , {tr("acheteur-code-postal"), [](const Shipment *shipment) -> QString{
             return shipment->getOrder()->getAddressTo().postalCode();}}
        , {tr("acheteur-ville"), [](const Shipment *shipment) -> QString{
             return shipment->getOrder()->getAddressTo().city();}}
        , {tr("acheteur-etat"), [](const Shipment *shipment) -> QString{
             return shipment->getOrder()->getAddressTo().state();}}
    };
    QStringList header;
    QString csvSep = ";";
    for (auto info : colCsvInfos) {
        header << info.name;
    }
    QStringList lines;
    lines << header.join(csvSep);
    for (auto itChannel = m_ordersByChannel.begin();
         itChannel != m_ordersByChannel.end();
         ++itChannel) {
        for (auto itYear = itChannel.value().shipmentByDate.begin();
             itYear != itChannel.value().shipmentByDate.end();
             ++itYear) {
            for (auto shipment : itYear.value()) {
                QStringList elements;
                for (auto info : colCsvInfos ) {
                    elements << info.getValueShipment(shipment.data());
                }
                lines << elements.join(csvSep);
            }
        }
    }
    QFile file(filePath);
    if (file.open(QFile::WriteOnly)) {
        QString fileContent = lines.join(SettingManager::instance()->returnLine());
        QTextStream stream(&file);
        stream << fileContent;
        file.close();
    }
}
//----------------------------------------------------------
QVariant OrderManager::headerData(
        int section, Qt::Orientation orientation, int role) const
{
    QVariant value;
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        value = colInfos()[section].name;
    }
    return value;
}
//----------------------------------------------------------
int OrderManager::rowCount(const QModelIndex &parent) const
{
    OrderManagerNode *itemParent = nullptr;
    if (parent.isValid()) {
        itemParent = static_cast<OrderManagerNode *>(
                    parent.internalPointer());
    } else {
        itemParent = m_rootItem;
    }
    int count = itemParent->rowCount();
    return count;
}
//----------------------------------------------------------
int OrderManager::columnCount(const QModelIndex &) const
{
    static int count = colInfos().size();
    return count;
    /*
    OrderManagerNode *itemParent = nullptr;
    if (parent.isValid()) {
        itemParent = static_cast<OrderManagerNode *>(
                    parent.internalPointer());
    } else {
        itemParent = m_rootTreeItem;
    }
    int count = itemParent->columnCount();
    return count;
    //*/
}
//----------------------------------------------------------
QVariant OrderManager::data(const QModelIndex &index, int role) const
{
    QVariant value;
    if (role == Qt::DisplayRole) {
        OrderManagerNode *item
                = static_cast<OrderManagerNode *>(
                    index.internalPointer());
        value = item->data(index.column(), role);
    } else if (role == Qt::BackgroundRole) {
        if (isIndexInOrder(index)) {
            OrderManagerNode *item
                    = static_cast<OrderManagerNode *>(
                        index.internalPointer());
            while(dynamic_cast<OrderManagerNodeOrder*>(item) == nullptr) {
                item = item->parent();
            }
            if (item->row() % 2 == 0) {
                value = SettingManager::instance()->brushLightBlue();
            }
        }
        if (isIndexOfShipmentOrOrderOneShip(index)
                && index.column() == colIndex("Address expédition")) {
            OrderManagerNode *item
                    = static_cast<OrderManagerNode *>(
                        index.internalPointer());
            OrderManagerNode *itemParent = item->parent();
            while (dynamic_cast<OrderManagerNodeImporter *>(itemParent) == nullptr) {
                itemParent = itemParent->parent();
            }
            QString channel = itemParent->value().toString();
            QString shipmentId;
            if (dynamic_cast<OrderManagerNodeOrder*>(item) != nullptr) {
                OrderManagerNodeOrder *itemOrder = static_cast<OrderManagerNodeOrder*>(item);
                QString orderId = itemOrder->value().toString();
                auto order = getOrder(channel, orderId);
                shipmentId = order->getShipmentFirst()->getId();
            } else {
                OrderManagerNodeShipment *itemShipment = static_cast<OrderManagerNodeShipment*>(item);
                shipmentId = itemShipment->value().toString();
            }
            if (m_updatedAddressByShipment.contains(shipmentId)) {
                value = SettingManager::instance()->brushRed();
            }
        }
    }
    return value;
}
//----------------------------------------------------------
Qt::ItemFlags OrderManager::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = Qt::ItemIsEnabled;
    OrderManagerNode *item
            = static_cast<OrderManagerNode *>(
                index.internalPointer());
    if (dynamic_cast<OrderManagerNodeOrder*>(item) != nullptr) {
        flags |= Qt::ItemIsSelectable;
    }
    return flags;
}
//----------------------------------------------------------
QModelIndex OrderManager::index(
        int row, int column, const QModelIndex &parent) const
{
    QModelIndex index;
    if (hasIndex(row, column, parent)) {
        OrderManagerNode *item = nullptr;
        if (parent.isValid()) {
            OrderManagerNode *itemParent
                    = static_cast<OrderManagerNode *>(
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
QModelIndex OrderManager::parent(const QModelIndex &index) const
{
    QModelIndex parentIndex;
    if (index.isValid()) {
        OrderManagerNode *item
                = static_cast<OrderManagerNode *>(
                    index.internalPointer());
        if (item->parent() != nullptr) {
            parentIndex = createIndex(item->parent()->row(), 0, item->parent());
        }
    }
    return parentIndex;
}
//----------------------------------------------------------
QList<OrderManager::ColInfo> OrderManager::colInfos() const
{
    static QList<OrderManager::ColInfo> colInfos
            = {{tr("Id"), [](const Order *order) -> QVariant{
                    return order->getId();},
                [](const Shipment *shipment) -> QVariant{
                    return shipment->getId();},
                [](const ArticleSold *article) -> QVariant{
                    return article->getSku();
                }},
               {tr("Id activité"), [](const Order *order) -> QVariant{
                    if (order->getShipmentCount() == 1) {
                        return order->getShipmentFirst()->getId();
                    }
                    return "";},
                [](const Shipment *shipment) -> QVariant{
                    return shipment->getId();},
                [](const ArticleSold *article) -> QVariant{
                    return article->getShipment()->getId();
                }},
               {tr("Sous-channel"), [](const Order *order) -> QVariant{
                    return order->getSubchannel();},
                [](const Shipment *) -> QVariant{
                    return "";},
                [](const ArticleSold *) -> QVariant{
                    return "";
                }},
               {tr("Expédié vendeur"), [](const Order *order) -> QVariant{
                    return order->getShippedBySeller()? tr("Oui"):tr("Non");},
                [](const Shipment *) -> QVariant{
                    return "";},
                [](const ArticleSold *) -> QVariant{
                    return "";
                }},
               {tr("Montant TTC"), [](const Order *order) -> QVariant{
                    return QString::number(order->getTotalPriceTaxed(), 'f', 2);},
                [](const Shipment *shipment) -> QVariant{
                    return QString::number(shipment->getTotalPriceTaxed(), 'f', 2);},
                [](const ArticleSold *article) -> QVariant{
                    return article->getTotalPriceTaxed();
                }},
               {COL_VAT, [](const Order *order) -> QVariant{
                    return QString::number(order->getTotalPriceTaxes(), 'f', 2);},
                [](const Shipment *shipment) -> QVariant{
                    return QString::number(shipment->getTotalPriceTaxes(), 'f', 2);},
                [](const ArticleSold *article) -> QVariant{
                    double price = article->getTotalPriceTaxes();
                    if (price < 0.001) {
                        return QString("0.00");
                    }
                    return QString::number(price, 'f', 2);
                }},
               {tr("N Expédition"), [](const Order *order) -> QVariant{
                    return order->getShipments().size();},
                [](const Shipment *) -> QVariant{
                    return "";},
                [](const ArticleSold *) -> QVariant{
                    return "";
                }},
               {tr("N Unité"), [](const Order *order) -> QVariant{
                    return order->getArticleUnitCount();},
                [](const Shipment *shipment) -> QVariant{
                    return shipment->getUnitCounts();},
                [](const ArticleSold *article) -> QVariant{
                    return article->getUnits();
                }},
               {tr("Professionnel"), [](const Order *order) -> QVariant{
                    return order->isBusinessCustomer()?"Oui":"Non";},
                [](const Shipment *) -> QVariant{
                    return "";},
                [](const ArticleSold *) -> QVariant{
                    return "";
                }},
               {tr("Numéro de TVA"), [](const Order *order) -> QVariant{
                    return order->getVatNumber();},
                [](const Shipment *) -> QVariant{
                    return "";},
                [](const ArticleSold *) -> QVariant{
                    return "";
                }},
               {tr("Pays expédition"), [](const Order *order) -> QVariant{
                    QString address;
                    if (order->getShipmentCount() == 1) {
                        address = CountryManager::instance()->countryName(
                        order->getShipments().values()[0]->getAddressFrom().countryCode());
                    }
                    return address;},
                [](const Shipment *shipment) -> QVariant{
                    return CountryManager::instance()->countryName(
                    shipment->getAddressFrom().countryCode());},
                [](const ArticleSold *) -> QVariant{
                    return "";
                }},
               {tr("Pays destination"), [](const Order *order) -> QVariant{
                    return CountryManager::instance()->countryName(order->getAddressTo().countryCode());},
                [](const Shipment *) -> QVariant{
                    return "";},
                [](const ArticleSold *) -> QVariant{
                    return "";
                }},
               {COL_COUNTRY_VAT, [](const Order *order) -> QVariant{
                    QString country;
                    if (order->getShipmentCount() == 1
                    && !order->getShipmentFirst()->getCountryCodeVat().isEmpty()) {
                        country = CountryManager::instance()->countryName(
                        order->getShipmentFirst()->getCountryCodeVat());
                    }
                    return country;},
                [](const Shipment *shipment) -> QVariant{
                    return CountryManager::instance()->countryName(shipment->getCountryCodeVat());},
                [](const ArticleSold *) -> QVariant{
                    return "";
                }},
               {COL_COUNTRY_VAT_DECL, [](const Order *order) -> QVariant{
                    QString country;
                    if (order->getShipmentCount() == 1
                    && !order->getShipmentFirst()->getCountryCodeVat().isEmpty()) {
                        country = CountryManager::instance()->countryName(
                        order->getShipmentFirst()->getCountrySaleDeclaration());
                    }
                    return country;},
                [](const Shipment *shipment) -> QVariant{
                    return CountryManager::instance()->countryName(shipment->getCountrySaleDeclaration());},
                [](const ArticleSold *) -> QVariant{
                    return "";
                }},
               {COL_VAT_REGIME, [](const Order *order) -> QVariant{
                    QString country;
                    if (order->getShipmentCount() == 1) {
                        country = order->getShipmentFirst()->getRegimeVat();
                    }
                    return country;},
                [](const Shipment *shipment) -> QVariant{
                    return shipment->getRegimeVat();},
                [](const ArticleSold *) -> QVariant{
                    return "";
                }},
               {tr("Address expédition"), [](const Order *order) -> QVariant{
                    QString address;
                    if (order->getShipmentCount() == 1) {
                        address = order->getShipments().values()[0]->getAddressFrom().fullName();
                    }
                    return address;},
                [](const Shipment *shipment) -> QVariant{
                    return shipment->getAddressFrom().fullName();},
                [](const ArticleSold *) -> QVariant{
                    return "";
                }},
               {tr("Date commande"), [](const Order *order) -> QVariant{
                    return order->getDateTime().toString("dd MMM yyyy hh:mm");},
                [](const Shipment *) -> QVariant{
                    return "";},
                [](const ArticleSold *) -> QVariant{
                    return "";
                }},
               {tr("Date expédition"), [](const Order *order) -> QVariant{
                    QString date;
                    if (order->getShipmentCount() == 1) {
                        date = order->getShipments().values()[0]->getDateTime().toString("dd MMM yyyy hh:mm");
                    }
                    return date;},
                [](const Shipment *shipment) -> QVariant{
                    return shipment->getDateTime().toString("dd MMM yyyy hh:mm");},
                [](const ArticleSold *) -> QVariant{
                    return "";
                }},
               {tr("Nom client"), [](const Order *order) -> QVariant{
                    return order->getAddressTo().fullName();},
                [](const Shipment *) -> QVariant{
                    return "";},
                [](const ArticleSold *) -> QVariant{
                    return "";
                }},
              };
    return colInfos;
}
//----------------------------------------------------------
int OrderManager::colIndex(const QString &name) const
{
    static auto colIndexMapping = [this]() -> QHash<QString, int> {
        int i = 0;
        QHash<QString, int> mapping;
        for (auto colInfo : colInfos()) {
            mapping[colInfo.name] = i;
            ++i;
        }
        return mapping;
    }();
    return colIndexMapping[name];
}
//----------------------------------------------------------
QString OrderManager::_settingKeyUpdatedDate() const
{
    if (m_settingKey.isEmpty()) {
        return "";
    }
    return m_settingKey + "_date";
}
//----------------------------------------------------------

