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

// libvorbis
#include <vorbis/vorbisfile.h>

const long long BUFFER_SIZE = 5242880; /** 5 MB size buffer. */
const char *OGG_HEADER = "OggS";  /** Ogg header signature. */

using namespace OGGWrapper;

//----------------------------------------------------------------
OGGExtractor::OGGExtractor(QWidget *parent, Qt::WindowFlags flags)
: QMainWindow    {parent, flags}
, m_cancelProcess{false}
{
  setupUi(this);

  m_cancel->hide();
  m_progress->hide();
  m_progress->setMaximum(100);

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
  startProcess();

  m_filesTable->clearContents();

  auto buffer = new char[BUFFER_SIZE];

  unsigned long long int partialSize = 0;
  unsigned long long int totalSize   = 0;

  for(auto filename: m_containers)
  {
    QFile file(filename);
    totalSize += file.size();
  }

  for(auto filename: m_containers)
  {
    if(m_cancelProcess) break;

    QFile file(filename);
    if(!file.open(QFile::ReadOnly))
    {
      auto error   = tr("Error opening file '%1'").arg(filename);
      auto details = tr("Error: %1").arg(file.errorString());
      errorDialog(error, details);

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
        // check for "OggS" header and flags
        if (buffer[loop] == 0x4F)
        {
          auto position   = file.pos();
          auto seekResult = file.seek(processed + loop);
          unsigned long long readResult = file.read(reinterpret_cast<char *>(&oggHeader[0]), sizeof(oggHeader));
          if(!seekResult || (sizeof(oggHeader) != readResult))
          {
            auto error   = tr("Error scanning file '%1'").arg(filename);
            auto details = tr("ERROR: %1").arg(file.errorString());
            errorDialog(error, details);

            eof = true;
            continue;
          }
          file.seek(position);

          if (0 == (strncmp((const char *) oggHeader, OGG_HEADER, 4)))
          {
            // detected beginning of ogg file
            if (oggHeader[5] == 0x02)
            {
              beginFound = true;
              oggBeginning = processed + loop;
              continue;
            }

            // detected ending of ogg file, more difficult because of trailing frames
            if (beginFound && ((oggHeader[5] == 0x04) || (oggHeader[5] == 0x05)))
            {
              endFound = true;
              oggEnding = processed + loop + 27;

              auto trailingSize   = static_cast<size_t>(oggHeader[26]);
              auto trailingFrames = new char[trailingSize];

              position   = file.pos();
              seekResult = file.seek(oggEnding);
              readResult = file.read(trailingFrames, trailingSize);

              if (!seekResult || (trailingSize != readResult))
              {
                auto error   = tr("I/O error reading input file, probably tried to read past EOF while scanning '%1'").arg(filename);
                auto details = tr("ERROR: %1").arg(file.errorString());
                errorDialog(error, details);

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

              OGGData data;
              data.container = filename;
              data.start     = oggBeginning;
              data.end       = oggEnding;

              m_soundFiles << data;

              insertDataInTable(data);
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

  endProcess();

  m_filesTable->setEnabled(!m_soundFiles.isEmpty());
  m_filesTable->adjustSize();

  m_extract->setEnabled(!m_soundFiles.isEmpty());
}

//----------------------------------------------------------------
void OGGExtractor::extractFiles()
{
  auto destination = QFileDialog::getExistingDirectory(centralWidget(), tr("Select destination directory"), QDir::currentPath());

  if(destination.isEmpty()) return;

  startProcess();

  for(int i = 0; i < m_soundFiles.size() && !m_cancelProcess; ++i)
  {
    m_progress->setValue(100.0*(static_cast<float>(i/m_soundFiles.size())));
    QApplication::processEvents();

    auto data = m_soundFiles.at(i);
    auto widget   = qobject_cast<QWidget *>(m_filesTable->cellWidget(i, 0));
    auto checkBox = qobject_cast<QCheckBox *>(widget->layout()->itemAt(0)->widget());

    if(!checkBox)
    {
      errorDialog(tr("Error extracting file '%1'").arg(i));
      return;
    }

    if(checkBox->isChecked())
    {
      QDir dir(destination);
      QFile file(dir.absoluteFilePath(tr("found_ogg_%1.ogg").arg(i, 6, 10, QChar('0'))));

      if(!file.open(QFile::Truncate|QFile::WriteOnly))
      {
        errorDialog(tr("Couldn't create file '%1'").arg(file.fileName()), tr("Error: %1").arg(file.errorString()));
        return;
      }

      QFile source(data.container);

      if(!source.open(QFile::ReadOnly))
      {
        errorDialog(tr("Couldn't open container file '%1'").arg(data.container), tr("Error: %1").arg(source.errorString()));
        return;
      }

      source.seek(data.start);
      file.write(source.read(data.end-data.start));

      if(!file.flush())
      {
        errorDialog(tr("Error flushing '%1'").arg(file.fileName()), tr("Error: %1").arg(file.errorString()));
        return;
      }

      file.close();
      source.close();
    }
  }

  endProcess();

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

//----------------------------------------------------------------
void OGGExtractor::insertDataInTable(const OGGData& data)
{
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

  auto seconds = oggTime(data);
  auto time    = tr("%1:%2:%3").arg(seconds/3600, 2, 10, QChar('0')).arg(seconds/60, 2, 10, QChar('0')).arg(seconds%60, 2, 10, QChar('0'));
  auto timeLabel = new QLabel{time};
  timeLabel->setAlignment(Qt::AlignCenter);
  m_filesTable->setCellWidget(row,3, timeLabel);

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

//----------------------------------------------------------------
void OGGExtractor::errorDialog(const QString& error, const QString& details) const
{
  QMessageBox dialog;
  dialog.setWindowTitle(tr("Error"));
  dialog.setWindowIcon(QIcon(":/OGGExtractor/application.svg"));
  dialog.setModal(true);
  dialog.setText(error);
  if(!details.isEmpty())
  {
    dialog.setDetailedText(details);
  }
  dialog.exec();
}

//----------------------------------------------------------------
void OGGExtractor::startProcess()
{
  m_cancelProcess = false;

  disconnect(m_containersList->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
             this,                               SLOT(onContainerSelectionChanged()));

  m_addFile->setEnabled(false);
  m_removeFile->setEnabled(false);

  m_progress->setValue(0);
  m_progress->show();
  m_cancel->show();
}

//----------------------------------------------------------------
void OGGExtractor::endProcess()
{
  m_cancelProcess = false;

  connect(m_containersList->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
          this,                               SLOT(onContainerSelectionChanged()));

  m_addFile->setEnabled(true);
  onContainerSelectionChanged();

  m_progress->setValue(0);
  m_progress->hide();
  m_cancel->hide();
}

//----------------------------------------------------------------
unsigned int OGGExtractor::oggTime(const OGGData& data) const
{
  unsigned int result = 0;

  OGGContainerWrapper wrapper{data};
  ov_callbacks callbacks;
  callbacks.read_func  = OGGWrapper::read;
  callbacks.seek_func  = OGGWrapper::seek;
  callbacks.close_func = OGGWrapper::close;
  callbacks.tell_func  = OGGWrapper::tell;

  OggVorbis_File oggFile;

  auto ov_result = ov_open_callbacks(reinterpret_cast<void *>(&wrapper), &oggFile, nullptr, 0, callbacks);

  if(ov_result != 0)
  {
    auto error = tr("Couldn't register OGG callbacks.");
    QString details;
    switch(ov_result)
    {
      case OV_EREAD:
        details = tr("A read from media returned an error.");
        break;
      case OV_ENOTVORBIS:
        details = tr("Bitstream does not contain any Vorbis data.");
        break;
      case OV_EVERSION:
        details = tr("Vorbis version mismatch.");
        break;
      case OV_EBADHEADER:
        details = tr("Invalid Vorbis bitstream header.");
        break;
      case OV_EFAULT:
        details = tr("Internal logic fault; indicates a bug or heap/stack corruption.");
        break;
      default:
        details = tr("Unknown error.");
    }

    errorDialog(error, details);
  }
  else
  {
    result = ov_time_total(&oggFile, -1);
  }

  return result;
}
