#include "filters.h"

namespace nails {

std::vector<std::pair<func<bool(Request&)>*,func<Response(Request&)>*>>
prematch_filters; //where one would implement an _method override

std::vector<std::pair<func<bool(Request&)>*,func<Response(Request&)>*>>
postmatch_filters; //where one would implement CSRF filtering (can whitelist)

std::vector<func<bool(Request&,Response&)>*> response_filters;

};