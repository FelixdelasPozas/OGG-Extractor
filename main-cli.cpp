/*
 File: main-cli.cpp
 Created on: 13/08/2023
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

// C++
#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <cstdlib>
#include <iomanip>
#include <set>

// Project
#include <OGGContainerWrapper.h>

const std::string VERSION = "version 1.8.0";
const long long BUFFER_SIZE = 5242880; /** 5 MB size buffer.     */
const char *OGG_HEADER = "OggS";       /** Ogg header signature. */

/** \class InputParser
 * \brief To parse arguments, modified from
 * https://stackoverflow.com/questions/865668/parsing-command-line-arguments-in-c
 */
class InputParser
{
  public:
    InputParser(int &argc, char **argv)
    {
      for (int i = 1; i < argc; ++i)
        this->tokens.push_back(std::string(argv[i]));

      clear();
    }

    void clear()
    {
      this->used = std::vector<bool>(this->tokens.size(), false);
    }

    const std::string& getCmdOption(const std::string &option)
    {
      auto it = std::find(this->tokens.cbegin(), this->tokens.cend(), option);
      if (it != this->tokens.cend() && ++it != this->tokens.cend())
      {
        const auto distance = std::distance(this->tokens.cbegin(), it);
        used[distance] = used[distance-1] = true;
        return *it;
      }

      static const std::string empty_string("");
      return empty_string;
    }

    bool cmdOptionExists(const std::string &option)
    {
      const auto it = std::find(this->tokens.cbegin(), this->tokens.cend(), option);
      bool found = it != this->tokens.end();
      if(found)
      {
        const auto distance = std::distance(this->tokens.cbegin(), it);
        used[distance] = true;
      }
      return found;
    }

    bool hasUnusedTokens() const
    {
      return std::count(this->used.cbegin(), this->used.cend(), false) != 0;
    }

    std::vector<std::string> getUnusedTokens() const
    {
      std::vector<std::string> unused;
      for(int i = 0; i < this->tokens.size(); ++i)
      {
        if(!this->used.at(i))
          unused.push_back(this->tokens[i]);
      }

      return unused;
    }

  private:
    std::vector<std::string> tokens;
    std::vector<bool> used;
};

/** \class RangeParser
 * \brief Class to parse the range description format.
 *
 */
class RangeParser
{
  public:
    /** \brief RangeParser class empty constructor.
     *
     */
    RangeParser() {};

    /** \brief RangeParser class constructor.
     * \param[in] desc Range description to parse.
     *
     */
    RangeParser(const std::string &desc)
    {
      std::stringstream tempStream(desc);
      std::string segment;
      while(std::getline(tempStream, segment, ','))
      {
        if(segment.find('-') != std::string::npos)
        {
          std::stringstream rangeStream(segment);
          std::string lower, upper;
          std::getline(rangeStream, lower, '-');
          std::getline(rangeStream, upper, '-');
          const int iLower = std::stoi(lower);
          const int iUpper = std::stoi(upper);
          for(int i = std::min(iLower, iUpper); i <= std::max(iLower, iUpper); ++i)
            m_selected.insert(i);

          continue;
        }

        const auto num = std::stoi(segment);
        if(num > 0)
          m_selected.insert(num);
      }
    }

    /** \brief Returns if the given position is selected.
     * \param[in] i Position.
     *
     */
    bool isSelected(const int i) const
    { return m_selected.find(i) != m_selected.end(); }

    /** \brief Returns the number of positions.
     *
     */
    unsigned int count() const
    { return m_selected.size(); }

  private:
    std::set<unsigned int> m_selected; /** selected positions set. */
};

/** \brief Helper method to generate a more detailed file name with the
 * range and size of the file.
 *
 */
std::string getOutputFilename(const int i, const OGGData &data, const unsigned long long totalSize)
{
  std::filesystem::path container(OGGWrapper::ws2s(data.container));

  std::stringstream sstr;
  sstr << std::setw(std::to_string(totalSize).length()) << std::setfill('0')
       << container.filename().string() << "_" << data.start << "-" << data.end << "_(" << data.end-data.start << ").ogg";

  return sstr.str();
}

/** \brief Helper to print help to console.
 *
 */
void print_help()
{
  std::cout << "\nUsage: OGGExtractor-cli.exe [options] -i <input_file>\n";
  std::cout << "Options:\n";
  std::cout << "\t-h               Show help text.\n";
  std::cout << "\t-s <number>      Minimum size of files to extract in Kb.\n";
  std::cout << "\t-l <number>      Minimum length in seconds of files to extract.\n";
  std::cout << "\t-o <output_dir>  Output directory for extracted files.\n";
  std::cout << "\t-i <input_file>  Input file to scan for OGG files.\n";
  std::cout << "\t-d               Dump file information in a CSV file and do not extract files.\n";
  std::cout << "\t-r <range_def>   Extract files in the given position/range (comma separated values and ranges like low-upp).\n";
  std::cout << "\t                 Specified positions are absolute, not relative to filtering by size or length.\n\n";
  std::cout << "Example: OGGExtractor-cli.exe -o D:\\output_dir\\ -s 100 -l 60 -i container_file.ext\n\n";
  std::cout << "\tExtracts all files inside container_file.ext with a size over 100Kb and a duration over 60 seconds\n";
  std::cout << "\tin the directory D:\\output_dir\\.\n\n";
  std::cout << "Example: OGGExtractor-cli.exe -o D:\\output_dir\\ -d -r 1,3,5-7 -i container_file.ext\n\n";
  std::cout << "\tDumps the information of files in container_file.ext in the positions 1,3,5,6 and 7 to the directory\n";
  std::cout << "\tD:\\output_dir\\ in a CSV format text file.";
  std::cout << std::endl;
  std::exit(-1);
}

/** \brief Helper to print narcissistic banner to console.
 *
 */
void app_banner()
{
  std::cout << "-----------------------------------------------------------------------------------\n";
  std::cout << "OGG Extractor " << VERSION << " -- https://github.com/FelixdelasPozas/OGG-Extractor\n";
  std::cout << "-----------------------------------------------------------------------------------\n";
}

int main(int argc, char *argv[])
{
  app_banner();
  int minSize = 0;
  int minLength = 0;
  std::filesystem::path output_dir = std::filesystem::current_path();
  std::filesystem::path input_file;
  bool dumpCSV = false;
  RangeParser rangeParser;

  // Parse arguments and fill parameter variables.
  InputParser parser(argc, argv);
  if(argc == 1 || parser.cmdOptionExists("-h") || parser.cmdOptionExists("--help"))
    print_help();

  if(parser.cmdOptionExists("-s"))
  {
    char *ptr = nullptr;
    const auto value = parser.getCmdOption("-s");
    const auto tempMinSize = std::strtol(value.c_str(), &ptr, 10);
    if(ptr != nullptr && tempMinSize > 0)
      minSize = tempMinSize;
    else
    {
      std::cerr << "ERROR - Invalid minimum size: " << value << std::endl;
      print_help();
    }
  }

  dumpCSV = parser.cmdOptionExists("-d");

  if(parser.cmdOptionExists("-l"))
  {
    char *ptr = nullptr;
    const auto value = parser.getCmdOption("-l");
    const auto tempMinLength = std::strtol(value.c_str(), &ptr, 10);
    if(ptr != nullptr && tempMinLength > 0)
      minLength = tempMinLength;
    else
    {
      std::cerr << "ERROR - Invalid file length: " << value << std::endl;
      print_help();
    }
  }

  if(parser.cmdOptionExists("-o"))
  {
    const auto temp_path = std::filesystem::path(parser.getCmdOption("-o"));
    if(std::filesystem::exists(temp_path) && std::filesystem::is_directory(temp_path))
      output_dir = std::filesystem::canonical(temp_path);
    else
    {
      std::cerr << "ERROR - Invalid output path: " << temp_path << std::endl;
      print_help();
    }
  }

  if(parser.cmdOptionExists("-i"))
  {
    const auto temp_path = std::filesystem::path(parser.getCmdOption("-i"));
    if(std::filesystem::exists(temp_path) && !std::filesystem::is_directory(temp_path))
      input_file = std::filesystem::canonical(temp_path);
    else
    {
      std::cerr << "ERROR - Invalid input file: " << temp_path << std::endl;
      print_help();
    }
  }

  if(parser.cmdOptionExists("-r"))
  {
    rangeParser = RangeParser(parser.getCmdOption("-r"));
  }

  if(parser.hasUnusedTokens())
  {
    for(const auto item: parser.getUnusedTokens())
      std::cout << "ERROR - Unknown option: " << item << std::endl;

    print_help();
  }

  if(input_file.empty())
  {
    std::cerr << "ERROR: An input file is necessary to scan!" << std::endl;
    print_help();
  }

  std::ifstream input_stream(input_file.c_str(), std::ios_base::in|std::ios_base::binary);
  if(!input_stream.is_open())
  {
    std::cerr << "ERROR: Unable to open '" << input_file.string() << "' as readonly!" << std::endl;
    std::exit(-1);
  }

  input_stream.seekg(0, std::ios_base::end);
  unsigned long long int totalSize   = input_stream.tellg();
  input_stream.seekg(0, std::ios_base::beg);

  if(totalSize == 0)
  {
    std::cerr << "ERROR: Input file '" << input_file.string() << "' is empty!" << std::endl;
    std::exit(-1);
  }

  // All done, begin scanning
  auto buffer = new char[BUFFER_SIZE];
  long long processed = 0;
  unsigned long long oggBeginning = 0;
  unsigned long long oggEnding    = 0;

  bool beginFound = false;
  bool endFound   = false;

  unsigned char oggHeader[27];
  int progressValue = 0;
  std::vector<OGGData> streams;

  while (true)
  {
    input_stream.seekg(processed);

    input_stream.read(buffer, BUFFER_SIZE);
    auto bytesRead = input_stream.gcount();

    if(bytesRead < BUFFER_SIZE) // reset fail bit if reached the end.
      input_stream.clear(std::ios_base::goodbit);

    for (long long loop = 0; loop < bytesRead && 0 != bytesRead; ++loop)
    {
      // check for "OggS" header and flags
      if (buffer[loop] == 0x4F)
      {
        input_stream.seekg(processed + loop);
        input_stream.read(reinterpret_cast<char *>(&oggHeader[0]), sizeof(oggHeader));
        unsigned long long readResult = input_stream.gcount();
        if(!input_stream || sizeof(oggHeader) != readResult)
        {
          std::cerr << "ERROR: I/O Error scanning file '" << input_file.string() << "' at position " << processed + loop << "." << std::endl;
          std::exit(-1);
        }

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

            input_stream.seekg(oggEnding);
            input_stream.read(trailingFrames, trailingSize);
            readResult = input_stream.gcount();

            if (!input_stream || (trailingSize != readResult))
            {
              std::cerr << "ERROR: I/O error reading file'" << input_file.string() << ", probably tried to read past EOF." << std::endl;
              std::exit(-1);
            }

            oggEnding += (unsigned long long) oggHeader[26];

            for (unsigned long loop2 = 0; loop2 < (unsigned long) oggHeader[26]; loop2++)
            {
              oggEnding += (unsigned long long) trailingFrames[loop2];
            }

            delete [] trailingFrames;
          }

          if ((beginFound == true) && (endFound == true))
          {
            beginFound = endFound = false;

            OGGData data;
            data.container = input_file.wstring();
            data.start     = oggBeginning;
            data.end       = oggEnding;

            OGGWrapper::oggInfo(data);

            streams.push_back(data);
          }
        }
      }
    }

    processed += bytesRead;

    const int value = 100.0*(static_cast<double>(processed)/totalSize);
    if(value != progressValue)
    {
      progressValue = value;
      std::cout << "\rScanning '" << input_file.string() << "': " << progressValue << "% - Found " << streams.size() << " files.";
    }

    // check for eof
    if(processed == totalSize) break;
  }
  std::cout << std::endl;

  // Input scanned, apply filters and dump data.

  // Dump to csv format.
  if(dumpCSV)
  {
    auto output_file = output_dir / input_file.filename();
    output_file.replace_extension(".csv");
    if(std::filesystem::exists(output_file))
    {
      std::cerr << "ERROR: Output file '" << output_file.string() << " already exists!" << std::endl;
      std::exit(-1);
    }

    std::ofstream output_stream(output_file.c_str(), std::ios_base::out|std::ios_base::trunc);
    output_stream << "track_number,position,length,time,num_channels,bitrate\n";

    for(int i = 0; i < streams.size(); ++i)
    {
      const auto &data = streams.at(i);
      output_stream << std::to_string(i+1) << "," << std::to_string(data.start) << ","
                    << std::to_string(data.end-data.start) << "," << std::to_string(data.duration) << ","
                    << std::to_string(data.channels) << "," << std::to_string(data.rate) << "\n";
    }

    output_stream.close();
    std::cout << "Dumped " << streams.size() << " OGG streams information to '" << output_file.string() << "'" << std::endl;
    return 0;
  }

  // Extract files.
  unsigned int extracted = 0;
  for(int i = 0; i < streams.size(); ++i)
  {
    input_stream.clear(std::ios_base::goodbit);
    const auto &data = streams.at(i);

    if(minSize > 0 && (minSize * 1024 > (data.end-data.start)))
      continue;

    if(minLength > 0 && data.duration < minLength)
      continue;

    if(rangeParser.count() > 0 && !rangeParser.isSelected(i+1))
      continue;

    input_stream.seekg(data.start);

    std::stringstream numstr;
    numstr << std::setw(std::to_string(streams.size()).length()) << std::setfill('0') << i+1 << "_";

    auto output_file = output_dir / (std::string(numstr.str()) + getOutputFilename(i, data, totalSize));

    // Beware Trucate
    std::ofstream output_stream(output_file.c_str(), std::ios_base::out|std::ios_base::binary|std::ios_base::trunc);

    auto remaining = data.end-data.start;
    while(remaining > BUFFER_SIZE)
    {
      input_stream.read(buffer, BUFFER_SIZE);
      remaining -= BUFFER_SIZE;
      output_stream.write(buffer, BUFFER_SIZE);
    }

    input_stream.read(buffer, remaining);
    output_stream.write(buffer, remaining);
    output_stream.close();

    std::cout << "Wrote '" << output_file.string() << "'\n";
    ++extracted;
  }
  std::cout << "Extracted " << extracted << " files according to given parameters." << std::endl;

  delete [] buffer;

  return 0;
}



