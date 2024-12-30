#include <qmessagebox.h>

#include "DialogOtherVatRates.h"
#include "ui_DialogOtherVatRates.h"

#include "model/vat/VatRateManager.h"

//----------------------------------------------------------
DialogOtherVatRates::DialogOtherVatRates(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogOtherVatRates)
{
    ui->setupUi(this);
}
//----------------------------------------------------------
DialogOtherVatRates::~DialogOtherVatRates()
{
    delete ui;
}
//----------------------------------------------------------
QString DialogOtherVatRates::getName() const
{
    return ui->lineEditName->text().trimmed();
}
//----------------------------------------------------------
void DialogOtherVatRates::clear()
{
    ui->lineEditName->clear();
}
//----------------------------------------------------------
void DialogOtherVatRates::accept()
{
    QString name = ui->lineEditName->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(
                    this,
                    tr("Erreur"),
                    tr("Un nom doit être saisi."));
    } else if (VatRateManager::instance()->containsVatModel(name)) {
        QMessageBox::warning(
                    this,
                    tr("Erreur"),
                    tr("Le nom est déjà utilisé."));
    } else {
        QDialog::accept();
    }
}
//----------------------------------------------------------
