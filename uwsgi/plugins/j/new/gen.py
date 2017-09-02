#!/usr/bin/python
# vim: set fileencoding=utf-8
import clang.cindex
import asciitree # must be version 0.2
import sys
import code

if len(sys.argv) < 4:
  print "usage: %s <output.h file> <output.cc file> [<controller.cc file>...]"
  sys.exit()

current_file = ''

def node_children(node):
  return (c for c in node.get_children() if c.location.file != None and c.location.file.name == current_file)

def print_node(node):
  text = node.spelling or node.displayname
  kind = str(node.kind)[str(node.kind).index('.')+1:]
  return '{} {}'.format(kind, text)

mapping = []
def check_class(node):
  name = node.displayname
  spelling = node.spelling
  if 'controller' not in name.lower():
    return
  isController = False
  for n in node_children(node):
    if n.kind == clang.cindex.CursorKind.CXX_BASE_SPECIFIER:
      nname = n.displayname
      if 'controller' in nname.lower():
        isController = True
        break
  if not isController:
    return
  viable_methods = []
  for n in node_children(node):
    if n.kind == clang.cindex.CursorKind.CXX_METHOD:
      params_types = []
      for nn in node_children(n):
        if nn.kind == clang.cindex.CursorKind.PARM_DECL:
          for nnn in node_children(nn):
            if nnn.kind == clang.cindex.CursorKind.TYPE_REF:
              params_types.append(nnn.displayname)
              break
      if len(params_types) == 2 and\
          params_types[0] == "class Request" and\
          params_types[1] == "class Response" and\
          n.access_specifier == clang.cindex.AccessSpecifier.PUBLIC:
        viable_methods.append(n)
  addrs = []
  for n in viable_methods:
    addrs.append(node.spelling + "::" + n.spelling)
  mapping.append([spelling, addrs])


def inspect(node):
  if node.kind == clang.cindex.CursorKind.CLASS_DECL:
    check_class(node)
  else:
    for n in node_children(node):
      inspect(n)  


clang.cindex.Config.set_library_path('/usr/lib/llvm-3.8/lib/')
for arg in sys.argv[3:]:
  current_file = arg.replace(".cc", ".h")
  #index = clang.cindex.Index.create()
  index = clang.cindex.Index(clang.cindex.conf.lib.clang_createIndex(False, True))

  tu = index.parse(arg, ['-x', 'c++', '-std=c++14', '-stdlib=libc++'])
  inspect(tu.cursor)


gen_h = '''\
#ifndef _GEN_H_
#define _GEN_H_
#include "controller-helper.h"

{include}

extern "C" {
  void init_controllers();
  void teardown_controllers();
}


{singleton}

#endif
'''

for map in mapping:
  gen_h = gen_h.replace("{include}\n", '#include "%s.h"\n{include}\n' % map[0])
  gen_h = gen_h.replace("{singleton}\n", "extern %s singleton_%s;\n{singleton}\n" % (map[0], map[0]))
gen_h = gen_h.replace("{include}\n", "")
gen_h = gen_h.replace("{singleton}\n", "")


gen_cc = '''\
#include "{gen_h}"
#include "controller-helper.h"

ControllerMapper controller_mapper;
ControllerFactoryMapper controller_factory_mapper;

{singleton}

ControllerNameMapper controller_name_mapper;

extern "C" void init_controllers() {

  {controllerinit}
}

extern "C" void teardown_controllers() {

  {controllerteardown}
}
'''.replace("{gen_h}", sys.argv[1])


for map in mapping:
  gen_cc = gen_cc.replace("{singleton}\n", "%s singleton_%s;\n{singleton}\n" % (map[0], map[0]))
  cinit =  '  controller_mapper.emplace(type_name(singleton_%s), ControllerActionMapper());\n' % map[0]
  cinit += '  controller_factory_mapper.emplace(type_name(singleton_%s), new ControllerFactory<%s>());\n' %  (map[0], map[0])
  cinit += '  controller_name_mapper.emplace("%s", type_name(singleton_%s));\n' % (map[0][:map[0].find('Controller')], map[0])
  cinit += '  auto& tcam_%s = controller_mapper.at(type_name(singleton_%s));\n' % (map[0], map[0])
  for method in map[1]:
    cinit += '  tcam_%s.add("%s", make_func(&%s));\n' % (map[0], method.split('::')[1], method)
  gen_cc = gen_cc.replace("  {controllerinit}\n", '%s\n  {controllerinit}\n' % cinit)

  cteardown =  '  auto* cfm_%s = controller_factory_mapper.at(type_name(singleton_%s));\n' % (map[0], map[0])
  cteardown += '  controller_factory_mapper.erase(type_name(singleton_%s));\n' % map[0]
  cteardown += '  delete cfm_%s;\n' % map[0]
  gen_cc = gen_cc.replace("  {controllerteardown}\n", '%s\n  {controllerteardown}\n' % cteardown)
  gen_cc = gen_cc.replace("{include}\n", "")
gen_cc = gen_cc.replace("{singleton}\n", "")
gen_cc = gen_cc.replace("  {controllerinit}\n", "")
gen_cc = gen_cc.replace("  {controllerteardown}\n", "")

  
with open(sys.argv[1], 'w') as fd:
  fd.write(gen_h)

with open(sys.argv[2], 'w') as fd:
  fd.write(gen_cc)
