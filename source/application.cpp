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
#include "aboutDialog.h"
#include "helpDialog.h"
#include "wallpaperGetter.h"
#include "applicationUpdater.h"


/**
 * Constructor
 */
Application::Application(int argc, char* argv[])
  : QApplication(argc, argv),
    mAboutDialog(0),
    mHelpDialog(0),
    mTray(new QSystemTrayIcon()),
    mTrayMenu(new QMenu()),
    mAppUpgradeActionGroup(NULL),
    mWallpaperGetter(NULL),
    mCurrentWallpaperMonth(0)
{
  QCoreApplication::setOrganizationName("Operation Mobilisation");
  QCoreApplication::setApplicationName("Logos Wallpaper Updater");

  setQuitOnLastWindowClosed(false);

  // Application updates
  ApplicationUpdater* appUpdater = new ApplicationUpdater(this);

  // Wallpaper getter
  mWallpaperGetter = new WallpaperGetter(this);
  connect(mWallpaperGetter, SIGNAL(wallpaperSet()),
          this, SLOT(wallpaperSet()));

  // System tray menu
  QAction* action;

  QString actionName;
  if (WallpaperGetter::canSetWallpaper()) {
    actionName = tr("Set wallpaper");
  } else {
    actionName = tr("Get wallpaper");
  }
  action = mTrayMenu->addAction(actionName);
  connect(action, SIGNAL(triggered(bool)),
          mWallpaperGetter, SLOT(refreshWallpaperWithProgress()));

  action = mTrayMenu->addAction(tr("Open website"));
  connect(action, SIGNAL(triggered(bool)),
          this, SLOT(openWebsite()));

  mAppUpgradeActionGroup = new QActionGroup(mTrayMenu.get());
  mAppUpgradeActionGroup->setVisible(false);
  connect(appUpdater, SIGNAL(newVersionAvailable()),
          this, SLOT(unhideAppUpgradeActionGroup()));

  action = mTrayMenu->addSeparator();
  action->setActionGroup(mAppUpgradeActionGroup);

  action = mTrayMenu->addAction(tr("Upgrade this application"));
  action->setActionGroup(mAppUpgradeActionGroup);
  connect(action, SIGNAL(triggered(bool)),
          appUpdater, SLOT(startUpdate()));

  action = mTrayMenu->addSeparator();

  action = mTrayMenu->addAction(tr("Help"));
  connect(action, SIGNAL(triggered(bool)),
          this, SLOT(showHelpDialog()));
  action = mTrayMenu->addAction(tr("About"));
  connect(action, SIGNAL(triggered(bool)),
          this, SLOT(showAboutDialog()));
  action = mTrayMenu->addAction(tr("Quit"));
  connect(action, SIGNAL(triggered(bool)),
          this, SLOT(quit()));

  // System tray
  if (WINDOWS) {
    mTray->setIcon(QIcon(":trayicon-16.png"));
  } else {
    mTray->setIcon(QIcon(":trayicon-18.png"));
  }
  mTray->setToolTip(QCoreApplication::applicationName());

  mTray->setContextMenu(mTrayMenu.get());
  mTray->show();

  // Set the wallpaper on startup
  mWallpaperGetter->refreshWallpaperWithProgress();

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
}

/**
 * Opens a QDialog of type T, or focuses it if it is already open.
 * This method creates a dialog of the specified type, and stores a pointer to
 *   it in the provided QPointer.  When the dialog is destroyed, the QPointer
 *   becomes null.  The dialog is automatically destroyed when it's closed.  If
 *   the method is called and the specified pointer is not null, the dialog is
 *   raised instead.
 * \arg \c dialogPointer Pointer to a QPointer for a QDialog
 * \arg \c createdNew If provided, the bool is set to true if a new dialog was
                      created, false if an old one was raised.
 */
template<class T>
void Application::showDialog(QPointer<T>* dialogPointer, bool* createdNewPtr)
{
  // Ensure that T inherits from QDialog
  const QDialog* check = *dialogPointer;

  QPointer<T>& dialog = *dialogPointer;
  bool createdNew = false;

  if (dialog) {
    dialog->activateWindow();
    dialog->raise();
  } else {
    dialog = new T();
    connect(this, SIGNAL(destroyed()),
            dialog, SLOT(deleteLater()));
    connect(dialog, SIGNAL(finished(int)),
            dialog, SLOT(deleteLater()));
    dialog->show();
    createdNew = true;
  }
  if (createdNewPtr) {
    *createdNewPtr = createdNew;
  }
}

/**
 * Opens the about dialog, or focuses it if it is already open
 */
void Application::showAboutDialog()
{
  showDialog<AboutDialog>(&mAboutDialog);
}

/**
 * Opens the help dialog, or focuses it if it is already open
 */
void Application::showHelpDialog()
{
  bool createdNew;
  showDialog<HelpDialog>(&mHelpDialog, &createdNew);
  if (createdNew) {
    connect(mHelpDialog, SIGNAL(clearCache()),
            mWallpaperGetter, SLOT(clearCache()));
  }
}

/**
 * Displays a message in the system tray
 */
void Application::showTrayMessage(QString message)
{
  mTray->showMessage(tr("Logos Wallpaper Updater"), message);
}

/**
 * Opens the website
 */
void Application::openWebsite() const
{
  QDesktopServices::openUrl(QUrl("http://www.omships.com"));
}

/**
 * Unhides the menu action that offers to upgrade this application to the latest
 * version
 */
void Application::unhideAppUpgradeActionGroup()
{
  mAppUpgradeActionGroup->setVisible(true);
}

/**
 * Update interval
 */
void Application::updateInterval()
{
  // refresh the wallpaper if the month has changed
  int currentMonth = QDate::currentDate().month();
  if (currentMonth != mCurrentWallpaperMonth) {
    mWallpaperGetter->refreshWallpaperQuietly();
  }
}

/**
 * Called when the wallpaper is updated; remembers which month it corresponds to
 * so we don't check for new wallpaper for the rest of the month.
 */
void Application::wallpaperSet()
{
  mCurrentWallpaperMonth = QDate::currentDate().month();
}
