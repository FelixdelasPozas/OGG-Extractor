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
#include "ui_OGGExtractor.h"

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

    /** \brief Updates the extraction button when a OGG selection checkbox changes status.
     *
     */
    void checkSelectedFiles();

  private:
    /** \brief Helper method that connects the signals of the UI with its correspondent slots.
     *
     */
    void connectSignals();

    struct ogg_data
    {
      unsigned long long start;
      unsigned long long end;
      QString container;
    };

    QStringList m_containers; /** file names of the containers. */
    QList<ogg_data> m_soundFiles;

    bool m_cancelProcess; /** true if current process has been cancelled. */
};
