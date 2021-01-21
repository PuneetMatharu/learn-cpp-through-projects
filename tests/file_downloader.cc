// Boost-specific libraries
#include <boost/asio.hpp>
#include <boost/test/unit_test.hpp>

// JSON parsing library
#include <nlohmann/json.hpp>

// Regular libraries
#include <filesystem>
#include <fstream>
#include <string>

// Headers we've defined
#include <network-monitor/file_downloader.h>

BOOST_AUTO_TEST_SUITE(network_monitor);

//=========================================================================

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
} // BOOST_AUTO_TEST_CASE(file_downloader)

//=========================================================================

// Our implementation and tests have some limitations:
//  * Missing test #1: File not found.       [DONE]
//  * Missing test #2: Empty file.           [DONE]
//  * Missing test #3: Invalid JSON format.  [DONE]

BOOST_AUTO_TEST_CASE(check_json_file_exists)
{
  // Make sure we were able to find the network layout .json file
  BOOST_CHECK(std::filesystem::exists(TESTS_NETWORK_LAYOUT_JSON));
} // BOOST_AUTO_TEST_CASE(network_layout_json)

//-------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(test_empty_json_file)
{
  // Is there anything in the file?
  BOOST_CHECK(!std::filesystem::is_empty(TESTS_NETWORK_LAYOUT_JSON));
} // BOOST_AUTO_TEST_CASE(network_layout_json)

//-------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(test_invalid_json_file)
{
  // Create a JSON object to store the contents of the .json file
  nlohmann::json json_obj
  {
    NetworkMonitor::parse_json_file(TESTS_NETWORK_LAYOUT_JSON)
  };

  // If the JSON object is empty, there was parse error
  BOOST_CHECK(json_obj.size()>0);
} // BOOST_AUTO_TEST_CASE(network_layout_json)

//-------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(parse_json_file)
{
  // Create a JSON object to store the contents of the .json file
  nlohmann::json json_obj
  {
    NetworkMonitor::parse_json_file(TESTS_NETWORK_LAYOUT_JSON)
  };

  // Check for certain keys and make sure the arrays have non-zero size
  BOOST_CHECK(json_obj.contains("lines"));
  BOOST_CHECK(json_obj.contains("stations"));
  BOOST_CHECK(json_obj.contains("travel_times"));
  BOOST_CHECK(json_obj["lines"].size()>0);
  BOOST_CHECK(json_obj["stations"].size()>0);
  BOOST_CHECK(json_obj["travel_times"].size()>0);
} // BOOST_AUTO_TEST_CASE(json_parser)

//=========================================================================

BOOST_AUTO_TEST_SUITE_END();



