# Mapnik Input Plugin for Vector Tiles

This repository holds the code of an [Mapnik](https://github.com/mapnik/mapnik) input plugin for vector tiles.

**This project is experimental.**

## Dependencies

* Mapnik (>= 4.0)
* SQlite
* Boost (String)

## Building

This project uses CMake for building:

```sh
mkdir build
cd build
cmake ..
make
```

If dependencies are missing or could not be found, run `ccmake ..` to set their pathes manually.

You can build a Debian package as well:

```sh
dpkg-buildpackage -us -uc -b
```

## License

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

Parts of the code come from the Mapnik project (LGPL) and Mapbox (BSD 3-clause).
The copyright notices can be found in the source and header files.
