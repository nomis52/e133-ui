#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <ola/e133/OLASLPThread.h>
#include <ola/rdm/CommandPrinter.h>
#include <ola/rdm/PidStoreHelper.h>
#include <QMainWindow>
#include <QStandardItemModel>
#include <QString>
#include <memory>
#include <sstream>
#include <string>
#include "devicetracker.h"
#include "e133devicelistmodel.h"
#include "e133devicetablemodel.h"
#include "olaworker.h"
#include "pidlistmodel.h"
#include <ola/util/SequenceNumber.h>

namespace Ui {
class MainWindow;
}

using ola::slp::URLEntries;

class MainWindow : public QMainWindow {
  Q_OBJECT

  public:
    explicit MainWindow(const UID &our_uid,
                        OLAWorker *worker,
                        QWidget *parent = 0);
    ~MainWindow();

    void DiscoveryCallback(bool status, const URLEntries &urls);

  public slots:
    void updateDeviceTable(const URLEntries &urls);
    void updateSLPServerInfo(bool da_enabled, uint16_t port,
                             const QString &scopes,
                             const QString &backend_type);
    void logTCPConnect(const IPV4Address &device);
    void logTCPDisconnect(const IPV4Address &device);
    void logTCPMessage(const IPV4Address &device, uint16_t endpoint,
                       const string &message);
    void logUDPStatusMessage(const E133StatusMessage &status_message);
    void logUDPRDMMessage(const E133RDMMessage &rdm_message);

  private slots:
    void on_slpDiscoveryButton_clicked();
    void on_clearTCPLogButton_clicked();
    void on_tabWidget_currentChanged(QWidget *arg1);
    void on_sendRDMButton_clicked();

    void SetStatusMessage(const QString &message);
    void ShowDialog(const QString &error);

    void on_pushButton_clicked();

    RDMRequest* BuildRequest(
        const UID &dest_uid,
        uint16_t pid,
        bool is_get,
        const uint8_t *param_data,
        unsigned int param_data_length);

  private:
    const UID m_uid;
    OLAWorker *m_worker;
    ola::SequenceNumber<uint8_t> m_transaction_number;
    Ui::MainWindow *ui;
    DeviceTracker m_device_tracker;
    E133DeviceTableModel m_model_table;
    E133DeviceListModel m_model_list;
    std::ostringstream m_command_str;
    ola::rdm::PidStoreHelper m_pid_helper;
    auto_ptr<PIDListModel> m_pid_list_model;
    ola::rdm::CommandPrinter m_command_printer;
};
#endif  // MAINWINDOW_H
