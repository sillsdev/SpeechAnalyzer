cmake_minimum_required (VERSION 2.8)

project(SA)

SET(CMAKE_INSTALL_PREFIX "/working/sil/msea22/output" CACHE PATH "Speech Analyzer install prefix") 

include_directories( 
	${PROJECT_SOURCE_DIR}/SaExe
	${PROJECT_SOURCE_DIR}/MBROLA
	${PROJECT_SOURCE_DIR}/SA_DSP
	${PROJECT_SOURCE_DIR}/SA_KLATT
	${PROJECT_SOURCE_DIR}/WaveUtils
	${PROJECT_SOURCE_DIR}/ZGraph
)

add_subdirectory(SaExe)
add_subdirectory(MBROLA)
add_subdirectory(SA_DSP)
add_subdirectory(SA_KLATT)
add_subdirectory(WaveUtils)
add_subdirectory(ZGraph)
#add_subdirectory(SA_ENU)
#add_subdirectory(SilUtils)
#add_subdirectory(SpeechToolsUtils)
#add_subdirectory(ST_Audio)
#add_subdirectory(yeti.mmedia)
#add_subdirectory(yeti.wmfsdk)

