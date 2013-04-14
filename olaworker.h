#ifndef OLATHREAD_H
#define OLATHREAD_H

#include <QObject>
#include <memory>
#include <ola/e133/OLASLPThread.h>
#include <ola/io/SelectServer.h>
#include <ola/slp/URLEntry.h>

using std::auto_ptr;
using ola::slp::URLEntries;

class OLAWorker : public QObject {
  Q_OBJECT
  public:
    explicit OLAWorker(unsigned int slp_discovery_interval,
                       QObject *parent = 0);
    ~OLAWorker();

    unsigned int DiscoveryInterval() const {
      return slp_discovery_interval_;
    }

    void RunSLPDiscoveryNow();
    void GetServerInfo();

    void DiscoveryCallback(bool status, const URLEntries &urls);

  signals:
    void newSLPDevices(const URLEntries &urls);
    void finished();

  public slots:
    void process();

  private:
    const unsigned int slp_discovery_interval_;
    auto_ptr<ola::io::SelectServer> ss_;
    auto_ptr<ola::e133::OLASLPThread> slp_thread_;
};
#endif // OLATHREAD_H
