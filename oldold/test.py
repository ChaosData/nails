#!/usr/bin/python
# vim: set fileencoding=utf-8
import clang.cindex
import asciitree # must be version 0.2
import sys
import code

def node_children(node):
    return (c for c in node.get_children() if c.location.file != None and c.location.file.name == sys.argv[1])

def print_node(node):
    text = node.spelling or node.displayname
    kind = str(node.kind)[str(node.kind).index('.')+1:]
    return '{} {}'.format(kind, text)

if len(sys.argv) != 2:
    print("Usage: dump_ast.py [header file name]")
    sys.exit()

clang.cindex.Config.set_library_path('/usr/lib/llvm-3.8/lib/')
index = clang.cindex.Index.create()
translation_unit = index.parse(sys.argv[1], ['-x', 'c++', '-std=c++11', '-stdlib=libc++'])

#code.interact(local=locals())

print(asciitree.draw_tree(translation_unit.cursor, node_children, print_node))
