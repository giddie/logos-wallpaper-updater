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

#include "applicationUpdater.moc"
#include "application.h"
#include "defines.h"
#include "versionNumber.h"


/**
 * Constructor
 */
ApplicationUpdater::ApplicationUpdater(QObject* parent)
  : QObject(parent),
    mNextMirrorIndex(0),
    mManager(),
    mUpdateData(),
    mUpdateFileMirrors(
      QStringList() <<
        "http://cloud.github.com/downloads/giddie/logos-wallpaper-updater/"
          "updates.txt" <<
        "http://www.danns.co.uk/webfm_send/57")
{
  connect(&mManager, SIGNAL(finished(QNetworkReply*)),
          this, SLOT(downloadFinished(QNetworkReply*)));

  // Check for new version every hour
  startTimer(60 * 60 * 1000);

  // Check straight away as well
  checkForNewVersion();
}

/**
 * Destructor
 */
ApplicationUpdater::~ApplicationUpdater()
{
}

/**
 * Checks to see if a new version of the application is available.
 */
void ApplicationUpdater::checkForNewVersion()
{
  // Start by trying the first mirror
  mNextMirrorIndex = 0;
  tryNextMirror();
}

/**
 * Parses the update file to see if a newer version is available
 */
void ApplicationUpdater::downloadFinished(QNetworkReply* reply)
{
  if (reply->error() == QNetworkReply::NoError) {
    while (!reply->atEnd()) {
      QString line = QString(reply->readLine());
      QString key = line.section(':', 0, 0).trimmed();
      QString value = line.section(':', 1, -1).trimmed();
      mUpdateData.insert(key, value);
    }
  }

  if (mUpdateData["Application"] == APP_NAME) {
    if (VersionNumber(mUpdateData["Version"]) > VersionNumber(APP_VERSION)) {
      emit newVersionAvailable();
      qobject_cast<Application*>(qApp)->
        showTrayMessage(tr("There is a new version of the "
                           "Logos Wallpaper Updater available."));
    }
  } else {
    // Something's wrong with this update file; try the next mirror.
    tryNextMirror();
  }

  reply->deleteLater();
}

/**
 * Starts upgrading the application
 */
void ApplicationUpdater::startUpdate()
{
  QUrl downloadSite = mUpdateData["DownloadSite"];
  if (!downloadSite.isEmpty()) {
    QDesktopServices::openUrl(downloadSite);
  }
}

/**
 * Timer event
 */
void ApplicationUpdater::timerEvent(QTimerEvent* event)
{
  checkForNewVersion();
}

/**
 * Tries to download the update file from the mirror corresponding to the given
 * index
 */
void ApplicationUpdater::tryNextMirror()
{
  if (mNextMirrorIndex < mUpdateFileMirrors.size()) {
    QString url = mUpdateFileMirrors[mNextMirrorIndex++];
    QNetworkReply* reply = mManager.get(QNetworkRequest(url));
  }
}
