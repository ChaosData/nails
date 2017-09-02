#ifndef _FILTERS_H_
#define _FILTERS_H_

#include <vector>
#include <utility>
#include "controller-helper.h"

namespace nails {

extern std::vector<std::pair<func<bool(Request&)>*,func<Response(Request&)>*>>
prematch_filters; //where one would implement an _method override

extern std::vector<std::pair<func<bool(Request&)>*,func<Response(Request&)>*>>
postmatch_filters; //where one would implement CSRF filtering (can whitelist)

extern std::vector<func<bool(Request&,Response&)>*> response_filters;

};

using nails::Request;
using nails::Response;

#endif