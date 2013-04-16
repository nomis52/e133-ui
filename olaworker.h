#ifndef OLATHREAD_H
#define OLATHREAD_H

#include <QObject>
#include <memory>
#include <string>
#include <ola/acn/CID.h>
#include <ola/e133/DeviceManager.h>
#include <ola/e133/MessageBuilder.h>
#include <ola/e133/OLASLPThread.h>
#include <ola/io/SelectServer.h>
#include <ola/network/IPv4Address.h>
#include <ola/slp/URLEntry.h>

using std::auto_ptr;
using std::string;
using ola::slp::URLEntries;
using ola::network::IPV4Address;

class OLAWorker : public QObject {
  Q_OBJECT
  public:
    explicit OLAWorker(unsigned int slp_discovery_interval,
                       QObject *parent = 0);
    ~OLAWorker();

    unsigned int DiscoveryInterval() const {
      return m_slp_discovery_interval;
    }

    void RunSLPDiscoveryNow();
    void GetServerInfo();

    void DiscoveryCallback(bool status, const URLEntries &urls);
    void newSLPServerInfo(bool ok,
                          const ola::e133::SLPThreadServerInfo &server_info);
    void DeviceConnect(const IPV4Address &ip);
    void DeviceDisconnect(const IPV4Address &ip);
    bool DeviceMessage(const IPV4Address &device, uint16_t endpoint,
                       const string &message);

  signals:
    void newSLPDevices(const URLEntries &urls);
    void SLPServerProperties(bool da_enabled, uint16_t port, const QString &scopes,
                             const QString &backend_type);
    void TCPConnect(const IPV4Address &device);
    void TCPDisconnect(const IPV4Address &device);
    void TCPMessage(const IPV4Address &device, uint16_t endpoint,
                    const string &message);

    void finished();

  public slots:
    void process();

  private:
    const unsigned int m_slp_discovery_interval;
    const ola::acn::CID m_cid;
    ola::io::SelectServer m_ss;
    ola::e133::MessageBuilder m_message_builder;
    ola::e133::DeviceManager m_device_manager;
    ola::e133::OLASLPThread m_slp_thread;

};
#endif // OLATHREAD_H
