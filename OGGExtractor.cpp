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
#include <AboutDialog.h>
#include <OGGExtractor.h>
#include <TableModel.h>

// libvorbis
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

// Qt
#include <QApplication>
#include <QAbstractItemModel>
#include <QAudioOutput>
#include <QBuffer>
#include <QCheckBox>
#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QMessageBox>
#include <QProgressBar>
#include <QToolButton>
#include <QSpinBox>
#include <QStringListModel>
#include <QTableWidget>
#include <QToolButton>
#ifdef Q_OS_WIN
#include <QtWinExtras/QWinTaskbarButton>
#include <QtWinExtras/QWinTaskbarProgress>
#endif
#include <QDebug>

using namespace OGGWrapper;

//----------------------------------------------------------------
OGGExtractor::OGGExtractor(QWidget *parent, Qt::WindowFlags flags)
: QMainWindow     (parent, flags)
, m_cancelProcess {false}
, m_volume        {1.0}
, m_playButton    {nullptr}
, m_sample        {nullptr}
, m_buffer        {nullptr}
, m_audio         {nullptr}
#ifdef Q_OS_WIN
, m_taskBarButton {nullptr}
#endif
, m_thread        {nullptr}
{
  setupUi(this);

  m_cancel->setEnabled(false);
  m_progress->setMaximum(100);
  m_progress->setValue(0);
  m_progress->setEnabled(false);

  m_containersList->setModel(new QStringListModel(m_containers));
  m_containersList->setSelectionMode(QListView::SelectionMode::MultiSelection);

  m_tableModel = new TableModel(m_filesTable);
  m_filesTable->setModel(m_tableModel);

  m_filesTable->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
  m_filesTable->horizontalHeader()->setSectionsMovable(false);
  m_filesTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
  m_filesTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
  m_filesTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
  m_filesTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
  m_filesTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
  m_filesTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);
  m_filesTable->horizontalHeader()->setSectionResizeMode(6, QHeaderView::ResizeToContents);
  m_filesTable->horizontalHeader()->setSectionResizeMode(7, QHeaderView::Stretch);

  m_previous->setEnabled(false);
  m_next->setEnabled(false);
  m_pageCount->setText("");
  m_pageCount->setEnabled(false);

  connectSignals();

  setMinimumWidth(1000);
  setMinimumHeight(800);
}

//----------------------------------------------------------------
OGGExtractor::~OGGExtractor()
{
  stopBuffer();
  m_tableModel->clearModel();
  m_soundFiles.clear();
  m_soundSelected.clear();
  insertWidgetsInTable();
}

//----------------------------------------------------------------
void OGGExtractor::connectSignals()
{
  auto selectionModel = m_containersList->selectionModel();
  connect(selectionModel, SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
          this,           SLOT(onContainerSelectionChanged()));

  connect(m_addFile,      SIGNAL(pressed()),
          this,           SLOT(onFileAdd()));

  connect(m_removeFile,   SIGNAL(pressed()),
          this,           SLOT(onFileRemove()));

  connect(m_about,        SIGNAL(pressed()),
          this,           SLOT(showAboutDialog()));

  connect(m_quit,         SIGNAL(pressed()),
          this,           SLOT(close()));

  connect(m_scan,         SIGNAL(pressed()),
          this,           SLOT(scanContainers()));

  connect(m_extract,      SIGNAL(pressed()),
          this,           SLOT(extractFiles()));

  connect(m_cancel,       SIGNAL(pressed()),
          this,           SLOT(cancelScan()));

  connect(m_size,         SIGNAL(stateChanged(int)),
          this,           SLOT(onSizeStateChange(int)));

  connect(m_time,         SIGNAL(stateChanged(int)),
          this,           SLOT(onTimeStateChange(int)));

  connect(m_volumeSlider, SIGNAL(valueChanged(int)),
          this,           SLOT(onVolumeChanged(int)));

  connect(m_previous,     SIGNAL(clicked()), 
          this,           SLOT(onMovementButtonClicked()));

  connect(m_next,         SIGNAL(clicked()), 
          this,           SLOT(onMovementButtonClicked()));
}

//----------------------------------------------------------------
void OGGExtractor::onFileAdd()
{
  static auto path = QDir::currentPath();
  auto containers  = QFileDialog::getOpenFileNames(centralWidget(),
                                                   tr("Add container files to scan"),
                                                   path,
                                                   QString(),
                                                   nullptr,
                                                   QFileDialog::Option::ReadOnly);

  if(!containers.empty())
  {
    m_containers << containers;

    auto model = qobject_cast<QStringListModel *>(m_containersList->model());
    model->setStringList(m_containers);

    QFileInfo file{containers.first()};
    path = file.absolutePath();
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

  m_removeFile->setEnabled(!m_containers.isEmpty());

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
  AboutDialog dialog{centralWidget()};
  dialog.exec();
}

//----------------------------------------------------------------
void OGGExtractor::cancelScan()
{
  if(m_thread)
    m_thread->abort();

  m_cancelProcess = true;

  m_cancel->setEnabled(false);
  m_progress->setEnabled(false);
  m_progress->setFormat("%p%");
  m_progress->setValue(0);
  #ifdef Q_OS_WIN
  m_taskBarButton->progress()->setValue(0);
  #endif
}

//----------------------------------------------------------------
void OGGExtractor::scanContainers()
{
  startProcess();
  m_scan->setEnabled(false);
  m_extract->setEnabled(false);

  stopBuffer();

  m_tableModel->clearModel();
  m_soundFiles.clear();
  m_soundSelected.clear();

  if(m_thread)
  {
    m_thread->abort();
    m_thread->thread()->wait();
  }

  m_thread = std::make_shared<ScanThread>(m_containers, this);

  if(m_size->isChecked())
  {
    m_thread->setMinimumStreamDuration(m_minimumSize->value());
  }

  if(m_time->isChecked())
  {
    m_thread->setMinimumStreamDuration(m_minimumTime->value());
  }

  m_progress->setFormat("Scanning... %p%");
  connect(m_thread.get(), SIGNAL(progress(int)), this, SLOT(onProgressSignaled(int)));
  connect(m_thread.get(), SIGNAL(error(const QString, const QString)), this, SLOT(onErrorSignaled(const QString, const QString)));
  connect(m_thread.get(), SIGNAL(finished()), this, SLOT(onThreadFinished()));

  m_thread->start();
}

//----------------------------------------------------------------
void OGGExtractor::extractFiles()
{
  auto destination = QFileDialog::getExistingDirectory(centralWidget(), tr("Select destination directory"), QDir::currentPath());

  if(destination.isEmpty()) return;

  startProcess();

  QApplication::setOverrideCursor(Qt::WaitCursor);

  int progressValue = 0;
  m_progress->setFormat("Extracting selected files... %p%");
  for(unsigned int i = 0; i < m_soundFiles.size() && !m_cancelProcess; ++i)
  {
    const int currentProgress = 100.0*(static_cast<float>(i/m_soundFiles.size()));
    if(progressValue != currentProgress)
    {
      progressValue = currentProgress;
      m_progress->setValue(currentProgress);
      #ifdef Q_OS_WIN
      m_taskBarButton->progress()->setValue(currentProgress);
      #endif
    }

    const auto data = m_soundFiles.at(i);
    const auto isChecked = m_soundSelected.at(i);
    if(isChecked && data.error.empty())
    {
      auto name = m_tableModel->dataDisplayRole(data, 1).toString(); // Filename.

      // play safe with names, only common characters to avoid unicode.
      name = name.replace(QRegExp("[^a-zA-Z0-9_- ]"),QString(""));

      QDir dir(destination);
      QFile file(dir.absoluteFilePath(tr("%1.ogg").arg(name)));

      if(!file.open(QFile::Truncate|QFile::WriteOnly))
      {
        errorDialog(tr("Couldn't create file '%1'").arg(file.fileName()), tr("Error: %1").arg(file.errorString()));
        continue;
      }

      const auto qContainer = QString::fromStdWString(data.container);
      QFile source(qContainer);

      if(!source.open(QFile::ReadOnly))
      {
        errorDialog(tr("Couldn't open container file '%1'").arg(qContainer), tr("Error: %1").arg(source.errorString()));
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
  m_progress->setFormat("%p%");

  QApplication::restoreOverrideCursor();
}

//----------------------------------------------------------------
void OGGExtractor::onSizeStateChange(int value)
{
  m_minimumSize->setEnabled(value == Qt::Checked);
}

//----------------------------------------------------------------
void OGGExtractor::onTimeStateChange(int value)
{
  m_minimumTime->setEnabled(value == Qt::Checked);
}

//----------------------------------------------------------------
void OGGExtractor::checkSelectedFiles()
{
  auto it = std::find_if(m_soundSelected.cbegin(), m_soundSelected.cend(), [](const bool &val) { return val; });
  m_extract->setEnabled(it != m_soundSelected.cend());
}

//----------------------------------------------------------------
void OGGExtractor::insertWidgetsInTable()
{
  m_filesTable->setUpdatesEnabled(false);

  int progressValue = 0;
  m_progress->setFormat("Inserting data... %p%");
  const auto rowsNum = m_tableModel->pageItems();
  
  for (unsigned int i = 0; i < rowsNum; ++i)
  {
    auto checkBoxWidget = new QWidget();
    auto layoutCheckBox = new QHBoxLayout(checkBoxWidget);
    auto checkBox = new QCheckBox();
    layoutCheckBox->addWidget(checkBox);
    layoutCheckBox->setAlignment(Qt::AlignCenter);
    layoutCheckBox->setContentsMargins(0,0,0,0);
    checkBox->setProperty("Index", i);
    checkBox->setChecked(m_soundSelected[i + (m_tableModel->page() * m_tableModel->pageSize())]);
    connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(onCheckboxModified()));
    m_filesTable->setIndexWidget(m_tableModel->index(i, 0), checkBoxWidget);

    auto button = new QToolButton(m_filesTable);
    button->setIcon(QIcon(":/OGGExtractor/play.svg"));
    button->setProperty("Index", i);
    button->resize(32, 32);
    connect(button, SIGNAL(clicked()), this, SLOT(onPlayButtonPressed()));
    m_filesTable->setIndexWidget(m_tableModel->index(i, 6), button);
    int currentProgress = static_cast<float>(i * 100) / rowsNum;
    if (currentProgress != progressValue)
    {
      progressValue = currentProgress;
      m_progress->setValue(progressValue);
    }
  }

  m_progress->setFormat("%p%");
  m_progress->setValue(0);
  m_filesTable->setUpdatesEnabled(true);
}

//----------------------------------------------------------------
void OGGExtractor::removeWidgetsFromTable()
{
  const auto rowsNum = m_tableModel->pageItems();
  for (unsigned int i = 0; i < rowsNum; ++i)
  {
    for(const auto idx: {0,6})
    {
      auto widget = m_filesTable->indexWidget(m_tableModel->index(idx, 0));
      if(widget)
        delete widget;
    }
  }
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
  m_progress->setEnabled(true);
  #ifdef Q_OS_WIN
  m_taskBarButton->progress()->setValue(0);
  m_taskBarButton->progress()->setVisible(true);
  #endif
  m_cancel->setEnabled(true);

  m_next->setEnabled(false);
  m_previous->setEnabled(false);
  m_pageCount->setText("");
}

//----------------------------------------------------------------
void OGGExtractor::endProcess()
{
  m_cancelProcess = false;

  connect(m_containersList->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
          this,                               SLOT(onContainerSelectionChanged()));

  m_addFile->setEnabled(true);
  onContainerSelectionChanged();

  m_progress->setEnabled(false);
  m_progress->setFormat("%p%");
  m_progress->setValue(0);
  #ifdef Q_OS_WIN
  m_taskBarButton->progress()->setValue(0);
  m_taskBarButton->progress()->setVisible(false);
  #endif

  m_streamsCount->setText(tr("%1").arg(m_soundFiles.size()));
}

//----------------------------------------------------------------
void OGGExtractor::onPlayButtonPressed()
{
  auto button = qobject_cast<QToolButton *>(sender());
  if(!button)
    return;

  bool ok = false;
  const auto buttonIdx = button->property("Index").toUInt(&ok);
  const auto index = buttonIdx + (m_tableModel->page() * m_tableModel->pageSize());
  if(ok && index >= 0 && index < m_soundFiles.size())
  {
    if(button == m_playButton)
    {
      stopBuffer();
      return;
    }

    // stop current playing sample, if any.
    stopBuffer();

    button->setIcon(QIcon(":/OGGExtractor/stop.svg"));

    m_playButton = button;

    auto data = m_soundFiles.at(index);
    m_sample = decodeOGG(data);

    if(!m_sample && !data.error.empty())
    {
      m_tableModel->setData(m_tableModel->index(buttonIdx, 7), QString::fromStdString(data.error), Qt::DisplayRole);
      m_filesTable->repaint();
      button->setIcon(QIcon(":/OGGExtractor/play.svg"));
    }
    else
    {
      playBufffer(m_sample, data);
    }
  }
}

//----------------------------------------------------------------
void OGGExtractor::onCheckboxModified()
{
  auto checkbox = qobject_cast<QCheckBox *>(sender());
  if(checkbox)
  {
    bool ok = false;
    auto index = checkbox->property("Index").toUInt(&ok) + (m_tableModel->page() * m_tableModel->pageSize());
    if(ok && index < m_soundFiles.size())
      m_soundSelected[index] = checkbox->isChecked();

    checkSelectedFiles();
  }
}

//----------------------------------------------------------------
std::shared_ptr<QByteArray> OGGExtractor::decodeOGG(OGGData& data)
{
  std::shared_ptr<QByteArray> result = nullptr;
  if(!data.error.empty()) return result;

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
    const auto error = tr("Couldn't register OGG callbacks.");

    switch(ov_result)
    {
      case OV_EREAD:
        data.error = std::string("A read from media returned an error.");
        break;
      case OV_ENOTVORBIS:
        data.error = std::string("Bitstream does not contain any Vorbis data.");
        break;
      case OV_EVERSION:
        data.error = std::string("Vorbis version mismatch.");
        break;
      case OV_EBADHEADER:
        data.error = std::string("Invalid Vorbis bitstream header.");
        break;
      case OV_EFAULT:
        data.error = std::string("Internal logic fault; indicates a bug or heap/stack corruption.");
        break;
      default:
        data.error = std::string("Unknown error.");
    }

    errorDialog(error, QString::fromStdString(data.error));

    return result;
  }

  const long decodeSize = ov_pcm_total(&oggFile,-1) * 4 + 256;

  result = std::make_shared<QByteArray>(decodeSize, 0);

  int section = 0;
  bool eof = false;
  auto ptr = result->data();
  while (!eof)
  {
    auto decoded = ov_read(&oggFile, ptr, decodeSize-(ptr-result->data()), 0, 2, 1, &section);
    if(decoded == 0)
    {
      eof = true;
    }
    else
    {
      if(decoded < 0)
      {
        const auto errorMessage = tr("Error during OGG file decoding.");
        data.error = errorMessage.toStdString();
        errorDialog(errorMessage);
        result = nullptr;
        break;
      }
      else
      {
        ptr += decoded;
      }
    }
  }

  ov_clear(&oggFile);

  return result;
}

//----------------------------------------------------------------
void OGGExtractor::showEvent(QShowEvent* e)
{
  QMainWindow::showEvent(e);

  #ifdef Q_OS_WIN
  m_taskBarButton = new QWinTaskbarButton(this);
  m_taskBarButton->setWindow(this->windowHandle());
  m_taskBarButton->progress()->setRange(0,100);
  m_taskBarButton->progress()->setVisible(false);
  #endif
}

//----------------------------------------------------------------
void OGGExtractor::playBufffer(std::shared_ptr<QByteArray> pcmBuffer, const OGGData &data)
{
  QAudioFormat format;
  format.setChannelCount(data.channels);
  format.setSampleRate(data.rate);
  format.setByteOrder(QAudioFormat::LittleEndian);
  format.setSampleSize(16);
  format.setCodec("audio/pcm");
  format.setSampleType(QAudioFormat::SampleType::SignedInt);

  m_buffer = std::make_shared<QBuffer>(pcmBuffer.get());
  m_buffer->open(QIODevice::ReadOnly);
  m_buffer->seek(0);

  QAudioDeviceInfo audioinfo(QAudioDeviceInfo::defaultOutputDevice());
  if (!audioinfo.isFormatSupported(format))
  {
    errorDialog(tr("Raw audio format not supported, cannot play audio."));
    stopBuffer();
    return;
  }

  m_audio = std::make_shared<QAudioOutput>(format, this);
  m_audio->setVolume(m_volume);
  m_audio->setNotifyInterval(100);
  m_audio->setProperty("Duration", data.duration);

  m_progress->setFormat("Playing... %p%");

  m_audio->start(m_buffer.get());

  connect(m_audio.get(), SIGNAL(stateChanged(QAudio::State)), this, SLOT(stopBuffer()));
  connect(m_audio.get(), SIGNAL(notify()), this, SLOT(onAudioNotify()));
}

//----------------------------------------------------------------
void OGGExtractor::stopBuffer()
{
  if(!m_audio || !m_playButton) return;

  if(m_playButton)
    m_playButton->setIcon(QIcon(":/OGGExtractor/play.svg"));

  m_playButton = nullptr;

  if (m_audio)
  {
    m_audio->stop();
    disconnect(m_audio.get(), SIGNAL(stateChanged(QAudio::State)), this, SLOT(stopBuffer()));
  }

  m_audio = nullptr;
  m_buffer = nullptr;
  m_sample = nullptr;

  m_progress->setFormat("%p%");
  m_progress->setValue(0);
}

//----------------------------------------------------------------
void OGGExtractor::onVolumeChanged(int value)
{
  if(m_volume == value) return;

  m_volume = value/100.0;
  m_volumeLabel->setText(tr("%1%").arg(value));

  if(m_audio)
    m_audio->setVolume(m_volume);
}

//----------------------------------------------------------------
void OGGExtractor::onThreadFinished()
{
  QApplication::setOverrideCursor(Qt::WaitCursor);

  m_cancel->setEnabled(false);

  if(m_thread && !m_thread->isAborted())
  {
    m_soundFiles = m_thread->streams();
    m_soundSelected.resize(m_soundFiles.size(), true);
  }

  m_tableModel->setModelData(m_soundFiles);
  insertWidgetsInTable();

  if(!m_soundFiles.empty())
  {
    const auto pageSize = m_tableModel->pageSize();
    m_previous->setEnabled(false);
    m_next->setEnabled(m_soundFiles.size() > pageSize);
    m_pageCount->setText(QString("%1 of %2").arg(m_tableModel->page()+1).arg(m_tableModel->maxPage()));
    m_pageCount->setEnabled(true);
  }

  m_scan->setEnabled(true);
  endProcess();

  m_filesTable->setEnabled(!m_soundFiles.empty());
  m_extract->setEnabled(!m_soundFiles.empty());

  m_thread = nullptr;

  QApplication::restoreOverrideCursor();
}

//----------------------------------------------------------------
void OGGExtractor::onProgressSignaled(int value)
{
  if(value == m_progress->value())
    return;

  m_progress->setValue(value);
  #ifdef Q_OS_WIN
  m_taskBarButton->progress()->setValue(value);
  #endif

  auto task = qobject_cast<ScanThread *>(sender());
  if(task)
    m_streamsCount->setText(tr("%1").arg(task->streamsNumber()));
}

//----------------------------------------------------------------
void OGGExtractor::onMovementButtonClicked()
{
  stopBuffer();

  auto button = qobject_cast<QToolButton *>(sender());
  if(button)
  {
    const auto nextPage = m_tableModel->page() + (button == m_next ? 1:-1);
    removeWidgetsFromTable();
    m_tableModel->setPage(nextPage);
    const auto currentPage = m_tableModel->page();
    m_previous->setEnabled(currentPage != 0);
    m_next->setEnabled(currentPage != m_tableModel->maxPage());
    m_pageCount->setText(QString("%1 of %2").arg(currentPage + 1).arg(m_tableModel->maxPage()));
    insertWidgetsInTable();
    m_filesTable->scrollTo(m_tableModel->index(0,0), QTableView::ScrollHint::EnsureVisible);
  }
}

//----------------------------------------------------------------
void OGGExtractor::onAudioNotify()
{
  bool ok = false;
  const auto duration = m_audio->property("Duration").toDouble(&ok);

  if(ok)
  {
    const int progressValue = m_audio->processedUSecs()/(duration*10000);
    m_progress->setValue(progressValue);
  }
}

//----------------------------------------------------------------
void OGGExtractor::onErrorSignaled(const QString message, const QString details)
{
  errorDialog(message, details);
}
