#ifndef E133DEVICETABLEMODEL_H
#define E133DEVICETABLEMODEL_H

#include <QObject>
#include <QModelIndex>
#include <QAbstractTableModel>

class E133DeviceTableModel : public QAbstractTableModel {
  Q_OBJECT

  public:
    E133DeviceTableModel(const class DeviceTracker *tracker,
                         QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;

  private:
    enum {
      COL_IP_ADDRESS,
      COL_UID,
      COL_LIFETIME,
      COL_TCP_STATE,
      COL_LAST,
    };

    const class DeviceTracker *m_tracker;

  private slots:
    void modelRowChanged(int start, int end);
    void modelDataChanged();
};
#endif  // E133DEVICETABLEMODEL_H
