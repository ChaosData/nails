#ifndef _NAILS_TEMPLATE_H_
#define _NAILS_TEMPLATE_H_

#include "http.h"
#include "reqres.h"

namespace nails {

//note: we don't actually want to recreate entire template objects and should
//      instead keep them in an unordered_map for reuse

Response render_template(std::string const& path, ParamMap const& params,
                         uint16_t status_code);

Response render_template(std::string const& path, json const& params,
                         uint16_t status_code);

};
#endif
