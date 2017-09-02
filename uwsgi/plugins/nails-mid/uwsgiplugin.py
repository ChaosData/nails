NAME = 'nails'

CFLAGS = ['-std=c11', '-Wall', '-Wextra', '-pedantic', '-g', '-Iplugins/nails/includes', '-DNAILS_DEBUG']
LDFLAGS = ['-Lplugins/nails/lib']
LIBS = ['-Wl,-Bstatic', '-lnails', '-Wl,-Bdynamic', '-lc++']
GCC_LIST = ['plugins/nails/lib/libnails.a', 'uwsgi/plugin']
