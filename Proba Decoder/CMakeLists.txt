cmake_minimum_required (VERSION 3.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project (Proba-Decoder)
file(GLOB_RECURSE Proba-Decoder_CPPS src/*.cpp src/viterbi_lib/metrics.c src/viterbi_lib/tab.c src/viterbi_lib/viterbi.c)

add_executable(Proba-Decoder ${Proba-Decoder_CPPS})

target_include_directories(Proba-Decoder PUBLIC src)

target_link_libraries(Proba-Decoder PUBLIC m)

find_package(Threads REQUIRED)
target_link_libraries(Proba-Decoder PUBLIC Threads::Threads)

target_link_libraries (Proba-Decoder PUBLIC "correct")

if(WIN32 AND NOT MINGW)
    find_package(PNG CONFIG REQUIRED)
else()
    find_package(PNG REQUIRED)
endif()
target_link_libraries(Proba-Decoder PUBLIC PNG::PNG)

find_library(JPEG_LIBRARY jpeg REQUIRED)
target_link_libraries(Proba-Decoder PUBLIC ${JPEG_LIBRARY})

install(TARGETS Proba-Decoder DESTINATION bin)