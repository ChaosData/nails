import asciitree, sys

sys.path.append('/home/jtd/code/c++/templates/test/clang-llvm/llvm/tools/clang/bindings/python/')
import clang.cindex

clang.cindex.Config.set_library_path("/usr/lib/llvm-3.8/lib/")
index = clang.cindex.Index(clang.cindex.conf.lib.clang_createIndex(False, True))
translation_unit = index.parse(sys.argv[1], ['-x', 'c++', '-std=c++14', '-stdlib=libc++'])

print asciitree.draw_tree(translation_unit.cursor,
  lambda n: n.get_children(),
  lambda n: "%s (%s)" % (n.spelling or n.displayname, str(n.kind).split(".")[1]))
