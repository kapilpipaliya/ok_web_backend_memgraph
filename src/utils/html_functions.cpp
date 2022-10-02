#include "utils/html_functions.hpp"
#include <stdlib.h>
#include <trantor/utils/Logger.h>
#include <fstream>
#include <iostream>
#include <boost/algorithm/string/replace.hpp>
#include <regex>
#include "jwt/jwt.hpp"
namespace ok::utils
{
namespace html
{
std::string getSubdomain(std::string const& host)
{
  std::string regex_ = "(\\w+).chat.";
  std::smatch match;
  if (std::regex_search(host, match, std::regex(regex_))) { return match[1]; }
  return "";
}
bool isMobile(std::string const& userAgent)
{
  //    String userAgent = request.headers.get("user-agent").value();
  //    Pattern patternOne = Pattern.compile(
  //        "(android|bb\\d+|meego).+mobile|avantgo|bada\\/|blackberry|blazer|compal|elaine|fennec|hiptop|iemobile|ip(hone|od)|iris|kindle|lge
  //        |maemo|midp|mmp|netfront|opera m(ob|in)i|palm(
  //        os)?|phone|p(ixi|re)\\/|plucker|pocket|psp|series(4|6)0|symbian|treo|up\\.(browser|link)|vodafone|wap|windows
  //        (ce|phone)|xda|xiino", Pattern.CASE_INSENSITIVE |
  //        Pattern.MULTILINE)
  //        ;
  //    Pattern patternTwo = Pattern.compile(
  //        "1207|6310|6590|3gso|4thp|50[1-6]i|770s|802s|a
  //        wa|abac|ac(er|oo|s\\-)|ai(ko|rn)|al(av|ca|co)|amoi|an(ex|ny|yw)|aptu|ar(ch|go)|as(te|us)|attw|au(di|\\-m|r
  //        |s
  //        )|avan|be(ck|ll|nq)|bi(lb|rd)|bl(ac|az)|br(e|v)w|bumb|bw\\-(n|u)|c55\\/|capi|ccwa|cdm\\-|cell|chtm|cldc|cmd\\-|co(mp|nd)|craw|da(it|ll|ng)|dbte|dc\\-s|devi|dica|dmob|do(c|p)o|ds(12|\\-d)|el(49|ai)|em(l2|ul)|er(ic|k0)|esl8|ez([4-7]0|os|wa|ze)|fetc|fly(\\-|_)|g1
  //        u|g560|gene|gf\\-5|g\\-mo|go(\\.w|od)|gr(ad|un)|haie|hcit|hd\\-(m|p|t)|hei\\-|hi(pt|ta)|hp(
  //        i|ip)|hs\\-c|ht(c(\\-|
  //        |_|a|g|p|s|t)|tp)|hu(aw|tc)|i\\-(20|go|ma)|i230|iac(
  //        |\\-|\\/)|ibro|idea|ig01|ikom|im1k|inno|ipaq|iris|ja(t|v)a|jbro|jemu|jigs|kddi|keji|kgt(
  //        |\\/)|klon|kpt |kwc\\-|kyo(c|k)|le(no|xi)|lg(
  //        g|\\/(k|l|u)|50|54|\\-[a-w])|libw|lynx|m1\\-w|m3ga|m50\\/|ma(te|ui|xo)|mc(01|21|ca)|m\\-cr|me(rc|ri)|mi(o8|oa|ts)|mmef|mo(01|02|bi|de|do|t(\\-|
  //        |o|v)|zz)|mt(50|p1|v
  //        )|mwbp|mywa|n10[0-2]|n20[2-3]|n30(0|2)|n50(0|2|5)|n7(0(0|1)|10)|ne((c|m)\\-|on|tf|wf|wg|wt)|nok(6|i)|nzph|o2im|op(ti|wv)|oran|owg1|p800|pan(a|d|t)|pdxg|pg(13|\\-([1-8]|c))|phil|pire|pl(ay|uc)|pn\\-2|po(ck|rt|se)|prox|psio|pt\\-g|qa\\-a|qc(07|12|21|32|60|\\-[2-7]|i\\-)|qtek|r380|r600|raks|rim9|ro(ve|zo)|s55\\/|sa(ge|ma|mm|ms|ny|va)|sc(01|h\\-|oo|p\\-)|sdk\\/|se(c(\\-|0|1)|47|mc|nd|ri)|sgh\\-|shar|sie(\\-|m)|sk\\-0|sl(45|id)|sm(al|ar|b3|it|t5)|so(ft|ny)|sp(01|h\\-|v\\-|v
  //        )|sy(01|mb)|t2(18|50)|t6(00|10|18)|ta(gt|lk)|tcl\\-|tdg\\-|tel(i|m)|tim\\-|t\\-mo|to(pl|sh)|ts(70|m\\-|m3|m5)|tx\\-9|up(\\.b|g1|si)|utst|v400|v750|veri|vi(rg|te)|vk(40|5[0-3]|\\-v)|vm40|voda|vulc|vx(52|53|60|61|70|80|81|83|85|98)|w3c(\\-|
  //        )|webc|whit|wi(g |nc|nw)|wmlb|wonu|x700|yas\\-|your|zeto|zte\\-",
  //        Pattern.CASE_INSENSITIVE | Pattern.MULTILINE)
  //        ;
  //    if ((patternOne.matcher(userAgent).find() ||
  //    patternTwo.matcher(userAgent.substring(0, 4)).find())) {
  //      return true;
  //    }
  return false;
}
// namespace impl
}  // namespace html
namespace jwt_functions
{
std::string encodeCookie(std::string const& normalString)
{
  auto key = "okOroSecret";  // Secret to use for the algorithm
  // Create JWT object
  jwt::jwt_object obj{jwt::params::algorithm("HS256"), jwt::params::payload({{"id", normalString}}), jwt::params::secret(key)};
  // Get the encoded string/assertion
  auto enc_str = obj.signature();
  std::cout << enc_str << std::endl;
  return enc_str;
}
std::string encodeCookie(std::map<std::string, std::string> const& normalValues)
{
  // Create JWT object
  jwt::jwt_object obj{jwt::params::algorithm("HS256"), jwt::params::payload({}), jwt::params::secret("okOroSecret")};
  for (auto const& elem : normalValues) { obj.add_claim(elem.first, elem.second); }
  // Get the encoded string/assertion
  auto enc_str = obj.signature();
  //  std::cout << enc_str << std::endl;
  return enc_str;
}
jwt::jwt_object decodeCookie(std::string const& encodedString)
{
  try
  {
    return jwt::decode(encodedString, jwt::params::algorithms({"HS256"}), jwt::params::secret("okOroSecret"));
  }
  catch (jwt::TokenExpiredError const& e)
  {
    LOG_DEBUG << "Handle Token expired exception here " << encodedString;
    //...
    return {};
  }
  catch (jwt::SignatureFormatError const& e)
  {
    LOG_DEBUG << "Handle invalid signature format error " << encodedString;
    //...
    return {};
  }
  catch (jwt::DecodeError const& e)
  {
    LOG_DEBUG << "Handle all kinds of other decode errors " << encodedString;
    //...
    return {};
  }
  catch (jwt::VerificationError const& e)
  {
    LOG_DEBUG << "Handle the base verification error. " << encodedString;
    // NOTE: There are other derived types of verification errors
    // which will be discussed in next topic.
    return {};
  }
  catch (...)
  {
    std::cerr << "Caught unknown exception" << std::endl;
    return {};
  }
  //  std::cout << dec_obj.header() << std::endl;
  //  std::cout << dec_obj.payload() << std::endl;
  //  if (dec_obj.payload().has_claim("id")) {
  //    return dec_obj.payload().get_claim_value<std::string>("id");
  //  }
  //  return "";
}
// namespace jwt_functions
}  // namespace jwt_functions
}  // namespace ok::utils
