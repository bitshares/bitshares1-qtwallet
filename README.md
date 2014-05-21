# QT Wallet (Based on Web Wallet)

## Configuration and installation

Download and install QT5, see http://qt-project.org/downloads
Set envronment variable CMAKE_PREFIX_PATH to point to clang_64 in your QT directory, e.g.:
    $ export CMAKE_PREFIX_PATH=/Users/user/Qt5.2.1/5.2.1/clang_64

Use CMake to configure bitshares_toolkit, set INCLUDE_QT_WALLET to TRUE or to ON, e.g.
    $ cmake -DINCLUDE_QT_WALLET=ON ../bitshares_toolkit

If there were no compilation errors, the executable should be located in programs/qt_wallet
Now you need to run it in a way similar to bitshares_client - it accepts the same command line parameters as bitshares_client or reads them from config.json.

Here is an example (needs to be updated - doesn't reflect recent changes):
    $ qt_wallet --data-dir /tmp/qt_wallet --trustee-address 9gWvCSLaAA67Rwg9AEvPAttUrdr9pXXKk --connect-to 107.170.30.182:8765

Note. Before starting qt_wallet please edit config.json and specify htdocs path as either web_wallet/dist (production) or web_wallet/generated.
