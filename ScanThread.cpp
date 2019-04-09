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
const QList<OGGData> &ScanThread::streams() const
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

    while (!eof)
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
              data.container = filename;
              data.start     = oggBeginning;
              data.end       = oggEnding;

              oggInfo(data);

              const auto time = data.duration;

              if(data.error.isEmpty() && m_minimumDuration > 0 && (time < m_minimumDuration))
              {
                // skip file because duration
                continue;
              }

              m_streams << data;
            }
          }
        }
      }

      processed += bytesRead;
      partialSize += bytesRead;

      if (bytesRead < BUFFER_SIZE) eof = true;
    }
  }

  delete [] buffer;
}

//----------------------------------------------------------------
bool ScanThread::oggInfo(OGGData& data) const
{
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
    switch(ov_result)
    {
      case OV_EREAD:
        data.error = tr("A read from media returned an error.");
        break;
      case OV_ENOTVORBIS:
        data.error = tr("Bitstream does not contain any Vorbis data.");
        break;
      case OV_EVERSION:
        data.error = tr("Vorbis version mismatch.");
        break;
      case OV_EBADHEADER:
        data.error = tr("Invalid Vorbis bitstream header.");
        break;
      case OV_EFAULT:
        data.error = tr("Internal logic fault; indicates a bug or heap/stack corruption.");
        break;
      default:
        data.error = tr("Unknown error.");
    }

    return false;
  }

  const auto info  = ov_info(&oggFile, 0);
  data.channels    = info->channels;
  data.rate        = info->rate;
  data.duration    = ov_time_total(&oggFile, -1);
  data.error       = QString();

  return true;
}
