#pragma once
#include <regex>
namespace ok::utils::string
{
// bool matched = regex_search(s, std::regex("[A-Za-z]")) && regex_search(s, std::regex("[0-9]"));
inline bool isAlphaNumeric(std::string const &str)
{
  // <VectorX>:
  // https://regex101.com/r/6ncMq7/1
  std::regex r(R"(^(?=.*?[a-zA-Z])(?=.*?\d))");
  std::smatch sm;
  if (std::regex_search(str, sm, r)) { return true; }
  return false;
}
inline bool isAlphaNumeric2(std::string const &str) { return regex_search(str, std::regex("[A-Za-z]")) && regex_search(str, std::regex("[0-9]")); }
inline bool ValidateEmail(std::string email)
{
  // https://emailregex.com/
  std::string officialRegex =
      R"((?:[a-z0-9!#$%&'*+/=?^_`{|}~-]+(?:\.[a-z0-9!#$%&'*+/=?^_`{|}~-]+)*|"(?:[\x01-\x08\x0b\x0c\x0e-\x1f\x21\x23-\x5b\x5d-\x7f]|\\[\x01-\x09\x0b\x0c\x0e-\x7f])*")@(?:(?:[a-z0-9](?:[a-z0-9-]*[a-z0-9])?\.)+[a-z0-9](?:[a-z0-9-]*[a-z0-9])?|\[(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?|[a-z0-9-]*[a-z0-9]:(?:[\x01-\x08\x0b\x0c\x0e-\x1f\x21-\x5a\x53-\x7f]|\\[\x01-\x09\x0b\x0c\x0e-\x7f])+)\]))";
  // simple: "([a-z]+)([_.a-z0-9]*)([a-z0-9]+)(@)([a-z]+)([.a-z]+)([a-z]+)"
  if (regex_match(email, std::regex(officialRegex))) return true;
  return false;
}
inline bool validateSubdomain(std::string const &url)
{
  // https://stackoverflow.com/a/7930801/4372670
  auto regex_ = R"(^\w+$$)";
  if (regex_match(url, std::regex(regex_))) return true;
  return false;
}
/*
void readFastAFile(std::vector<std::string> &reads, std::vector<std::string>
&headers)
{
    std::string header;
    std::string read;

while(std::getline(ifs, header) && std::getline(ifs, read))
{
  headers.push_back(header);
  reads.push_back(read);
}
}
*/
}  // namespace ok::utils::string
