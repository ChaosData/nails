#!/usr/bin/python
# vim: set fileencoding=utf-8
import clang.cindex
import asciitree # must be version 0.2
import sys
import code

if len(sys.argv) < 3:
  print "usage: %s <output.c file> [<controller.cc file>...]"
  sys.exit()

current_file = ''

def node_children(node):
  print current_file
  return (c for c in node.get_children() if c.location.file != None and c.location.file.name == current_file)

def print_node(node):
  text = node.spelling or node.displayname
  kind = str(node.kind)[str(node.kind).index('.')+1:]
  return '{} {}'.format(kind, text)

mapping = []
def check_class(node):
  # clang.cindex.CursorKind.CLASS_DECL
  name = node.displayname
  spelling = node.spelling
  if 'controller' not in name.lower():
    return
  print name
  isController = False
  for n in node_children(node):
    if n.kind == clang.cindex.CursorKind.CXX_BASE_SPECIFIER:
      nname = n.displayname
      if 'controller' in nname.lower():
        print "found: " + nname
        print n.access_specifier
        print dir(n)
        isController = True
        break
  if not isController:
    return
  viable_methods = []
  for n in node_children(node):
    if n.kind == clang.cindex.CursorKind.CXX_METHOD:
      #print n.spelling +  " / " + n.displayname
      params_types = []
      for nn in node_children(n):
        if nn.kind == clang.cindex.CursorKind.PARM_DECL:
          #print nn.spelling + " / " + nn.displayname
          #print dir(nn)
          for nnn in node_children(nn):
            if nnn.kind == clang.cindex.CursorKind.TYPE_REF:
              #print nnn.spelling + " / " + nnn.displayname
              #print dir(nnn)
              params_types.append(nnn.displayname)
              break
      #print repr(params_types)
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
  print 'called inspect with ' + repr(node)
  if node.kind == clang.cindex.CursorKind.CLASS_DECL:
    check_class(node)
  else:
    for n in node_children(node):
      inspect(n)  


clang.cindex.Config.set_library_path('/usr/lib/llvm-3.8/lib/')
for arg in sys.argv[2:]:
  print arg
  current_file = arg.replace(".cc", ".h")
  #index = clang.cindex.Index.create()
  index = clang.cindex.Index(clang.cindex.conf.lib.clang_createIndex(False, True))

  tu = index.parse(arg, ['-x', 'c++', '-std=c++14', '-stdlib=libc++'])
  inspect(tu.cursor)

print mapping

outstr = '''\
#include "gen.h"
#include "controller-helper.h"

#include "controller.h"
'''

for map in mapping:
  pass
  #outstr += '#include "%s.h"\n' % map[0]
outstr += "\n"

outstr += "extern ControllerMapper controller_mapper;\n"

outstr += "void init_controllers() {\n"

for map in mapping:
  outstr += '  controller_mapper.emplace("%s", ControllerActionMapper());\n' % map[0]
  outstr += '  auto& tcam_%s = controller_mapper.at("%s");\n' % (map[0], map[0])
  for method in map[1]:
    outstr += '  tcam_%s.add("%s", make_func(&%s));\n' % (map[0], method.split('::')[1], method)
  outstr += "\n"

outstr += "}\n"


with open(sys.argv[1], 'w') as fd:
  fd.write(outstr)
