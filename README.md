# Compilation and installation of the `flatbuffers` objects
Note: Do this before you compile the server!
```
> cd fbs/
> make && make install
```

# Installation of python libraries for python client

We provide all requirements for the python client within the `requirements.txt` file.
This file was obtained via `pip freeze`, thus one can use `pip install -r requirements.txt` in order to install the requirements.
We highly encourage you to use [virtualenv](https://virtualenv.pypa.io).

# Start of the server
We provide a Dockerfile.
```
> docker build -t gravioli-server .
> docker run --rm -p 8888:8888 gravioli-server
```
