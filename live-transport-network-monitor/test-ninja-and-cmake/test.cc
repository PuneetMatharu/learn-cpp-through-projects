#include <iostream>
#include <utility>

int main()
{
  std::pair<std::string,double> pair({"Hello, World", 42});

  // Structured bindings are a C++17 feature.
  const auto& [str, num] = pair;
  std::cout << str << ": " << num << std::endl;
}