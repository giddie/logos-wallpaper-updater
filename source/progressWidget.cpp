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

#include "progressWidget.moc"


/**
 * Constructor
 */
ProgressWidget::ProgressWidget(QWidget* parent)
  : QWidget(parent)
{
  this->ui.setupUi(this);
}

/**
 * Destructor
 */
ProgressWidget::~ProgressWidget()
{
}

/**
 * Updates the progress bar
 */
void ProgressWidget::setProgress(qint64 value, qint64 total)
{
  this->ui.progressBar->setMaximum(total);
  this->ui.progressBar->setValue(value);
}

/**
 * Reports an error
 */
void ProgressWidget::reportError(QString errorString)
{
  // This widget may not be showing, in which case we won't show the error
  if (this->isVisible()) {
    QMessageBox::critical(this, tr("Error"),
                          tr("An error occured when trying to set the "
                             "latest wallpaper:\n\n") + errorString);
  }
}
