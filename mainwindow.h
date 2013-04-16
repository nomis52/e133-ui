#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <sstream>
#include <QMainWindow>
#include <QString>
#include <QStandardItemModel>
#include <ola/e133/OLASLPThread.h>
#include <ola/rdm/CommandPrinter.h>
#include <ola/rdm/PidStoreHelper.h>
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
    void updateSLPServerInfo(bool da_enabled, uint16_t port, const QString &scopes,
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
    QStandardItemModel *device_model_;
    std::ostringstream m_command_str;;
    ola::rdm::PidStoreHelper m_pid_helper;
    ola::rdm::CommandPrinter m_command_printer;
};

#endif // MAINWINDOW_H
