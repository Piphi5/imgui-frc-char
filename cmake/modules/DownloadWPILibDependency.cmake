set(artifactory_url "https://frcmaven.wpi.edu/artifactory/release/edu/wpi/first/")

macro(download_wpilib_dependency library_name version os arch)
  if (NOT EXISTS ${library_name})
    set(library_base "${artifactory_url}${library_name}/${library_name}-cpp/${version}/${library_name}-cpp-${version}")
    
    file(DOWNLOAD "${library_base}-headers.zip" headers.zip)
    file(DOWNLOAD "${library_base}-${os}${arch}staticdebug.zip" library.zip)

    file(ARCHIVE_EXTRACT INPUT headers.zip DESTINATION "${library_name}/include/")
    file(ARCHIVE_EXTRACT INPUT library.zip DESTINATION "lib/" PATTERNS "${os}/${arch}/static/*.a")

    file(REMOVE headers.zip library.zip)
  endif()
endmacro()