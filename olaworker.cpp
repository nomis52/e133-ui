#include <ola/acn/ACNPort.h>
#include <ola/acn/ACNVectors.h>
#include <ola/StringUtils.h>
#include <ola/e133/E133URLParser.h>
#include <ola/io/IOStack.h>
#include <ola/network/SocketAddress.h>
#include <ola/rdm/RDMCommand.h>
#include <ola/rdm/RDMCommandSerializer.h>

#include <QDebug>
#include <QThread>
#include <string>
#include "olaworker.h"

using ola::NewCallback;
using ola::NewSingleCallback;
using ola::io::IOStack;
using ola::network::IPV4SocketAddress;
using ola::rdm::RDMCommand;
using ola::rdm::RDMRequest;




OLAWorker::OLAWorker(const UID &our_uid,
                     unsigned int  slp_discovery_interval,
                     QObject *parent)
    : QObject(parent),
      m_slp_discovery_interval(slp_discovery_interval),
      m_uid(our_uid),
      m_cid(ola::acn::CID::Generate()),
      m_message_builder(m_cid, "OLA Controller"),
      m_e133_receiver(&m_udp_socket,
                      NewCallback(this, &OLAWorker::HandleStatusMessage),
                      NewCallback(this, &OLAWorker::HandleRDMMessage)),
      m_device_manager(&m_ss, &m_message_builder),
      m_slp_thread(&m_ss) {
  m_device_manager.SetAcquireDeviceCallback(
      NewCallback(this, &OLAWorker::DeviceConnect));
  m_device_manager.SetReleaseDeviceCallback(
      NewCallback(this, &OLAWorker::DeviceDisconnect));
  m_device_manager.SetRDMMessageCallback(
      NewCallback(this, &OLAWorker::DeviceMessage));
}

OLAWorker::~OLAWorker() {}

bool OLAWorker::Init() {
  if (!m_udp_socket.Init())
    return false;

  if (!m_udp_socket.Bind(IPV4SocketAddress(IPV4Address(), 0))) {
    qWarning() << "Failed to bind to UDP port";
    return false;
  }

  m_ss.AddReadDescriptor(&m_udp_socket);

  m_slp_thread.SetNewDeviceCallback(
      ola::NewCallback(this, &OLAWorker::DiscoveryCallback));
  if (!m_slp_thread.Init()) {
    qWarning() << "Failed to start slp thread";
    return false;
  }
  return true;
}

void OLAWorker::RunSLPDiscoveryNow() {
  m_slp_thread.RunDeviceDiscoveryNow();
}

void OLAWorker::GetServerInfo() {
  m_slp_thread.ServerInfo(
    ola::NewSingleCallback(this, &OLAWorker::newSLPServerInfo));
}

void OLAWorker::SendRequest(const IPV4Address &ip,
                            uint16_t endpoint,
                            RDMRequest *request) {
  m_ss.Execute(NewSingleCallback(
      this, &OLAWorker::SendRDMRequest,
      ip, endpoint, request));
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
  m_slp_thread.Start();
  m_ss.Run();
  m_slp_thread.Join(NULL);
  m_slp_thread.Cleanup();

  emit finished();
}

void OLAWorker::SendRDMRequest(const IPV4Address ip,
                               uint16_t endpoint,
                               RDMRequest *request_ptr) {
  auto_ptr<RDMRequest> request(request_ptr);
  IPV4SocketAddress target(ip, ola::acn::E133_PORT);

  // Build the E1.33 packet.
  IOStack packet(m_message_builder.pool());
  ola::rdm::RDMCommandSerializer::Write(*request, &packet);
  m_message_builder.PrependRDMHeader(&packet);
  m_message_builder.BuildUDPRootE133(
      &packet, ola::acn::VECTOR_FRAMING_RDMNET, m_sequence_number.Next(),
      endpoint);

  m_udp_socket.SendTo(&packet, target);
  if (!packet.Empty()) {
    qDebug() << "Failed to send entire packet";
    return;
  }
}


void OLAWorker::HandleStatusMessage(const E133StatusMessage &status_message) {
  emit UDPStatusMessage(status_message);
}

void OLAWorker::HandleRDMMessage(const E133RDMMessage &rdm_message) {
  emit UDPRDMMessage(rdm_message);
}



