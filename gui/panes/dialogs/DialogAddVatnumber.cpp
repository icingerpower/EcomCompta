#include <qmessagebox.h>
#include <QtCore/qregexp.h>

#include "model/SettingManager.h"
#include "DialogAddVatnumber.h"
#include "ui_DialogAddVatnumber.h"

//----------------------------------------------------------
DialogAddVatnumber::DialogAddVatnumber(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogAddVatnumber)
{
    ui->setupUi(this);
}
//----------------------------------------------------------
DialogAddVatnumber::~DialogAddVatnumber()
{
    delete ui;
}
//----------------------------------------------------------
VatNumberData DialogAddVatnumber::getVatNumber() const
{
    VatNumberData vatNumberData;
    vatNumberData.number = ui->lineEditVatNumber->text().trimmed();
    vatNumberData.dateRegistration = ui->dateEditRegistration->date();
    return vatNumberData;
}
//----------------------------------------------------------
void DialogAddVatnumber::clear()
{
    ui->dateEditRegistration->setDate(QDate(1800, 1, 1));
    ui->lineEditVatNumber->clear();
}
//----------------------------------------------------------
void DialogAddVatnumber::accept()
{
    QString vatNumber = ui->lineEditVatNumber->text().trimmed();
    QRegularExpression reg("^\\D\\D\\w+$"); // TODO starts only with 2 letters
    if (vatNumber.size() < 5) {
        QMessageBox::warning(this,
                             tr("Erreur"),
                             tr("Il faut saisir le numéro de TVA"));
    } else if (ui->dateEditRegistration->date() == QDate(1800, 1, 1)) {
        QMessageBox::warning(this,
                             tr("Erreur"),
                             tr("Il faut saisir la date d'enregistrement"));
    } else if (!reg.match(vatNumber).hasMatch()) {
        QMessageBox::warning(this,
                             tr("Erreur"),
                             tr("Le numéro de TVA doit commencé par 2 lettres en majuscule suivies de chiffres"));
    } else if (!SettingManager::countriesUEfrom2020()->contains(vatNumber.left(2).toUpper())
               && vatNumber.left(2).toUpper() != "IM"
               && vatNumber.left(2).toUpper() != "EU") {
        QMessageBox::warning(this,
                             tr("Erreur"),
                             tr("Le numéro de TVA doit commencé par un code pays de l'UE ou bien IM pour le numéro IOSS"));
    } else {
        QDialog::accept();
    }
}
//----------------------------------------------------------

