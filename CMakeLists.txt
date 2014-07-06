cmake_minimum_required(VERSION 2.8.11)

message(STATUS "Configuring QT Wallet")

project(qt_wallet)
set( APP_NAME "BitSharesXT" )

# Tell CMake where to find our custom Info.plist template
list( APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules" )

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Find the QtWidgets library
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5WebKit REQUIRED)
find_package(Qt5WebKitWidgets REQUIRED)

include_directories( ${CMAKE_SOURCE_DIR} ${Qt5WebKitWidgets_INCLUDE_DIRS} )
include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/include" )
include_directories( "${CMAKE_SOURCE_DIR}/libraries/db/include" )
include_directories( "${CMAKE_SOURCE_DIR}/libraries/blockchain/include" )
include_directories( "${CMAKE_SOURCE_DIR}/libraries/wallet/include" )
include_directories( "${CMAKE_SOURCE_DIR}/libraries/client/include" )
include_directories( "${CMAKE_SOURCE_DIR}/libraries/net/include" )
include_directories( "${CMAKE_SOURCE_DIR}/libraries/rpc/include" )
include_directories( "${CMAKE_SOURCE_DIR}/libraries/cli/include" )
include_directories( "${CMAKE_SOURCE_DIR}/libraries/utilities/include" )
include_directories( "${CMAKE_SOURCE_DIR}/libraries/bitcoin_import/include" )

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
  html5viewer/html5viewer.cpp
  images/bitshares.icns
)

set( CUSTOM_URL_SCHEME "xts" )
configure_file( "${CMAKE_CURRENT_SOURCE_DIR}/config.hpp.in" "${CMAKE_CURRENT_BINARY_DIR}/config.hpp" )

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
    SET( MACOSX_BUNDLE_GUI_IDENTIFIER "org.bitshares.xts" ) 
    SET( MACOSX_BUNDLE_ICON_FILE "bitshares.icns" )
    SET( MACOSX_BUNDLE_INFO_STRING "BitShares XTS - version ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" ) 
    SET( MACOSX_BUNDLE_SHORT_VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" ) 
    SET( MACOSX_BUNDLE_LONG_VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" ) 
    SET( MACOSX_BUNDLE_BUNDLE_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" ) 
    SET( MACOSX_BUNDLE_COPYRIGHT "(C) 2014 God" )
    SET( MACOSX_BUNDLE_URL_HANDLER_NAME "org.bitshares.xts" )
    SET( MACOSX_BUNDLE_URL_HANDLER_SCHEME "${CUSTOM_URL_SCHEME}" )
    set_source_files_properties("images/bitshares.icns" PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
ENDIF()

IF( WIN32 )
    configure_file( windows/resource.rc resource.rc COPYONLY )
    configure_file( windows/resource.h resource.h COPYONLY )
    configure_file( images/qtapp.ico qtapp.ico COPYONLY )
    list( APPEND SOURCES ${CMAKE_CURRENT_BINARY_DIR}/resource.rc )
    configure_file( windows/setup.iss.in setup.iss )
ENDIF()

IF( IS_DIRECTORY "${CMAKE_SOURCE_DIR}/programs/web_wallet" )
    add_custom_target(Lineman lineman build WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/programs/web_wallet")
    add_custom_target(NukeHtdocs rm -rf "${CMAKE_SOURCE_DIR}/programs/qt_wallet/htdocs")
    add_custom_target(buildweb cp -r "${CMAKE_SOURCE_DIR}/programs/web_wallet/dist" "${CMAKE_SOURCE_DIR}/programs/qt_wallet/htdocs" DEPENDS Lineman NukeHtdocs)
ELSE()
    message( FATAL_ERROR "Cannot find web wallet. Unable to continue." )
ENDIF()

# Tell CMake to create the qt_wallet executable
add_executable( ${APP_NAME} WIN32 MACOSX_BUNDLE ${SOURCES} )

# Workaround because CMake is broken and won't copy the stupid icon in itself
#IF( APPLE )
#    add_custom_command(TARGET ${APP_NAME} POST_BUILD COMMAND cp "${OSX_ICON_FILE}" "${CMAKE_CURRENT_BINARY_DIR}/${APP_NAME}.app/Contents/Resources/" )
#ENDIF()

QT5_ADD_RESOURCES( BitSharesQRC  bitshares.qrc )
QT5_ADD_RESOURCES( HTDOCS  htdocs.qrc )

# Use the Widgets module from Qt 5.
target_link_libraries( ${APP_NAME} Qt5::Widgets Qt5::WebKit Qt5::WebKitWidgets
  bts_wallet bts_rpc bts_cli bts_blockchain bts_db bts_net bts_client fc
  ${READLINE_LIBRARIES} ${CMAKE_DL_LIBS} ${PLATFORM_SPECIFIC_LIBS} ${crypto_library} ${ZLIB_LIBRARY} upnpc-static )

include( DeployQt4 )
include( InstallRequiredSystemLibraries )
install( TARGETS ${APP_NAME} DESTINATION "." )

IF( APPLE )
  set( CPACK_GENERATOR "DragNDrop" )
  include( CPack )
  set( PLUGINS "")
  list( APPEND PLUGINS "${QT_PATH}/plugins/platforms/libqcocoa.dylib" )
  list( APPEND PLUGINS "${QT_PATH}/plugins/imageformats/libqjpeg.dylib" )
  install_qt4_executable( ${APP_NAME}.app "${PLUGINS}" )
ENDIF()


