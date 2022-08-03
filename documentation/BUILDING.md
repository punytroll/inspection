# Building

Building might be as easy as:

```bash
make
```

But since that build system is handcrafted, I don't expect it to be very portable or even robust.
You need only a few helping programs for the building process:

- make
- python (3.*)
- g++ (with C++ 20)
- meson (>= 0.63.0)

The actual commands names for python or g++ can be adjusted in Makefile.include.

## Meson

Additionally I am moving toward a Meson build system.
This should eventually take over all of the building and most of the testing.
Meson is currently being invoked automatically by the make build system.
Only some external libraries are pulled in and compiled as meson subprojects.
Linking these libraries into the inspector and test executables is done by hand using a fixed meson build path.
