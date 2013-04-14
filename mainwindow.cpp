#include <QDebug>
#include <QThread>
#include <ola/e133/E133URLParser.h>
#include <ola/e133/SLPThread.h>
#include <ola/network/IPV4Address.h>>
#include <ola/rdm/UID.h>

#include "mainwindow.h"
#include "ui_mainwindow.h"

using ola::slp::URLEntries;

// this is dumb
template<typename T>
QString ToQString(T t) {
  QString s;
  s.setNum(t);
  return s;
}

static const char DISCOVERY_MESSAGE[] = "Running SLP Discovery...";

MainWindow::MainWindow(OLAWorker *worker, QWidget *parent)
    : QMainWindow(parent),
      worker_(worker),
      ui(new Ui::MainWindow),
      device_model_(new QStandardItemModel(0, 3, this)) {
  ui->setupUi(this);
  SetStatusMessage(DISCOVERY_MESSAGE);

  device_model_->setHorizontalHeaderItem(
      0, new QStandardItem(QString("IP Address")));
  device_model_->setHorizontalHeaderItem(
      1, new QStandardItem(QString("E1.33 UID")));
  device_model_->setHorizontalHeaderItem(
      2, new QStandardItem(QString("SLP Lifetime")));

  ui->tableView->setModel(device_model_);
  ui->tableView->horizontalHeader()->setResizeMode(QHeaderView::Stretch);

  // ui->   ->setText(ToQString(worker->DiscoveryInterval()));
  QObject::connect(worker_, SIGNAL(newSLPDevices(const URLEntries&)),
                   this, SLOT(updateDeviceTable(const URLEntries&)),
                   Qt::BlockingQueuedConnection);
}

MainWindow::~MainWindow() {
  delete ui;
}

void MainWindow::on_pushButton_clicked()
{
  qDebug() << "button was pressed";
  SetStatusMessage(DISCOVERY_MESSAGE);
  worker_->RunSLPDiscoveryNow();
}

void MainWindow::updateDeviceTable(const URLEntries &urls) {
  SetStatusMessage("");

  qDebug() << "in updateDeviceTable in " << QThread::currentThreadId();
  int i = 0;

  for (const auto& url : urls) {
    ola::rdm::UID uid(0, 0);
    ola::network::IPV4Address ip;
    if (!ola::e133::ParseE133URL(url.url(), &uid, &ip)) {
      qWarning() << "Invalid E1.33 URL: " << url.url().c_str();
      continue;
    }
    QString lifetime = ToQString(url.lifetime());
    device_model_->setItem(
          i, 0, new QStandardItem(QString(ip.ToString().c_str())));
    device_model_->setItem(
          i, 1, new QStandardItem(QString(uid.ToString().c_str())));
    device_model_->setItem(i, 2, new QStandardItem(lifetime));
    qDebug() << url.url().c_str();
    i++;
  }
}

void MainWindow::SetStatusMessage(const QString &message) {
  statusBar()->showMessage(message);
}

void MainWindow::on_clearTCPLogButton_clicked() {
  ui->TCPLog->appendPlainText(QString("hello"));
}
