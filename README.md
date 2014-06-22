# QT Wallet (Based on Web Wallet)

## Configuration and installation

### Unix

Download and install QT 5.3, see http://qt-project.org/downloads
Set envronment variable CMAKE_PREFIX_PATH to point to clang_64 in your QT directory, e.g.:
    $ export CMAKE_PREFIX_PATH=/Users/user/Qt5.3.0/5.3/clang_64

Use CMake to configure bitshares_toolkit, set INCLUDE_QT_WALLET to TRUE or to ON, e.g.
    $ cmake -DINCLUDE_QT_WALLET=ON ../bitshares_toolkit

The wallet needs access to the built web application in order to build. This can be built automatically by invoking the 'buildweb' rule via 'make buildweb'.
Note that the web wallet is expected to be in .../bitshares_toolkit/programs/web_wallet.

If there were no compilation errors, the executable will be located in programs/qt_wallet
Now you need to run it in a way similar to bitshares_client - it accepts the same command line parameters as bitshares_client or reads them from config.json.

To create installation package, type:
    $ make package


### Windows

Clone qt_wallet repo into bitshares_toolkit/programs/qt_wallet.

Download and install Windows binary version of QT 5.3 into the same folder as bitshares_toolkit.

Edit setenv.bat and add CMAKE_PREFIX_PATH:

set CMAKE_PREFIX_PATH=%BITSHARES_ROOT%\Qt5.3.0_x86\5.3\msvc2013


Add %CMAKE_PREFIX_PATH%\bin to PATH variable:

set PATH=%BITSHARES_ROOT%\bin;%BITSHARES_ROOT%\Cmake\bin;%BITSHARES_ROOT%\boost\stage\lib;%CMAKE_PREFIX_PATH%\bin;%PATH%


Follow Windows build instructions from BUILD_WIN32.md.

Don't forget to enable INCLUDE_QT_WALLET option in CMake.


After qt_wallet is built (Release version is required) you can create installation package using Inno Setup: 
    1. Download and install Inno Setup http://www.jrsoftware.org/isdl.php
    2. Double click setup.iss located in BUILD_DIR\programs\qt_wallet
    3. Compile setup project, the installation package will be placed in the same folder
