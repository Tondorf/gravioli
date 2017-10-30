# Compilation and installation of the `flatbuffers` objects
Note: Do this before you compile the server!
```> cd fbs/
> make && make install
```

# Installation of python libraries for python client

We provide all requirements for the python client within the `requirements.txt` file.
This file was obtained via `pip freeze`, thus one can use `pip install -r requirements.txt` in order to install the requirements.
We highly encourage you to use [virtualenv](https://virtualenv.pypa.io).

# Installation of the server
```> cd server/
> mkdir build && cd build/
> cmake -DFLATBUFFERS_HEADERS=<path to flatbuffers headers> ..
> make && ./runAllUnitTests
```
where you have to replace `<path to flatbuffers/include/flatbuffers>` with the proper path to the `flatbuffers` header files, e.g.:
```> cd ~/flatbuffers/include/flatbuffers/ && ls -1
base.h
code_generators.h
flatbuffers.h
flatc.h
flexbuffers.
...
```
