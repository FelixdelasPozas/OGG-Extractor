/*
 File: ScanThread.cpp
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

// Project
#include <OGGExtractor.h>
#include <ScanThread.h>

// Qt
#include <QFile>

// libvorbis
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

const long long BUFFER_SIZE = 5242880; /** 5 MB size buffer.     */
const char *OGG_HEADER = "OggS";       /** Ogg header signature. */

using namespace OGGWrapper;

//--------------------------------------------------------------------
ScanThread::ScanThread(const QStringList containers, QObject *parent)
: QThread          {parent}
, m_containers     {containers}
, m_aborted        {false}
, m_minimumSize    {-1}
, m_minimumDuration{0}
{
}

//--------------------------------------------------------------------
void ScanThread::abort()
{
  m_aborted = true;
}

//--------------------------------------------------------------------
const std::vector<OGGData> &ScanThread::streams() const
{
  return m_streams;
}

//--------------------------------------------------------------------
void ScanThread::run()
{
  auto buffer = new char[BUFFER_SIZE];

  unsigned long long int partialSize = 0;
  unsigned long long int totalSize   = 0;

  for(auto filename: m_containers)
  {
    QFile file(filename);
    totalSize += file.size();
  }

  if(totalSize == 0) return;

  int progressValue = 0;
  for(auto filename: m_containers)
  {
    if(m_aborted) break;

    QFile file(filename);
    if(!file.open(QFile::ReadOnly))
    {
      auto message = tr("Error opening file '%1'").arg(filename);
      auto details = tr("Error: %1").arg(file.errorString());
      emit error(message, details);

      continue;
    }

    long long processed             = 0;
    unsigned long long oggBeginning = 0;
    unsigned long long oggEnding    = 0;

    bool beginFound = false;
    bool endFound   = false;
    bool eof        = false;

    unsigned char oggHeader[27];

    while (!eof && !m_aborted)
    {
      const int value = 100.0*(static_cast<double>(partialSize)/totalSize);
      if(value != progressValue)
      {
        progressValue = value;
        emit progress(value);
      }

      file.seek(processed);

      auto bytesRead = file.read(buffer, BUFFER_SIZE);

      for (long long loop = 0; loop < bytesRead && !eof && !m_aborted; ++loop)
      {
        // check for "OggS" header and flags
        if (buffer[loop] == 0x4F)
        {
          auto position   = file.pos();
          auto seekResult = file.seek(processed + loop);
          unsigned long long readResult = file.read(reinterpret_cast<char *>(&oggHeader[0]), sizeof(oggHeader));
          if(!seekResult || (sizeof(oggHeader) != readResult))
          {
            auto message = tr("Error scanning file '%1'").arg(filename);
            auto details = tr("ERROR: %1").arg(file.errorString());
            emit error(message, details);

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
                auto message = tr("I/O error reading input file, probably tried to read past EOF while scanning '%1'").arg(filename);
                auto details = tr("ERROR: %1").arg(file.errorString());
                emit error(message, details);

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
              if (m_minimumSize > 0 && (size < (m_minimumSize * 1024)))
              {
                // skip file because size
                continue;
              }

              OGGData data;
              data.container = filename.toStdWString();
              data.start     = oggBeginning;
              data.end       = oggEnding;

              OGGWrapper::oggInfo(data);

              const auto time = data.duration;

              if(data.error.empty() && m_minimumDuration > 0 && (time < m_minimumDuration))
              {
                // skip file because duration
                continue;
              }

              m_streams.push_back(data);
            }
          }
        }
      }

      processed += bytesRead;
      partialSize += bytesRead;

      if (bytesRead < BUFFER_SIZE) eof = true;
    }

    emit progress(100);
  }

  delete [] buffer;
}
