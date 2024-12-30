#include <qmessagebox.h>
#include "PaneFbaCenters.h"
#include "ui_PaneFbaCenters.h"

#include "model/vat/AmazonFulfillmentAddressModel.h"
#include "dialogs/DialogFbaAddress.h"
#include "itemdelegates/CountryItemDelegate.h"

//----------------------------------------------------------
PaneFbaCenters::PaneFbaCenters(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PaneFbaCenters)
{
    ui->setupUi(this);
    ui->treeViewCenters->setModel(AmazonFulfillmentAddressModel::instance());
    ui->treeViewCenters->setItemDelegate(
                new CountryItemDelegate(ui->treeViewCenters));
    ui->treeViewCenters->header()->resizeSection(0, 180);
    ui->treeViewCenters->expandAll();
    m_dialogFbaAddress = nullptr;
    _connectSlots();
}
//----------------------------------------------------------
PaneFbaCenters::~PaneFbaCenters()
{
    delete ui;
}
//----------------------------------------------------------
void PaneFbaCenters::_connectSlots()
{
    connect(ui->buttonAddCenter,
            &QPushButton::clicked,
            this,
            &PaneFbaCenters::addCenter);
    connect(ui->buttonRemoveSelCenter,
            &QPushButton::clicked,
            this,
            &PaneFbaCenters::removeSelectedCenters);
}
//----------------------------------------------------------
void PaneFbaCenters::addCenter()
{
    if (m_dialogFbaAddress == nullptr) {
        m_dialogFbaAddress = new DialogFbaAddress(this);
        connect(m_dialogFbaAddress,
                &DialogFbaAddress::accepted,
                [this](){
            Address address = m_dialogFbaAddress->getAddress();
            AmazonFulfillmentAddressModel::instance()
                    ->add(address);
        });
    }
    m_dialogFbaAddress->clear();
    m_dialogFbaAddress->show();
}
//----------------------------------------------------------
void PaneFbaCenters::removeSelectedCenters()
{
    int ret = QMessageBox::warning(
                this, tr("Suppression des centres FBA sélectionnés"),
                tr("Êtes-vous sûr de vouloir supprimer le(s) centre(s) FBA sélectionné(s) ?\n"
                   "Cette action n'est pas réversible."),
                QMessageBox::Ok | QMessageBox::Cancel);
    if (ret == QMessageBox::Ok) {
        auto indexes = ui->treeViewCenters->selectionModel()->selectedRows();
        std::sort(indexes.begin(), indexes.end());
        for (auto it = indexes.rbegin(); it != indexes.rend(); ++it) {
            AmazonFulfillmentAddressModel::instance()->remove(it->row());
        }
    }
}
//----------------------------------------------------------
