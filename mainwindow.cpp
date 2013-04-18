#include <ola/StringUtils.h>
#include <ola/e133/SLPThread.h>
#include <ola/network/IPV4Address.h>
#include <ola/rdm/RDMCommand.h>
#include <QDebug>
#include <QMessageBox>
#include <QThread>
#include <sstream>
#include <string>
#include <vector>

#include "mainwindow.h"
#include "ui_mainwindow.h"

using ola::rdm::RDMCommand;
using ola::slp::URLEntries;
using std::endl;

static const char DISCOVERY_MESSAGE[] = "Running SLP Discovery...";

MainWindow::MainWindow(const UID &our_uid,
                       OLAWorker *worker,
                       QWidget *parent)
    : QMainWindow(parent),
      m_uid(our_uid),
      m_worker(worker),
      ui(new Ui::MainWindow),
      m_device_tracker(),
      m_model_table(&m_device_tracker),
      m_model_list(&m_device_tracker),
      m_pid_helper("/usr/local/share/ola/pids", 4),  // hack alert!
      m_command_printer(&m_command_str, &m_pid_helper) {
  ui->setupUi(this);
  SetStatusMessage(DISCOVERY_MESSAGE);

  if (!m_pid_helper.Init()) {
    qWarning() << "Failed to init PID helper";
  }

  m_pid_list_model.reset(new PIDListModel(&m_pid_helper));

  ui->tableView->setModel(&m_model_table);
  ui->tableView->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
  ui->tableView->verticalHeader()->hide();
  ui->ipComboBox->setModel(&m_model_list);
  ui->pidComboBox->setModel(m_pid_list_model.get());

  QString refresh_str = QString::number(worker->DiscoveryInterval());
  refresh_str.append(" seconds");
  ui->SLPRefreshLabel->setText(refresh_str);
  QObject::connect(m_worker, SIGNAL(newSLPDevices(const URLEntries&)),
                   this, SLOT(updateDeviceTable(const URLEntries&)),
                   Qt::BlockingQueuedConnection);
  QObject::connect(m_worker,
                   SIGNAL(SLPServerProperties(bool, uint16_t, const QString&,
                                              const QString&)),
                   this,
                   SLOT(updateSLPServerInfo(bool, uint16_t, const QString&,
                                            const QString&)),
                   Qt::BlockingQueuedConnection);
  QObject::connect(m_worker,
                   SIGNAL(TCPConnect(const IPV4Address&)),
                   this,
                   SLOT(logTCPConnect(const IPV4Address&)),
                   Qt::BlockingQueuedConnection);
  QObject::connect(m_worker,
                   SIGNAL(TCPDisconnect(const IPV4Address&)),
                   this,
                   SLOT(logTCPDisconnect(const IPV4Address&)),
                   Qt::BlockingQueuedConnection);
  QObject::connect(
        m_worker,
        SIGNAL(TCPMessage(const IPV4Address&, uint16_t, const string&)),
        this,
        SLOT(logTCPMessage(const IPV4Address&, uint16_t, const string&)),
        Qt::BlockingQueuedConnection);
  QObject::connect(
        m_worker,
        SIGNAL(UDPStatusMessage(const E133StatusMessage&)),
        this,
        SLOT(logUDPStatusMessage(const E133StatusMessage&)),
        Qt::BlockingQueuedConnection);
  QObject::connect(
        m_worker,
        SIGNAL(UDPRDMMessage(const E133RDMMessage&)),
        this,
        SLOT(logUDPRDMMessage(const E133RDMMessage&)),
        Qt::BlockingQueuedConnection);

  m_worker->GetServerInfo();
}

MainWindow::~MainWindow() {
  delete ui;
}

void MainWindow::on_slpDiscoveryButton_clicked() {
  SetStatusMessage(DISCOVERY_MESSAGE);
  m_worker->RunSLPDiscoveryNow();
}

void MainWindow::updateDeviceTable(const URLEntries &urls) {
  SetStatusMessage("");
  m_device_tracker.UpdateFromSLP(urls);
}

void MainWindow::updateSLPServerInfo(bool da_enabled, uint16_t port,
                                     const QString &scopes,
                                     const QString &backend_type) {
  ui->SLPDAEnabledLabel->setText(da_enabled ? "Enabled" : "Disabled");
  ui->SLPPortLabel->setText(QString::number(port));
  ui->SLPScopesLabel->setText(scopes);
  ui->SLPBackendType->setText(backend_type);
}

void MainWindow::logTCPConnect(const IPV4Address &device) {
  m_device_tracker.MarkAsConnected(device);
  QString output("Connected to: ");
  output.append(device.ToString().c_str());
  ui->TCPLog->appendPlainText(output);
}

void MainWindow::logTCPDisconnect(const IPV4Address &device) {
  m_device_tracker.MarkAsDisconnected(device);
  QString output("Disconnected from: ");
  output.append(device.ToString().c_str());
  ui->TCPLog->appendPlainText(output);
}

void MainWindow::logTCPMessage(const IPV4Address &device, uint16_t endpoint,
                               const string &message) {
  m_command_str.str("");
  m_command_str << "From " << device << ", endpoint " << endpoint << endl;
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

void MainWindow::logUDPStatusMessage(const E133StatusMessage &status_message) {
  m_command_str.str("");
  m_command_str << status_message.device << ", Endpoint "
                << status_message.endpoint << ", #"
                << status_message.sequence_number << endl;
  m_command_str << "  Status " << status_message.status_code << ": "
                << status_message.status_message;

  ui->UDPLog->appendPlainText(QString(m_command_str.str().c_str()));
}

void MainWindow::logUDPRDMMessage(const E133RDMMessage &rdm_message) {
  m_command_str.str("");
  m_command_str << rdm_message.device << ", Endpoint "
                << rdm_message.endpoint << ", #"
                << rdm_message.sequence_number << endl;
  rdm_message.response->Print(&m_command_printer, false, true);
  ui->UDPLog->appendPlainText(QString(m_command_str.str().c_str()));
}

void MainWindow::on_clearTCPLogButton_clicked() {
  ui->TCPLog->clear();
}

void MainWindow::on_tabWidget_currentChanged(QWidget *arg1) {
  if (arg1 == ui->RDMTab) {
    if (ui->ipComboBox->count() && ui->ipComboBox->currentIndex() == -1) {
      ui->ipComboBox->setCurrentIndex(0);
    }
  }
}

void MainWindow::on_sendRDMButton_clicked() {
  IPV4Address ip_address = m_model_list.IPAddressAt(
        ui->ipComboBox->currentIndex());

  if (!ip_address.AsInt()) {
    ShowDialog("Invalid IP Address");
    return;
  }

  uint16_t endpoint = ui->endpointSpinBox->value();

  auto_ptr<UID> uid(UID::FromString(ui->uidInput->text().toLocal8Bit().data()));
  if (!uid.get()) {
    ShowDialog("Invalid UID");
    return;
  }

  bool get = ui->commandClassComboBox->currentIndex() ? false : true;
  uint16_t pid = m_pid_list_model->GetPidAtIndex(
        ui->pidComboBox->currentIndex());

  const ola::rdm::PidDescriptor *pid_descriptor = m_pid_helper.GetDescriptor(
      pid, 0);
  if (!pid_descriptor) {
    ShowDialog("Unknown PID");
    return;
  }

  const ola::messaging::Descriptor *descriptor = get ?
      pid_descriptor->GetRequest() : pid_descriptor->SetRequest();
  if (!descriptor) {
    QString label = QString("%1 not supported for %2")
        .arg(get ? "GET" : "SET")
        .arg(pid_descriptor->Name().c_str());
    ShowDialog(label);
    return;
  }

  vector<string> inputs;
  string param_data_str(ui->paramDataInput->text().toLocal8Bit().data());
  if (!param_data_str.empty())  {
    ola::StringSplit(param_data_str, inputs);
  }
  auto_ptr<const ola::messaging::Message> message(m_pid_helper.BuildMessage(
      descriptor, inputs));
  if (!message.get()) {
    ShowDialog("Bad Param data");
    qDebug() << m_pid_helper.SchemaAsString(descriptor).c_str();
    return;
  }

  unsigned int param_data_length;
  const uint8_t *param_data = m_pid_helper.SerializeMessage(
      message.get(), &param_data_length);

  RDMRequest *request(
      BuildRequest(*uid, pid, get, param_data, param_data_length));

  m_command_str.str("");
  m_command_str << ip_address.ToString().c_str() << ", Endpoint "
                << endpoint << endl;
  request->Print(&m_command_printer, false, true);
  ui->UDPLog->appendPlainText(QString(m_command_str.str().c_str()));

  qDebug() << "Sending " << ip_address.ToString().c_str() << " : "
           << endpoint << " : " << uid->ToString().c_str() << " : " << pid;
  m_worker->SendRequest(ip_address, endpoint, request);
}

void MainWindow::SetStatusMessage(const QString &message) {
  statusBar()->showMessage(message);
}

void MainWindow::ShowDialog(const QString &error) {
  QMessageBox dialog(this);
  dialog.setText(error);
  dialog.exec();
}

void MainWindow::on_pushButton_clicked() {
  ui->UDPLog->clear();
}

RDMRequest* MainWindow::BuildRequest(const UID &dest_uid,
                                     uint16_t pid,
                                     bool is_get,
                                     const uint8_t *param_data,
                                     unsigned int param_data_length) {
  if (is_get) {
    return new ola::rdm::RDMGetRequest(
        m_uid, dest_uid, m_transaction_number.Next(),
        1,  // port id
        0,  // message count
        ola::rdm::ROOT_RDM_DEVICE,  // sub device
        pid, param_data, param_data_length);
  } else {
    return new ola::rdm::RDMSetRequest(
        m_uid, dest_uid, m_transaction_number.Next(),
        1,  // port id
        0,  // message count
        ola::rdm::ROOT_RDM_DEVICE,  // sub device
        pid, param_data, param_data_length);
  }
}
