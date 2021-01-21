# Live transport network monitor project

Personal repository for working through the live transport network monitor project available at https://www.learncppthroughprojects.com.

### Project description

The city transport company wants to distribute the number of passengers across their lines to balance the network load. They want to create a program that receives live information about passengers' trips and produces travel suggestions that will help keep the network balanced.

Learn to write asynchronous code in C++ that can handle tens of thousands of events from different sources and dispatch operations in reaction to them.

#### Tools used

* **Build system:** Ninja
* **Build generator:** CMake
* **Dependency manager:** Conan (requires Python 3)
* **Version control:** Git

#### Third-party libraries used

* [Boost](https://www.boost.org/) (Boost.Asio & Boost.Beast): Network programming libraries.
* [libcurl](https://curl.se/libcurl/): File transfer library.
* [OpenSSL](https://www.openssl.org/): TLS and SSL protocols.
* [nlohmann](https://github.com/nlohmann/json): JSON for modern C++.
