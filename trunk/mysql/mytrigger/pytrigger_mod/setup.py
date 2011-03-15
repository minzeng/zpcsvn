
from distutils.core import setup
from distutils.extension import Extension
from Cython.Distutils import build_ext

ext_modules = [
    Extension( name = "pytrigger",
               sources = ["pytrigger.pyx"],
               include_dirs = ['.'],
               libraries = ['c'],
               library_dirs = ['./']
    ),
]

setup(
    name = 'pytrigger',
    cmdclass = { 'build_ext': build_ext },
    ext_modules = ext_modules,
)
