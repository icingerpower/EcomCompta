#ifndef VATRATEMANAGER_H
#define VATRATEMANAGER_H

#include <QtCore/qstring.h>
#include <QtCore/qsharedpointer.h>
#include <QtCore/qmap.h>
#include <QtCore/qabstractitemmodel.h>


class VatRatesModel;
class VatRatesModelDates;
class SelectedSkusListModel;

class VatRateManager : public QAbstractListModel
{
    Q_OBJECT
public:
    static VatRateManager *instance();
    double vatRateDefault(const QString &countryCode, const QDate &date) const;
    double vatRate(const QString &countryCode, const QDate &date, const QString &sku) const;
    VatRatesModel *getDefautVatModel() const;
    VatRatesModelDates *getDefautVatModelDates() const;
    QSharedPointer<VatRatesModel> getOtherVatModel(
            const QString &name) const;
    QSharedPointer<VatRatesModelDates> getOtherVatModelDate(
            const QString &name) const;
    QSharedPointer<SelectedSkusListModel> getSelectedSkusModel(
            const QString &name) const;
    bool containsVatModel(const QString &name) const;
    void addOtherRates(const QString &name);
    void remove(const QString &name);
    void remove(int index);
    void loadFromSettings();
    void saveInSettings() const;
    void clear();

    int	rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant headerData(
            int section,
            Qt::Orientation orientation,
            int role) const override;
    QVariant data(const QModelIndex &index,
                  int role) const override;
    bool setData(
            const QModelIndex &index,
            const QVariant &value,
            int role = Qt::EditRole) override;

public slots:
    void onCustomerSelectedChanged(const QString &customerId);

private:
    VatRateManager(QObject *object = nullptr);
    ~VatRateManager() override;
    VatRatesModel *m_defautVatModel;
    VatRatesModelDates *m_defautVatModelDates;
    QMap<QString, QSharedPointer<VatRatesModel>> m_otherVatModels;
    QMap<QString, QSharedPointer<VatRatesModelDates>> m_otherVatModelsDate;
    QMap<QString, QSharedPointer<SelectedSkusListModel>> m_otherSectedSkusModels;
    QString m_settingKey;
};

#endif // VATRATEMANAGER_H
