//
//          Copyright (c) 2018, Scientific Toolworks, Inc.
//
// This software is licensed under the MIT License. The LICENSE.md file
// describes the conditions under which this software may be distributed.
//
// Author: Jason Haslam
//

#include "GitCredential.h"
#include "conf/Settings.h"
#include <QCoreApplication>
#include <QDir>
#include <QProcess>
#include <QTextStream>
#include <QUrl>

namespace {

QString host(const QString &url)
{
  QString host = QUrl(url).host();
  if (!host.isEmpty())
    return host;

  // Extract hostname from SSH URL.
  int end = url.indexOf(':');
  int begin = url.indexOf('@') + 1;
  return url.mid(begin, end - begin);
}

QString protocol(const QString &url)
{
  QString scheme = QUrl(url).scheme();
  return !scheme.isEmpty() ? scheme : "ssh";
}

} // anon. namespace

GitCredential::GitCredential(const QString &name)
  : mName(name)
{}

bool GitCredential::get(
  const QString &url,
  QString &username,
  QString &password)
{
  QProcess process;
  process.start(command(), {"get"});
  if (!process.waitForStarted())
    return false;

  QTextStream out(&process);
  out << "protocol=" << protocol(url) << endl;
  out << "host=" << host(url) << endl;
  if (!username.isEmpty())
    out << "username=" << username << endl;
  out << endl;

  process.closeWriteChannel();
  process.waitForFinished();

  QString output = process.readAllStandardOutput();
  foreach (const QString &line, output.split('\n')) {
    QStringList components = line.split('=');
    if (components.size() != 2)
      continue;

    QString key = components.first();
    QString value = components.last();
    if (key == "username") {
      username = value;
    } else if (key == "password") {
      password = value;
    }
  }

  return true;
}

bool GitCredential::store(
  const QString &url,
  const QString &username,
  const QString &password)
{
  QProcess process;
  process.start(command(), {"store"});
  if (!process.waitForStarted())
    return false;

  QTextStream out(&process);
  out << "protocol=" << protocol(url) << endl;
  out << "host=" << host(url) << endl;
  out << "username=" << username << endl;
  out << "password=" << password << endl;
  out << endl;

  process.closeWriteChannel();
  process.waitForFinished();

  return true;
}

QString GitCredential::command() const
{
  QDir dir(Settings::helpersDir());
  return dir.filePath(QString("git-credential-%1").arg(mName));
}
