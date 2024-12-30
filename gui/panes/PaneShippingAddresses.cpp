#include <qmessagebox.h>
#include "PaneShippingAddresses.h"
#include "ui_PaneShippingAddresses.h"
#include "model/orderimporters/Address.h"
#include "model/orderimporters/ShippingAddressesManager.h"
#include "model/CustomerManager.h"
#include "dialogs/DialogAddShippingAddress.h"

//----------------------------------------------------------
PaneShippingAddresses::PaneShippingAddresses(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PaneShippingAddresses)
{
    ui->setupUi(this);
    m_dialogAddShippingAddress = nullptr;
    ui->listViewAddresses->setModel(ShippingAddressesManager::instance());
    _hideAddressFields();
    _connectSlots();
}
//----------------------------------------------------------
PaneShippingAddresses::~PaneShippingAddresses()
{
    delete ui;
}
//----------------------------------------------------------
void PaneShippingAddresses::addAddress()
{
    if (m_dialogAddShippingAddress == nullptr) {
        m_dialogAddShippingAddress = new DialogAddShippingAddress(this);
        connect(m_dialogAddShippingAddress,
                &DialogAddShippingAddress::accepted,
                [this](){
            Address address = m_dialogAddShippingAddress->getAddress();
            ShippingAddressesManager::instance()->addAddress(address);
        });
    }
    m_dialogAddShippingAddress->clear();
    m_dialogAddShippingAddress->show();
}
//----------------------------------------------------------
void PaneShippingAddresses::removeAddress()
{
    int ret = QMessageBox::warning(
                this, tr("Suppression de l'adresse sélectionné"),
                tr("Êtes-vous sûr de vouloir supprimer l'adresse séléctionnée ?\n"
                   "Cette action n'est pas réversible.\n"
                   "Si cette adresse était utilisée, elle sera remplacée par celle par défaut."),
                QMessageBox::Ok | QMessageBox::Cancel);
    if (ret == QMessageBox::Ok) {
        auto indexes = ui->listViewAddresses->selectionModel()->selectedRows();
        std::sort(indexes.begin(), indexes.end());
        for (auto it = indexes.rbegin(); it != indexes.rend(); ++it) {
            ShippingAddressesManager::instance()->removeAddress(it->row());
        }
    }
}
//----------------------------------------------------------
void PaneShippingAddresses::saveAddressInPosition(int index) const
{
    Address address = ui->widgetAddress->getAddress();
    ShippingAddressesManager::instance()->updateAddress(index, address);
}
//----------------------------------------------------------
void PaneShippingAddresses::saveAddress() const
{
    auto indexes = ui->listViewAddresses->selectionModel()->selectedRows();
    if (indexes.size() > 0) {
        saveAddressInPosition(indexes.first().row());
    }
}
//----------------------------------------------------------
void PaneShippingAddresses::displayAddress(
        QItemSelection newSelection, QItemSelection previousSelection)
{
    if (newSelection.isEmpty()) {
        _hideAddressFields();
    } else {
        if (!previousSelection.isEmpty()) {
            Address editedAddress = ui->widgetAddress->getAddress();
            int indexPrevious = previousSelection.indexes().first().row();
            Address previousAddress
                    = ShippingAddressesManager::instance()->getAddress(indexPrevious);
            if (editedAddress != previousAddress) {
                int ret = QMessageBox::warning(this, tr("Sauvegarder les modifications"),
                                               tr("Avant de changer d'adresse, souhaitez-vous\n"
                                                  "sauvegarder l'addresse ?"),
                                               QMessageBox::Ok | QMessageBox::No);
                if (ret == QMessageBox::Ok) {
                    saveAddressInPosition(indexPrevious);
                }
            }
        }
        int indexNew = newSelection.indexes().first().row();
        Address address = ShippingAddressesManager::instance()->getAddress(indexNew);
        ui->widgetAddress->setAddress(address);
        _showAddressFields();
    }

}
//----------------------------------------------------------
void PaneShippingAddresses::_connectSlots()
{
    connect(ui->buttonAdd,
            &QPushButton::clicked,
            this,
            &PaneShippingAddresses::addAddress);
    connect(ui->buttonRemove,
            &QPushButton::clicked,
            this,
            &PaneShippingAddresses::removeAddress);
    connect(ui->buttonSave,
            &QPushButton::clicked,
            this,
            &PaneShippingAddresses::saveAddress);
    connect(ui->listViewAddresses->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &PaneShippingAddresses::displayAddress);
}
//----------------------------------------------------------
void PaneShippingAddresses::_showAddressFields()
{
    ui->widgetAddress->show();
}
//----------------------------------------------------------
void PaneShippingAddresses::_hideAddressFields()
{
    ui->widgetAddress->hide();
}
//----------------------------------------------------------

