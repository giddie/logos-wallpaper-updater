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

#ifdef Q_WS_WIN
#include <windows.h>
#endif


/**
 * Constructor
 */
WallpaperGetter::WallpaperGetter(QObject* parent)
  : QObject(parent),
    mManager(new QNetworkAccessManager()),
    mProgressWidget(new ProgressWidget()),
    mWallpaperDir(QDesktopServices::storageLocation(
                    QDesktopServices::DataLocation))
{
  QRect screen = QApplication::desktop()->screenGeometry();
  QPoint topLeft = screen.center() -
                     QPoint(mProgressWidget->width() / 2,
                            mProgressWidget->height() / 2);
  mProgressWidget->move(topLeft);

  connect(mManager.data(), SIGNAL(finished(QNetworkReply*)),
          this, SLOT(loadingFinished(QNetworkReply*)));
  connect(mManager.data(), SIGNAL(finished(QNetworkReply*)),
          mProgressWidget.data(), SLOT(hide()));
}

/**
 * Destructor
 */
WallpaperGetter::~WallpaperGetter()
{
}

/**
 * Clears the cache, for use when the cached wallpaper is corrupted
 */
void WallpaperGetter::clearCache()
{
  // (This list will just be empty if the directory doesn't exist)
  QStringList entries = mWallpaperDir.entryList(QDir::Files);
  foreach (QString entry, entries) {
    mWallpaperDir.remove(entry);
  }
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
  const double widescreen = 16.0 / 10;
  const double standard = 4.0 / 3;
  bool closerToWidescreen = (qAbs(ratio - widescreen) < qAbs(ratio - standard));

  QString size = closerToWidescreen ? "1280x800" : "1280x960";

  QString filename =
    QString("%1-%2-%3.jpg").arg(month, 2, 10, QChar('0')).arg(year).arg(size);

  QUrl url = "http://www.omships.org/images/desktops/" + filename;
  QFile file(mWallpaperDir.path() + "/" + filename);

  if (file.exists()) {
    if (canSetWallpaper()) {
      setWallpaper(file);
      if (progressReportType == REPORT_WHEN_DONE) {
        reportWallpaperChange();
      }
    }
  } else {
    QNetworkReply* reply = mManager->get(QNetworkRequest(url));
    connect(reply, SIGNAL(downloadProgress(qint64, qint64)),
            mProgressWidget.data(), SLOT(setProgress(qint64, qint64)));

    // If we're going to display a message when done, we tag the reply
    if (progressReportType == REPORT_WHEN_DONE) {
      reply->setProperty("reportWhenDone", true);
    }

    // Show progress window
    if (progressReportType == SHOW_PROGRESS_WIDGET) {
      mProgressWidget->setProgress(0, 1);
      mProgressWidget->show();
      mProgressWidget->raise();
    }
  }
}

/**
 * Convenience slot
 */
void WallpaperGetter::refreshWallpaperQuietly()
{
  refreshWallpaper(REPORT_WHEN_DONE);
}

/**
 * Convenience slot
 */
void WallpaperGetter::refreshWallpaperWithProgress()
{
  refreshWallpaper(SHOW_PROGRESS_WIDGET);
}

/**
 * Called when the wallpaper has finished downloading
 */
void WallpaperGetter::loadingFinished(QNetworkReply* reply)
{
  reply->deleteLater();

  if (reply->error() != QNetworkReply::NoError) {
    reportNetworkError(reply);
    return;
  }

  if (mWallpaperDir.exists()) {
    // Clear out directory contents to avoid it just building
    clearCache();
  } else {
    // Create our cache directory as it doesn't exist
    if (!mWallpaperDir.mkpath(".")) {
      mProgressWidget->
        reportError(tr("Unable to create directory:\n") +
                    mWallpaperDir.path());
      return;
    }
  }

  QString filename =
    reply->url().path().split('/', QString::SkipEmptyParts).last();
  QFile file(mWallpaperDir.path() + '/' + filename);
  if (!file.open(QIODevice::WriteOnly)) {
    mProgressWidget->
      reportError(tr("Unable to write to file:\n") + file.fileName());
    return;
  }
  file.write(reply->readAll());
  file.close();

  if (canSetWallpaper()) {
    setWallpaper(file);
  } else {
    const QString message =
      tr("Your wallpaper has been downloaded to the following directory:\n\n%1"
         "\n\nRegrettably, %2 is not currently able to set the wallpaper on "
         "your platform, so you will have to make your own arrangements for "
         "your wallpaper to be updated when a new image is downloaded.").
        arg(mWallpaperDir.path()).arg(APP_NAME);
    mProgressWidget->reportSuccess(message);
  }

  // Display a message if requested
  if (reply->property("reportWhenDone").toBool()) {
    reportWallpaperChange();
  }
}

/**
 * Reports the given network error using the progress widget
 * @param code Error code to report
 */
void WallpaperGetter::reportNetworkError(const QNetworkReply* reply)
{
  QString errorString;
  switch (reply->error()) {
    case QNetworkReply::ContentNotFoundError:
      errorString = tr("This month's wallpaper could not be found in the "
                       "expected place on the website.  It could be that it "
                       "has not yet been made available.");
      break;
    case QNetworkReply::HostNotFoundError:
      errorString = tr("The website is not available.  Are you sure you're "
                       "connected to the internet?");
      break;
    default:
      errorString = reply->errorString();
      break;
  }
  mProgressWidget->reportError(errorString);
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
  } else if (WINDOWS) {
    // Convert the JPG to BMP (because Windows is stupid)
    QString path = file.fileName();
    QImage image(path);

    // Windows has a specific location set aside for this
    QString dest =
      QDesktopServices::storageLocation(QDesktopServices::HomeLocation) +
      "/Local Settings/Application Data/Microsoft/Wallpaper1.bmp";
    image.save(dest);

    // Set the wallpaper using the Win API
    QByteArray pathByteArray = QDir::toNativeSeparators(dest).toLatin1();
#ifdef Q_WS_WIN
    SystemParametersInfoA(SPI_SETDESKWALLPAPER, 0, (void*)pathByteArray.data(),
                          SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
#endif
  }
  emit wallpaperSet();
}

/**
 * Displays a message to the user telling him/her that the wallpaper has changed
 */
void WallpaperGetter::reportWallpaperChange()
{
  qobject_cast<Application*>(qApp)->
    showTrayMessage(tr("Your wallpaper has been updated."));
}
