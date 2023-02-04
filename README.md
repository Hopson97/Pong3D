# Pong3D

Pong except it is 3D.

## Building and Running

### Libraries

SFML and GLM are required.

These can be installed from your project manager. For example, on Debian/ Ubuntu:

```sh
sudo apt install libsfml-dev libglm-dev
```

If this is not possible (eg windows), you can install these manually from their respective websites:

https://www.sfml-dev.org/download.php

https://github.com/g-truc/glm/tags

### Linux

Requires conan

```sh
python3 -m pip install conan
```

To build, at the root of the project:

```sh
./scripts/build.sh install
```

If the build fails, try installing:

```
sudo apt install libx11-dev
sudo apt install libgl1-mesa-dev
sudo apt install libegl-dev
```

The install argument is only needed for the first time compilation as this is what grabs the libraries from Conan

To run, at the root of the project:

```sh
./scripts/run.sh
```

To build and run in release mode, simply add the `release` suffix:

```sh
./scripts/build.sh release
./scripts/run.sh release
```
