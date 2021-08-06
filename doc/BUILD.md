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
you@AdventureGalley:~/# cd zacarias/src
you@AdventureGalley:~/zacarias/src# hefesto
```

Due to library hooking mitigations, by default, ``Zacarias`` command line tool (``zc``) is static linked. In this way,
at the first time you run ``Zacarias`` build it is necessary to build all ``Xorg`` dependencies used by ``zc``. Thus,
if you want to follow using the default static linking you will need to download the following ``Xorg`` tarballs:
``libX11``, ``libXau``, ``libXdmcp``, ``libXext``, ``libXtst`` and ``libxcb``. You need to watch the correct versions
used on your system. Once downloaded all you should do is indicating to ``hefesto`` where those tarballs are located
and they will built and copied into your ``Zacarias`` repo ``lib`` sub-directory, your system will not be changed.
Follows the command line sample based on my own environment:

```
you@AdventureGalley:~/zacarias/src# hefesto --libX11-pkg=/tmp/libX11-1.6.3.tar.gz \
> --libXtst-pkg=/tmp/libXtst-1.2.2.tar.gz --libxcb-pkg=/tmp/libxcb-1.11.tar.bz2 \
> --libXext-pkg=/tmp/libXext-1.3.3.tar.gz --libXdcmp-pkg=/tmp/libXdcmp-1.1.1.tar.bz2 \
> --libXau-pkg=/tmp/libXau-1.0.8.tar.bz2
```

This command will build the ``Xorg`` dependencies and after build ``Zacarias`` by linking its command line tool statically.

You can find the related ``Xorg`` tarballs at: <https://www.x.org/releases/individual/lib> and
<https://xcb.freedesktop.org/dist>.

Anyway, if you do not mind about library hooking, you can use shared linking by passing ``--disable-static`` to
``Hefesto`` as follows:

```
you@AdvendureGalley:~/zacarias/src# hefesto --disable-static
```

Independently of using static or shared linking on ``zc``, the whole build task will build the device driver,
the user-mode tool, run some testing and after a successful build you should run ``hefesto --install``.
Once installed you will be able to start using ``Zacarias`` as your password manager. If you are new here it
is time to start reading the [user's manual](https://github.com/rafael-santiago/zacarias/blob/main/doc/MANUAL.md).

If you want to uninstall, use: ``hefesto --uninstall``.

[``Back``](#contents)
