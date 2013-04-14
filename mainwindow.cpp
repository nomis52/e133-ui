#include <QDebug>
#include <QThread>
#include <ola/e133/E133URLParser.h>
#include <ola/e133/SLPThread.h>
#include <ola/network/IPV4Address.h>
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

  QString refresh_str = ToQString(worker->DiscoveryInterval());
  refresh_str.append(" seconds");
  ui->SLPRefreshLabel->setText(refresh_str);
  QObject::connect(worker_, SIGNAL(newSLPDevices(const URLEntries&)),
                   this, SLOT(updateDeviceTable(const URLEntries&)),
                   Qt::BlockingQueuedConnection);
  QObject::connect(worker_,
                   SIGNAL(SLPServerProperties(bool, uint16_t, const QString&,
                                              const QString&)),
                   this,
                   SLOT(updateSLPServerInfo(bool, uint16_t, const QString&,
                                            const QString&)),
                   Qt::BlockingQueuedConnection);
  worker_->GetServerInfo();
}

MainWindow::~MainWindow() {
  delete ui;
}

void MainWindow::on_pushButton_clicked()
{
  SetStatusMessage(DISCOVERY_MESSAGE);
  worker_->RunSLPDiscoveryNow();
}

void MainWindow::updateDeviceTable(const URLEntries &urls) {
  SetStatusMessage("");

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

void MainWindow::updateSLPServerInfo(bool da_enabled, uint16_t port,
                                     const QString &scopes,
                                     const QString &backend_type) {
  ui->SLPDAEnabledLabel->setText(da_enabled ? "Enabled" : "Disabled");
  ui->SLPPortLabel->setText(ToQString(port));
  ui->SLPScopesLabel->setText(scopes);
  ui->SLPBackendType->setText(backend_type);
}

void MainWindow::SetStatusMessage(const QString &message) {
  statusBar()->showMessage(message);
}

void MainWindow::on_clearTCPLogButton_clicked() {
  ui->TCPLog->appendPlainText(QString("hello"));
}
