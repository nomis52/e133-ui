#include "mainwindow.h"
#include "olaworker.h"
#include <QApplication>
#include <QDebug>
#include <QThread>
#include <ola/Logging.h>

static const unsigned int DEFAULT_SLP_DISCOVERY_INTERVAL = 60;

int main(int argc, char *argv[]) {
  ola::InitLogging(ola::OLA_LOG_INFO, ola::OLA_LOG_STDERR);
  QApplication a(argc, argv);
  QThread *thread = new QThread;
  OLAWorker *worker = new OLAWorker(DEFAULT_SLP_DISCOVERY_INTERVAL);
  worker->moveToThread(thread);
  QObject::connect(thread, SIGNAL(started()), worker, SLOT(process()));
  QObject::connect(worker, SIGNAL(finished()), thread, SLOT(quit()));
  QObject::connect(worker, SIGNAL(finished()), worker, SLOT(deleteLater()));
  QObject::connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

  thread->start();
  MainWindow w(worker);
  w.show();
    
  return a.exec();
}
