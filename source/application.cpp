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

#include <QtCore>
#include "application.moc"
#include "defines.h"


/**
 * Constructor
 */
Application::Application(int& argc, char** argv)
  : QApplication(argc, argv), mCurrentWallpaperMonth(0)
{
  QCoreApplication::setOrganizationName("Operation Mobilisation");
  QCoreApplication::setApplicationName("Logos Wallpaper Updater");

  this->setQuitOnLastWindowClosed(false);

  this->mWallpaperGetter = new WallpaperGetter(this);
  connect(this->mWallpaperGetter, SIGNAL(wallpaperSet()),
          this, SLOT(wallpaperSet()));

  this->mTray = new QSystemTrayIcon(NULL);
  if (WINDOWS) {
    mTray->setIcon(QIcon(":trayicon-16.png"));
  } else {
    mTray->setIcon(QIcon(":trayicon-18.png"));
  }
  mTray->setToolTip(QCoreApplication::applicationName());

  this->mTrayMenu = new QMenu(NULL);
  QAction* action;

  action = this->mTrayMenu->addAction(tr("Set wallpaper"));
  connect(action, SIGNAL(triggered(bool)),
          this->mWallpaperGetter, SLOT(refreshWallpaperWithProgress()));

  action = this->mTrayMenu->addAction(tr("Open website"));
  connect(action, SIGNAL(triggered(bool)),
          this, SLOT(openWebsite()));

  action = this->mTrayMenu->addSeparator();

  action = this->mTrayMenu->addAction(tr("Close"));
  connect(action, SIGNAL(triggered(bool)),
          this, SLOT(quit()));

  mTray->setContextMenu(mTrayMenu);
  mTray->show();

  this->mWallpaperGetter->refreshWallpaperWithProgress();
  // Check the month every minute
  this->startTimer(60 * 1000);
}

/**
 * Destructor
 */
Application::~Application()
{
  delete this->mTrayMenu;
  delete this->mTray;
}

/**
 * Displays a message in the system tray
 */
void Application::showTrayMessage(QString title, QString message)
{
  this->mTray->showMessage(title, message);
}

/**
 * Opens the website
 */
void Application::openWebsite()
{
  QDesktopServices::openUrl(QUrl("http://www.logoshope.com"));
}

/**
 * Timer event
 */
void Application::timerEvent(QTimerEvent* event)
{
  // refresh the wallpaper if the month has changed
  int currentMonth = QDate::currentDate().month();
  if (currentMonth != this->mCurrentWallpaperMonth) {
    this->mWallpaperGetter->refreshWallpaperQuietly();
  }
}

/**
 * Called when the wallpaper is updated; remembers which month it corresponds to
 * so we don't check for new wallpaper for the rest of the month.
 */
void Application::wallpaperSet()
{
  this->mCurrentWallpaperMonth = QDate::currentDate().month();
}
