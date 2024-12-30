#include <qmessagebox.h>
#include <QTimer>

#include "OrderImporterWidget.h"
#include "ui_OrderImporterWidget.h"
#include "OrderReportsWidget.h"
#include "model/orderimporters/ShippingAddressesManager.h"
#include "model/CustomerManager.h"
#include "gui/panes/dialogs/DialogCreateRefund.h"

//----------------------------------------------------------
OrderImporterWidget::OrderImporterWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OrderImporterWidget)
{
    ui->setupUi(this);
    //ui->treeViewOrders->setModel(OrderManager::instance());
    //ui->treeViewOrders->header()->resizeSection(0, 200);
    //ui->comboBoxShippingAddress->setModel(ShippingAddressesManager::instance());
    m_dialogRefund = nullptr;
    m_changingCustomer = false;
    _connectSlots();
}
//----------------------------------------------------------
OrderImporterWidget::~OrderImporterWidget()
{
    /*
    auto currentModel = ui->treeViewOrders->model();
    if (currentModel != m_orderManager
            && currentModel != nullptr) {
        currentModel->deleteLater();
    }
    //*/
    delete ui;
}
//----------------------------------------------------------
void OrderImporterWidget::_connectSlots()
{
    /*
    connect(CustomerManager::instance(),
            &CustomerManager::selectedCustomerChanged,
            [this](const QString &){
        loadImportersSettings();
    });
    connect(ui->buttonFilter,
            &QPushButton::clicked,
            this,
            &OrderImporterWidget::filter);
    connect(ui->buttonResetFilter,
            &QPushButton::clicked,
            this,
            &OrderImporterWidget::resetFilter);
    connect(ui->buttonExpandAllOrders,
            &QPushButton::clicked,
            this,
            &OrderImporterWidget::expandAllOrders);
    connect(ui->buttonAddRefund,
            &QPushButton::clicked,
            this,
            &OrderImporterWidget::addRefund);
    connect(ui->buttoncancelAddresChange,
            &QPushButton::clicked,
            this,
            &OrderImporterWidget::cancelAddressChange);
    connect(ui->buttonUpdateShippingAddress,
            &QPushButton::clicked,
            this,
            &OrderImporterWidget::updateShippingAddress);
    connect(ui->buttonRemoveRefund,
            &QPushButton::clicked,
            this,
            &OrderImporterWidget::removeSelectedRefunds);
            //*/
}
//----------------------------------------------------------
void OrderImporterWidget::init(AbstractOrderImporter *importer)
{
    m_importer = importer;
    for (auto reportType : importer->reportTypes()) {
        auto reportWidget = new OrderReportsWidget(ui->stackedWidgetReportType);
        ui->listWidgetReportType->addItem(reportType.shortName);
        ui->stackedWidgetReportType->addWidget(reportWidget);
            //ui->treeViewOrders->setModel(m_orderManager);
            //ui->treeViewOrders->header()->resizeSection(0, 200);
            //ui->treeViewRefunds->setModel(m_refundManager);
        reportWidget->init(reportType, importer);
        if (ui->listWidgetReportType->selectedItems().size() == 0) {
            ui->listWidgetReportType->setCurrentRow(0);
        }
    }
    loadImportersSettings();
    /*
    connect(ui->comboShippingAddressDefault,
            static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            [importer](int index) {
        Address address;
        if (index >=0) {
            address = ShippingAddressesManager::instance()
                ->getAddress(index);
        }
        importer->setDefaultShippingAddress(address);
    });
    //*/
    connect(ui->checkBoxRecomputeVat,
                             &QCheckBox::clicked,
                             [this](bool checked) {
        if (!m_changingCustomer) {
            m_importer->setVatToRecompute(checked);
        }
    });
    connect(ui->comboShippingAddressDefault,
                             static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                             [this](int index) {
        if (!m_changingCustomer) {
            Address address;
            if (index >=0) {
                address = ShippingAddressesManager::instance()
                        ->getAddress(index);
            }
            m_importer->setDefaultShippingAddress(address);
        }
    });
}
//----------------------------------------------------------
void OrderImporterWidget::onCustomerSelectedChanged(const QString &customerId)
{
    m_changingCustomer = true;
    bool isVatToRecompute = m_importer->isVatToRecompute();
    ui->checkBoxRecomputeVat->setChecked(isVatToRecompute);
    ui->comboShippingAddressDefault->setModel(ShippingAddressesManager::instance());
    QString defaultShippingAddressId = m_importer->getDefaultShippingAddressId();
    if (!defaultShippingAddressId.isEmpty()) {
        Address address = ShippingAddressesManager::instance()
                ->getAddress(defaultShippingAddressId);
        ui->comboShippingAddressDefault->setCurrentText(address.label());
    } else if (ShippingAddressesManager::instance()->rowCount() > 0){
        Address address = ShippingAddressesManager::instance()
                ->getAddress(0);
        //m_importer->setDefaultShippingAddress(address);
        ui->comboShippingAddressDefault->setCurrentText(address.label());
    }
    QTimer::singleShot(2000, [this](){
        m_changingCustomer = false;
    });
}
//----------------------------------------------------------
QString OrderImporterWidget::uniqueId() const
{
    return "OrderImporterWidget";
}
//----------------------------------------------------------
void OrderImporterWidget::loadImportersSettings()
{
    QString customerId = CustomerManager::instance()->getSelectedCustomerId();
    onCustomerSelectedChanged(customerId);
}
/*
//----------------------------------------------------------
bool OrderImporterWidget::filterHideOrder(const Order *order) const
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
void OrderImporterWidget::filter()
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
void OrderImporterWidget::resetFilter()
{
    auto currentModel = ui->treeViewOrders->model();
    ui->treeViewOrders->setModel(m_orderManager);
    if (currentModel != m_orderManager && currentModel != nullptr) {
        currentModel->deleteLater();
    }
    //ui->treeViewOrders->expandAll();
}
//----------------------------------------------------------
void OrderImporterWidget::expandAllOrders()
{
    ui->treeViewOrders->expandAll();
}
//----------------------------------------------------------
void OrderImporterWidget::addRefund()
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
            auto order = m_orderManager->getOrder(channel, orderId);
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
            });
            m_dialogRefund->show();
        }
    }
}
//----------------------------------------------------------
void OrderImporterWidget::updateShippingAddress()
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
            }
        }
    }
}
//----------------------------------------------------------
void OrderImporterWidget::cancelAddressChange()
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
void OrderImporterWidget::removeSelectedRefunds()
{
    auto selection = ui->treeViewRefunds->selectionModel()->selectedRows();
    if (selection.size() > 0) {
        try {
            int ret = QMessageBox::warning(
                        this, tr("Suppression des remboursements"),
                        tr("Êtes-vous sûr de vouloir supprimer les remboursements sélectionnés ?") + "\n"
                        + tr("Cette action n'est pas réversible."),
                        QMessageBox::Ok | QMessageBox::Cancel);
            if (ret == QMessageBox::Ok) {
                while (selection.size() > 0) {
                    auto sel = selection.takeFirst();
                    m_refundManager->removeRefund(sel);
                }
            }
        } catch (const RefundIdException &e) {
            QMessageBox::critical(
                        this, tr("Erreur"),
                        e.error() + "\n" + tr("La suppression a été annulée car il est uniquement possible de supprimer les remboursements manuels"),
                        QMessageBox::Ok);
        }
    }
}
    //*/
