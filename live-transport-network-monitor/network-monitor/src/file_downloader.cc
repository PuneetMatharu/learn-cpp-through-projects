// The libcurl library
#include <curl/curl.h>

// Regular libraries
#include <filesystem>
#include <iostream>
#include <string>

// The matching header
#include "network-monitor/file_downloader.h"

namespace NetworkMonitor
{
  //===========================================================================
  /*! \brief Download a file from a remote HTTPS URL.

     \param destination   The full path and filename of the output file. The
                          path to the file must exist.
     \param ca_cert_file  The path to a cacert.pem file to perform certificate
                          verification in an HTTPS connection.
  */
  //===========================================================================
  bool download_file(const std::string& file_url,
                     const std::filesystem::path& destination,
                     const std::filesystem::path& ca_cert_file)
  {
    CURL *curl_pt{curl_easy_init()};
    if (curl_pt==nullptr)
    {
      std::cout << "Unable to construct a CURL pointer." << std::endl;
      return false;
    }

    // Configure curl.
    curl_easy_setopt(curl_pt,CURLOPT_URL,file_url.c_str());
    curl_easy_setopt(curl_pt,CURLOPT_FOLLOWLOCATION,1L);
    curl_easy_setopt(curl_pt,CURLOPT_CAINFO,ca_cert_file.c_str());
    curl_easy_setopt(curl_pt,CURLOPT_SSL_VERIFYPEER,1L);
    curl_easy_setopt(curl_pt,CURLOPT_SSL_VERIFYHOST,2L);

    // Open the file.
    std::FILE* fp{fopen(destination.c_str(),"wb")};
    if (fp==nullptr)
    {
      // Remember to clean up in all program paths.
      curl_easy_cleanup(curl_pt);
      return false;
    }

    // Configure curl.
    curl_easy_setopt(curl_pt,CURLOPT_WRITEDATA,fp);

    // Perform the request.
    CURLcode result=curl_easy_perform(curl_pt);
    curl_easy_cleanup(curl_pt);

    // Close the file.
    fclose(fp);

    // Return the outcome of the curl request
    return result==CURLE_OK;
  } // End of download_file
} // namespace NetworkMonitor