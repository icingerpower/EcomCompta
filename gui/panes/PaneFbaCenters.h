#ifndef PANEFBACENTERS_H
#define PANEFBACENTERS_H

#include <QWidget>
class DialogFbaAddress;

namespace Ui {
class PaneFbaCenters;
}

class PaneFbaCenters : public QWidget
{
    Q_OBJECT

public:
    explicit PaneFbaCenters(QWidget *parent = nullptr);
    ~PaneFbaCenters();

public slots:
    void addCenter();
    void removeSelectedCenters();

private:
    Ui::PaneFbaCenters *ui;
    void _connectSlots();
    DialogFbaAddress *m_dialogFbaAddress;
};

#endif // PANEFBACENTERS_H
