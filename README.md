# MicroIni 1.0.0

### What is MicroIni?
MicroIni is an INI file parser written in ANSI C. This is a derivative work of [iniparser](https://github.com/ndevilla/iniparser) which was originally forked on April 26, 2016.  MicroIni's purpose was to create as a redesign of [iniparser](https://github.com/ndevilla/iniparser) to be an extremely lightweight INI file parser, suitable for running on embedded platforms with very limited RAM availability.  It does this by eliminating all dynamic allocations and reports data read from INI files back to user code via callbacks rather than an explicit query interface. MicroIni also allows flexibility in where the INI file data is read from.  Several functions are provided to allow loading from a file path, a `FILE*` object, or a custom stream, allowing INI files to be parsed from buffers in memory, over network streams, or anywhere else as long as the appropriate functions are provided to the parser. However, these redesigns do raise new limitations.  Since no dynamic memory is allocated, each line of an INI file may only be up to 512 characters long. Lines longer than this will be truncated.

### What software license does MicroIni use?
MicroIni is provided under the MIT license. This is the same license used by [iniparser](https://github.com/ndevilla/iniparser) at the time it was forked. For additional details, please see the file `LICENSE`.  The original license is provided in the file `THIRD_PARTY`.

### How do I build MicroIni?
No build scripts are currently provided, but MicroIni is small enough at a single C source file that it can easily be built through any build system as either a static or dynamic library. A user may also embed the source directly into their own project (recommended).

However, should a user with to build MicroIni separately as its own dynamic library on Windows, please remember to define `MICRO_INI_API_EXPORT` and `MICRO_INI_API_IMPORT` in your build scripts when compiling the library and importing it into a project, respectively. This does not need to be done when building as a static library or embedding the source directly into a project.
