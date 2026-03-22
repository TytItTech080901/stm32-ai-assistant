cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME aiaudio)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# User sources maintained explicitly to keep the build graph predictable.
set(USER_FUNC_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/User/Func/Src/OLED.c
    ${CMAKE_CURRENT_SOURCE_DIR}/User/Func/Src/audio_frame.c
    ${CMAKE_CURRENT_SOURCE_DIR}/User/Func/Src/audio_prompt.c
    ${CMAKE_CURRENT_SOURCE_DIR}/User/Func/Src/fft_visualer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/User/Func/Src/inmp441.c
    ${CMAKE_CURRENT_SOURCE_DIR}/User/Func/Src/speaker.c
    ${CMAKE_CURRENT_SOURCE_DIR}/User/Func/Src/uart_cli.c
    ${CMAKE_CURRENT_SOURCE_DIR}/User/Func/Src/w25qxx.c
)

set(USER_TASK_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/User/Tasks/Src/user_AudioFbankTask.c
    ${CMAKE_CURRENT_SOURCE_DIR}/User/Tasks/Src/user_KwsInferTask.c
    ${CMAKE_CURRENT_SOURCE_DIR}/User/Tasks/Src/user_OledDisplayTask.c
    ${CMAKE_CURRENT_SOURCE_DIR}/User/Tasks/Src/user_ShowAudioTask.c
    ${CMAKE_CURRENT_SOURCE_DIR}/User/Tasks/Src/user_SpeakerTask.c
    ${CMAKE_CURRENT_SOURCE_DIR}/User/Tasks/Src/user_TasksInit.c
    ${CMAKE_CURRENT_SOURCE_DIR}/User/Tasks/Src/user_UsartCLITask.c
)

set(AUDIO_PREPROCESSING_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/AudioPreprocessing/Src/common_tables.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/AudioPreprocessing/Src/dct.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/AudioPreprocessing/Src/fbank.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/AudioPreprocessing/Src/feature_extraction.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/AudioPreprocessing/Src/fifo.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/AudioPreprocessing/Src/mel_filterbank.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/AudioPreprocessing/Src/window.c
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${USER_FUNC_SOURCES}
    ${USER_TASK_SOURCES}
    ${AUDIO_PREPROCESSING_SOURCES}
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    User/Tasks/Inc
    User/Func/Inc
    Middlewares/AudioPreprocessing/Inc
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Keep a legacy map filename for tools that still look for stm32-ai-assistant.map.
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>/${CMAKE_PROJECT_NAME}.map
        $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>/stm32-ai-assistant.map
)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)
