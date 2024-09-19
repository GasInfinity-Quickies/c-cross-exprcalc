project('quickies-exprcalc', 'c')

cc = meson.get_compiler('c')
math_dep = cc.find_library('m', required: false)

executable('calc', 'main.c', dependencies: math_dep)
