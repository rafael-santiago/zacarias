# Build

``Zacarias`` build is based on another tool of mine called [Hefesto](https://github.com/rafael-santiago/hefesto).
``Hefesto`` is a well-simple and contained build system if you have a C compiler on your system you are able to
build and install ``Hefesto``. There is no reason to be scared! ;)

## Contents

- [Setup your environment](#setup-your-environment)
- [Cloning Zacarias repository](#cloning-zacarias-repository)
- [Building](#building)
- [Dealing with lkm signing on Linux](#dealing-with-lkm-signing-on-linux)
- [Secure boot, signature enforcement and software bureaucracy hypes et al](#secure-boot-signature-enforcement-and-software-bureaucracy-hypes-et-al)

## Setup your environment

After putting ``Hefesto`` to work on you environment you need to clone another repository called ``Helios`` and
then do the following if you are on ``Linux``:

```
you@AdventureGalley:~/# git clone https://github.com/rafael-santiago/helios
you@AdventureGalley:~/# cd helios
you@AdventureGalley:~/helios# hefesto --install=lnx-module-toolset,forge-bootstrapper
you@AdventureGalley:~/helios# cd ..
you@AdventureGalley:~/# rm -rf helios
```

On ``FreeBSD``:

```
you@AdventureGalley:~/# git clone https://github.com/rafael-santiago/helios
you@AdventureGalley:~/# cd helios
you@AdventureGalley:~/helios# hefesto --install=freebsd-module-toolset,forge-bootstrapper
you@AdventureGalley:~/helios# cd ..
you@AdventureGalley:~/# rm -rf helios
```

On ``Windows``:

```
C:\Users\You> git clone https://github.com/rafael-santiago/helios
C:\Users\You> cd helios
C:\Users\You> hefesto --install=windows-driver-toolset,forge-bootstrapper
C:\Users\You> cd ..
C:\Users\You> rmdir helios /s /q
```

Maybe ``xorgproto`` headers will be missing on your ``FreeBSD`` box, in this case you should try:

```
you@AdventureGalley:~/# pkg install -f xorgproto
you@AdventureGalley:~/# pkg install xcb-proto
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
and ``libpthread`` required to build the tests. If you are on Windows you need to have ``WDK`` and ``MSVC``
installed.

Filling up those requirements all you should do is to change to ``src`` sub-directory of your repo's copy and
invoke ``Hefesto`` from there:

```
you@AdventureGalley:~/# cd zacarias/src
you@AdventureGalley:~/zacarias/src# hefesto
```

Since the build involves tests for a command line tool and also a device driver, at some point this driver will
be loaded during the tests due to it you need to run build task as ``root`` (Unix-like) or ``Administrator`` (Windows).

On ``Linux`` and ``FreeBSD``, due to library hooking mitigations, by default, ``Zacarias`` command line tool (``zc``)
is static linked. In this way, at the first time you run ``Zacarias`` build it is necessary to build all ``Xorg``
dependencies used by ``zc``. Thus, if you want to follow using the default static linking you will need to download
the following ``Xorg`` tarballs: ``libX11``, ``libXau``, ``libXdmcp``, ``libXext``, ``libXtst`` and ``libxcb``.
You need to watch the correct versions used on your system. Once downloaded all you should do is indicating to ``hefesto``
where those tarballs are located and they will built and copied into your ``Zacarias`` repo ``lib`` sub-directory, your
system will not be changed. Follows the command line sample based on my own environment:

```
you@AdventureGalley:~/zacarias/src# hefesto --libX11-pkg=/tmp/libX11-1.6.3.tar.gz \
> --libXtst-pkg=/tmp/libXtst-1.2.2.tar.gz --libxcb-pkg=/tmp/libxcb-1.11.tar.bz2 \
> --libXext-pkg=/tmp/libXext-1.3.3.tar.gz --libXdmcp-pkg=/tmp/libXdmcp-1.1.1.tar.bz2 \
> --libXau-pkg=/tmp/libXau-1.0.8.tar.bz2
```

This command will build the ``Xorg`` dependencies and after building ``Zacarias`` by linking its command line tool statically.

You can find the related ``Xorg`` tarballs at: <https://www.x.org/releases/individual/lib> and
<https://xcb.freedesktop.org/dist>.

Maybe your unix-like system will also need to install dev packages of X11 try to ask your package manager to install
the following packages: ``libx11-dev``, ``libxtst-dev``, ``xcb-proto`` and finally ``python3-xcbgen``.

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

On ``Windows`` the installer is able to export the command line tool path, but you need to run the install from
a command prompt with Administrator privileges and after that you need to do a log-off and finally log-on again.

[``Back``](#contents)

## Dealing with lkm signing on Linux

If your Linux kernel was built with module signature verification the ``Zacarias`` device driver
must be signed. In this way you will be able to insert and use it accordingly, otherwise an
error suggesting you to sign the lkm before using it will be displayed.

Currently the build has the ability of signing the generated device drivers. In order to do it
you must flag out your signing intentions by passing the option ``--sign-lkm``.

When not indicating any previous generated key pair (private and public) the build will
try to generate a new one at the toplevel ``etc`` from your repo copy, under the names
``zcdev_priv.pem`` and ``zcdev_pub.pem``. The build also will try to add the public key to your
system keyring, due to it is necessary to indicate the id of your system keyring by passing the
option ``--system-keyring-id``, too. E.g.:

```
you@AdvendureGalley:~/zacarias/src# hefesto --sign-lkm \
> --system-keyring-id=0x10abcabc
```

In order to figure out the address of your system keyring run the command ``cat /proc/keys``,
looking for ``.system_keyring`` entry. More information can be found [here](https://www.kernel.org/doc/html/v4.10/admin-guide/module-signing.html).

If you have already a key pair (private and public) previously added to your system keyring you
can indicate the fullpath to them by using ``--priv-key-path`` and ``--pub-key-path``. E.g.:

```
you@AdvendureGalley:~/zacarias/src# hefesto --sign-lkm \
> --priv-key-path=/etc/my_local_lkm_keys_store/priv.pem \
> --pub-key-path=/etc/my_local_lkm_keys_store/pub.pem
```

[``Back``](#contents)

## Secure boot, signature enforcement and software bureaucracy hypes et al

First of all a mantra:

>I do not provide signed drivers and I am not interested in doing it. If you want to do so, go ahead but I will not endorse you.

Until now (2021) ``FreeBSD`` is the only supported platform that does not offer those complications. Some
``Linux`` versions use ``UEFI`` boot in order to control which driver should really be loaded or not. Similarly
``Windows`` does it in a less acrobatic and more self-sufficient way. In both cases the drivers must be signed
and you need to follow some additional steps to make the driver loading happens on those platforms.

On ``Linux`` the process only includes sign the kernel module by using a PK key pair and a password. During
the next boot after doing it the password will be asked during boot. Does it boring? A little, Zzzz.

On ``Windows`` you need to attest your device driver against some ``Microsoft`` testing suite by using their
``HLK Studio``. You need a Windows server and a machine running the Windows version that you are intending to
load this attested device driver. They provide some virtual machines but only Hypher-V based. After that you
generate a package and submit it at Microsoft site to be signed. Finally, you download the signed version. Yes,
it is a long and pretty boring way. Every little change on device driver will make you re-run all those tasks.
At least I have executed all driver verifier tests against the device driver, so HLK tests should not complain
about anything. Good luck!  

I am a software developer not a lawyer and all this talk about certification make me fall asleep. Now you should
understand why I do not provide signed drivers and will not do it so.

If you want to load the unsigned drivers on ``Linux`` you need to run it in a non-uefi boot based machine. On
``Windows`` you can disable the driver enforcement by running ``bcdedit /set testsigning on`` (``UEFI`` must
be previously disabled on your ``BIOS``).

Quite frankly, if you have loaded a rootkit into your system, that requires root or administrator privileges
to make it happen, you really deserve a rootkit into your system. Sorry for the sincerity.

[``Back``](#contents)
