/*
 File: OGGExtractor.h
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

// Qt
#include <QMainWindow>
#include <QAudioDevice>
#include "ui_OGGExtractor.h"

// Project
#include <OGGContainerWrapper.h>
#include <ScanThread.h>
#include <external/QTaskbarButton.h>

// C++
#include <memory>

class QByteArray;
class QToolButton;
class QBuffer;
class QAudioOutput;
class QWinTaskbarButton;
class QAudioSink;
class QAudioBuffer;
class TableModel;

/** \class OGGExtractor
 * \brief Main dialog class.
 *
 */
class OGGExtractor
: public QMainWindow
, private Ui_OGGExtractorMainWindow
{
    Q_OBJECT
  public:
    /** \brief OGGExtractor class constructor.
     * \param[in] parent raw pointer of the QWidget parent of this one.
     * \param[in] flags window flags.
     *
     */
    explicit OGGExtractor(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

    /** \brief OGGExtractor class virtual destructor.
     *
     */
    virtual ~OGGExtractor();

  protected:
    virtual void showEvent(QShowEvent *e) override;

  private slots:
    /** \brief Opens a file selection dialog to select container files.
     *
     */
    void onFileAdd();

    /** \brief Removes the selected container from the contailers list.
     *
     */
    void onFileRemove();

    /** \brief Shows the about dialog.
     *
     */
    void showAboutDialog();

    /** \brief Scans the container files for OGG music files.
     *
     */
    void scanContainers();

    /** \brief Shows the file dialog to select destination and extracts the found music files.
     *
     */
    void extractFiles();

    /** \brief Updates the remove container button state. Disables it if no selection and enables on selection.
     *
     */
    void onContainerSelectionChanged();

    /** \brief Cancels scanning process.
     *
     */
    void cancelScan();

    /** \brief Updates the UI when the state of the size checkbox changes.
     * \param[in] value checkbox state.
     *
     */
    void onSizeStateChange(int value);

    /** \brief Updates the UI when the state of the time checkbox changes.
     * \param[in] value checkbox state.
     *
     */
    void onTimeStateChange(int value);

    /** \brief Updates the extraction button when a OGG selection checkbox changes status.
     *
     */
    void checkSelectedFiles();

    /** \brief Plays/stops the corresponding OGG file and updates the UI.
     *
     */
    void onPlayButtonPressed();

    /** \brief Updates the boolean vector with the value of the changed checkbox. 
     * 
     */
    void onCheckboxModified();

    /** \brief Stops the audio currently playing and frees the resources.
     *
     */
    void stopBuffer();

    /** \brief Updates audio and the UI with the new volume value.
     * \param[in] value volume value in 0-100.
     */
    void onVolumeChanged(int value);

    /** \brief Gets the found containers and frees the thread.
     *
     */
    void onThreadFinished();

    /** \brief Shows a dialog with the error message.
     * \param[in] message Error message.
     * \param[in] details Error details.
     *
     */
    void onErrorSignaled(const QString message, const QString details);

    /** \brief Updats the progress value.
     * \param[in] value Progress value in [0,100].
     *
     */
    void onProgressSignaled(int value);

    /**\brief Updates the UI buttons and sets the page in the model. 
     * 
     */
    void onMovementButtonClicked();

    /** \brief Updates the progres bar with the current audio play progress.
     * 
     */
    void onAudioNotify();

  private:
    /** \brief Helper method that connects the signals of the UI with its correspondent slots.
     *
     */
    void connectSignals();

    /** \brief Inserts the widgets in the table and connects the signals. 
     *
     */
    void insertWidgetsInTable();

    /** \brief Removes the widgets in the table and disconnects the signals. 
     *
     */
    void removeWidgetsFromTable();

    /** \brief Shows an error dialog with the given error mesage and details.
     * \param[in] error error message.
     * \param[in] details error details message.
     *
     */
    void errorDialog(const QString &error, const QString &details = QString()) const;

    /** \brief Disables parts of the UI when starting a process.
     *
     */
    void startProcess();

    /** \brief Enables parts of the UI when ending a process.
     *
     */
    void endProcess();

    /** \brief Returns true if the file could be accessed and decoded, fills the relevant information in the OGGData struct.
     * \param[in] data OGG file data.
     *
     */
    bool oggInfo(OGGData &data) const;

    /** \brief Returns the decodec pcm data of the given OGG file.
     * \param[in] data OGG file data.
     *
     * NOTE: this method fills the 'channels' and 'rate' fields of the OGGData struct.
     *
     */
    std::shared_ptr<QByteArray> decodeOGG(OGGData &data);

    /** \brief Plays the pcm buffer.
     * \param[in] pcmBuffer pcm data to play.
     * \param[in] data OGG file data.
     *
     */
    void playBufffer(std::shared_ptr<QByteArray> pcmBuffer, const OGGData &data);

    /** \brief Helper method to set operation progress.
     * \param[in] value Progress value in [0,100];
     * \param format Progress bar text format.ABC
     *
     */
    void setProgress(int value, QString format = "");

    QStringList          m_containers;     /** file names of the containers.               */
    std::vector<OGGData> m_soundFiles;     /** found OGG files information.                */
    std::vector<bool>    m_soundSelected;  /** true if selected and false otherwise.       */
    bool                 m_cancelProcess;  /** true if current process has been cancelled. */
    float                m_volume;         /** value of volume slider in [0-1]             */

    QToolButton                  *m_playButton;    /** button of a currently playing sound.                         */
    std::shared_ptr<QByteArray>   m_sample;        /** raw buffer of currently playing sound.                       */
    std::shared_ptr<QBuffer>      m_buffer;        /** Sound pcm buffer as QIODevice                                */
    std::shared_ptr<QAudioSink>   m_audio;           /** sound player.                                                */
    QTaskBarButton                m_taskBarButton; /** taskbar progress widget.                                     */
    std::shared_ptr<ScanThread>   m_thread;        /** thread for scanning containers.                              */
    TableModel                   *m_tableModel;    /** table internal model.                                        */
    QAudioDevice                  m_audioDevice;   /** Default audio device.                                        */
};
