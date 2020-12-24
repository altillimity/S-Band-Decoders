cmake_minimum_required (VERSION 3.0.0)

project (Proba-Viterbi-Decoder)
file(GLOB_RECURSE Proba-Viterbi-Decoder_CPPS src/*.cpp src/viterbi_lib/metrics.c src/viterbi_lib/tab.c src/viterbi_lib/viterbi.c)

add_executable(Proba-Viterbi-Decoder ${Proba-Viterbi-Decoder_CPPS})

target_include_directories(Proba-Viterbi-Decoder PUBLIC src)

target_link_libraries(Proba-Viterbi-Decoder PUBLIC m)

find_package(Threads REQUIRED)
target_link_libraries(Proba-Viterbi-Decoder PUBLIC Threads::Threads)

target_link_libraries (Proba-Viterbi-Decoder PUBLIC "correct")


install(TARGETS Proba-Viterbi-Decoder DESTINATION bin)