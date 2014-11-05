#include "CrashReport.hpp"

#include "CrashRpt/include/CrashRpt.h"

namespace bts { namespace rpt {

CrashReport::CrashReport()  
{
}

CrashReport::~CrashReport()
{
}

#ifdef WIN32
  void CrashReport::install(const char* appName, const char* appVersion, const QFile& logFilePath)
  {
    // Define CrashRpt configuration parameters
    CR_INSTALL_INFO info = { 0 };
    info.cb = sizeof(CR_INSTALL_INFO);
    info.pszAppName = appName;
    info.pszAppVersion = appVersion;
    info.pszEmailSubject = nullptr;
    info.pszEmailTo = "sales@syncad.com";
    info.pszUrl = "http://invictus.syncad.com/crash_report.html";
    info.uPriorities[CR_HTTP] = 3;  // First try send report over HTTP 
    info.uPriorities[CR_SMTP] = 2;  // Second try send report over SMTP  
    info.uPriorities[CR_SMAPI] = 1; // Third try send report over Simple MAPI    
    // Install all available exception handlers
    info.dwFlags |= CR_INST_ALL_POSSIBLE_HANDLERS | CR_INST_CRT_EXCEPTION_HANDLERS;
    info.dwFlags |= CR_INST_SEND_QUEUED_REPORTS;
    // Define the Privacy Policy URL 
    info.pszPrivacyPolicyURL = "http://invictus.syncad.com/crash_privacy.html";

    // Install crash reporting
    int nResult = crInstall(&info);
    if (nResult != 0)
    {
      // Something goes wrong. Get error message.
      char szErrorMsg[512] = { 0 };
      crGetLastErrorMsg(szErrorMsg, 512);
      elog("Cannot install CrsshRpt error handler: ${e}", ("e", szErrorMsg));
      return;
    }
    else
    {
      wlog("CrashRpt handler installed successfully");
    }

    auto logPathString = logFilePath.fileName().toStdString();

    // Add our log file to the error report
    crAddFile2(logPathString.c_str(), NULL, "Log File", CR_AF_MAKE_FILE_COPY);

    // We want the screenshot of the entire desktop is to be added on crash
    crAddScreenshot2(CR_AS_PROCESS_WINDOWS | CR_AS_USE_JPEG_FORMAT, 0);
  }

  void CrashReport::uninstall()
  {
    crUninstall();
  }

#else

  void CrashReport::install(const char* appName, const char* appVersion, const QFile& logFilePath)
  {
    /// Nothing to do here since no crash report support available
  }

  void CrashReport::uninstall()
  {
    /// Nothing to do here since no crash report support available
  }

#endif

} } // bts:rpt
