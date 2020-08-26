cmake_minimum_required(VERSION 3.8)
project("ps1_shell")
#project("Design and Engineering of Computing Systems")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_VERBOSE_MAKEFILE on)

file(GLOB sourceFILES *.cpp *.c)

FOREACH (sourceFile ${sourceFILES})
    get_filename_component(sourceFile_Path ${sourceFile} PATH)
    MESSAGE(STATUS "Process file, path: ${sourceFile}")

    get_filename_component(sourceFile_Name ${sourceFile} NAME)
    string(REPLACE " " "_" sourceFile_Name ${sourceFile_Name})
    string(REPLACE "'" "" sourceFile_Name ${sourceFile_Name})
    string(REPLACE "\"" "" sourceFile_Name ${sourceFile_Name})
    string(REPLACE "(" "_" sourceFile_Name ${sourceFile_Name})
    string(REPLACE ")" "_" sourceFile_Name ${sourceFile_Name})
    MESSAGE(STATUS "Process file, name: \"${sourceFile_Name}\"")

    add_executable("${sourceFile_Name}" "${sourceFile}")
endforeach (sourceFile)
