from pathlib import Path

from Cython.Build import cythonize
from setuptools import setup

# import os
# fileset = [os.path.join(dp, f) for dp, dn, filenames in os.walk("./groot") for f in filenames if os.path.splitext(f)[-1] == '.py']

fileset = [str(p) for p in Path("api_wrapper").rglob("*.py")]

setup(
    name='groot',
    ext_modules=cythonize(fileset, language='c', language_level='3')
)
