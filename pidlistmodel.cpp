#include <algorithm>
#include <vector>
#include "pidlistmodel.h"

PIDListModel::PIDListModel(const ola::rdm::PidStoreHelper *pid_helper,
                           QObject *parent)
    : QAbstractListModel(parent) {
  vector<const ola::rdm::PidDescriptor*> descriptors;
  pid_helper->SupportedPids(0, &descriptors);
  for (const auto* descriptor : descriptors) {
    QString label = QString("%1 (0x%2)")
        .arg(descriptor->Name().c_str())
        .arg(descriptor->Value(), 4, 16, QChar('0'));
    m_pids.push_back(PIDData(descriptor->Value(), label));
  }

  std::sort(m_pids.begin(), m_pids.end());
}

uint16_t PIDListModel::GetPidAtIndex(int index) const {
  if (index < 0 || index >= rowCount()) {
    return 0;
  }
  return m_pids[index].param_id;
}

int PIDListModel::rowCount(const QModelIndex&) const {
  return m_pids.size();
}

int PIDListModel::columnCount(const QModelIndex&) const {
  return 1;
}

QVariant PIDListModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid() || index.row() >= rowCount() ||
      role != Qt::DisplayRole) {
    return QVariant();
  }

  return m_pids[index.row()].label;
}
