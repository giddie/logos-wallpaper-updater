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
#include "applicationUpdater.h"
#include "instanceManager.h"


/**
 * Constructor
 */
Application::Application(int& argc, char** argv)
  : QApplication(argc, argv),
    mAboutDialog(NULL),
    mHelpDialog(NULL),
    mTray(NULL),
    mTrayMenu(NULL),
    mAppUpgradeActionGroup(NULL),
    mWallpaperGetter(NULL),
    mCurrentWallpaperMonth(0)
{
  QCoreApplication::setOrganizationName("Operation Mobilisation");
  QCoreApplication::setApplicationName("Logos Wallpaper Updater");

  QStringList appArgs = Application::arguments();

  // This object will ensure we only have one running instance
  InstanceManager* instanceManager =
    new InstanceManager("logos-wallpaper-updater", this);
  if (appArgs.size() > 1 && appArgs[1] == "--quit") {
    connect(instanceManager, SIGNAL(singleInstanceAssured()),
            this, SLOT(quit()), Qt::QueuedConnection);
    instanceManager->ensureSingleInstance(InstanceManager::ThisInstanceWins);
  } else {
    instanceManager->ensureSingleInstance(InstanceManager::HighestVersionWins);
  }

  this->setQuitOnLastWindowClosed(false);

  // Application updates
  ApplicationUpdater* appUpdater = new ApplicationUpdater(this);

  // Wallpaper getter
  this->mWallpaperGetter = new WallpaperGetter(this);
  connect(this->mWallpaperGetter, SIGNAL(wallpaperSet()),
          this, SLOT(wallpaperSet()));

  // Dialogs
  this->mAboutDialog = new AboutDialog();
  this->mHelpDialog = new HelpDialog();
  connect(mHelpDialog, SIGNAL(clearCache()),
          mWallpaperGetter, SLOT(clearCache()));

  // System tray menu
  this->mTrayMenu = new QMenu();
  QAction* action;

  action = this->mTrayMenu->addAction(tr("Set wallpaper"));
  connect(action, SIGNAL(triggered(bool)),
          this->mWallpaperGetter, SLOT(refreshWallpaperWithProgress()));

  action = this->mTrayMenu->addAction(tr("Open website"));
  connect(action, SIGNAL(triggered(bool)),
          this, SLOT(openWebsite()));

  this->mAppUpgradeActionGroup = new QActionGroup(this->mTrayMenu);
  this->mAppUpgradeActionGroup->setVisible(false);
  connect(appUpdater, SIGNAL(newVersionAvailable()),
          this, SLOT(unhideAppUpgradeActionGroup()));

  action = this->mTrayMenu->addSeparator();
  action->setActionGroup(this->mAppUpgradeActionGroup);

  action = this->mTrayMenu->addAction(tr("Upgrade this application"));
  action->setActionGroup(this->mAppUpgradeActionGroup);
  connect(action, SIGNAL(triggered(bool)),
          appUpdater, SLOT(startUpdate()));

  action = this->mTrayMenu->addSeparator();

  action = this->mTrayMenu->addAction(tr("Help"));
  connect(action, SIGNAL(triggered(bool)),
          mHelpDialog, SLOT(show()));
  action = this->mTrayMenu->addAction(tr("About"));
  connect(action, SIGNAL(triggered(bool)),
          mAboutDialog, SLOT(show()));
  action = this->mTrayMenu->addAction(tr("Quit"));
  connect(action, SIGNAL(triggered(bool)),
          this, SLOT(quit()));

  // System tray
  this->mTray = new QSystemTrayIcon();
  if (WINDOWS) {
    mTray->setIcon(QIcon(":trayicon-16.png"));
  } else {
    mTray->setIcon(QIcon(":trayicon-18.png"));
  }
  mTray->setToolTip(QCoreApplication::applicationName());

  mTray->setContextMenu(mTrayMenu);
  mTray->show();

  // Set the wallpaper on startup
  this->mWallpaperGetter->refreshWallpaperWithProgress();

  // Check the month every minute (startTimer doesn't work well here)
  QTimer* timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(updateInterval()));
  timer->start(60 * 1000);
}

/**
 * Destructor
 */
Application::~Application()
{
  delete this->mAboutDialog;
  delete this->mHelpDialog;
  delete this->mTrayMenu;
  delete this->mTray;
}

/**
 * Displays a message in the system tray
 */
void Application::showTrayMessage(QString message)
{
  this->mTray->showMessage(tr("Logos Wallpaper Updater"), message);
}

/**
 * Opens the website
 */
void Application::openWebsite()
{
  QDesktopServices::openUrl(QUrl("http://www.logoshope.com"));
}

/**
 * Unhides the menu action that offers to upgrade this application to the latest
 * version
 */
void Application::unhideAppUpgradeActionGroup()
{
  this->mAppUpgradeActionGroup->setVisible(true);
}

/**
 * Update interval
 */
void Application::updateInterval()
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
