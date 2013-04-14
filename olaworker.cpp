#include <QDebug>
#include <ola/StringUtils.h>
#include "olaworker.h"

OLAWorker::OLAWorker(unsigned int  slp_discovery_interval,
                     QObject *parent)
    : QObject(parent),
       slp_discovery_interval_( slp_discovery_interval) {
}

OLAWorker::~OLAWorker() {
}

void OLAWorker::RunSLPDiscoveryNow() {
  slp_thread_->RunDeviceDiscoveryNow();
}


void OLAWorker::GetServerInfo() {
  slp_thread_->ServerInfo(
    ola::NewSingleCallback(this, &OLAWorker::newSLPServerInfo));
}

void OLAWorker::DiscoveryCallback(bool status, const URLEntries &urls) {
  if (status) {
    emit newSLPDevices(urls);
  } else {
    qWarning() << "SLP Discovery Failed";
  }
}

void OLAWorker::newSLPServerInfo(bool ok,
                                 const ola::e133::SLPThreadServerInfo &server_info) {
  if (ok) {
    QString scopes(ola::StringJoin(", ", server_info.scopes).c_str());
    emit SLPServerProperties(server_info.da_enabled, server_info.port, scopes,
                             server_info.backend_type.c_str());
  } else {
    qWarning() << "SLP Server Info Failed";
  }
}

void OLAWorker::process() {
  ss_.reset(new ola::io::SelectServer());
  slp_thread_.reset(new ola::e133::OLASLPThread(ss_.get()));

  slp_thread_->SetNewDeviceCallback(
      ola::NewCallback(this, &OLAWorker::DiscoveryCallback));
  if (!slp_thread_->Init()) {
    qWarning() << "Failed to start slp thread";
    return;
  }
  slp_thread_->Start();

  ss_->Run();
  slp_thread_->Join(NULL);
  slp_thread_->Cleanup();

  emit finished();
}
