# ![Z](https://github.com/rafael-santiago/zacarias/blob/master/etc/zc_logo.png "Zacarias password manager") acarias

# Zacarias

``Zacarias`` is a password manager with ``kernel-mode`` powers. With this tool all password database encryption/decryption
is done from ``kernel``. The idea here is isolate as much as possible your secrets avoiding any involuntary data leaking,
easier to happen from ``user-space`` context. The name ``Zacarias`` means ``the Lord recalled``.

Until now this tool features:

- ``AES-256/GCM`` to encrypt the password database.

- ``Argon2i`` as its ``KDF``.

- Predefined passwords storing.

- Generate new passwords.

- Session password definition.

- Clipboard use avoidance, it types your passwords for you.

- Password database I/O done from ``kernel-space`` context.

Until now you can use it on ``Linux`` (4.4.14 or above).

- If you want to know how to build this tool take a look at [doc/BUILD.md](https://github.com/rafael-santiago/zacarias/blob/master/doc/BUILD.md).

- If you want to learn more details about it take a look at [doc/MANUAL.md](https://github.com/rafael-santiago/zacarias/blob/master/doc/MANUAL.md).
