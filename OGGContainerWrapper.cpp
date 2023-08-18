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

// C++
#include <fstream>
#include <codecvt>
#include <locale>
#include <cassert>

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
  std::ifstream container{ws2s(m_data.container), std::ios_base::in|std::ios_base::binary};

  if(!container.is_open()) return 0;

  container.seekg(m_data.start + m_position);

  auto readSize = nmemb * size;
  auto fileSize = m_data.end-m_data.start;

  if(fileSize < m_position + readSize)
  {
    readSize = fileSize - m_position;
  }

  if(readSize > 0)
  {
    container.read(reinterpret_cast<char *>(ptr), readSize);
    m_position += container.gcount();
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
      assert(false);
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
  if(!wrapper) assert(false);

  return wrapper->read(ptr, size, nmemb);
}

//----------------------------------------------------------------
int OGGWrapper::seek(void* datasource, ogg_int64_t offset, int whence)
{
  auto wrapper = reinterpret_cast<OGGContainerWrapper *>(datasource);
  if(!wrapper) assert(false);

  return wrapper->seek(offset, whence);
}

//----------------------------------------------------------------
int OGGWrapper::close(void* datasource)
{
  auto wrapper = reinterpret_cast<OGGContainerWrapper *>(datasource);
  if(!wrapper) assert(false);

  return wrapper->close();
}

//----------------------------------------------------------------
long OGGWrapper::tell(void* datasource)
{
  auto wrapper = reinterpret_cast<OGGContainerWrapper *>(datasource);
  if(!wrapper) assert(false);

  return wrapper->tell();
}

//----------------------------------------------------------------
bool OGGWrapper::oggInfo(OGGData& data)
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

    return false;
  }

  const auto info  = ov_info(&oggFile, 0);
  data.channels    = info->channels;
  data.rate        = info->rate;
  data.duration    = ov_time_total(&oggFile, -1);

  return true;
}

//----------------------------------------------------------------
std::wstring OGGWrapper::s2ws(const std::string& str)
{
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;

    return converterX.from_bytes(str);
}

//----------------------------------------------------------------
std::string OGGWrapper::ws2s(const std::wstring& wstr)
{
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;

    return converterX.to_bytes(wstr);
}
