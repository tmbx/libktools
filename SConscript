Import('env build_dir')

import os

lib_NAME = "libktools"

main_env = env

# build the lib
env = main_env.Clone()
env.Append(CPPPATH = os.path.join(os.getcwd(), 'src'),
           VERSION = '0.0',
           CPPDEFINES = ['_LARGEFILE64_SOURCE', '_FILE_OFFSET_BITS=64', 'BUILDING_KTOOLS'])
shared_LIBS, static_LIBS, OBJS, install = SConscript(os.path.join('src', 'SConscript'), build_dir=build_dir, duplicate=0, exports='env lib_NAME')

# build the test
PROGS = []
if env['test']:
    env = main_env.Clone()
    env.Append(CPPPATH=[os.path.join(os.getcwd(), 'test'), os.path.join(os.getcwd(), 'src')])
    NOBJS, PROG = SConscript(os.path.join('test', 'SConscript'), build_dir=os.path.join(build_dir, 'test'), duplicate=0, exports='env shared_LIBS static_LIBS lib_NAME')
    OBJS += NOBJS
    PROGS += PROG

builds = OBJS + PROGS + static_LIBS + shared_LIBS

Return('builds install')
