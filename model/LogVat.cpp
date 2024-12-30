#include <QTextStream>

#include "orderimporters/Shipment.h"
#include "orderimporters/Order.h"

#include "LogVat.h"

//----------------------------------------------------------
LogVat *LogVat::instance()
{
    static LogVat instance;
    return &instance;
}
//----------------------------------------------------------
void LogVat::recordAmount(
        const QString &typeComputing,
        const Shipment *shipmentOrRefund)
{
    recordAmount(typeComputing,
                 shipmentOrRefund,
                 shipmentOrRefund->getTotalPriceUntaxedConverted(),
                 shipmentOrRefund->getTotalPriceTaxesConverted());
}
//----------------------------------------------------------
void LogVat::recordAmount(const QString &typeComputing,
                          const Shipment *shipmentOrRefund,
                          double amountUntaxed,
                          double amountTaxed)
{
    const auto &dateTime = shipmentOrRefund->getDateTime();
    const auto &year = dateTime.date().year();
    const auto &vatRegime = shipmentOrRefund->getRegimeVat();
    const auto &vatCountry = shipmentOrRefund->getCountryNameVat();
    const auto &orderId = shipmentOrRefund->getOrder()->getId();
    const auto &shipmentId = shipmentOrRefund->getId();
    auto pricesByVat = shipmentOrRefund->getTotalPriceTaxesByVatRateConverted();
    for (auto itSaleType = pricesByVat.begin();
         itSaleType != pricesByVat.end(); ++itSaleType) {
        const auto &saleType = itSaleType.key();
        for (auto it = itSaleType.value().begin();
             it != itSaleType.value().end(); ++it) {
            const auto &vatRate = it.key();
            const auto &untaxed = it.value().untaxed;
            const auto &taxes = it.value().taxes;
            QPair<double, double> pair{untaxed, taxes};
            m_amounts[typeComputing][year][dateTime][vatRegime][vatCountry][saleType][vatRate][orderId][shipmentId] = pair;
        }
    }
}
//----------------------------------------------------------
void LogVat::saveLog(
        const QString &typeComputing,
        const QString &fileName,
        int year,
        int monthFrom,
        int monthTo)
{
    QString filePath = SettingManager::instance()->workingDirectory().filePath(fileName);
    QFile file(filePath);
    if (file.open(QFile::WriteOnly)) {
        QTextStream stream(&file);
        const auto & amountsYear = m_amounts[typeComputing][year];
        QStringList lines;
        for (auto itDate = amountsYear.begin();
             itDate != amountsYear.end(); ++itDate) {
            const auto &date = itDate.key().date();
            if (date.month() >= monthFrom && date.month() <= monthTo) {
                for (auto itVatRegime = itDate.value().begin();
                     itVatRegime != itDate.value().end(); ++itVatRegime) {
                    const QString &vatRegime = itVatRegime.key();
                    for (auto itVatCountry = itVatRegime.value().begin();
                         itVatCountry != itVatRegime.value().end(); ++itVatCountry) {
                        const QString &vatCountry = itVatCountry.key();
                        for (auto itSaleType = itVatCountry.value().begin();
                             itSaleType != itVatCountry.value().end(); ++itSaleType) {
                            const QString &saleType = itSaleType.key();
                            for (auto itVatRate = itSaleType.value().begin();
                                 itVatRate != itSaleType.value().end(); ++itVatRate) {
                                const QString &vatRate = itVatRate.key();
                                for (auto itOrder = itVatRate.value().begin();
                                     itOrder != itVatRate.value().end(); ++itOrder) {
                                    const QString &orderId = itOrder.key();
                                    for (auto itShipmentRefund = itOrder.value().begin();
                                         itShipmentRefund != itOrder.value().end(); ++itShipmentRefund) {
                                        const QString &shipmentId = itShipmentRefund.key();
                                        double untaxed = itShipmentRefund.value().first;
                                        double taxes = itShipmentRefund.value().second;
                                        QStringList elements{
                                            vatRegime,
                                                    vatCountry,
                                                    saleType,
                                                    vatRate,
                                                    orderId,
                                                    shipmentId,
                                                    QString::number(untaxed),
                                                    QString::number(taxes)
                                        };
                                        lines << elements.join("\t");
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        stream << lines.join("\n");
        file.close();
    }
}
//----------------------------------------------------------

