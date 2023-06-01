# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/darks/esp/esp-idf/components/bootloader/subproject"
  "C:/Users/darks/Documents/IFPB/Engenharia/embarcados/project-embarcados_2/build/bootloader"
  "C:/Users/darks/Documents/IFPB/Engenharia/embarcados/project-embarcados_2/build/bootloader-prefix"
  "C:/Users/darks/Documents/IFPB/Engenharia/embarcados/project-embarcados_2/build/bootloader-prefix/tmp"
  "C:/Users/darks/Documents/IFPB/Engenharia/embarcados/project-embarcados_2/build/bootloader-prefix/src/bootloader-stamp"
  "C:/Users/darks/Documents/IFPB/Engenharia/embarcados/project-embarcados_2/build/bootloader-prefix/src"
  "C:/Users/darks/Documents/IFPB/Engenharia/embarcados/project-embarcados_2/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/darks/Documents/IFPB/Engenharia/embarcados/project-embarcados_2/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
