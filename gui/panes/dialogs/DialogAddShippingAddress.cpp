#include <qmessagebox.h>

#include "DialogAddShippingAddress.h"
#include "ui_DialogAddShippingAddress.h"

//----------------------------------------------------------
DialogAddShippingAddress::DialogAddShippingAddress(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogAddShippingAddress)
{
    ui->setupUi(this);
    m_accepted = false;
}
//----------------------------------------------------------
DialogAddShippingAddress::~DialogAddShippingAddress()
{
    delete ui;
}
//----------------------------------------------------------
Address DialogAddShippingAddress::getAddress()
{
    Address address;
    address = ui->widgeAddress->getAddress();
    return address;
}
//----------------------------------------------------------
void DialogAddShippingAddress::clear()
{
    ui->widgeAddress->clear();
}
//----------------------------------------------------------
void DialogAddShippingAddress::accept()
{
    Address address = getAddress();
    if (address.label().isEmpty()) {
        QMessageBox::warning(this,
                             tr("Erreur"),
                             tr("Il faut saisir un label."));
    } else if (address.fullName().isEmpty()) {
        QMessageBox::warning(this,
                             tr("Erreur"),
                             tr("Il faut saisir un nom."));
    } else if (address.postalCode().isEmpty()) {
        QMessageBox::warning(this,
                             tr("Erreur"),
                             tr("Il faut saisir un code postal."));
    } else if (address.city().isEmpty()) {
        QMessageBox::warning(this,
                             tr("Erreur"),
                             tr("Il faut saisir une ville."));
    } else if (address.street1().isEmpty()) {
        QMessageBox::warning(this,
                             tr("Erreur"),
                             tr("Il faut saisir une rue."));
    } else if (address.countryCode().isEmpty()) {
        QMessageBox::warning(this,
                             tr("Erreur"),
                             tr("Il faut sélectionner un pays."));
    } else {
        m_accepted = true;
        QDialog::accept();
    }
}
//----------------------------------------------------------
bool DialogAddShippingAddress::wasAccepted() const
{
    return m_accepted;
}
//----------------------------------------------------------
