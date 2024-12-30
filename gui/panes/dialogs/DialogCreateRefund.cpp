#include <qmessagebox.h>

#include "DialogCreateRefund.h"
#include "ui_DialogCreateRefund.h"
#include "model/orderimporters/Order.h"
#include "model/orderimporters/RefundManager.h"
#include "model/orderimporters/Refund.h"

//----------------------------------------------------------
DialogCreateRefund::DialogCreateRefund(
        Order *order,
        RefundManager *refundManager,
        QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogCreateRefund)
{
    ui->setupUi(this);
    m_order = order;
    m_refundManager = refundManager;
    ui->checkBoxRefundAll->setChecked(true);
    ui->tableWidgetArticles->setEnabled(false);
    ui->lineEditId->setText(m_order->getId() + "-refund");
    ui->dateTimeEdit->setDateTime(m_order->getDateTime().addDays(1));
    ui->spindBoxExtraTaxed->setValue(0.);
    ui->spindBoxExtraTaxed->setEnabled(false);
    connect(ui->checkBoxRefundAll,
            &QCheckBox::clicked,
            ui->tableWidgetArticles,
            &QTableWidget::setDisabled);
    connect(ui->checkBoxRefundAll,
            &QCheckBox::clicked,
            ui->spindBoxExtraTaxed,
            &QTableWidget::setDisabled);
    auto articles = m_order->articlesSold();
    int i=0;
    auto shipments = m_order->getShipments();
    for (auto shipment : qAsConst(shipments)) {
        ui->tableWidgetArticles->setItem(
                    i, 0, new QTableWidgetItem(shipment->getId()));
        auto articlesShipped = shipment->getArticlesShipped();
        for (auto itArt = articlesShipped.begin(); itArt != articlesShipped.end(); ++itArt) {
            ui->tableWidgetArticles->setRowCount(i+1);
            auto article = itArt.value();
            QString articleId = itArt.key();
            QString articleName = article->getName();
            double priceTaxed = article->getTotalPriceTaxed();
            ui->tableWidgetArticles->setItem(
                        i, 1, new QTableWidgetItem(articleId));
            ui->tableWidgetArticles->setItem(
                        i, 2, new QTableWidgetItem(articleName));
            auto itemPrice = new QTableWidgetItem();
            itemPrice->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
            itemPrice->setData(Qt::DisplayRole, priceTaxed);
            ui->tableWidgetArticles->setItem(
                        i, 3, itemPrice);
            auto itemShippingPrice = new QTableWidgetItem();
            itemShippingPrice->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
            double shippingPriceTaxed = article->getShipping().totalPriceTaxed();
            itemShippingPrice->setData(Qt::DisplayRole, shippingPriceTaxed);
            ui->tableWidgetArticles->setItem(
                        i, 4, itemShippingPrice);
            ++i;
        }
    }
    /*
    ui->tableWidgetArticles->setItem(
                i, 1, new QTableWidgetItem(tr("Shipping or discount")));
    auto itemPrice = new QTableWidgetItem();
    itemPrice->setData(Qt::DisplayRole, shipping.totalPriceTaxed());
    itemPrice->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
    ui->tableWidgetArticles->setItem(
                i, 2, itemPrice);
                //*/
}
//----------------------------------------------------------
DialogCreateRefund::~DialogCreateRefund()
{
    delete ui;
}
//----------------------------------------------------------
Refund *DialogCreateRefund::createRefund()
{
    Refund *refund = nullptr;
    QString refundId = ui->lineEditId->text().trimmed();
    bool refundAll = ui->checkBoxRefundAll->isChecked();
    QHash<QString, QSharedPointer<ArticleSold>> articles; //TODO
    Shipping shipping = m_order->getShipping();
    //bool isFromMarketplace = false;
    double sumShippingTaxed = shipping.totalPriceTaxed();
    double sumShippingTaxes = shipping.totalTaxes();
    if (refundAll) {
        auto shipments = m_order->getShipments();
        double taxesNotRounded = 0;;
        for (auto shipment : qAsConst(shipments)) {
            auto articlesShipped = shipment->getArticlesShipped();
            for (auto it=articlesShipped.begin(); it!=articlesShipped.end();++it) {
                QSharedPointer<ArticleSold> articleSold(
                            ArticleSold::fromString(
                                it.value()->toString()));
                articleSold->reversePrice();
                articles.insert(it.key(), articleSold);
            }
            sumShippingTaxed += shipment->getShipping().totalPriceTaxed();
            sumShippingTaxes += shipment->getShipping().totalTaxes();
            taxesNotRounded += shipment->getTotalPriceTaxes();
        }
        Shipping finalShipping(-sumShippingTaxed,
                               -sumShippingTaxes,
                               m_order->getCurrency());
        refund = new Refund(
                    refundId,
                    m_order->getId(),
                    articles,
                    finalShipping,
                    ui->dateTimeEdit->dateTime(),
                    Address(),
                    m_order->getCurrency(),
                    taxesNotRounded);
    } else {
        //double refundAmount = ui->spindBoxAmountTaxed->value();
        auto shipments = m_order->getShipments();
        double taxesNotRounded = 0;
        int i=0;
        for (auto shipment : qAsConst(shipments)) {
            auto articlesShipped = shipment->getArticlesShipped();
            for (auto it=articlesShipped.begin(); it!=articlesShipped.end();++it) {
                QSharedPointer<ArticleSold> articleSold(
                            ArticleSold::fromString(
                                it.value()->toString()));
                articleSold->reversePrice();
                double currentPriceTaxed = articleSold->getTotalPriceTaxed();
                double currentPriceTaxes = articleSold->getTotalPriceTaxes();
                auto articleShipping = articleSold->getShipping();
                double currentPriceTaxedShipping = articleShipping.totalPriceTaxed();
                double currentPriceTaxesShipping = articleShipping.totalTaxes();
                double refundUserTaxed = ui->tableWidgetArticles->item(i, 3)
                        ->data(Qt::DisplayRole).toDouble();
                if (qAbs(qAbs(refundUserTaxed) - qAbs(currentPriceTaxed)) > 0.001) {
                    double ratioAmount = qAbs(refundUserTaxed) / qAbs(currentPriceTaxed);
                    currentPriceTaxed *= ratioAmount;
                    currentPriceTaxes *= ratioAmount;
                    articleSold->setTotalPriceTaxed(currentPriceTaxed);
                    articleSold->setTotalPriceTaxes(currentPriceTaxes);
                }
                double refundUserShippingTaxed = ui->tableWidgetArticles->item(i, 4)
                        ->data(Qt::DisplayRole).toDouble();
                if (qAbs(qAbs(refundUserShippingTaxed) - qAbs(currentPriceTaxedShipping)) > 0.001) {
                    double ratioAmountShipping = qAbs(refundUserShippingTaxed) / qAbs(currentPriceTaxedShipping);
                    currentPriceTaxedShipping *= ratioAmountShipping;
                    currentPriceTaxesShipping *= ratioAmountShipping;
                    auto shippingArticle = articleSold->getShipping();
                    shippingArticle.setTotalPriceTaxed(currentPriceTaxedShipping);
                    shippingArticle.setTotalTaxes(currentPriceTaxesShipping);
                    articleSold->setShipping(shippingArticle);
                }
                articles.insert(it.key(), articleSold);
                ++i;
            }
        }
        double ajustAmount = ui->spindBoxExtraTaxed->value();
        Shipping finalShipping(-ajustAmount,
                               0.,
                               m_order->getCurrency());
        refund = new Refund(
                    refundId,
                    m_order->getId(),
                    articles,
                    finalShipping,
                    ui->dateTimeEdit->dateTime(),
                    Address(),
                    m_order->getCurrency(),
                    taxesNotRounded);
    }
    refund->init(m_order);
    refund->setChannel(m_order->getChannel());
    refund->setSubchannel(m_order->getSubchannel());
    return refund;
    //ui->spindBoxAmountTaxed->value(),
}
//----------------------------------------------------------
void DialogCreateRefund::clear()
{
    ui->dateTimeEdit->setDateTime(QDateTime(QDate(2000,1,1)));
    ui->lineEditId->clear();
    ui->spindBoxExtraTaxed->setValue(0.);
    //ui->spindBoxTaxes->setValue(0.);
}
//----------------------------------------------------------
void DialogCreateRefund::accept()
{
    if (ui->dateTimeEdit->dateTime() == QDateTime(QDate(2000,1,1))) {
        QMessageBox::warning(this,
                             tr("Erreur"),
                             tr("Il faut sélectionner une date."));
    } else if (ui->lineEditId->text().trimmed().isEmpty()) {
        QMessageBox::warning(this,
                             tr("Erreur"),
                             tr("Il faut saisir un identifiant unique."));
        /*
    } else if (qAbs(ui->spindBoxAmountTaxed->value()) < 0.001) {
        QMessageBox::warning(this,
                             tr("Erreur"),
                             tr("Il faut saisir un montant non nul."));
    } else if (ui->spindBoxAmountTaxed->value()-m_order->getTotalPriceTaxed() > 0.001) {
        QMessageBox::warning(this,
                             tr("Erreur"),
                             tr("Il faut saisir un montant inférieur au total de la commande."));
    } else if (!m_refundManager->canRefund(
                   m_order, ui->spindBoxAmountTaxed->value())) {
        QMessageBox::warning(this,
                             tr("Erreur"),
                             tr("En additionnant ce montant et celui des autres remboursements, le total dépasse celui de la commande."));
                             //*/

    } else if (m_refundManager->contains(
                   m_order->getChannel(), ui->lineEditId->text().trimmed())) {
        QMessageBox::warning(this,
                             tr("Erreur"),
                             tr("L'identifiant du remboursement est déjà utilisé."));
    } else {
        QDialog::accept();
    }
}
//----------------------------------------------------------
