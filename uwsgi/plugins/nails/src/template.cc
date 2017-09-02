#include "template.h"

//Note: mustache is arguably the best template language choice in C++.
//      When implemented properly, it output encodes data by default and should
//      be fairly lightwight. Unfortunately, the implementations all suffer for
//      various reasons. I don't think it's fair to add a dependency on boost
//      for libraries that recompile the template every time its rendered. I
//      also won't add in ghetto implementations that rely on preprocessor
//      voodoo or don't actually perform proper output encoding. Until this has
//      changed, I'm going to use client-side frameworks. If you wish to
//      venture into the wilderness to add support for templates, you should do
//      it here.

namespace nails {

Response render_template(std::string const&, ParamMap const&,
                         uint16_t) {
  throw std::runtime_error("Not implemented.");
}

Response render_template(std::string const&, json const&,
                         uint16_t) {
  throw std::runtime_error("Not implemented.");
}

};