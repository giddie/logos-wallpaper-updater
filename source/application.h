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

#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#include <QtGui>

class AboutDialog;
class HelpDialog;
class WallpaperGetter;

class Application : public QApplication
{
  Q_OBJECT

  public:
    Application(int& argc, char* argv[]);
    ~Application();
    void showTrayMessage(QString message);

  private slots:
    void showAboutDialog();
    void showHelpDialog();
    void openWebsite() const;
    void unhideAppUpgradeActionGroup();
    void updateInterval();
    void wallpaperSet();

  private:
    template<class T>
      void showDialog(QPointer<T>* dialogPointer, bool* createdNewPtr = 0);
    QPointer<AboutDialog> mAboutDialog;
    QPointer<HelpDialog> mHelpDialog;
    QScopedPointer<QSystemTrayIcon> mTray;
    QScopedPointer<QMenu> mTrayMenu;
    QActionGroup* mAppUpgradeActionGroup;
    WallpaperGetter* mWallpaperGetter;
    int mCurrentWallpaperMonth;
};

#endif
