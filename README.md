# QT Wallet (Based on Web Wallet)

## Configuration and installation

Download and install QT 5.3, see http://qt-project.org/downloads
Set envronment variable CMAKE_PREFIX_PATH to point to clang_64 in your QT directory, e.g.:
    $ export CMAKE_PREFIX_PATH=/Users/user/Qt5.3.0/5.3/clang_64

Use CMake to configure bitshares_toolkit, set INCLUDE_QT_WALLET to TRUE or to ON, e.g.
    $ cmake -DINCLUDE_QT_WALLET=ON ../bitshares_toolkit

The wallet needs access to the built web application in order to build. This can be built automatically by invoking the 'buildweb' rule via 'make buildweb'.
Note that the web wallet is expected to be in .../bitshares_toolkit/programs/web_wallet.

If there were no compilation errors, the executable should be located in programs/qt_wallet
Now you need to run it in a way similar to bitshares_client - it accepts the same command line parameters as bitshares_client or reads them from config.json.
Currently it expects rpc user to be 'user' and password to be 'pass' (this may change in future) and http rpc port 5680.

Here is an example:
    $ qt_wallet --data-dir /tmp/qt_wallet

Note. Before starting qt_wallet please edit config.json and specify htdocs path as either web_wallet/dist (production) or web_wallet/generated (development).

# Note. Temporary splash screen png path is hardcoded os the splash screen may not displayed correclty.
