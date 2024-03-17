/*
 File: ScanThread.h
 Created on: 9 abr. 2019
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

#ifndef SCANTHREAD_H_
#define SCANTHREAD_H_

// Project
#include <OGGContainerWrapper.h>

// Qt
#include <QThread>

/** \class ScanThread
 * \brief Thread for scanning containers.
 *
 */
class ScanThread
: public QThread
{
    Q_OBJECT
  public:
    /** \brief ScanThread class constructor.
     *
     */
    explicit ScanThread(const QStringList containers, QObject *parent = nullptr);

    /** \brief ScanThread class virtual destructor.
     *
     */
    virtual ~ScanThread()
    {}

    /** \brief Cancels the scanning process.
     *
     */
    void abort();

    /** \brief Returns true if aborted and false otherwise.
     *
     */
    const bool isAborted() const
    { return m_aborted; }

    /** \brief Returns the list of found streams.
     *
     */
    const std::vector<OGGData> &streams() const;

    /** \brief Returns the number of found streams.
     *
     */
    const int streamsNumber() const
    { return m_streams.size(); }

    /** \brief Sets the minimum stream size to add it to the found list.
     * \param[in] size Stream size in bytes.
     *
     */
    void setMinimumStreamSize(const long long size)
    { m_minimumSize = size; }

    /** \brief Sets the minimum stream duration to add it to the found list.
     * \param[in] duration Stream duration in seconds.
     *
     */
    void setMinimumStreamDuration(const unsigned int duration)
    { m_minimumDuration = duration; }

  signals:
    void progress(int);
    void error(const QString, const QString);

  protected:
      virtual void run();

  private:
      const QStringList    m_containers;      /** list of container files.                          */
      std::vector<OGGData> m_streams;         /** found streams data.                               */
      bool                 m_aborted;         /** true if aborted, false otherwise.                 */
      long long            m_minimumSize;     /** minimum file size to add to the found list.       */
      unsigned int         m_minimumDuration; /** minimum stream duration to add to the found list. */

};

#endif // SCANTHREAD_H_
