from distutils.core import setup, Extension

tracer = Extension('stacksieve_c',
                    sources=['ext/trace.c'])

setup(name = 'StackSieve',
      version = '0.1',
      description = 'Trace Python programs to assist reverse engineering',
      packages=['stacksieve'],
      ext_modules=[tracer])





