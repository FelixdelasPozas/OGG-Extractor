/*
 File: main.cpp
 Created on: 27/02/2004
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
#include "OGGExtractor.h"

// Qt
#include <QApplication>
#include <QMainWindow>

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	OGGExtractor extractor;
	extractor.show();

	return app.exec();
}

//#define VERSION "v1.4a"
//
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <malloc.h>
//#include <io.h>
//#include <unistd.h>
//#include <ctype.h>
//
//typedef enum {_error_ = -1, _success_, _false_ = 0, _true_} booleantype;
//
///* global variables */
//static unsigned long long num_files = 0;
//
//static unsigned long long ogg_found = 0;
//static unsigned long long ogg_skip = 0;
//static unsigned long long ogg_extract = 0;
//static unsigned long long ogg_size = 0;
//
//static unsigned long long to_skip = 0;
//static unsigned long long to_extract = 0;
//static unsigned long long to_skip_size = 0;
//
//static booleantype do_extract = _false_;
//
///* Ogg files signature */
//static const char *OGG_HEADER = (const char *)"OggS";
//
///* let's use a 5 mb buffer, I don't want to getc the entire file because game
//** datafiles tend to be huge it's a bit less simple but who cares. */
//static const unsigned long long BUFFER_SIZE = 5242880;
//
///* Information shown at the end of the program
//*/
//static void stats(void)
//{
//    printf("| Processed %I64u files.\n", num_files);
//    printf("| Found %I64u Ogg files.\n", ogg_found);
//
//    if (to_skip != 0)
//        printf("| Skipped %I64u files.\n", to_skip-ogg_skip);
//
//    if (to_skip_size != 0)
//        printf("| Skipped %I64u files because of it's size.\n", to_skip_size);
//
//    if(to_extract != 0)
//        printf("| Extracted %I64u files.\n", to_extract-ogg_extract);
//    return;
//}
//
///* After the beginning and end of an Ogg file has been detected this function
//** dumps it as a separate file. Only ogg_search() calls this function.
//*/
//static void dumpfile(FILE *file,
//                     unsigned long long begin,
//                     unsigned long long end)
//{
//    unsigned char *buffer;
//    char filename[30], temp[10];
//    unsigned long long old_filepointer;
//    unsigned long long filesize = end-begin;
//    FILE *output;
//
//    /* generate name for output */
//    (void) sprintf(temp,"%08lu", (unsigned long)ogg_found);
//    strcpy(filename,"found_");
//    strcat(filename,temp);
//    strcat(filename,".ogg");
//
//    printf("(%08ld) Ogg found at : %I64u - ", (unsigned long)ogg_found, begin);
//    printf("Writting : '%s'\n", filename);
//
//    if (NULL == (output = fopen(filename, "wb")))
//    {
//        printf("ERROR: Error opening output file %s.\n",filename);
//        exit(_error_);
//    }
//
//    if (NULL == (buffer = calloc(1, BUFFER_SIZE)))
//    {
//        printf("ERROR: Not enough memory for output buffer (%I64u bytes).\n",
//               (unsigned long long) sizeof(BUFFER_SIZE));
//        exit(_error_);
//    }
//
//    /* ogg_search needs the file pointer to stay the same */
//    old_filepointer = (unsigned long) ftell(file);
//    (void) fseek(file, begin, SEEK_SET);
//
//    while (BUFFER_SIZE < filesize)
//    {
//        if (BUFFER_SIZE != fread(buffer, 1, BUFFER_SIZE, file))
//        {
//            printf("ERROR: I/O error reading input file.\n");
//            exit(_error_);
//        }
//
//        if (BUFFER_SIZE != fwrite(buffer, 1, BUFFER_SIZE, output))
//        {
//            printf("ERROR: I/O error writing output file %s.\n", filename);
//            exit(_error_);
//        }
//
//        filesize -= BUFFER_SIZE;
//    }
//
//    if (filesize != fread(buffer, 1, filesize, file))
//    {
//        printf("ERROR: I/O error reading input file.\n");
//        exit(_error_);
//    }
//
//    if (filesize != fwrite(buffer, 1, filesize, output))
//    {
//        printf("ERROR: I/O error writing output file %s.\n", filename);
//        exit(_error_);
//    }
//
//    free(buffer);
//
//    if (fclose(output) == EOF)
//    {
//        printf("ERROR: Error closing output file %s.\n", filename);
//        exit(_error_);
//    }
//
//    /* ogg_search needs the file pointer to stay the same */
//    (void) fseek(file, old_filepointer, SEEK_SET);
//    return;
//}
//
///* When the file to inspect has been opened this function searchs for
//** Ogg file signatures, and extracts or skips found files depending
//** on command-line options. If 'extract' option has been set this
//** function exits the program without searching for more files.
//*/
//static void ogg_search(FILE *filename)
//{
//    unsigned long loop, loop2;
//    unsigned char *buffer, *trailing_frames, *ogg_header_buf;
//    unsigned long long filepointer, bytes, filesize = 0;
//    unsigned long long ogg_beginning= 0;
//    unsigned long long ogg_ending = 0;
//    booleantype begin_found = _false_;
//    booleantype end_of_file = _false_;
//    booleantype end_found = _false_;
//
//    if (NULL == (buffer = calloc(1, BUFFER_SIZE)))
//    {
//        printf("ERROR: Not enough memory for %I64u bytes buffer.\n",
//               (unsigned long long) sizeof(BUFFER_SIZE));
//        exit(_error_);
//    }
//
//    while (end_of_file != _true_)
//    {
//        bytes = fread(buffer, 1, BUFFER_SIZE, filename);
//
//        for (loop = 0; loop < bytes; loop++)
//        {
//            /* check for "OggS" header and flags */
//            if (buffer[loop] == 0x4F)
//            {
//                /* we must be paranoids because loop can be BUFFER_SIZE */
//                /* and looking buffer[loop+5] is out of the buffer      */
//                if (NULL == (ogg_header_buf = calloc(1, 27)))
//                {
//                    printf("ERROR: Not enough memory for 27 bytes, absurd!.\n");
//                    free(buffer);
//                    exit(_error_);
//                }
//
//                filepointer = (unsigned long long) ftell(filename);
//                (void) fseek(filename, filesize+loop, SEEK_SET);
//
//                if (27 != fread(ogg_header_buf, 1, 27, filename))
//                {
//                    printf("ERROR: I/O error reading input file, probably tried to read past EOF (%I64u).\n", filesize+loop);
//                    free(ogg_header_buf);
//                    free(buffer);
//                    return;
//                }
//
//                (void) fseek(filename, filepointer, SEEK_SET);
//
//                if (0 == (strncmp((const char *)ogg_header_buf, OGG_HEADER, 4)))
//                {
//                    /* detected beginning of ogg file */
//                    if (ogg_header_buf[5] == 0x02)
//                    {
//                        begin_found = _true_;
//                        ogg_beginning = filesize + loop;
//                        continue;
//                    }
//
//                    /* detected ending of ogg file, more difficult because of trailing frames */
//                    if ((ogg_header_buf[5] == 0x04) || (ogg_header_buf[5] == 0x05))
//                    {
//                        end_found = _true_;
//                        ogg_ending = filesize + loop + 27;
//
//                        /* we need to do this because we can be at the very end */
//                        /* of the buffer and don't want to look outside it      */
//                        if (NULL == (trailing_frames = calloc(1, ogg_header_buf[26])))
//                        {
//                            printf("ERROR: Not enough memory for %c bytes, absurd!.\n",
//                                   ogg_header_buf[26]);
//                            free(ogg_header_buf);
//                            free(buffer);
//                            exit(_error_);
//                        }
//
//                        filepointer = (unsigned long long) ftell(filename);
//                        (void) fseek(filename, ogg_ending, SEEK_SET);
//                        if (ogg_header_buf[26] != fread(trailing_frames, 1, ogg_header_buf[26], filename))
//                        {
//                            printf("ERROR: I/O error reading input file, probably tried to read past EOF.\n");
//                            free(trailing_frames);
//                            free(ogg_header_buf);
//                            free(buffer);
//                            return;
//                        }
//
//                        (void) fseek(filename, filepointer, SEEK_SET);
//                        ogg_ending += (unsigned long long)ogg_header_buf[26];
//
//                        for (loop2 = 0; loop2 < (unsigned long)ogg_header_buf[26]; loop2++)
//                            ogg_ending += (unsigned long long)trailing_frames[loop2];
//
//                        free(trailing_frames);
//                    }
//
//                    /* every beginning has an end ;-) */
//                    if ((begin_found == _true_) && (end_found == _true_))
//                    {
//                        begin_found = _false_;
//                        end_found = _false_;
//                        ogg_found++;
//
//                        if (ogg_skip > 0)
//                        {
//                            ogg_skip--;
//                            printf("(%08ld) Ogg found at : %I64u - Skipped.\n",
//                                   (unsigned long)ogg_found, ogg_beginning);
//                            continue;
//                        }
//
//                        if (ogg_ending-ogg_beginning < (ogg_size * 1024))
//                        {
//                            to_skip_size++;
//                            printf("(%08ld) Ogg found at : %I64u - Skipped because of it's size.\n",
//                                   (unsigned long)ogg_found, ogg_beginning);
//                            continue;
//                        }
//
//                        if (ogg_extract > 0)
//                        {
//                            ogg_extract--;
//                            dumpfile(filename, ogg_beginning, ogg_ending);
//                            if (ogg_extract != 0)
//                                continue;
//                        }
//
//                        if (do_extract == _true_)
//                        {
//                            printf("\n| Partial statistics, not all files were fully processed:\n");
//                            stats();
//                            free(ogg_header_buf);
//                            free(buffer);
//                            exit(_success_);
//                        }
//                        else
//                            dumpfile(filename, ogg_beginning, ogg_ending);
//                    }
//                }
//                free(ogg_header_buf);
//            }
//        }
//        filesize += bytes;
//        /* reached end of file? */
//        if (bytes < BUFFER_SIZE)
//            end_of_file = _true_;
//    }
//    free(buffer);
//    return;
//}
//
///* Information about the options and some examples
//*/
//static void usage(char* name)
//{
//    printf("Usage: %s [-snnn] [-ennn] [-znnn] file1 [file2] ... [fileN]\n\n", name);
//
//    printf("Options:\n");
//    printf("  -snnn      Skip the first nnn files.\n");
//    printf("  -ennn      Extract only nnn files.\n");
//    printf("  -znnn      Do not extract files smaller than nnn kb.\n");
//    printf("  file       File(s) to inspect, wilcards accepted.\n\n");
//
//    printf("Example: %s *.* ..\\*.pak ..\\..\\*.wav\n", name);
//    printf("         Extracts all ogg files found in the specified files.\n\n");
//
//    printf("Example: %s -s10 -e15 *.pak\n",name);
//    printf("         Skip the first 10 files and extract the next 15 in all .pak files.\n\n");
//
//    printf("Example: %s -z1024 *.pak\n", name);
//    printf("         Do not extract files smaller than 1024kb.\n\n");
//
//    printf("Note: 'skip', 'extract' and 'size' options cross file boundaries.\n");
//    printf("      'file' is the only parameter required, others are optional.\n");
//    return;
//}
//
//int main(int argc, char *argv[])
//{
//    unsigned char *path;
//    unsigned char full_path[260];
//    unsigned long file_number = 0;
//    char **unused = NULL;
//    int error_handle = 0;
//    struct _finddata_t files;
//    FILE *filename;
//
//    /* getopt() var */
//    int options;
//
//    /* The egocentric banner */
//    printf("Generic Ogg Ripper %s\n", VERSION);
//    printf("Felix de las Pozas Alvarez, %s\n\n",__DATE__);
//
//    while ((options = getopt (argc, argv, ":s:e:z:")) != -1)
//        switch (options)
//        {
//        case 's':
//            to_skip = strtol(optarg, unused, 0);
//            break;
//        case 'e':
//            if ((to_extract = strtol(optarg, unused, 0)) != 0)
//                do_extract = _true_;
//            break;
//        case 'z':
//            ogg_size = strtol(optarg, unused, 0);
//            ;
//            break;
//        case ':':
//            printf("ERROR: Option '%c' requires an argument.\n", optopt);
//            return _error_;
//            break;
//        case '?':
//            if (isprint (optopt))
//                printf("ERROR: Unknown option `-%c'.\n", optopt);
//            else
//                printf("ERROR: Unknown option character `\\x%x'.\n", optopt);
//            return _error_;
//        default:
//            exit(_error_);
//        }
//
//    if (argc == 1)
//    {
//        usage(argv[0]);
//        return _error_;
//    }
//
//    if (optind == argc)
//    {
//        printf("ERROR: Missing file(s) parameter(s).\n");
//        return _error_;
//    }
//
//    ogg_skip = to_skip;
//    ogg_extract = to_extract;
//
//
//    /* Test if the file exists and tries to open it, notify errors */
//    for (file_number = optind; file_number < (unsigned long)argc; file_number++)
//    {
//        /* ok, who goes first? */
//        if (-1 == (error_handle = _findfirst(argv[file_number], &files)))
//        {
//            printf("ERROR: Error opening file %s\n", argv[file_number]);
//            continue;
//        }
//        strcpy((char *)full_path, argv[file_number]);
//
//        do
//        {
//            /* quick and dirty way to jump to directories */
//            if (NULL != (path = (unsigned char *)strrchr((char const *)full_path,92)))
//            {
//                path++;
//                strcpy((char *)path, files.name);
//            }
//            else
//                strcpy((char *)full_path, files.name);
//
//            /* i don't want those listed */
//            if (_A_SUBDIR == (files.attrib & 0x10))
//                continue;
//
//            /* try to open, notify errors and continue with next file */
//            if (NULL == (filename = fopen((char const *)full_path, "rb")))
//            {
//                printf("ERROR: Error opening file %s\n",(char *)full_path);
//                continue;
//            }
//
//            /* search for ogg and update stats */
//            printf("Searching in %s\n", (char *)full_path);
//            num_files++;
//            ogg_search(filename);
//
//            /* Close the file*/
//            if (fclose(filename) == EOF)
//                printf("ERROR: Error closing the file %s.\n", (char*)full_path);
//        }
//        while (0 == _findnext(error_handle,&files));
//
//        (void) _findclose(error_handle);
//    }
//    printf("\n| Full statistics:\n");
//    stats();
//    return _success_;
//}
