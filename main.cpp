#include "BitSharesApp.hpp"
#include <boost/filesystem.hpp>

int main( int argc, char** argv )
{
  #ifdef WIN32
    // Due to a bug in boost that we don't fully understand, we get errors in boost::filesystem::path
    // when used from a thread if we don't do this first.
    boost::filesystem::path::imbue(std::locale());
  #endif
  return BitSharesApp::run(argc, argv);
}
