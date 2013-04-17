#ifndef DEVICETRACKER_H
#define DEVICETRACKER_H

#include <QModelIndex>
#include <ola/network/IPV4Address.h>
#include <ola/rdm/UID.h>
#include <ola/slp/URLEntry.h>
#include <vector>

using ola::network::IPV4Address;
using ola::rdm::UID;

class E133Device {
  public:
    E133Device(const IPV4Address &ip, const UID &uid, uint16_t lifetime)
        : ip_address(ip),
          uid(uid),
          lifetime(lifetime),
          tcp_connected(false) {
    }

    E133Device(const E133Device &other)
        : ip_address(other.ip_address),
          uid(other.uid),
          lifetime(other.lifetime),
          tcp_connected(false) {
    }

    bool operator==(const E133Device &other) const {
      return ip_address == other.ip_address;
    }

    bool operator<(const E133Device &other) const {
      return ip_address < other.ip_address;
    }

    IPV4Address ip_address;
    UID uid;
    uint16_t lifetime;
    bool tcp_connected;
};

class DeviceTracker : public QObject {
  Q_OBJECT

  public:
    DeviceTracker();
    ~DeviceTracker();

    void UpdateFromSLP(const ola::slp::URLEntries &urls);
    void MarkAsConnected(const IPV4Address &device);
    void MarkAsDisconnected(const IPV4Address &device);

    int NumberOfDevices() const { return m_devices.size(); }
    const E133Device* DeviceAt(int i) const { return m_devices[i]; }

  signals:
    void rowChanged(int first, int last);
    void dataChanged();

  private:
    std::vector<E133Device*> m_devices;

    int FindDeviceByIP(const IPV4Address &ip);
};
#endif  // DEVICETRACKER_H
