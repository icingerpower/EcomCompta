#include <qmessagebox.h>
#include <qfiledialog.h>
#include <QtCore/qsettings.h>
#include <QtCore/qfile.h>
#include <QtCore/qtextstream.h>

#include "WidgetOrders.h"
#include "ui_WidgetOrders.h"
#include "model/orderimporters/OrderManager.h"
#include "model/orderimporters/ShippingAddressesManager.h"
#include "model/orderimporters/RefundManager.h"
#include "gui/panes/dialogs/DialogCreateRefund.h"
#include "model/CustomerManager.h"
#include "model/SettingManager.h"
#include "model/ChangeNotifier.h"

//----------------------------------------------------------
WidgetOrders::WidgetOrders(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetOrders)
{
    ui->setupUi(this);
    ui->treeViewOrders->header()->resizeSection(0, 200);
    m_dialogRefund = nullptr;
    m_orderManager = nullptr;
    _connectSlots();
}
//----------------------------------------------------------
void WidgetOrders::_connectSlots()
{
    connect(CustomerManager::instance(),
            &CustomerManager::selectedCustomerChanged,
            [this](const QString &){
        loadCustomerSettings();
    });
    connect(ui->buttonFilter,
            &QPushButton::clicked,
            this,
            &WidgetOrders::filter);
    connect(ui->buttonResetFilter,
            &QPushButton::clicked,
            this,
            &WidgetOrders::resetFilter);
    connect(ui->buttonExpandAllOrders,
            &QPushButton::clicked,
            this,
            &WidgetOrders::expandAllOrders);
    connect(ui->buttonAddRefund,
            &QPushButton::clicked,
            this,
            &WidgetOrders::addRefund);
    connect(ui->buttoncancelAddresChange,
            &QPushButton::clicked,
            this,
            &WidgetOrders::cancelAddressChange);
    connect(ui->buttonUpdateShippingAddress,
            &QPushButton::clicked,
            this,
            &WidgetOrders::updateShippingAddress);
    connect(ui->buttonExport,
            &QPushButton::clicked,
            this,
            &WidgetOrders::updateShippingAddress);
}

OrderManager *WidgetOrders::orderManager() const
{
    return m_orderManager;
}

//----------------------------------------------------------
WidgetOrders::~WidgetOrders()
{
    clear();
    delete ui;
}
//----------------------------------------------------------
void WidgetOrders::init(
        OrderManager *orderManager, RefundManager *refundManager)
{
    m_orderManager = orderManager;
    m_refundManager = refundManager;
    ui->treeViewOrders->setModel(m_orderManager);
    ui->treeViewOrders->header()->resizeSection(0, 200);
    loadCustomerSettings();
}
//----------------------------------------------------------
void WidgetOrders::clear()
{
    auto currentModel = ui->treeViewOrders->model();
    if (currentModel != m_orderManager
            && currentModel != nullptr) {
        currentModel->deleteLater();
    }
    ui->treeViewOrders->setModel(nullptr);
}
//----------------------------------------------------------
bool WidgetOrders::filterHideOrder(const Order *order) const
{
    bool filter = false;
    if (ui->checkBoxOnlyPro->isChecked() && !order->isBusinessCustomer()) {
        filter = true;
    } else if (ui->checkBoxOnlySeveralShipping->isChecked()
               && order->getShipmentCount() <= 1) {
        filter = true;
    } else if (ui->checkBoxShippedNextYear->isChecked()
               && !order->isShippedNextYear()) {
        filter = true;
    } else if (!ui->lineEditOrderId->text().trimmed().isEmpty()
               && order->getId() != ui->lineEditOrderId->text().trimmed()) {
        filter = true;
    } else if (!ui->comboCountryFilter->currentText().isEmpty()
               && !order->containsCountry(ui->comboCountryFilter->currentText())) {
        filter = true;
    } else if (!ui->lineEditCustomerName->text().trimmed().isEmpty()
               && !order->getAddressTo().fullName().contains(
                   ui->lineEditCustomerName->text().trimmed())) {
        filter = true;
    } else if (!ui->comboBoxSubChannel->currentText().isEmpty()
               && order->getSubchannel() != ui->comboBoxSubChannel->currentText()) {
        filter = true;
    } else if (ui->spinMinAmount->value() > 0.
               && order->getTotalPriceTaxed() < ui->spinMinAmount->value()) {
        filter = true;
    } else if (ui->spinMaxAmount->value() > 0.
               && order->getTotalPriceTaxed() > ui->spinMaxAmount->value()) {
        filter = true;
    } else if (ui->dateEditMin->date() != QDate(2000,1,1)
               && order->getDateTime().date() < ui->dateEditMin->date()) {
        filter = true;
    } else if (ui->dateEditMax->date() != QDate(2000,1,1)
               && order->getDateTime().date() > ui->dateEditMax->date()) {
        filter = true;
    } else if (!ui->lineEditSku->text().trimmed().isEmpty()
               && order->containsSku(ui->lineEditSku->text().trimmed())) {
        filter = true;
    }
    return filter;
}
//----------------------------------------------------------
int WidgetOrders::getOrderCount() const
{
    auto model = ui->treeViewOrders->model();
    if (model != nullptr) {
        int rowCount = model->rowCount();
        if (rowCount == 1) {
            auto indexFirst = model->index(0, 0, QModelIndex());
            if (model->rowCount(indexFirst) == 0) {
                return 0;
            }
        }
        return model->rowCount();
    }
    return 0;
}
//----------------------------------------------------------
void WidgetOrders::hideVatColumns()
{
    auto model = ui->treeViewOrders->model();
    if (model != nullptr) {
        auto indexes = OrderManager::instance()->vatColIndexes();
        for (auto index = indexes.rbegin(); index != indexes.rend(); ++index) {
            ui->treeViewOrders->hideColumn(*index);
        }
    }
}
//----------------------------------------------------------
void WidgetOrders::loadCustomerSettings()
{
    ui->comboBoxShippingAddress->setModel(ShippingAddressesManager::instance());
    //ui->comboBoxShippingAddress->setModel(ShippingAddressesManager::instance());
    /*
    QString defaultShippingAddressId = m_importer->getDefaultShippingAddressId();
    if (!defaultShippingAddressId.isEmpty()) {
        Address address = ShippingAddressesManager::instance()
                ->getAddress(defaultShippingAddressId);
        ui->comboShippingAddressDefault->setCurrentText(address.label());
    } else if (ShippingAddressesManager::instance()->rowCount() > 0){
        Address address = ShippingAddressesManager::instance()
                ->getAddress(0);
        m_importer->setDefaultShippingAddress(address);
    }
    //*/
}
//----------------------------------------------------------
void WidgetOrders::filter()
{
    auto copyOrderManager = m_orderManager->copyFilter(
                [this](const Order * order) -> bool {
        return !filterHideOrder(order);
    });
    auto currentModel = ui->treeViewOrders->model();
    ui->treeViewOrders->setModel(copyOrderManager);

    ui->treeViewOrders->expandAll();
    if (currentModel != m_orderManager && currentModel != nullptr) {
        currentModel->deleteLater();
    }
}
//----------------------------------------------------------
void WidgetOrders::resetFilter()
{
    auto currentModel = ui->treeViewOrders->model();
    ui->treeViewOrders->setModel(m_orderManager);
    if (currentModel != m_orderManager && currentModel != nullptr) {
        currentModel->deleteLater();
    }
    ui->treeViewOrders->expandAll();
}
//----------------------------------------------------------
void WidgetOrders::expandAllOrders()
{
    ui->treeViewOrders->expandAll();
}
//----------------------------------------------------------
void WidgetOrders::addRefund()
{
    auto selection = ui->treeViewOrders->selectionModel()->selectedRows();
    if (selection.size() == 0) {
        QMessageBox::information(
                    this, tr("Sélection"),
                    tr("Vous devez sélectionner une commande"),
                    QMessageBox::Ok);
    } else if (selection.size() > 1) {
        QMessageBox::information(
                    this, tr("Sélection"),
                    tr("Vous devez sélectionner une seule commande"),
                    QMessageBox::Ok);
    } else {
        bool isOrder = m_orderManager->isIndexOfOrder(selection[0]);
        if (!isOrder) {
            QMessageBox::information(
                        this, tr("Sélection"),
                        tr("Vous devez sélectionner la ligne d'une commande"),
                        QMessageBox::Ok);
        } else {
            auto index = selection[0];
            OrderManagerNodeOrder *orderItem
                    = static_cast<OrderManagerNodeOrder *>(index.internalPointer());
            QString channel = index.parent().parent().data().toString();
            QString orderId = orderItem->value().toString();
            auto order = m_orderManager->getOrderNotConst(channel, orderId);
            if (m_dialogRefund != nullptr) {
                m_dialogRefund->deleteLater();
            }
            m_dialogRefund = new DialogCreateRefund(order, m_refundManager, this);
            connect(m_dialogRefund,
                    &DialogCreateRefund::accepted,
                    [this, channel](){
                Refund *refund = m_dialogRefund->createRefund();
                m_refundManager->addRefundAndUniteAll(
                            channel,
                            QSharedPointer<Refund>(refund));
                ChangeNotifier::instance()->emitDataForVatChanged();
            });
            m_dialogRefund->show();
        }
    }
}
//----------------------------------------------------------
void WidgetOrders::updateShippingAddress()
{
    auto selection = ui->treeViewOrders->selectionModel()->selectedRows();
    if (selection.size() == 0) {
        QMessageBox::information(
                    this, tr("Sélection vide"),
                    tr("Vous devez sélectionner au moins une commande ou expédition"),
                    QMessageBox::Ok);
    } else {
        int ret = QMessageBox::Ok;
        if (ui->checkBoxConfirmAddress->isChecked()) {
            ret = QMessageBox::warning(
                        this, tr("Changement de l'adresse d'expédition"),
                        tr("Êtes-vous sûr de vouloir changer l'adresse d'expédition ?\n"),
                        QMessageBox::Ok | QMessageBox::Cancel);
        }
        if (ret == QMessageBox::Ok) {
            while (selection.size() > 0) {
                auto sel = selection.takeLast();
                int index = ui->comboBoxShippingAddress->currentIndex();
                auto address = ShippingAddressesManager::instance()->getAddress(index);
                m_orderManager->updateShippingAddress(sel, address);
                ChangeNotifier::instance()->emitDataForVatChanged();
            }
        }
    }
}
//----------------------------------------------------------
void WidgetOrders::cancelAddressChange()
{
    auto selection = ui->treeViewOrders->selectionModel()->selectedRows();
    if (selection.size() == 0) {
        QMessageBox::information(
                    this, tr("Sélection vide"),
                    tr("Vous devez sélectionner au moins une commande ou expédition"),
                    QMessageBox::Ok);
    } else {
        while (selection.size() > 0) {
            auto sel = selection.takeLast();
            m_orderManager->cancelUpdatedShippingAddress(sel);
        }
    }
}
//----------------------------------------------------------
void WidgetOrders::exportShipments()
{
    QSettings settings;
    QString key = "WidgetOrders__exportCsv";
    QString lastDirPath = settings.value(
                key, QDir().absolutePath()).toString();
    QString filePath = QFileDialog::getSaveFileName(
                this, tr("Choisir un fichier"),
                lastDirPath,
                "CSV (*.csv)");
    if (!filePath.isEmpty()) {
        if (!filePath.toLower().endsWith(".csv")) {
            filePath += ".csv";
        }
        QFileInfo fileInfo(filePath);
        settings.setValue(key, fileInfo.absoluteDir().absolutePath());
        QStringList lines;
        int nCols = m_orderManager->columnCount();
        QStringList elements;
        for (int i=0; i<nCols; ++i) {
            elements << m_orderManager->headerData(i, Qt::Horizontal).toString();
        }
        QString sep = ";";
        lines << elements.join(sep);
        int nRows = m_orderManager->rowCount();
        for (int i = 0; i<nRows; ++i) {
            QModelIndex indexImporter = m_orderManager->index(i, 0, QModelIndex());
            for (int j = 0; j < m_orderManager->rowCount(indexImporter); ++j) {
                QModelIndex indexYear = m_orderManager->index(j, 0, indexImporter);
                for (int k = 0; k < m_orderManager->rowCount(indexYear); ++k) {
                    bool hidden = ui->treeViewOrders->isRowHidden(k, indexYear);
                    if (!hidden) {
                        QModelIndex indexOrder = m_orderManager->index(k, 0, indexYear);
                        int nChildOrder = m_orderManager->rowCount(indexOrder);
                        QModelIndex firstChildOrder = m_orderManager->index(0, 0, indexOrder);
                        QString idOrder = m_orderManager->data(indexOrder).toString();
                        OrderManagerNode *item = static_cast<OrderManagerNode *>(firstChildOrder.internalPointer());
                        if (dynamic_cast<OrderManagerNodeArticle *>(item) == nullptr) {
                            /// Several shipment item
                            for (int kk = 0; kk < nChildOrder; ++kk) {
                                elements.clear();
                                for (int l=0; l<nCols; ++l) {
                                    auto index = m_orderManager->index(kk, l, indexOrder);
                                    elements << m_orderManager->data(index).toString();
                                }
                                elements[0] = idOrder;
                                lines << elements.join(sep);
                            }
                        } else {
                            elements.clear();
                            for (int l=0; l<nCols; ++l) {
                                auto index = m_orderManager->index(k, l, indexYear);
                                elements << m_orderManager->data(index).toString();
                            }
                            lines << elements.join(sep);
                        }
                    }
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
}
//----------------------------------------------------------
