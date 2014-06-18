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

IF( APPLE )
    SET_SOURCE_FILES_PROPERTIES( "images/keyhotee.icns"
                                  PROPERTIES
                                   MACOSX_PACKAGE_LOCATION Resources
                                )
    SET( MACOSX_BUNDLE_BUNDLE_NAME "BitShares XTS" )
    SET( MACOSX_BUNDLE_GUI_IDENTIFIER "org.bitshares.xts" ) 
    set( MACOSX_BUNDLE_ICON_FILE keyhotee.icns )
    SET( MACOSX_BUNDLE_INFO_STRING "BitShares XTS - version ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" ) 
    SET( MACOSX_BUNDLE_SHORT_VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" ) 
    SET( MACOSX_BUNDLE_LONG_VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" ) 
    SET( MACOSX_BUNDLE_BUNDLE_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" ) 
    SET( MACOSX_BUNDLE_COPYRIGHT "(C) 2014 God" ) 
  
ENDIF( APPLE )

IF( IS_DIRECTORY "${CMAKE_SOURCE_DIR}/programs/web_wallet" )
    add_custom_target(Lineman lineman build WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/programs/web_wallet")
    add_custom_target(NukeHtdocs rm -rf "${CMAKE_SOURCE_DIR}/programs/qt_wallet/htdocs")
    add_custom_target(buildweb cp -r "${CMAKE_SOURCE_DIR}/programs/web_wallet/dist" "${CMAKE_SOURCE_DIR}/programs/qt_wallet/htdocs" DEPENDS Lineman NukeHtdocs)
ELSE()
    message( FATAL_ERROR "Cannot find web wallet. Unable to continue." )
ENDIF()

# Tell CMake to create the qt_wallet executable
add_executable(qt_wallet WIN32 MACOSX_BUNDLE htdocs.qrc bitshares.qrc qrc_bitshares.cpp qrc_htdocs.cpp main.cpp ClientWrapper.cpp Utilities.cpp html5viewer/html5viewer.cpp)

QT5_ADD_RESOURCES( BitSharesQRC  bitshares.qrc )
QT5_ADD_RESOURCES( HTDOCS  htdocs.qrc )

# Use the Widgets module from Qt 5.
target_link_libraries(qt_wallet Qt5::Widgets Qt5::WebKit Qt5::WebKitWidgets
  bts_wallet bts_rpc bts_cli bts_blockchain bts_db bts_net bts_client fc
  ${READLINE_LIBRARIES} ${CMAKE_DL_LIBS} ${PLATFORM_SPECIFIC_LIBS} ${crypto_library} ${ZLIB_LIBRARY} upnpc-static)

configure_file(config_dev.hpp.in config_dev.hpp)

include(DeployQt4)
install(TARGETS qt_wallet DESTINATION .)

set(CPACK_BUNDLE_NAME "BitSharesXTS")
set(CPACK_PACKAGE_NAME ${CPACK_BUNDLE_NAME})
set(CPACK_PACKAGE_EXECUTABLES qt_wallet ${CPACK_BUNDLE_NAME})
set(CPACK_PACKAGE_DESCRIPTION_FILE ${CMAKE_CURRENT_SOURCE_DIR}/README.md)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "package description summary")
set(CPACK_PACKAGE_VENDOR "BitShares")
set(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})

IF( APPLE )
  set(CPACK_GENERATOR "DragNDrop")
  set(APP_EXTENSION ".app")
ELSEIF ( WIN32 )
  set(CPACK_GENERATOR "NSIS")
  set(APP_EXTENSION ".exe")
ENDIF()

include(CPack)
include( InstallRequiredSystemLibraries )
install_qt4_executable(qt_wallet${APP_EXTENSION} "${Qt5Core_DIR}/../../../plugins/platforms/libqcocoa.dylib")
