#pragma once
#include "src/protection/identity.h"
namespace maenbrowser::protection {
// Public compatibility scripts contain no user data. Private native responses
// always retain the exact extension origin, even if both flags are supplied.
inline const char* ResponseCorsOrigin(bool private_api,bool public_script) {
  return private_api ? kExtensionOrigin : public_script ? "*" : nullptr;
}
}
