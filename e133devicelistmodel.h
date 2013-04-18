#ifndef E133DEVICELISTMODEL_H
#define E133DEVICELISTMODEL_H

#include <ola/network/IPV4Address.h>
#include <QObject>
#include <QModelIndex>
#include <QAbstractListModel>

class E133DeviceListModel : public QAbstractListModel {
  Q_OBJECT

  public:
    E133DeviceListModel(const class DeviceTracker *tracker,
                        QObject *parent = 0);

    ola::network::IPV4Address IPAddressAt(int index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;

  private:
    const class DeviceTracker *m_tracker;

  private slots:
    void modelDataChanged();
};
#endif  // E133DEVICELISTMODEL_H
