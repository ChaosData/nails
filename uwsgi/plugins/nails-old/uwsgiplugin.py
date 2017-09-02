NAME = 'nails'

CFLAGS = ['-std=c11', '-Wall', '-Wextra', '-pedantic', '-g']
LDFLAGS = ['-L/home/jtd/code/c++/railsreflect/basicstring/uwsgi/plugins/nails']
LIBS = ['-lcontroller']
GCC_LIST = ['base.c', 'plugin']
