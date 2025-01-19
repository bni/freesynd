include(FetchContent)

FetchContent_Declare(
  libAdl
  GIT_REPOSITORY https://github.com/Wohlstand/libADLMIDI.git
  # We use the version in the main branch because release 1.5.1
  # does not build on MacOS
  # TODO : when future release is available, switch to it
  #GIT_TAG        84d27bc2bdbd6dd249537a7f7d2450cbd402482e # release-1.5.1
)

set(USE_JAVA_EMULATOR OFF CACHE INTERNAL "Turn off JAva Emulator")
set(WITH_MUS_SUPPORT OFF CACHE INTERNAL "Turn off Mus Support")

FetchContent_MakeAvailable(libAdl)