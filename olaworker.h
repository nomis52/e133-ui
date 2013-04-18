#ifndef OLATHREAD_H
#define OLATHREAD_H

#include <QObject>
#include <memory>
#include <string>
#include <ola/acn/CID.h>
#include <ola/e133/DeviceManager.h>
#include <ola/e133/MessageBuilder.h>
#include <ola/e133/OLASLPThread.h>
#include <ola/e133/E133Receiver.h>
#include <ola/io/SelectServer.h>
#include <ola/network/IPv4Address.h>
#include <ola/network/Socket.h>
#include <ola/rdm/RDMCommand.h>
#include <ola/slp/URLEntry.h>
#include <ola/util/SequenceNumber.h>

using ola::e133::E133RDMMessage;
using ola::e133::E133StatusMessage;
using ola::network::IPV4Address;
using ola::rdm::RDMRequest;
using ola::rdm::RDMResponse;
using ola::rdm::UID;
using ola::slp::URLEntries;
using std::auto_ptr;
using std::string;

class OLAWorker : public QObject {
  Q_OBJECT
  public:
    explicit OLAWorker(const UID &our_uid,
                       unsigned int slp_discovery_interval,
                       QObject *parent = 0);
    ~OLAWorker();

    bool Init();

    unsigned int DiscoveryInterval() const {
      return m_slp_discovery_interval;
    }

    void RunSLPDiscoveryNow();
    void GetServerInfo();

    // Ownership of the request object is transferred.
    void SendRequest(const IPV4Address &ip,
                     uint16_t endpoint,
                     RDMRequest *request);

    void DiscoveryCallback(bool status, const URLEntries &urls);
    void newSLPServerInfo(bool ok,
                          const ola::e133::SLPThreadServerInfo &server_info);
    void DeviceConnect(const IPV4Address &ip);
    void DeviceDisconnect(const IPV4Address &ip);
    bool DeviceMessage(const IPV4Address &device, uint16_t endpoint,
                       const string &message);

  signals:
    void newSLPDevices(const URLEntries &urls);
    void SLPServerProperties(bool da_enabled, uint16_t port,
                             const QString &scopes,
                             const QString &backend_type);
    void TCPConnect(const IPV4Address &device);
    void TCPDisconnect(const IPV4Address &device);
    void TCPMessage(const IPV4Address &device, uint16_t endpoint,
                    const string &message);
    void UDPStatusMessage(const E133StatusMessage &status_message);
    void UDPRDMMessage(const E133RDMMessage &rdm_message);

    void finished();

  public slots:
    void process();

  private:
    const unsigned int m_slp_discovery_interval;
    const UID m_uid;
    const ola::acn::CID m_cid;
    ola::io::SelectServer m_ss;
    ola::network::UDPSocket m_udp_socket;
    ola::e133::E133Receiver m_e133_receiver;
    ola::e133::MessageBuilder m_message_builder;
    ola::e133::DeviceManager m_device_manager;
    ola::e133::OLASLPThread m_slp_thread;
    ola::SequenceNumber<uint8_t> m_transaction_number;
    // This violates the standard since we use a single sequence # for all
    // devices. Fix this sometime
    ola::SequenceNumber<uint32_t> m_sequence_number;

    void SendRDMRequest(const IPV4Address ip,
                        uint16_t endpoint,
                        RDMRequest *request);

    void HandleStatusMessage(const E133StatusMessage &status_message);
    void HandleRDMMessage(const E133RDMMessage &rdm_message);
};
#endif  // OLATHREAD_H
