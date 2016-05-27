/*
 File: OGGExtractor.cpp
 Created on: 26/05/2016
 Author: Felix de las Pozas Alvarez

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// Project
#include "OGGExtractor.h"
#include "AboutDialog.h"

// Qt
#include <QFileDialog>
#include <QStringListModel>
#include <QMessageBox>
#include <QApplication>
#include <QDebug>

// C++
#include <iostream>

const long long BUFFER_SIZE = 5242880; /** 5 MB size buffer. */
const char *OGG_HEADER = "OggS";  /** Ogg header signature. */

//----------------------------------------------------------------
OGGExtractor::OGGExtractor(QWidget *parent, Qt::WindowFlags flags)
: QMainWindow    {parent, flags}
, m_cancelProcess{false}
{
  setupUi(this);

  m_cancel->hide();
  m_progress->hide();

  m_containersList->setModel(new QStringListModel(m_containers));
  m_containersList->setSelectionMode(QListView::SelectionMode::MultiSelection);

  m_filesTable->setSortingEnabled(false);
  m_filesTable->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
  m_filesTable->horizontalHeader()->setSectionsMovable(false);
  m_filesTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
  m_filesTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
  m_filesTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
  m_filesTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
  m_filesTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);

  connectSignals();
}

//----------------------------------------------------------------
OGGExtractor::~OGGExtractor()
{
}

//----------------------------------------------------------------
void OGGExtractor::connectSignals()
{
  connect(m_addFile,    SIGNAL(pressed()), this, SLOT(onFileAdd()));
  connect(m_removeFile, SIGNAL(pressed()), this, SLOT(onFileRemove()));
  connect(m_about,      SIGNAL(pressed()), this, SLOT(showAboutDialog()));
  connect(m_quit,       SIGNAL(pressed()), this, SLOT(close()));
  connect(m_scan,       SIGNAL(pressed()), this, SLOT(scanContainers()));
  connect(m_extract,    SIGNAL(pressed()), this, SLOT(extractFiles()));
  connect(m_cancel,     SIGNAL(pressed()), this, SLOT(cancelScan()));
  connect(m_size,       SIGNAL(stateChanged(int)), this, SLOT(onSizeStateChange(int)));

  connect(m_containersList->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
          this,                               SLOT(onContainerSelectionChanged()));
}

//----------------------------------------------------------------
void OGGExtractor::onFileAdd()
{
  auto containers = QFileDialog::getOpenFileNames(centralWidget(), tr("Add container files to scan"), QDir::currentPath(), QString(), nullptr, QFileDialog::Option::ReadOnly);

  if(!containers.empty())
  {
    m_containers << containers;

    auto model = qobject_cast<QStringListModel *>(m_containersList->model());
    model->setStringList(m_containers);
  }

  m_scan->setEnabled(!m_containers.isEmpty());
}

//----------------------------------------------------------------
void OGGExtractor::onFileRemove()
{
  auto indexes = m_containersList->selectionModel()->selectedIndexes();

  std::vector<int> positions;
  for(auto index: indexes)
  {
    if(index.isValid())
    {
      auto row = index.row();
      positions.push_back(row);
    }
  }

  auto compare = [](int a, int b) { return a > b; };
  std::sort(positions.begin(), positions.end(), compare);

  while(!positions.empty())
  {
    m_containers.removeAt(positions.front());
    positions.erase(positions.begin());
  }

  auto model = qobject_cast<QStringListModel *>(m_containersList->model());
  model->setStringList(m_containers);

  m_scan->setEnabled(!m_containers.isEmpty());
}

//----------------------------------------------------------------
void OGGExtractor::onContainerSelectionChanged()
{
  auto selection = m_containersList->selectionModel()->selectedIndexes();
  auto valid     = false;

  for(auto index: selection)
  {
    valid |= index.isValid();
  }

  m_removeFile->setEnabled(valid);
}

//----------------------------------------------------------------
void OGGExtractor::showAboutDialog()
{
  AboutDialog dialog;
  dialog.exec();
}

//----------------------------------------------------------------
void OGGExtractor::cancelScan()
{
  m_cancelProcess = true;

  m_cancel->hide();
  m_progress->hide();
}

//----------------------------------------------------------------
void OGGExtractor::scanContainers()
{
  m_cancelProcess = false;

  disconnect(m_containersList->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
             this,                               SLOT(onContainerSelectionChanged()));

  m_addFile->setEnabled(false);
  m_removeFile->setEnabled(false);

  m_filesTable->clearContents();

  auto buffer = new char[BUFFER_SIZE];

  unsigned long long int partialSize = 0;
  unsigned long long int totalSize   = 0;

  for(auto filename: m_containers)
  {
    QFile file(filename);
    totalSize += file.size();
  }

  m_progress->setValue(0);
  m_progress->setMaximum(100);
  m_progress->show();
  m_cancel->show();

  for(auto filename: m_containers)
  {
    if(m_cancelProcess) break;

    QFile file(filename);
    if(!file.open(QFile::ReadOnly))
    {
      QMessageBox dialog;
      dialog.setWindowTitle(tr("Error reading file"));
      dialog.setWindowIcon(QIcon(":/OGGExtractor/application.svg"));
      dialog.setModal(true);
      dialog.setText(tr("Error opening file '%1'").arg(filename));
      dialog.setDetailedText(tr("Error: %2").arg(file.errorString()));

      dialog.exec();
      continue;
    }

    long long processed             = 0;
    unsigned long long oggBeginning = 0;
    unsigned long long oggEnding    = 0;

    bool beginFound = false;
    bool endFound   = false;
    bool eof        = false;

    unsigned char oggHeader[27];

    while (!eof)
    {
      file.seek(processed);

      auto bytesRead = file.read(buffer, BUFFER_SIZE);

      for (long long loop = 0; loop < bytesRead && !eof && !m_cancelProcess; ++loop)
      {
        /* check for "OggS" header and flags */
        if (buffer[loop] == 0x4F)
        {
          auto position   = file.pos();
          auto seekResult = file.seek(processed + loop);
          unsigned long long readResult = file.read(reinterpret_cast<char *>(&oggHeader[0]), sizeof(oggHeader));
          if(!seekResult || (sizeof(oggHeader) != readResult))
          {
            QMessageBox dialog;
            dialog.setWindowTitle(tr("Error reading file"));
            dialog.setWindowIcon(QIcon(":/OGGExtractor/application.svg"));
            dialog.setModal(true);
            dialog.setText(tr("Error scanning file '%1'").arg(filename));
            dialog.exec();

            eof = true;
            continue;
          }
          file.seek(position);

          if (0 == (strncmp((const char *) oggHeader, OGG_HEADER, 4)))
          {
            /* detected beginning of ogg file */
            if (oggHeader[5] == 0x02)
            {
              beginFound = true;
              oggBeginning = processed + loop;
              continue;
            }

            /* detected ending of ogg file, more difficult because of trailing frames */
            if (beginFound && ((oggHeader[5] == 0x04) || (oggHeader[5] == 0x05)))
            {
              endFound = true;
              oggEnding = processed + loop + 27;

              /* we need to do this because we can be at the very end */
              /* of the buffer and don't want to look outside it      */
              auto trailingSize = static_cast<size_t>(oggHeader[26]);

              auto trailingFrames = new char[trailingSize];

              position   = file.pos();
              seekResult = file.seek(oggEnding);
              readResult = file.read(trailingFrames, trailingSize);

              if (!seekResult || (trailingSize != readResult))
              {
                QMessageBox dialog;
                dialog.setWindowTitle(tr("Error reading file"));
                dialog.setWindowIcon(QIcon(":/OGGExtractor/application.svg"));
                dialog.setModal(true);
                dialog.setText(tr("I/O error reading input file, probably tried to read past EOF while scanning '%1'").arg(filename));
                dialog.setDetailedText(tr("ERROR: %1").arg(file.errorString()));
                dialog.exec();

                delete [] trailingFrames;

                eof = true;
                continue;
              }
              file.seek(position);

              oggEnding += (unsigned long long) oggHeader[26];

              for (unsigned long loop2 = 0; loop2 < (unsigned long) oggHeader[26]; loop2++)
              {
                oggEnding += (unsigned long long) trailingFrames[loop2];
              }

              delete [] trailingFrames;
            }

            /* every beginning has an end ;-) */
            if ((beginFound == true) && (endFound == true))
            {
              beginFound = false;
              endFound = false;

              long long size = oggEnding - oggBeginning;
              if (size < (m_minimum->value() * 1024))
              {
                // skip file
                continue;
              }

              ogg_data data;
              data.start     = oggBeginning;
              data.end       = oggEnding;
              data.container = filename;

              m_soundFiles << data;

              auto row = m_soundFiles.size() - 1;
              m_filesTable->insertRow(row);

              auto widget = new QWidget();
              auto checkBox = new QCheckBox();
              checkBox->setChecked(true);

              connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(checkSelectedFiles()));
              auto layout = new QHBoxLayout(widget);
              layout->addWidget(checkBox);
              layout->setAlignment(Qt::AlignCenter);
              layout->setContentsMargins(0,0,0,0);
              widget->setLayout(layout);
              m_filesTable->setCellWidget(row,0,widget);

              auto name = new QLabel(tr("found_ogg_%1").arg(row, 6, 10, QChar('0')));
              name->setAlignment(Qt::AlignCenter);
              m_filesTable->setCellWidget(row,1, name);

              auto containerName = data.container.split('/').last().split('.').first();
              auto containerWidget = new QLabel(containerName);
              containerWidget->setAlignment(Qt::AlignCenter);
              m_filesTable->setCellWidget(row,2, containerWidget);

              m_filesTable->setCellWidget(row,3, new QLabel("HH:MM:SS"));

              widget = new QWidget();
              auto button = new QPushButton(QIcon(":/OGGExtractor/play.svg"), "");
              button->setFixedSize(24,24);
              layout = new QHBoxLayout(widget);
              layout->addWidget(button);
              layout->setAlignment(Qt::AlignCenter);
              layout->setContentsMargins(0,0,0,0);
              widget->setLayout(layout);
              m_filesTable->setCellWidget(row,4,widget);

            }
          }
        }
      }

      processed += bytesRead;
      partialSize += bytesRead;

      if (bytesRead < BUFFER_SIZE) eof = true;

      auto value = 100.0*(static_cast<double>(partialSize)/totalSize);
      m_progress->setValue(value);
      QApplication::processEvents();
    }
  }

  delete [] buffer;

  m_progress->hide();
  m_progress->setValue(0);
  m_cancel->hide();

  connect(m_containersList->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
          this,                               SLOT(onContainerSelectionChanged()));

  m_addFile->setEnabled(true);
  onContainerSelectionChanged();

  m_filesTable->setEnabled(!m_soundFiles.isEmpty());
  m_filesTable->adjustSize();

  m_extract->setEnabled(!m_soundFiles.isEmpty());
}

//----------------------------------------------------------------
void OGGExtractor::extractFiles()
{
  auto destination = QFileDialog::getExistingDirectory(centralWidget(), tr("Select destination directory"), QDir::currentPath());
  if(destination.isEmpty()) return;

  m_cancelProcess = false;
  m_progress->setValue(0);
  m_progress->show();
  m_cancel->show();

  for(int i = 0; i < m_soundFiles.size() && !m_cancelProcess; ++i)
  {
    m_progress->setValue(100.0*(static_cast<float>(i/m_soundFiles.size())));
    QApplication::processEvents();

    auto data = m_soundFiles.at(i);
    auto widget   = qobject_cast<QWidget *>(m_filesTable->cellWidget(i, 0));
    auto checkBox = qobject_cast<QCheckBox *>(widget->layout()->itemAt(0)->widget());

    if(!checkBox)
    {
      QMessageBox dialog;
      dialog.setWindowTitle(tr("Error extracting OGG file"));
      dialog.setWindowIcon(QIcon(":/OGGExtractor/application.svg"));
      dialog.setModal(true);
      dialog.setText(tr("Error extracting file '%1'").arg(i));
      dialog.exec();

      return;
    }

    if(checkBox->isChecked())
    {
      QDir dir(destination);
      QFile file(dir.absoluteFilePath(tr("found_ogg_%1.ogg").arg(i, 6, 10, QChar('0'))));

      if(!file.open(QFile::Truncate|QFile::WriteOnly))
      {
        QMessageBox dialog;
        dialog.setWindowTitle(tr("Error extracting OGG file"));
        dialog.setWindowIcon(QIcon(":/OGGExtractor/application.svg"));
        dialog.setModal(true);
        dialog.setText(tr("Couldn't create file '%1'").arg(file.fileName()));
        dialog.setDetailedText(tr("Error: %1").arg(file.errorString()));
        dialog.exec();

        return;
      }

      QFile source(data.container);

      if(!source.open(QFile::ReadOnly))
      {
        QMessageBox dialog;
        dialog.setWindowTitle(tr("Error extracting OGG file"));
        dialog.setWindowIcon(QIcon(":/OGGExtractor/application.svg"));
        dialog.setModal(true);
        dialog.setText(tr("Couldn't open container file '%1'").arg(data.container));
        dialog.setDetailedText(tr("Error: %1").arg(source.errorString()));
        dialog.exec();

        return;
      }

      source.seek(data.start);
      file.write(source.read(data.end-data.start));

      if(!file.flush())
      {
        QMessageBox dialog;
        dialog.setWindowTitle(tr("Error extracting OGG file"));
        dialog.setWindowIcon(QIcon(":/OGGExtractor/application.svg"));
        dialog.setModal(true);
        dialog.setText(tr("Error flushing '%1'").arg(file.fileName()));
        dialog.setDetailedText(tr("Error: %1").arg(file.errorString()));
        dialog.exec();

        return;
      }

      file.close();
      source.close();
    }
  }

  m_cancelProcess = false;
  m_progress->setValue(0);
  m_progress->hide();
  m_cancel->hide();
  QApplication::processEvents();
}

//----------------------------------------------------------------
void OGGExtractor::onSizeStateChange(int value)
{
  m_minimum->setEnabled(value == Qt::Checked);
}

//----------------------------------------------------------------
void OGGExtractor::checkSelectedFiles()
{
  bool enable = false;
  for(int i = 0; i < m_soundFiles.size(); ++i)
  {
    auto widget   = qobject_cast<QWidget *>(m_filesTable->cellWidget(i, 0));
    auto checkBox = qobject_cast<QCheckBox *>(widget->layout()->itemAt(0)->widget());

    if(checkBox && checkBox->isChecked())
    {
      enable = true;
      break;
    }
  }

  m_extract->setEnabled(enable);
}
