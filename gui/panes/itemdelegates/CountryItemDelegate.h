#ifndef COUNTRYITEMDELEGATE_H
#define COUNTRYITEMDELEGATE_H

#include <qstyleditemdelegate.h>


class CountryItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit CountryItemDelegate(QObject *parent = nullptr);
    QWidget *createEditor(
            QWidget *parent,
            const QStyleOptionViewItem &option,
            const QModelIndex &index) const override;
    void setEditorData(QWidget *editor,
                       const QModelIndex &index) const override;
};

#endif // COUNTRYITEMDELEGATE_H
