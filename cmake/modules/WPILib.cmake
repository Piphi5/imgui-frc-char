# ==============================================================================
# Platform and Architecture Detection
# ==============================================================================
if (WIN32)
  # Windows is currently not supported.
  message(FATAL_ERROR "Windows is currently not a supported platform.")
elseif(APPLE)
  # Only Intel macOS support exists here; Apple Silicon will be added
  # when upstream adds support.
  set(WPILIB_PLATFORM    "osx")
  set(WPILIB_ARCH       "x86-64")
  set(WPILIB_STATIC_EXT ".a")
  set(WPILIB_SHARED_EXT ".dylib")
else()
  set(WPILIB_PLATFORM    "linux")
  set(WPILIB_ARCH       "x86-64")
  set(WPILIB_STATIC_EXT ".a")
  set(WPILIB_SHARED_EXT ".so")
endif()

# ==============================================================================
# Artifactory Setup
# ==============================================================================
set(WPILIB_ARTIFACTORY_URL "https://frcmaven.wpi.edu/artifactory/release")

# ==============================================================================
# Individual Library Setup
# ==============================================================================
macro(add_individual_library dirs artifact version lib_name linkage)
  if (${linkage} STREQUAL "STATIC")
    set(LIB_LOC "${lib_name}/bin/${WPILIB_PLATFORM}/${WPILIB_ARCH}/static/lib${lib_name}${WPILIB_STATIC_EXT}")
  else()
    set(LIB_LOC "${lib_name}/bin/${WPILIB_PLATFORM}/${WPILIB_ARCH}/shared/lib${lib_name}${WPILIB_SHARED_EXT}")
  endif()

  if (NOT EXISTS ${LIB_LOC})
    # Get the base URL to the artifact.
    set(BASE_ARTIFACT_URL "${WPILIB_ARTIFACTORY_URL}/${dirs}/${version}")
    set(BASE_ARTIFACT_URL "${BASE_ARTIFACT_URL}/${artifact}-${version}")

    # Create the URL for the headers.
    set(HEADER_ARTIFACT_URL "${BASE_ARTIFACT_URL}-headers.zip")

    # Create the URL for the binaries.
    set(BINARY_ARTIFACT_URL "${BASE_ARTIFACT_URL}-${WPILIB_PLATFORM}${WPILIB_ARCH}")

    # Add the "static" classifier if we want to build static libraries.
    if (${linkage} STREQUAL "STATIC")
      set(BINARY_ARTIFACT_URL "${BINARY_ARTIFACT_URL}static")
    endif()

    # Add the ".zip extension"
    set(BINARY_ARTIFACT_URL "${BINARY_ARTIFACT_URL}.zip")

    message("Downloading... ${BINARY_ARTIFACT_URL}")

    # Download the artifacts.
    file(DOWNLOAD ${HEADER_ARTIFACT_URL} hdr.zip)
    file(DOWNLOAD ${BINARY_ARTIFACT_URL} bin.zip)

    # Extract the artifacts.
    file(ARCHIVE_EXTRACT INPUT hdr.zip DESTINATION "${lib_name}/include")
    file(ARCHIVE_EXTRACT INPUT bin.zip DESTINATION "${lib_name}/bin")

    # Delete the ZIP files.
    file(REMOVE hdr.zip bin.zip)
  endif()

  add_library(${lib_name} ${linkage} IMPORTED)
  set_property(TARGET ${lib_name} PROPERTY IMPORTED_LOCATION ${LIB_LOC})
  target_include_directories(${lib_name} INTERFACE "${CMAKE_BINARY_DIR}/${lib_name}/include")
endmacro()


# ==============================================================================
# Setup All Targets
# ==============================================================================
macro(add_wpilib_targets version linkage)
  find_package(OpenCV REQUIRED)
  add_individual_library(
    edu/wpi/first/wpigui/wpigui-cpp wpigui-cpp ${version} wpigui STATIC)
  add_individual_library(
    edu/wpi/first/wpimath/wpimath-cpp wpimath-cpp ${version} wpimath ${linkage})
  add_individual_library(
    edu/wpi/first/wpiutil/wpiutil-cpp wpiutil-cpp ${version} wpiutil ${linkage})
  add_individual_library(
    edu/wpi/first/wpilibc/wpilibc-cpp wpilibc-cpp ${version} wpilibc ${linkage})
  add_individual_library(
    edu/wpi/first/hal/hal-cpp hal-cpp ${version} wpiHal ${linkage})
  add_individual_library(
    edu/wpi/first/ntcore/ntcore-cpp ntcore-cpp ${version} ntcore ${linkage})
  add_individual_library(
    edu/wpi/first/cscore/cscore-cpp cscore-cpp ${version} cscore ${linkage})
  add_individual_library(
    edu/wpi/first/cameraserver/cameraserver-cpp cameraserver-cpp ${version} cameraserver ${linkage})
endmacro()
