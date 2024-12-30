#include <qmessagebox.h>
#include <QtCore/qglobal.h>


#include "PaneCustomers.h"
#include "ui_PaneCustomers.h"

#include "model/CustomerManager.h"
#include "model/SettingManager.h"
#include "model/orderimporters/ShippingAddressesManager.h"
#include "dialogs/DialogNewCustomer.h"

//----------------------------------------------------------
PaneCustomers::PaneCustomers(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PaneCustomers)
{
    ui->setupUi(this);
    ui->widgetCustomers->hide();
    ui->comboCurrency->addItems(SettingManager::instance()->currencies());
    ui->listViewCustomers->setModel(CustomerManager::instance());
    m_dialogNewCustomer = nullptr;
    _connectSlots();
}
//----------------------------------------------------------
void PaneCustomers::_connectSlots()
{
    connect(ui->buttonAdd,
            &QPushButton::clicked,
            this,
            &PaneCustomers::addCustomer);
    connect(ui->buttonRemove,
            &QPushButton::clicked,
            this,
            &PaneCustomers::removeSelectedCustomer);
    connect(ui->buttonSave,
            &QPushButton::clicked,
            this,
            static_cast<void (PaneCustomers::*)()>(&PaneCustomers::saveSelectedCustomer));
    connect(ui->listViewCustomers->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &PaneCustomers::displayCustomer);
    }
//----------------------------------------------------------
PaneCustomers::~PaneCustomers()
{
    _showCustomerFields();
    delete ui;
}
//----------------------------------------------------------
void PaneCustomers::addCustomer()
{
    if (m_dialogNewCustomer == nullptr) {
        m_dialogNewCustomer = new DialogNewCustomer(this);
        connect(m_dialogNewCustomer,
                &DialogNewCustomer::accepted,
                [this](){
            Customer customer = m_dialogNewCustomer->getCustomer();
            CustomerManager::instance()->addCustomerAvailable(customer);
            Address address = m_dialogNewCustomer->getAddress();
            ShippingAddressesManager::instance()->addAddress(address);
        });
    }
    m_dialogNewCustomer->clear();
    m_dialogNewCustomer->show();
}
//----------------------------------------------------------
void PaneCustomers::addFirstCustomerForcing()
{
    auto dialog = new DialogNewCustomer(this);
    dialog->blockCancel();
    connect(dialog,
            &DialogNewCustomer::accepted,
            [this, dialog](){
        Customer customer = dialog->getCustomer();
        CustomerManager::instance()->addCustomerAvailable(customer);
        Address address = dialog->getAddress();
        ShippingAddressesManager::instance()->addAddress(address);
        delete dialog;
    });
    dialog->setModal(true);
    dialog->show();
}
//----------------------------------------------------------
void PaneCustomers::removeSelectedCustomer()
{
    auto selection = ui->listViewCustomers->selectionModel()->selection();
    if (!selection.isEmpty()) {
        int ret = QMessageBox::warning(
                    this, tr("Suppression d'un client"),
                    tr("Êtes-vous sûr de vouloir supprimer le client ?\n"
                       "Cela supprimera toutes ses données."
                       "Cette action n'est pas réversible."),
                    QMessageBox::Ok | QMessageBox::Cancel);
        if (ret == QMessageBox::Ok) {
            QString customerName = selection.indexes().first().data().toString();
            CustomerManager::instance()->removeCustomer(customerName);
        }
    }
}
//----------------------------------------------------------
void PaneCustomers::saveSelectedCustomer()
{
    auto selection = ui->listViewCustomers->selectionModel()->selection();
    saveSelectedCustomer(selection);
}
//----------------------------------------------------------
void PaneCustomers::saveSelectedCustomer(QItemSelection selection)
{
    if (!selection.isEmpty()) {
        QString customerName = selection.indexes().first().data().toString();
        Customer customer = getCustomerEdition();
        CustomerManager::instance()->editCustomer(customerName, customer);
    }
}
//----------------------------------------------------------
Customer PaneCustomers::getCustomerEdition() const
{
    Customer customer(ui->lineEditCustomerName->text(),
                      ui->plainTextEditNotes->toPlainText(),
                      ui->lineEditSiret->text(),
                      ui->lineEditPhonNumber->text(),
                      ui->lineEditEmail->text(),
                      ui->comboCurrency->currentText());
    return customer;
}
//----------------------------------------------------------
void PaneCustomers::displayCustomer(QItemSelection newSelection,
                                    QItemSelection previousSelection)
{
    if (newSelection.isEmpty()) {
        _hideCustomerFields();
    } else {
        if (!previousSelection.isEmpty()) {
            Customer editedCustomer = getCustomerEdition();
            QString previousCustomerName = previousSelection.indexes().first().data().toString();
            Customer existingCustomer = CustomerManager::instance()->getCustomer(
                        previousCustomerName);
            if (editedCustomer != existingCustomer) {
                int ret = QMessageBox::warning(this, tr("Sauvegarder les modifications"),
                                               tr("Avant de changer de client, souhaitez-vous\n"
                                                  "sauvegarder les données saisies\n"
                                                  "pour le client ") + previousCustomerName + tr(" ?"),
                                               QMessageBox::Ok | QMessageBox::No);
                if (ret == QMessageBox::Ok) {
                    saveSelectedCustomer(previousSelection);
                }
            }
        }
        QString newSelectedCustomerName = newSelection.indexes().first().data().toString();
        Customer customer = CustomerManager::instance()->getCustomer(newSelectedCustomerName);
        _showCustomerFields();
        ui->lineEditCustomerName->setText(customer.name());
        ui->lineEditEmail->setText(customer.email());
        ui->lineEditSiret->setText(customer.siret());
        ui->lineEditPhonNumber->setText(customer.phoneNumber());
        ui->plainTextEditNotes->setPlainText(customer.notes());
    }
}
//----------------------------------------------------------
void PaneCustomers::_showCustomerFields()
{
    if (!ui->widgetCustomers->isVisible()) {
        ui->widgetCustomers->show();
        ui->verticalLayoutMain->removeItem(ui->verticalSpacerButtons);
    }
}
//----------------------------------------------------------
void PaneCustomers::_hideCustomerFields()
{
    ui->widgetCustomers->hide();
    ui->verticalLayoutMain->addItem(ui->verticalSpacerButtons);
}
//----------------------------------------------------------
