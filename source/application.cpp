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


/**
 * Constructor
 */
Application::Application(int& argc, char** argv)
  : QApplication(argc, argv)
{
  QCoreApplication::setOrganizationName("Operation Mobilisation");
  QCoreApplication::setApplicationName("Logos Monthly");

  this->setQuitOnLastWindowClosed(false);

  this->mWallpaperGetter = new WallpaperGetter(this);
  this->mMonthAtLastCheck = QDate::currentDate().month();

  this->mTray = new QSystemTrayIcon(this);
  mTray->setIcon(QIcon(":trayicon-16.png"));

  this->mTrayMenu = new QMenu(NULL);
  QAction* action;

  action = this->mTrayMenu->addAction(tr("Set wallpaper now"));
  connect(action, SIGNAL(triggered(bool)),
          this->mWallpaperGetter, SLOT(refreshWallpaperWithProgress()));

  action = this->mTrayMenu->addAction(tr("Widescreen"));
  action->setCheckable(true);
  action->setChecked(this->mWallpaperGetter->widescreen());
  connect(action, SIGNAL(triggered(bool)),
          this->mWallpaperGetter, SLOT(setWidescreen(bool)));

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
}

/**
 * Displays a message in the system tray
 */
void Application::showTrayMessage(QString title, QString message)
{
  this->mTray->showMessage(title, message);
}

/**
 * Timer event
 */
void Application::timerEvent(QTimerEvent* event)
{
  int currentMonth = QDate::currentDate().month();
  if (currentMonth != this->mMonthAtLastCheck) {
    this->mWallpaperGetter->refreshWallpaperQuietly();
    this->mMonthAtLastCheck = currentMonth;
  }
}
