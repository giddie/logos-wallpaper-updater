/**
 * Copyright (c) 2008, Paul Gideon Dann
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "instanceManager.moc"
#include "defines.h"


/**
 * Constructor
 */
InstanceManager::InstanceManager(QString key, QObject* parent)
  : QObject(parent), mKey(key), mReadyReadMapper(0), mServer(0), mSocket(0)
{
  mSharedMemory.setKey(mKey);
  if (mSharedMemory.attach()) {
    // There is another instance running -- connect to it
    mSocket = new QLocalSocket(this);
    connect(mSocket, SIGNAL(connected()), this, SLOT(connected()));
    mSocket->connectToServer(mKey);
  } else {
    if (mSharedMemory.create(1)) {
      // There is no prior instance of this application running
      this->startServer();
    } else {
      qWarning() <<
        QString("Unable to create shared memory with key \"%1\".").arg(mKey);
    }
  }
}

/**
 * Destructor
 */
InstanceManager::~InstanceManager()
{
}

/**
 * Called when a connection to the remote server is successful
 */
void InstanceManager::connected()
{
  connect(mSocket, SIGNAL(disconnected()), mSocket, SLOT(deleteLater()));

  // Ask the remote for its version number
  QTextStream stream(mSocket);
  stream << "version\n";
  stream.flush();
  mSocket->waitForReadyRead();
  QString remoteVersion = stream.readLine();

  // Only the instance with the higher version number can remain
  if (remoteVersion < APP_VERSION) {
    stream << "quit\n";
    stream.flush();
    // The server will reply "ok" when it has closed the server
    mSocket->waitForReadyRead();
    this->startServer();
  } else {
    QTimer::singleShot(0, qApp, SLOT(quit()));
  }

  mSocket->disconnectFromServer();
}

/**
 * Called when a remote client has connected to the server
 */
void InstanceManager::serverConnection()
{
  QLocalSocket* socket = mServer->nextPendingConnection();
  connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));

  connect(socket, SIGNAL(readyRead()), mReadyReadMapper, SLOT(map()));
  mReadyReadMapper->setMapping(socket, socket);
}

/**
 * Called when a remote client is sending data
 */
void InstanceManager::serverReadyRead(QObject* socketObject)
{
  QLocalSocket* socket = qobject_cast<QLocalSocket*>(socketObject);
  QTextStream stream(socket);
  while (!stream.atEnd()) {
    QString line = stream.readLine();
    if (line == "quit") {
      mServer->close();
      stream << "ok\n";
      stream.flush();
      socket->disconnectFromServer();
      QTimer::singleShot(0, qApp, SLOT(quit()));
    } else if (line == "version") {
      stream << APP_VERSION;
      stream.flush();
    }
  }
}

/**
 * Starts a local socket server
 */
void InstanceManager::startServer()
{
  mServer = new QLocalServer(this);

  if (mServer->listen(mKey)) {
    mReadyReadMapper = new QSignalMapper(this);
    connect(mReadyReadMapper, SIGNAL(mapped(QObject*)),
            this, SLOT(serverReadyRead(QObject*)));
    connect(mServer, SIGNAL(newConnection()), this, SLOT(serverConnection()));
  } else {
    qWarning() << "Unable to start local socket server:" <<
                  mServer->errorString();
  }
}
