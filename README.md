# A simple recursive descent parser calculator in C
Simple calculator that also comes as a header-only library with one function:
```c
gec_eval_result eval(char* str);
```

Why? To review my knowledge, knowledge and knowledge!

## Build instructions

Install `meson ninja` and `gcc` or `clang`. Should work with a windows setup but untested :p

That's it, then run `meson setup <dir> && cd <dir> && ninja` to build the sample `calc` binary
