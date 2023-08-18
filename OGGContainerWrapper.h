/*
 File: OGGContainerWrapper.h
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

#ifndef OGGCONTAINERWRAPPER_H_
#define OGGCONTAINERWRAPPER_H_

// libvorbis
#include <vorbis/vorbisfile.h>

// Qt
#include <string>

struct OGGData
{
  std::wstring       container; /** container file name.                       */
  unsigned long long start;     /** start position in container file.          */
  unsigned long long end;       /** end position in container file.            */
  int                channels;  /** number of channels of OGG file.            */
  int                rate;      /** sample rate of the OGG file.               */
  unsigned int       duration;  /** file duration in seconds.                  */
  std::string        error;     /** empty on success, error message otherwise. */

  OGGData(): start{0}, end{0}, channels{0}, rate{0}, duration{0} {};
};

namespace OGGWrapper
{
  /** \class OGGContainerWrapper
   * \brief Wrapper around a OGG file container to provide the needed functions
   *        to operate those files with libvorbis library.
   *
   */
  class OGGContainerWrapper
  {
    public:
      /** \brief OGGContainerWrapper class constructor.
       * \param[in] data OGG file data.
       *
       */
      OGGContainerWrapper(const OGGData &data);

      /** \brief OGGContainerWrapper class virtual destructor.
       *
       */
      virtual ~OGGContainerWrapper()
      {};

      /** \brief Reads size bytes of nmemb size to ptr and returns the number of elements read.
       * \param[in] ptr data buffer
       * \param[in] size size in bytes of an element.
       * \param[in] nmemb number of elements to read.
       *
       */
      size_t read(void *ptr, size_t size, size_t nmemb);

      /** \brief Moves the read pointer.
       * \param[in] offset distance.
       * \param[in] whence initial position of the movement.
       *
       */
      int seek(ogg_int64_t offset, int whence);

      /** \brief Closes the container. Returns true on sucess and false otherwise.
       *
       */
      int close();

      /** \brief Returns the current position.
       *
       */
      long tell();

    private:
      const OGGData m_data;     /** OGG file data.         */
      ogg_int64_t   m_position; /** current file position. */
  };

  /** \brief Callback methods as defined in ov_callbacks structure (vorbisfile.h line 39).
   *         In this case datasource will be our OGGContainerWrapper. The methods work as
   *         the fread, fseek, fclose & ftell equivalents for files.
   *
   */
  size_t read(void *ptr, size_t size, size_t nmemb, void *datasource);
  int seek(void *datasource, ogg_int64_t offset, int whence);
  int close(void *datasource);
  long tell(void *datasource);

  /** \brief Returns true if the file could be accessed and decoded, fills the relevant information in the OGGData struct.
   * \param[in] data OGG file data.
   *
   */
  bool oggInfo(OGGData &data);

  /** \brief Helper to convert string to wstring
   * \param[in] str string to convert.
   *
   */
  std::wstring s2ws(const std::string& str);

  /** \brief Helper to convert wstring to string
   * \param[in] wstr wstring to convert.
   *
   */
  std::string ws2s(const std::wstring& wstr);

}

#endif // OGGCONTAINERWRAPPER_H_
