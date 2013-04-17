#include <QDebug>
#include <string>
#include <ola/StringUtils.h>
#include <ola/e133/E133URLParser.h>
#include "olaworker.h"

using ola::NewCallback;

OLAWorker::OLAWorker(unsigned int  slp_discovery_interval,
                     QObject *parent)
    : QObject(parent),
      m_slp_discovery_interval(slp_discovery_interval),
      m_cid(ola::acn::CID::Generate()),
      m_message_builder(m_cid, "OLA Controller"),
      m_device_manager(&m_ss, &m_message_builder),
      m_slp_thread(&m_ss) {
  m_device_manager.SetAcquireDeviceCallback(
      NewCallback(this, &OLAWorker::DeviceConnect));
  m_device_manager.SetReleaseDeviceCallback(
      NewCallback(this, &OLAWorker::DeviceDisconnect));
  m_device_manager.SetRDMMessageCallback(
      NewCallback(this, &OLAWorker::DeviceMessage));
}

OLAWorker::~OLAWorker() {
}

void OLAWorker::RunSLPDiscoveryNow() {
  m_slp_thread.RunDeviceDiscoveryNow();
}

void OLAWorker::GetServerInfo() {
  m_slp_thread.ServerInfo(
    ola::NewSingleCallback(this, &OLAWorker::newSLPServerInfo));
}

void OLAWorker::DiscoveryCallback(bool status, const URLEntries &urls) {
  if (status) {
    for (const auto& url : urls) {
      ola::rdm::UID uid(0, 0);
      ola::network::IPV4Address ip;
      if (!ola::e133::ParseE133URL(url.url(), &uid, &ip)) {
        qWarning() << "Invalid E1.33 URL: " << url.url().c_str();
        continue;
      }
      m_device_manager.AddDevice(ip);
    }
    emit newSLPDevices(urls);
  } else {
    qWarning() << "SLP Discovery Failed";
  }
}

void OLAWorker::newSLPServerInfo(
    bool ok,
    const ola::e133::SLPThreadServerInfo &server_info) {
  if (ok) {
    QString scopes(ola::StringJoin(", ", server_info.scopes).c_str());
    emit SLPServerProperties(server_info.da_enabled, server_info.port, scopes,
                             server_info.backend_type.c_str());
  } else {
    qWarning() << "SLP Server Info Failed";
  }
}

void OLAWorker::DeviceConnect(const IPV4Address &ip) {
  emit TCPConnect(ip);
}

void OLAWorker::DeviceDisconnect(const IPV4Address &ip) {
  emit TCPDisconnect(ip);
}

bool OLAWorker::DeviceMessage(const IPV4Address &device, uint16_t endpoint,
                              const string &message) {
  emit TCPMessage(device, endpoint, message);
  return true;  // ack
}

void OLAWorker::process() {
  m_slp_thread.SetNewDeviceCallback(
      ola::NewCallback(this, &OLAWorker::DiscoveryCallback));
  if (!m_slp_thread.Init()) {
    qWarning() << "Failed to start slp thread";
    return;
  }
  m_slp_thread.Start();
  m_ss.Run();
  m_slp_thread.Join(NULL);
  m_slp_thread.Cleanup();

  emit finished();
}
