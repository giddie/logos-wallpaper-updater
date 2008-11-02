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

#include <QtGui>
#include "wallpaperGetter.moc"
#include "application.h"
#include "defines.h"


/**
 * Constructor
 */
WallpaperGetter::WallpaperGetter(QObject* parent)
  : QObject(parent)
{
  this->mManager = new QNetworkAccessManager(this);
  this->mProgressWidget = new ProgressWidget(NULL);
  this->mWallpaperDir =
    QDesktopServices::storageLocation(QDesktopServices::DataLocation);

  connect(this->mManager, SIGNAL(finished(QNetworkReply*)),
          this, SLOT(loadingFinished(QNetworkReply*)));
  connect(this->mManager, SIGNAL(finished(QNetworkReply*)),
          this->mProgressWidget, SLOT(hide()));
}

/**
 * Destructor
 */
WallpaperGetter::~WallpaperGetter()
{
  delete this->mProgressWidget;
}

/**
 * Starts downloading this month's wallpaper
 */
void WallpaperGetter::refreshWallpaper(ProgressReportType progressReportType)
{
  int month = QDate::currentDate().month();
  int year = QDate::currentDate().year();

  // Determine screen ratio (widescreen or not)
  QDesktopWidget* desktop = qobject_cast<Application*>(qApp)->desktop();
  int primaryScreen = desktop->primaryScreen();
  QRect screen = desktop->screenGeometry(primaryScreen);
  double ratio = (double)screen.width() / screen.height();
  QString size = (ratio == 1.6) ? "1280x800" : "1280x960";

  QString filename =
    QString("%1-%2-%3.jpg").arg(month, 2, 10, QChar('0')).arg(year).arg(size);

  QUrl url = "http://www.omships.org/images/desktops/" + filename;
  QFile file(this->mWallpaperDir.path() + "/" + filename);

  if (file.exists()) {
    this->setWallpaper(file);
    if (progressReportType == REPORT_WHEN_DONE) {
      this->reportWallpaperChange();
    }
  } else {
    QNetworkReply* reply = this->mManager->get(QNetworkRequest(url));
    connect(reply, SIGNAL(downloadProgress(qint64, qint64)),
            this->mProgressWidget, SLOT(setProgress(qint64, qint64)));

    // If we're going to display a message when done, we tag the reply
    if (progressReportType == REPORT_WHEN_DONE) {
      reply->setProperty("reportWhenDone", true);
    }

    // Show progress window
    if (progressReportType == SHOW_PROGRESS_WIDGET) {
      this->mProgressWidget->setProgress(0, 1);
      this->mProgressWidget->show();
      this->mProgressWidget->raise();
    }
  }
  month++;
}

/**
 * Convenience slot
 */
void WallpaperGetter::refreshWallpaperQuietly()
{
  this->refreshWallpaper(REPORT_WHEN_DONE);
}

/**
 * Convenience slot
 */
void WallpaperGetter::refreshWallpaperWithProgress()
{
  this->refreshWallpaper(SHOW_PROGRESS_WIDGET);
}

/**
 * Called when the wallpaper has finished downloading
 */
void WallpaperGetter::loadingFinished(QNetworkReply* reply)
{
  if (reply->error() == QNetworkReply::NoError) {
    if (!this->mWallpaperDir.exists()) {
      if (!this->mWallpaperDir.mkpath(".")) {
        this->mProgressWidget->
          reportError(tr("Unable to create directory:\n") +
                      this->mWallpaperDir.path());
        return;
      }
    }

    QString filename =
      reply->url().path().split('/', QString::SkipEmptyParts).last();
    QFile file(this->mWallpaperDir.path() + '/' + filename);
    if (!file.open(QIODevice::WriteOnly)) {
      this->mProgressWidget->
        reportError(tr("Unable to write to file:\n") + file.fileName());
      return;
    }
    file.write(reply->readAll());
    file.close();

    this->setWallpaper(file);

    // Display a message if requested
    if (reply->property("reportWhenDone").toBool()) {
      this->reportWallpaperChange();
    }
  } else {
    this->mProgressWidget->reportError(reply->errorString());
  }
  reply->deleteLater();
}

/**
 * Set the wallpaper to the given file
 */
void WallpaperGetter::setWallpaper(QFile& file)
{
  if (MACOS_X) {
    QProcess proc;
    QDir scriptDir(QCoreApplication::applicationDirPath());
    scriptDir.cd("../Resources/Scripts");
    proc.setWorkingDirectory(scriptDir.path());
    proc.start("./setWallpaper", QStringList() << file.fileName());
    proc.waitForFinished();
  } else {
    this->mProgressWidget->
      reportError(tr("This platform is not supported; "
                     "unable to set the wallpaper."));
  }
}

/**
 * Displays a message to the user telling him/her that the wallpaper has changed
 */
void WallpaperGetter::reportWallpaperChange()
{
  qobject_cast<Application*>(qApp)->
    showTrayMessage(tr("Logos Hope Wallpaper"),
                    tr("Your wallpaper has been updated."));
}
