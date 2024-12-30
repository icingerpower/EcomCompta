#ifndef PANEVATTHRESHOLD_H
#define PANEVATTHRESHOLD_H

#include <QWidget>

namespace Ui {
class PaneVatThreshold;
}

class PaneVatThreshold : public QWidget
{
    Q_OBJECT

public:
    explicit PaneVatThreshold(QWidget *parent = nullptr);
    ~PaneVatThreshold();

private:
    Ui::PaneVatThreshold *ui;
};

#endif // PANEVATTHRESHOLD_H
