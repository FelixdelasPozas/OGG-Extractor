/*
 File: OGGContainerWrapper.cpp
 Created on: 28/05/2016
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
#include "OGGContainerWrapper.h"
#include "OGGExtractor.h"

// Qt
#include <QFile>

using namespace OGGWrapper;

//----------------------------------------------------------------
OGGWrapper::OGGContainerWrapper::OGGContainerWrapper(const OGGData& data)
: m_data    (data)
, m_position{0}
{
}

//----------------------------------------------------------------
size_t OGGWrapper::OGGContainerWrapper::read(void* ptr, size_t size, size_t nmemb)
{
  QFile container{m_data.container};

  if(!container.open(QFile::ReadOnly)) return 0;

  container.seek(m_data.start + m_position);

  auto readSize = nmemb * size;
  auto fileSize = m_data.end-m_data.start;

  if(fileSize < m_position + readSize)
  {
    readSize = fileSize - m_position;
  }

  if(readSize > 0)
  {
    auto data = container.read(readSize);

    m_position += readSize;

    memcpy(ptr, data.constData(), readSize);
  }

  return readSize;
}

//----------------------------------------------------------------
int OGGWrapper::OGGContainerWrapper::seek(ogg_int64_t offset, int whence)
{
  ogg_int64_t filesize = m_data.end-m_data.start;

  switch (whence)
  {
    case SEEK_SET:
      if(filesize > offset)
      {
        m_position = offset;
      }
      else
      {
        m_position = filesize;
      }
      break;
    case SEEK_CUR:
      {
        ogg_int64_t toEOF = filesize - m_position;

        if(offset < toEOF)
        {
          m_position += offset;
        }
        else
        {
          m_position = filesize;
        }
      }
      break;
    case SEEK_END:
      m_position = filesize + 1;
      break;
    default:
      Q_ASSERT(false);
      break;
  };

  return 0;
}

//----------------------------------------------------------------
int OGGWrapper::OGGContainerWrapper::close()
{
  return 0;
}

//----------------------------------------------------------------
long OGGWrapper::OGGContainerWrapper::tell()
{
  return m_position;
}

//----------------------------------------------------------------
size_t OGGWrapper::read(void* ptr, size_t size, size_t nmemb, void* datasource)
{
  auto wrapper = reinterpret_cast<OGGContainerWrapper *>(datasource);
  if(!wrapper) Q_ASSERT(false);

  return wrapper->read(ptr, size, nmemb);
}

//----------------------------------------------------------------
int OGGWrapper::seek(void* datasource, ogg_int64_t offset, int whence)
{
  auto wrapper = reinterpret_cast<OGGContainerWrapper *>(datasource);
  if(!wrapper) Q_ASSERT(false);

  return wrapper->seek(offset, whence);
}

//----------------------------------------------------------------
int OGGWrapper::close(void* datasource)
{
  auto wrapper = reinterpret_cast<OGGContainerWrapper *>(datasource);
  if(!wrapper) Q_ASSERT(false);

  return wrapper->close();
}

//----------------------------------------------------------------
long OGGWrapper::tell(void* datasource)
{
  auto wrapper = reinterpret_cast<OGGContainerWrapper *>(datasource);
  if(!wrapper) Q_ASSERT(false);

  return wrapper->tell();
}
