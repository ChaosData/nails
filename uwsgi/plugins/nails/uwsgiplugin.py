NAME = 'nails'

CFLAGS = [
  '-std=c11', '-Wall', '-Wextra', '-pedantic', '-g', '-O2',
  '-Iplugins/nails/includes',
  '-fPIC', '-fPIE', '-fstack-protector-all', '-D_FORTIFY_SOURCE=2',
  '-DNAILS_DEBUG'
]

LDFLAGS = ['-Lplugins/nails/output',
           '-Lplugins/nails/vendor/libsodium/src/libsodium/.libs']
LIBS = ['-lNails', '-Wl,-Bstatic -lsodium -Wl,-Bdynamic', '-lc++']
GCC_LIST = [
  'src/plugin'
]
