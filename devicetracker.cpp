#include <ola/e133/E133URLParser.h>
#include <ola/rdm/UID.h>
#include <ola/stl/STLUtils.h>
#include <QDebug>
#include <set>
#include <vector>
#include "devicetracker.h"

using ola::network::IPV4Address;
using ola::rdm::UID;
using ola::slp::URLEntries;
using std::set;
using std::vector;

DeviceTracker::DeviceTracker() {}

DeviceTracker::~DeviceTracker() {
  ola::STLDeleteElements(&m_devices);
}

void DeviceTracker::UpdateFromSLP(const URLEntries &urls) {
  set<E133Device> new_devices;

  for (const auto& url : urls) {
    ola::rdm::UID uid(0, 0);
    ola::network::IPV4Address ip;
    if (ola::e133::ParseE133URL(url.url(), &uid, &ip)) {
      new_devices.insert(E133Device(ip, uid, url.lifetime()));
    } else {
      qWarning() << "Invalid E1.33 URL: " << url.url().c_str();
    }
  }

  set<E133Device>::const_iterator iter1 = new_devices.begin();
  vector<E133Device*>::iterator iter2 = m_devices.begin();
  int row_delta = 0;

  while (iter1 != new_devices.end() && iter2 != m_devices.end()) {
    const E133Device &new_device = *iter1;
    E133Device &existing_device = **iter2;

    if (existing_device < new_device) {
      delete *iter2;
      iter2 = m_devices.erase(iter2);
      row_delta--;
    } else if (existing_device == new_device) {
      existing_device.uid = new_device.uid;
      existing_device.lifetime = new_device.lifetime;
      iter1++;
      iter2++;
    } else {
      m_devices.insert(iter2, new E133Device(new_device));
      row_delta++;
    }
  }

  // remove any remaining nodes
  while (iter2 != m_devices.end()) {
    delete *iter2;
    row_delta--;
    iter2 = m_devices.erase(iter2);
  }

  // add any remaining items
  while (iter1 != new_devices.end()) {
    m_devices.push_back(new E133Device(*iter1));
    row_delta++;
    iter1++;
  }
  emit dataChanged();
}

void DeviceTracker::MarkAsConnected(const IPV4Address &device) {
  int index = FindDeviceByIP(device);
  if (index >= 0) {
    m_devices[index]->tcp_connected = true;
    emit rowChanged(index, index);
  }
}

void DeviceTracker::MarkAsDisconnected(const IPV4Address &device) {
  int index = FindDeviceByIP(device);
  if (index >= 0) {
    m_devices[index]->tcp_connected = false;
    emit rowChanged(index, index);
  }
}

const E133Device* DeviceTracker::DeviceAt(int i) const {
  if (i < 0 || i >= m_devices.size())
    return NULL;
  return m_devices[i];
}

int DeviceTracker::FindDeviceByIP(const IPV4Address &ip) {
  int start = 0;
  int end = m_devices.size();
  while (start < end) {
    int middle = start + (end - start) / 2;
    const E133Device *device = m_devices[middle];
    if (device->ip_address < ip) {
      start = middle + 1;
    } else if (device->ip_address > ip) {
      end = middle;
    } else {
      return middle;
    }
  }
  return -1;
}
