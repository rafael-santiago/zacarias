# Build

``Zacarias`` build is based on another tool of mine called [Hefesto](https://github.com/rafael-santiago/hefesto).
``Hefesto`` is a well-simple and contained build system if you have a C compiler on your system you are able to
build and install ``Hefesto``. There is no reason to be scared! ;)

## Contents

- [Setup your environment](#setup-your-environment)
- [Cloning Zacarias repository](#cloning-zacarias-repository)
- [Building](#building)

## Setup your environment

After putting ``Hefesto`` to work on you environment you need to clone another repository called ``Helios`` and
then do the following:

```
you@AdventureGalley:~/# git clone https://github.com/rafael-santiago/helios
you@AdventureGalley:~/# cd helios
you@AdventureGalley:~/helios# hefesto --install=lnx-module-toolset,forge-bootstrapper
you@AdventureGalley:~/helios# cd ..
you@AdventureGalley:~/# rm -rf helios
```

Nice. Now your ``Hefesto`` copy knows how to build ``Zacarias`` device, user tool, tests and also how to install
all of it into your system.

[``Back``](#contents)

## Cloning Zacarias repository

The easiest way to clone ``Zacarias`` is as follows:

```
you@AdventureGalley:~/# git clone https://github.com/rafael-santiago/zacarias --recursive
```

[``Back``](#contents)

## Building

Since you will build a device driver you need to have the source of your operating system well-installed and
accessible. Besides it you need a C compiler (``GCC`` or ``Clang``) and ``libc``, ``X11`` (including ``Xtst``)
and ``libpthread`` required to build the tests.

Filling up those requirements all you should do is to change to ``src`` sub-directory of your repo's copy and
invoke ``Hefesto`` from there:

```
you@AdventureGallye:~/# cd zacarias/src
you@AdventureGallye:~/zacarias/src# hefesto
```

It will build the device driver, the user-mode tool, run some testing and after a successful build you should
run ``hefesto --install``. Once installed you will be able to start using ``Zacarias`` as your password manager.
If you are new here it is time to start reading the [user's manual](https://github.com/rafael-santiago/zacarias/blob/master/doc/MANUAL.md).

[``Back``](#contents)
