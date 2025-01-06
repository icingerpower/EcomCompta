#include <QComboBox>

#include "model/inventory/SaleGroups.h"

#include "SaleGroupsDelegate.h"


SaleGroupsDelegate::SaleGroupsDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QWidget *SaleGroupsDelegate::createEditor(
        QWidget *parent,
        const QStyleOptionViewItem &option,
        const QModelIndex &index) const
{
    QWidget *widget = nullptr;
    if (index.column() == 1)
    {
        QComboBox *comboBox = new QComboBox(parent);
        comboBox->addItems(SaleGroups::AMAZONS);
        widget = comboBox;
    }
    else
    {
        widget =  QStyledItemDelegate::createEditor(
                    parent, option, index);
    }
    return widget;

}

void SaleGroupsDelegate::setEditorData(
        QWidget *editor, const QModelIndex &index) const
{
    if (index.column() == 1)
    {
        QComboBox *comboBox = static_cast<QComboBox *>(editor);
        QString amazon = index.data().toString();
        comboBox->setCurrentText(amazon);
    }
}
