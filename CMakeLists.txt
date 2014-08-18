cmake_minimum_required(VERSION 2.8.11)

project(qt_wallet)
set( APP_NAME "BitSharesX" )

# Tell CMake where to find our custom Info.plist template
list( APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules" )

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

IF(WIN32)
   # Build all windows projects with /EHsc, which will prevent the compiler from 
   # translating structured exceptions into C++ exceptions (which would make
   # our crash reporter less useful)
   IF (NOT CMAKE_CXX_FLAGS MATCHES "/EHsc")
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
   ENDIF()
ENDIF(WIN32)

SET (BITSHARES_ROOT $ENV{BITSHARES_ROOT})
IF("${BITSHARES_ROOT}" STREQUAL "")
  SET(BITSHARES_ROOT "./")
ENDIF("${BITSHARES_ROOT}" STREQUAL "")

# Let's configure binaries output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BITSHARES_ROOT}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${BITSHARES_ROOT}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${BITSHARES_ROOT}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${BITSHARES_ROOT}/bin)

#This variable will be filled just for Win32 platform
SET (CrashRpt_LIBRARIES "")

# Find the QtWidgets library
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5WebKit REQUIRED)
find_package(Qt5WebKitWidgets REQUIRED)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} ${Qt5WebKitWidgets_INCLUDE_DIRS} )
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/include" )
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/libraries/db/include" )
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/libraries/blockchain/include" )
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/libraries/wallet/include" )
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/libraries/client/include" )
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/libraries/net/include" )
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/libraries/rpc/include" )
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/libraries/cli/include" )
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/libraries/utilities/include" )
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/libraries/bitcoin_import/include" )

ADD_DEFINITIONS(-DQT_NO_KEYWORDS)

set( SOURCES
  htdocs.qrc
  bitshares.qrc
  qrc_bitshares.cpp
  qrc_htdocs.cpp
  main.cpp
  ClientWrapper.cpp 
  Utilities.cpp 
  MainWindow.cpp
  BitSharesApp.cpp
  html5viewer/html5viewer.cpp
  images/bitshares.icns
)

set( CPACK_BUNDLE_NAME ${APP_NAME} )
set( CPACK_PACKAGE_NAME ${CPACK_BUNDLE_NAME} )
set( CPACK_PACKAGE_EXECUTABLES qt_wallet ${CPACK_BUNDLE_NAME} )
set( CPACK_PACKAGE_DESCRIPTION_SUMMARY "package description summary" )
set( CPACK_PACKAGE_VENDOR "BitShares" )
set( CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR} )
set( CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR} )
set( CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH} )
set( CPACK_DMG_VOLUME_NAME ${CPACK_BUNDLE_NAME} )
set( CPACK_PACKAGE_FILE_NAME ${CPACK_BUNDLE_NAME} )
get_filename_component( QT_PATH ${Qt5Core_DIR}/../../../ ABSOLUTE )

IF( APPLE )
    set ( OSX_ICON_FILE "${CMAKE_CURRENT_SOURCE_DIR}/images/bitshares.icns" )
    SET_SOURCE_FILES_PROPERTIES( "${OSX_ICON_FILE}" PROPERTIES MACOSX_PACKAGE_LOCATION Resources )
    SET( MACOSX_BUNDLE_BUNDLE_NAME ${CPACK_BUNDLE_NAME} )
    SET( MACOSX_BUNDLE_EXECUTABLE_NAME ${CPACK_BUNDLE_NAME} )
    SET( MACOSX_BUNDLE_GUI_IDENTIFIER "org.bitshares.btsx" ) 
    SET( MACOSX_BUNDLE_ICON_FILE "bitshares.icns" )
    SET( MACOSX_BUNDLE_INFO_STRING "BitShares BTSX - version ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" ) 
    SET( MACOSX_BUNDLE_SHORT_VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" ) 
    SET( MACOSX_BUNDLE_LONG_VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" ) 
    SET( MACOSX_BUNDLE_BUNDLE_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" ) 
    SET( MACOSX_BUNDLE_COPYRIGHT "(C) 2014 Dac Sun Limited" )
    SET( MACOSX_BUNDLE_URL_HANDLER_NAME "${MACOSX_BUNDLE_GUI_IDENTIFIER}" )
    SET( MACOSX_BUNDLE_URL_HANDLER_SCHEME "${CUSTOM_URL_SCHEME}" )
    set_source_files_properties("images/bitshares.icns" PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
ENDIF()

IF( "${CMAKE_SYSTEM_NAME}" MATCHES "Linux" )
    CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/BitSharesXT.desktop.in" "${CMAKE_CURRENT_BINARY_DIR}/${APP_NAME}.desktop")
ENDIF()

IF( WIN32 )
    configure_file( windows/resource.rc resource.rc COPYONLY )
    configure_file( windows/resource.h resource.h COPYONLY )
    configure_file( images/qtapp.ico qtapp.ico COPYONLY )
    list( APPEND SOURCES ${CMAKE_CURRENT_BINARY_DIR}/resource.rc )
    set(OPENSSL_ROOT_DIR $ENV{OPENSSL_ROOT_DIR} )
    configure_file( windows/setup.iss.in setup.iss )

    SET (CrashRpt_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../CrashRpt")

    # Warning this lib doesn't have an associated .cmake
    find_library(CrashRpt_LIBRARIES_RELEASE CrashRpt1402 HINTS "${CrashRpt_DIR}/lib")
    LIST(APPEND CrashRpt_LIBRARIES optimized ${CrashRpt_LIBRARIES_RELEASE})
    MESSAGE(STATUS "Found CrashRpt libraries: ${CrashRpt_LIBRARIES}")
    set(CRASHRPT_BINARIES_TO_INSTALL "${CrashRpt_DIR}/bin/crashrpt_lang.ini"
                                     "${CrashRpt_DIR}/bin/CrashRpt1402.dll"
                                     "${CrashRpt_DIR}/bin/CrashSender1402.exe"
                                     "${CrashRpt_DIR}/bin/dbghelp.dll")
ENDIF(WIN32)

# Tell CMake to create the qt_wallet executable
add_executable( ${APP_NAME} WIN32 MACOSX_BUNDLE ${SOURCES} )

IF(NOT CMAKE_GENERATOR STREQUAL "Xcode") # for some reason lineman couldn't be invoked from XCode, so let's skip this in XCode
  IF( IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../web_wallet")
    IF( UNIX )
      add_custom_target(Lineman ./node_modules/lineman/cli.js build WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../web_wallet")
    ELSE()
      add_custom_target(Lineman lineman build WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../web_wallet")
    ENDIF()
    add_custom_target(NukeHtdocs ${CMAKE_COMMAND} -E remove_directory "${CMAKE_CURRENT_SOURCE_DIR}/htdocs")
    add_custom_target(buildweb ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/../web_wallet/dist" "${CMAKE_CURRENT_SOURCE_DIR}/htdocs" DEPENDS Lineman NukeHtdocs)
    # If you are tempted to uncomment the following line, DO NOT DO IT! Do not! Just run `make buildweb` manually.
#    add_dependencies(${APP_NAME} buildweb)
  ELSE()
      message( FATAL_ERROR "Cannot find web wallet. Unable to continue." )
  ENDIF()
ENDIF()

# Looks like multiple post build commands doesn't work correctly (last one overwrites previous defs)
SET(POST_BUILD_STEP_COMMANDS )

SET(POST_BUILD_STEP_COMMANDS ${POST_BUILD_STEP_COMMANDS}
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/htdocs" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/htdocs")

IF(WIN32)
  foreach(binary ${CRASHRPT_BINARIES_TO_INSTALL})
    set(POST_BUILD_STEP_COMMANDS ${POST_BUILD_STEP_COMMANDS}
      COMMAND ${CMAKE_COMMAND} -E copy_if_different "${binary}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
  endforeach()
ENDIF()

IF( APPLE )
    find_library(APP_SERVICES_LIBRARY ApplicationServices)
    target_link_libraries(${APP_NAME} ${APP_SERVICES_LIBRARY})
ENDIF()

QT5_ADD_RESOURCES( BitSharesQRC  bitshares.qrc )
QT5_ADD_RESOURCES( HTDOCS  htdocs.qrc )

# Use the Widgets module from Qt 5.
target_link_libraries( ${APP_NAME} Qt5::Widgets Qt5::WebKit Qt5::WebKitWidgets
  bts_wallet bts_rpc bts_cli bts_blockchain bts_db bts_net bts_client fc  
  ${READLINE_LIBRARIES} ${CMAKE_DL_LIBS} ${PLATFORM_SPECIFIC_LIBS} ${crypto_library} ${CrashRpt_LIBRARIES} ${ZLIB_LIBRARY} upnpc-static )

include( DeployQt4 )
include( InstallRequiredSystemLibraries )
install( TARGETS ${APP_NAME} DESTINATION "." )

IF(WIN32)
  INSTALL(FILES ${CRASHRPT_BINARIES_TO_INSTALL} DESTINATION . CONFIGURATIONS Release COMPONENT Runtime)
ENDIF()

ADD_CUSTOM_COMMAND(TARGET ${APP_NAME} POST_BUILD ${POST_BUILD_STEP_COMMANDS} 
  COMMENT "Copying binaries and other files into target directory."
)

IF( APPLE )
  set( CPACK_GENERATOR "DragNDrop" )
  include( CPack )
  set( PLUGINS "")
  list( APPEND PLUGINS "${QT_PATH}/plugins/platforms/libqcocoa.dylib" )
  list( APPEND PLUGINS "${QT_PATH}/plugins/imageformats/libqjpeg.dylib" )
  install_qt4_executable( ${APP_NAME}.app "${PLUGINS}" )
ENDIF()


