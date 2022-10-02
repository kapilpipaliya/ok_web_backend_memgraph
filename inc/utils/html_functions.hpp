#pragma once
#include <map>
#include <string>

namespace jwt
{
class jwt_object;
}
namespace ok::utils
{
namespace html
{
std::string getSubdomain(std::string const &host);
bool isMobile(std::string const &userAgent);
}  // namespace html
namespace jwt_functions
{
std::string encodeCookie(std::string const &normalString);
std::string encodeCookie(std::map<std::string, std::string> const &normalValues);
jwt::jwt_object decodeCookie(std::string const &encodedString);
}  // namespace jwt_functions
}  // namespace ok::utils
