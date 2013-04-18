#ifndef PIDLISTMODEL_H
#define PIDLISTMODEL_H

#include <ola/rdm/PidStoreHelper.h>
#include <vector>
#include <QObject>
#include <QModelIndex>
#include <QAbstractListModel>

class PIDListModel : public QAbstractListModel {
  Q_OBJECT

  public:
    PIDListModel(const ola::rdm::PidStoreHelper *pid_helper,
                 QObject *parent = 0);

    uint16_t GetPidAtIndex(int index) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;

  private:
    class PIDData {
      public:
        PIDData(uint16_t pid, const QString &label)
            : param_id(pid),
              label(label) {
        }

        bool operator<(const PIDData &other) const {
          return label < other.label;
        }

        uint16_t param_id;
        QString label;
    };
    std::vector<PIDData> m_pids;
};
#endif  // PIDLISTMODEL_H
