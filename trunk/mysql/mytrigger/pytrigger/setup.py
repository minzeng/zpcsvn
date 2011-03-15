
from distutils.core import setup
from distutils.extension import Extension
from Cython.Distutils import build_ext

ext_modules = [
    Extension( name = "py_mtmod",
               sources = ["py_mtmod.pyx"],
               include_dirs = ['.'],
               libraries = ['c'],
               library_dirs = ['./']
    ),
]

setup(
    name = 'py_mtmod',
    cmdclass = { 'build_ext': build_ext },
    ext_modules = ext_modules,
)
