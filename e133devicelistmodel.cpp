#include "devicetracker.h"
#include "e133devicelistmodel.h"

E133DeviceListModel::E133DeviceListModel(const DeviceTracker *tracker,
                                         QObject *parent)
    : QAbstractListModel(parent),
      m_tracker(tracker) {
  QObject::connect(tracker, SIGNAL(dataChanged()),
                   this, SLOT(modelDataChanged()));
}

ola::network::IPV4Address E133DeviceListModel::IPAddressAt(int index) const {
  const E133Device *device = m_tracker->DeviceAt(index);
  if (device) {
    return device->ip_address;
  } else {
    return IPV4Address();
  }
}

ola::rdm::UID E133DeviceListModel::UIDAt(int index) const {
  const E133Device *device = m_tracker->DeviceAt(index);
  if (device) {
    return device->uid;
  } else {
    return UID::AllDevices();
  }
}

int E133DeviceListModel::rowCount(const QModelIndex&) const {
  return m_tracker->NumberOfDevices();
}

int E133DeviceListModel::columnCount(const QModelIndex&) const {
  return 1;
}

QVariant E133DeviceListModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid() || index.row() >= rowCount() ||
      role != Qt::DisplayRole) {
    return QVariant();
  }

  const E133Device *device = m_tracker->DeviceAt(index.row());
  return QString(device->ip_address.ToString().c_str());
}

void E133DeviceListModel::modelDataChanged() {
  emit dataChanged(createIndex(0, 0),
                   createIndex(m_tracker->NumberOfDevices(), 1));
}
