/** vpr_relocalization: a library for visual place recognition in changing 
** environments with efficient relocalization step.
** Copyright (c) 2017 O. Vysotska, C. Stachniss, University of Bonn
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in
** all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE.
**/


#include "list_dir.h"

#include <dirent.h>
#include <algorithm>
#include <iostream>

std::vector<std::string> listDir(const std::string &dir_name) {
  std::vector<std::string> file_names;
  DIR *dir;
  if ((dir = opendir(dir_name.c_str())) != NULL) {
    struct dirent *ent;
    /* print all the files and directories within directory */
    while ((ent = readdir(dir)) != NULL) {
      file_names.push_back( dir_name + ent->d_name);
    }
    std::sort(file_names.begin(), file_names.end());
    closedir(dir);
    // deleting "." and ".." files from consideration.
    file_names.erase(file_names.begin());
    file_names.erase(file_names.begin());
  } else {
    /* could not open directory */
    printf("[ERROR][List_dir] The directory could not be opened %s\n",
           dir_name.c_str());
    exit(EXIT_FAILURE);
  }
  return file_names;
}