// Boost-specific libraries
#include <boost/asio.hpp>
#include <boost/test/unit_test.hpp>

// Regular libraries
#include <filesystem>
#include <fstream>
#include <string>

// Headers we've defined
#include <network-monitor/file_downloader.h>

BOOST_AUTO_TEST_SUITE(network_monitor);

// Our implementation and tests have some limitations:
//  * Missing test #1: Failed download.
//  * Missing test #2: File destination does not exist or cannot be written to.
//  * Missing test #3: Successful download, empty file.
//  * Missing test #4: Plain HTTP file download.

BOOST_AUTO_TEST_CASE(file_downloader)
{
  const std::string file_url
  {
    "https://ltnm.learncppthroughprojects.com/network-layout.json"
  };
  const auto destination
  {
    std::filesystem::temp_directory_path() / "network-layout.json"
  };

  // Download the file.
  bool downloaded
  {
    NetworkMonitor::download_file(file_url,destination,TESTS_CACERT_PEM)
  };
  BOOST_CHECK(downloaded);
  BOOST_CHECK(std::filesystem::exists(destination));

  // Check the content of the file.
  // We cannot check the whole file content as it changes over time, but we
  // can at least check some expected file properties.
  const std::string expected_string {"\"stations\": ["};
  std::ifstream file {destination};
  std::string line{};
  bool found_expected_string {false};
  while (std::getline(file,line))
  {
    if (line.find(expected_string)!=std::string::npos)
    {
      found_expected_string=true;
      break;
    }
  }
  BOOST_CHECK(found_expected_string);

  // Clean up.
  std::filesystem::remove(destination);
}

BOOST_AUTO_TEST_SUITE_END();