#include <QtWidgets/qcombobox.h>

#include "CountryItemDelegate.h"
#include "model/SettingManager.h"


//----------------------------------------------------------
CountryItemDelegate::CountryItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}
//----------------------------------------------------------
QWidget *CountryItemDelegate::createEditor(
        QWidget *parent,
        const QStyleOptionViewItem &option,
        const QModelIndex &index) const
{
    QWidget *widget = nullptr;
    if (index.column() == 1)
    {
        QComboBox *comboBox = new QComboBox(parent);
        comboBox->addItems(*SettingManager::countriesUEfrom2020());
        widget = comboBox;
    }
    else
    {
        widget =  QStyledItemDelegate::createEditor(
                    parent, option, index);
    }
    return widget;
}
//----------------------------------------------------------
void CountryItemDelegate::setEditorData(
        QWidget *editor, const QModelIndex &index) const
{
    if (index.column() == 1)
    {
        QComboBox *comboBox = static_cast<QComboBox *>(editor);
        QString country = index.data().toString();
        comboBox->setCurrentText(country);
    }
}
//----------------------------------------------------------

