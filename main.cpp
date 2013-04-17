#include <ola/Logging.h>
#include <QApplication>
#include <QDebug>
#include <QThread>
#include "mainwindow.h"
#include "olaworker.h"

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

  MainWindow w(worker);
  thread->start();
  w.show();

  return a.exec();
}
