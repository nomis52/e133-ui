#include <QDebug>
#include <QThread>
#include <sstream>
#include <ola/StringUtils.h>
#include <ola/e133/E133URLParser.h>
#include <ola/e133/SLPThread.h>
#include <ola/network/IPV4Address.h>
#include <ola/rdm/RDMCommand.h>
#include <ola/rdm/UID.h>

#include "mainwindow.h"
#include "ui_mainwindow.h"

using ola::slp::URLEntries;
using ola::rdm::RDMCommand;

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
      device_model_(new QStandardItemModel(0, 3, this)),
      m_pid_helper("/usr/local/share/ola/pids", 4),  // hack alert!
      m_command_printer(&m_command_str, &m_pid_helper) {
  ui->setupUi(this);
  SetStatusMessage(DISCOVERY_MESSAGE);

  if (!m_pid_helper.Init()) {
    qWarning() << "Failed to init PID helper";
  }

  device_model_->setHorizontalHeaderItem(
      0, new QStandardItem(QString("IP Address")));
  device_model_->setHorizontalHeaderItem(
      1, new QStandardItem(QString("E1.33 UID")));
  device_model_->setHorizontalHeaderItem(
      2, new QStandardItem(QString("SLP Lifetime")));
  device_model_->setHorizontalHeaderItem(
      3, new QStandardItem(QString("Designated Controller")));

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
  QObject::connect(worker_,
                   SIGNAL(TCPConnect(const IPV4Address&)),
                   this,
                   SLOT(logTCPConnect(const IPV4Address&)),
                   Qt::BlockingQueuedConnection);
  QObject::connect(worker_,
                   SIGNAL(TCPDisconnect(const IPV4Address&)),
                   this,
                   SLOT(logTCPDisconnect(const IPV4Address&)),
                   Qt::BlockingQueuedConnection);
  QObject::connect(worker_,
                   SIGNAL(TCPMessage(const IPV4Address&, uint16_t, const string&)),
                   this,
                   SLOT(logTCPMessage(const IPV4Address&, uint16_t, const string&)),
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

void MainWindow::logTCPConnect(const IPV4Address &device) {
  QString output("Connected to: ");
  output.append(device.ToString().c_str());
  ui->TCPLog->appendPlainText(output);
}

void MainWindow::logTCPDisconnect(const IPV4Address &device) {
  QString output("Disconnected from: ");
  output.append(device.ToString().c_str());
  ui->TCPLog->appendPlainText(output);
}

void MainWindow::logTCPMessage(const IPV4Address &device, uint16_t endpoint,
                               const string &message) {
  m_command_str.str("");
  m_command_str << "From " << device << ", endpoint " << endpoint << std::endl;
  unsigned int data_size = message.size();
  const uint8_t *rdm_data = reinterpret_cast<const uint8_t*>(
    message.data());

  auto_ptr<RDMCommand> command(RDMCommand::Inflate(rdm_data, data_size));

  if (command.get()) {
    command->Print(&m_command_printer, false, true);
  } else {
    ola::FormatData(&m_command_str, rdm_data, data_size, 2);
  }

  ui->TCPLog->appendPlainText(QString(m_command_str.str().c_str()));
}

void MainWindow::SetStatusMessage(const QString &message) {
  statusBar()->showMessage(message);
}

void MainWindow::on_clearTCPLogButton_clicked() {
  ui->TCPLog->clear();
}
