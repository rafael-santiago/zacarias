# Zacarias - user's guide

``Zacarias`` is a password manager with ``kernel-mode`` powers. It is suitable for freak paranoid users that do not like
the idea of having a simple user application encrypting/decrypting your passwords sit on ``user-space`` sending their
passwords to clipboard. A simple user application that can be accessible from other applications or even easily debugged,
probed, logged, hooked, swapped out from memory etc. You prefer mitigating those issues by letting ``kernel`` takes the control
during some critical parts of your passwords management.

As you may know the name ``Zacarias`` means ``the Lord recalled``. ``Lord`` here relates to the ``kernel`` of your operating
system. Only it will access directly your password database, never ever the ``user-space`` portion. In this sense if we got
a leak, this leak will be less harmful.

## Contents
- [How Zacarias works](#how-zacarias-works)
- [The device](#the-device)
    - [Installing Zacarias device](#installing-zacarias-device)
    - [Uninstalling Zacarias device](#uninstalling-zacarias-device)
- [The zc tool](#the-zc-tool)
    - [Attaching your profile](#attach-your-profile)
    - [Adding a password](#add-a-password)
    - [Deleting a password](#deleting-a-password)
    - [Getting a password](#getting-a-password)
    - [Detaching your profile](#detach-your-profile)
    - [Sessioned profiles](#sessioned-profiles)

## How Zacarias works

``Zacarias`` works based on a char device. Through this char device users can store and access their passwords by requesting
it by using the ``zc`` command line tool. The zacarias binary package is divided into two parts: 
``zacarias.ko`` (the device driver) and ``zc`` (the user device interaction tool).

[``Back``](#contents)

## The device

The password management here works based on a device driver that performs all real I/O stuff over the encrypted password
databases. This device driver can handle more than one attached user.

[``Back``](#contents)

### Installing Zacarias device

All password management depends on the device. It must be previously installed before any using attempt. You can do it by
using the general operating system driver loading command or simply using ``zc`` passing the command ``device`` and its
sub-command ``install``:

```
you@AdventureGalley:~/# zc device --install --device-driver-path=/usr/local/share/zacarias/mod/zacarias.ko
```

Once done the device under ``/dev/zacarias`` is ready.

[``Back``](#contents)

### Uninstalling Zacarias device

If you do not want to let the device loaded during all your session, you can uninstall it by using the general operating
system driver unloading command or simply using ``zc`` passing the command ``device`` and its sub-command ``uninstall``:

```
you@AdventureGalley:~/# zc device --uninstall
```

After unloading any previously loaded sessioned profile will be lost but all its stored passwords will be consistent and safe.

[``Back``](#contents)

## The zc tool

The ``zc`` tool provides the channel between the user and the ``kernel-mode`` portion of this password manager.

This tool only can work properly with a previously well-loaded device driver on your [system](#installing-zacarias-device).

[``Back``](#contents)

### Attacing your profile

When you attach your profile you are informing to ``Zacarias`` device driver who are you and where to find your passwords
(where your password database are stored in).

This can be done by using the following command:

```
you@AdventureGalley:~/# zc attach --user=kidd --pwdb=kidd.pwdb
```

The general password will be asked and if it authenticates your profile will be loaded and put into a ready state.

However, the command above only will work with a previous ``kidd.pwdb`` database. At the first time you need to initialize
it by using the option ``--init``:

```
you@AdventureGalley:~/# zc attach --user=kidd --pwdb=/kidd/stuff/kidd.pwdb --init
```

After running this command a general password will be asked and confirmed. An important detail here is that from now on you
need to remember your informed user name (filled in ``--user`` option) otherwise the profile loading will fail.

The password database path informed by using ``--pwdb`` option can be relative without problem. Its content is about ``PEM``
authenticated data by using ``AES-256/GCM``.

[``Back``](#contents)

### Adding a password

In order to add a new password you need to use the command ``password`` and its sub-command ``add``. But you need to
inform:

1. Who you are by using the option ``--user``.
2. What alias this password will have into your password database (PWDB) through the option ``--alias``.
3. If you want to ``Zacarias`` device generates for you a password you need to pass the option flag ``--generate``.


```
you@AdventureGalley:~/# zc password add --user=kidd --alias=treasure@island.nowherehere
```

It will ask your general PWDB password.

When ``--generate`` is not passed the password related with the new alias being added will be asked and confirmed, for sure.

[``Back``](#contents)

### Deleting a password

Delete a password is pretty easy, being needed you to inform:

1. Who you are by using ``--user`` option.
2. What alias you want to delete by using ``--alias`` option.
3. The PWDB password.

```
you@AdventureGalley:~/# zc password del --user=kidd --alias=treasure@island.nowherehere
```

[``Back``](#contents)

### Getting a password

Maybe you are thinking about how a device driver based password manager would be able to expose easily the stored passwords to
the user. Well, the idea here is avoid spreading those secrets across the system.

The found way was simulate the password typing. So you inform ``Zacarias`` device about what password you are looking for,
it will get the password, give you a timeout to positioning your cursor focus where this password must be put and abracadabra.

You can do it by using the command ``password`` and its sub-command ``get``. This sub-command need the following information:

1. Who you are. Use ``--user`` option to inform it.
2. What password you are looking for. Use ``--alias`` option to inform it.
3. How much (in seconds) ``Zacarias`` must wait for you to be ready to receive your password at the expected place.
   Use the option ``--timeout``. By default it is fixed to 3 seconds.

```
you@AdventureGalley:~/# zc password get --user=kidd --alias=treasure@island.pirate-bay.com
```

It will ask your general PWDB password. Being it well authenticated you will get your password soon.

[``Back``](#contents)

### Detaching your profile

It is pointless to let your profile attached in ``Zacarias`` device if you are not actually using it or will keep for a long
time during your session without using any password authentication. Remember that the idea is reduce disclosure, leaking etc.
You have to help doing your part, too. In this case, you should detach your profile:

```
you@AdventureGalley:~/# zc detach --user=kidd
```

It will ask your PWDB password. Being it well-authenticated your profile will be detached.

[``Back``](#contents)

### Sessioned profiles

Maybe you can see the necessity of informing the general PWDB password as a weak point into the workflow. If you want to avoid
almost possible the necessity of informing those critical secret you could give sessined profiles a try.

A sessioned profile is a profile that at the moment of its attachement a session password is configured. Once configured any
``password get`` command will only requires this temporary password instead of the general one. However, bearmind if you want
to add or delete passwords from your PWDB, the two passwords will be needed (the general and the session password).

Attach a sessioned profile is strainghtforward:

```
you@AdventureGalley:~/# zc attach --user=kidd --pwdb=kidd.pwdb --sessioned
```

Ask for a password is the same as before but the PWDB password ``must be the sessioned one``:

```
you@AdventureGalley:~/# zc get --user=kidd --alias=treasure@island.pirate-bay.com
```

Add a new password into with a sessioned profile requires the flag option ``--sessioned``. You will need to
inform the general password and the session password, too:

```
you@AdventureGalley:~/# zc password add --user=kidd --alias=treasure@island.nowherehere --sessioned
```

Delete is the same. You need to inform the general and session passwords:

```
you@AdventureGalley:~/# zc password del --user=kidd --alias=treasure@island.nowherehere --sessioned
```

Sessioned profiles detaching only requires the session password and no flag option:

```
you@AdventureGalley:~/# zc detach --user=kidd
```

[``Back``](#contents)