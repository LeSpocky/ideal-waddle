# ideal-waddle

[![standard-readme compliant](https://img.shields.io/badge/readme%20style-standard-brightgreen.svg?style=flat-square)](https://github.com/RichardLitt/standard-readme)

This tiny project implements a TCP echo server on standard port 7 in C
on GNU/Linux, nothing special about that in general. The purpose is to
show why TCP KEEPALIVE packets may be useful from the server side.

## Background

[RFC1122 Section 4.2.3.6](https://tools.ietf.org/html/rfc1122#section-4.2.3.6) 
describes “TCP Keep-Alives” and says:

> A TCP keep-alive mechanism should only be invoked in server
> applications that might otherwise hang indefinitely and consume
> resources unnecessarily if a client crashes or aborts a connection
> during a network failure.

This echo server can demonstrate exactly that _and_ also how to mitigate
it with TCP keep-alive packets.

Note: you may not need this mechanism if your layer 5 to 7 protocol
lets the server part send data every now and then, has its own keep
alive packets or any other mechanism to detect those inactive
connections.

## Install

This project can be built with [CMake](https://cmake.org/) 3.1 and later
and a C compiler which is C99 compliant. It uses pthreads and some GNU
and/or glibc extensions. Nothing more is required. Don't worry, any non
ancient GNU/Linux with GCC should work without problems.

## Usage

From your build directory use one of those:

```
./ideal-waddle -h
sudo ./ideal-waddle
sudo ./ideal-waddle -k
```

The program uses one (named) thread per client connection. Those
“indefinitely consumed resources“ are threads in this program.

## Contribute

PRs accepted.

Small note: If editing the Readme, please conform to the
[standard-readme](https://github.com/RichardLitt/standard-readme)
specification.

## License

* Copyright 2018 Alexander Dahl
* [GPL-3.0-or-later](LICENSE)
