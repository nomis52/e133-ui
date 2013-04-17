#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <ola/e133/OLASLPThread.h>
#include <ola/rdm/CommandPrinter.h>
#include <ola/rdm/PidStoreHelper.h>
#include <QMainWindow>
#include <QStandardItemModel>
#include <QString>
#include <sstream>
#include <string>
#include "devicetracker.h"
#include "e133devicelistmodel.h"
#include "e133devicetablemodel.h"
#include "olaworker.h"

namespace Ui {
class MainWindow;
}

using ola::slp::URLEntries;

class MainWindow : public QMainWindow {
  Q_OBJECT

  public:
    explicit MainWindow(OLAWorker *worker, QWidget *parent = 0);
    ~MainWindow();

    void DiscoveryCallback(bool status, const URLEntries &urls);
    void SetStatusMessage(const QString &message);

  public slots:
    void updateDeviceTable(const URLEntries &urls);
    void updateSLPServerInfo(bool da_enabled, uint16_t port,
                             const QString &scopes,
                             const QString &backend_type);
    void logTCPConnect(const IPV4Address &device);
    void logTCPDisconnect(const IPV4Address &device);
    void logTCPMessage(const IPV4Address &device, uint16_t endpoint,
                       const string &message);

  private slots:
    void on_pushButton_clicked();

    void on_clearTCPLogButton_clicked();

  private:
    OLAWorker *worker_;
    Ui::MainWindow *ui;
    DeviceTracker m_device_tracker;
    E133DeviceTableModel m_model_table;
    E133DeviceListModel m_model_list;
    std::ostringstream m_command_str;
    ola::rdm::PidStoreHelper m_pid_helper;
    ola::rdm::CommandPrinter m_command_printer;
};
#endif  // MAINWINDOW_H
