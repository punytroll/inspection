# Building

Building might be as easy as:

```bash
meson setup build
meson compile -C build
```

You need only a few helping programs for the building process:

- meson (>= 0.63.0)
- ninja (>= 1.10.1)
- python (3.*)
- g++ (with C++ 20)

# Testing

```bash
meson test -C build
meson compile check -C build
```

I'm trying to integrate my own testing framework into meson but so far I'm unsuccessful.
