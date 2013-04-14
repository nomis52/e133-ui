#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <olaworker.h>
#include <QMainWindow>
#include <QString>
#include <QStandardItemModel>
#include <ola/e133/OLASLPThread.h>

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

signals:
    void newDevices(const URLEntries &urls);

public slots:
    void updateDeviceTable(const URLEntries &urls);

private slots:
    void on_pushButton_clicked();

    void on_clearTCPLogButton_clicked();

private:
    OLAWorker *worker_;
    Ui::MainWindow *ui;
    QStandardItemModel *device_model_;
};

#endif // MAINWINDOW_H
