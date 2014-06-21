cmake_minimum_required(VERSION 2.8.11)

message(STATUS "Configuring QT Wallet")

project(qt_wallet)

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

#message(${Qt5WebKitWidgets_INCLUDE_DIRS})

include_directories(${CMAKE_SOURCE_DIR} ${Qt5WebKitWidgets_INCLUDE_DIRS})

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

set( SOURCES
  htdocs.qrc
  bitshares.qrc
  qrc_bitshares.cpp
  qrc_htdocs.cpp
  main.cpp
  ClientWrapper.cpp 
  Utilities.cpp 
  html5viewer/html5viewer.cpp
)

set(CPACK_BUNDLE_NAME "BitSharesXT")
set(CPACK_PACKAGE_NAME ${CPACK_BUNDLE_NAME})
set(CPACK_PACKAGE_EXECUTABLES qt_wallet ${CPACK_BUNDLE_NAME})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "package description summary")
set(CPACK_PACKAGE_VENDOR "BitShares")
set(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})
set(CPACK_NSIS_MUI_ICON ${CMAKE_CURRENT_SOURCE_DIR}/images/qtapp.ico)
set(CPACK_NSIS_MUI_UNIICON ${CMAKE_CURRENT_SOURCE_DIR}/images/qtapp.ico)
get_filename_component(QT_PATH ${Qt5Core_DIR}/../../../ ABSOLUTE)

IF( APPLE )
    set ( OSX_ICON_FILE "${CMAKE_CURRENT_SOURCE_DIR}/images/bitshares.icns" )
    SET_SOURCE_FILES_PROPERTIES( "${OSX_ICON_FILE}" PROPERTIES MACOSX_PACKAGE_LOCATION Resources )
    SET( MACOSX_BUNDLE_BUNDLE_NAME ${CPACK_BUNDLE_NAME} )
    SET( MACOSX_BUNDLE_GUI_IDENTIFIER "org.bitshares.xts" ) 
    SET( MACOSX_BUNDLE_ICON_FILE bitshares.icns )
    SET( MACOSX_BUNDLE_INFO_STRING "BitShares XTS - version ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" ) 
    SET( MACOSX_BUNDLE_SHORT_VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" ) 
    SET( MACOSX_BUNDLE_LONG_VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" ) 
    SET( MACOSX_BUNDLE_BUNDLE_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" ) 
    SET( MACOSX_BUNDLE_COPYRIGHT "(C) 2014 God" )
ENDIF()

IF( WIN32 )
    configure_file(windows/resource.rc resource.rc COPYONLY)
    configure_file(windows/resource.h resource.h COPYONLY)
    configure_file(images/qtapp.ico qtapp.ico COPYONLY)
    list(APPEND SOURCES ${CMAKE_CURRENT_BINARY_DIR}/resource.rc)
    configure_file(windows/setup.iss.in setup.iss)
ENDIF()

set( QT_WALLET_REBUILD_WEB_INTERFACE  FALSE CACHE BOOL "Build web interface via lineman ")

IF( ${QT_WALLET_REBUILD_WEB_INTERFACE} )
   IF( IS_DIRECTORY "${CMAKE_SOURCE_DIR}/programs/web_wallet" )
       add_custom_target(Lineman lineman build WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/programs/web_wallet")
       add_custom_target(NukeHtdocs rm -rf "${CMAKE_SOURCE_DIR}/programs/qt_wallet/htdocs")
       add_custom_target(buildweb cp -r "${CMAKE_SOURCE_DIR}/programs/web_wallet/dist" "${CMAKE_SOURCE_DIR}/programs/qt_wallet/htdocs" DEPENDS Lineman NukeHtdocs)
   ELSE()
       message( FATAL_ERROR "Cannot find web wallet. Unable to continue." )
   ENDIF()
ENDIF()

# Tell CMake to create the qt_wallet executable
add_executable(qt_wallet WIN32 MACOSX_BUNDLE ${SOURCES} "${OSX_ICON_FILE}" )
# Workaround because CMake is broken and won't copy the stupid icon in itself
IF( APPLE )
    add_custom_command(TARGET qt_wallet POST_BUILD COMMAND cp "${OSX_ICON_FILE}" "${CMAKE_CURRENT_BINARY_DIR}/qt_wallet.app/Contents/Resources/" )
ENDIF()

QT5_ADD_RESOURCES( BitSharesQRC  bitshares.qrc )
QT5_ADD_RESOURCES( HTDOCS  htdocs.qrc )

# Use the Widgets module from Qt 5.
target_link_libraries(qt_wallet Qt5::Widgets Qt5::WebKit Qt5::WebKitWidgets
  bts_wallet bts_rpc bts_cli bts_blockchain bts_db bts_net bts_client fc
  ${READLINE_LIBRARIES} ${CMAKE_DL_LIBS} ${PLATFORM_SPECIFIC_LIBS} ${crypto_library} ${ZLIB_LIBRARY} upnpc-static)

include(DeployQt4)

IF( APPLE )
  set(CPACK_GENERATOR "DragNDrop")
  set(APP_EXTENSION ".app")
  set(APP_DIR ".")
ELSEIF ( WIN32 )
  set(CPACK_GENERATOR "NSIS")
  set(APP_EXTENSION ".exe")
  set(APP_DIR "bin")
ENDIF()

install(TARGETS qt_wallet DESTINATION ${APP_DIR})

include(CPack)
include( InstallRequiredSystemLibraries )

IF( APPLE )
  install_qt4_executable(qt_wallet${APP_EXTENSION} "${QT_PATH}/plugins/platforms/libqcocoa.dylib")
ELSEIF ( WIN32 )
  install_qt4_executable(${APP_DIR}/qt_wallet${APP_EXTENSION} "${QT_PATH}/plugins/platforms/qwindows.dll")
ENDIF()

