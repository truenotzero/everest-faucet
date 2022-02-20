# Faucet
A tiny C library to help with finding memory leaks 

## Issues
Faucet redefines the `malloc` family of functions as macros. As such, `#include`ing `stdlib.h` will break your code.

Instead, declare the function you need with the `extern` specifier.
