#pragma once

namespace bts { namespace rpt {

  #ifdef WIN32
  #define APP_TRY /*try*/
  #define APP_CATCH /*Nothing*/
  #else
  #define APP_TRY try
  #define APP_CATCH \
    catch(const fc::exception& e) \
      {\
      onExceptionCaught(e);\
    }\
    catch(...)\
      {\
      onUnknownExceptionCaught();\
    }
  #endif

  class CrashReport
  {
  public:
    CrashReport();
    virtual ~CrashReport();
    /// installCrashRptHandler
    void install(const char* appName, const char* appVersion, const QFile& logFilePath);
    /// uninstallCrashRptHandler
    void uninstall();

  };

} }// bts:rpt
