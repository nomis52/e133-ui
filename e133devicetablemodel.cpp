#include "e133devicetablemodel.h"
#include "devicetracker.h"

E133DeviceTableModel::E133DeviceTableModel(const DeviceTracker *tracker,
                                           QObject *parent)
    : QAbstractTableModel(parent),
      m_tracker(tracker) {
  QObject::connect(tracker, SIGNAL(rowChanged(int, int)),
                   this, SLOT(modelRowChanged(int, int)));
  QObject::connect(tracker, SIGNAL(dataChanged()),
                   this, SLOT(modelDataChanged()));
}

int E133DeviceTableModel::rowCount(const QModelIndex &parent) const {
  return m_tracker->NumberOfDevices();
}

int E133DeviceTableModel::columnCount(const QModelIndex &parent) const {
  return COL_LAST;
}

QVariant E133DeviceTableModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid() || index.row() >= rowCount() ||
      role != Qt::DisplayRole) {
    return QVariant();
  }

  const E133Device *device = m_tracker->DeviceAt(index.row());
  switch (index.column()) {
    case COL_IP_ADDRESS:
      return QString(device->ip_address.ToString().c_str());
    case COL_UID:
      return QString(device->uid.ToString().c_str());
    case COL_LIFETIME:
      return QString::number(device->lifetime);
    case COL_TCP_STATE:
      return QString(device->tcp_connected ? "Yes" : "No");
    default:
      {}
  }
  return QVariant();
}

QVariant E133DeviceTableModel::headerData(int section,
                                          Qt::Orientation orientation,
                                          int role) const {
  if (role != Qt::DisplayRole)
    return QVariant();

  if (orientation == Qt::Horizontal) {
    switch (section) {
    case COL_IP_ADDRESS:
        return QString("IP Address");
      case COL_UID:
        return QString("E1.33 UID");
      case COL_LIFETIME:
        return QString("SLP Lifetime");
      case COL_TCP_STATE:
        return QString("Designated Controller");
      default:
        return QString("Untitled Column");
    }
  } else {
    return QVariant();
  }
}

void E133DeviceTableModel::modelRowChanged(int start, int end) {
  emit dataChanged(createIndex(start, 0), createIndex(start, COL_LAST - 1));
}

void E133DeviceTableModel::modelDataChanged() {
  emit layoutChanged();
}
