# ![Z](https://github.com/rafael-santiago/zacarias/blob/main/etc/zc_logo.png "a lousy logo goes here...") Zacarias

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

- User command line tool static linked in order to mitigate library hooking.

- Anti-debugging mechanism to mitigate/alert data leaking.

Until now you can use it on ``Linux`` (4.4.14 or above), ``FreeBSD`` (12.1 but I believe that with lower versions it will work, too) and ``Windows``.

- If you want to know how to build this tool take a look at [doc/BUILD.md](https://github.com/rafael-santiago/zacarias/blob/main/doc/BUILD.md).

- If you want to learn more details about it take a look at [doc/MANUAL.md](https://github.com/rafael-santiago/zacarias/blob/main/doc/MANUAL.md).

- If you have new ideas and are up to some contribution take a look at [CONTRIBUTING.md](https://github.com/rafael-santiago/zacarias/blob/main/CONTRIBUTING.md) and thank you!

- If you have found a bug or even have a question, let me know by opening a new [issue](https://github.com/rafael-santiago/zacarias/issues).


---

**Bear in mind**: Use this software at your own risk. I am not responsible for any misuse of it, including some kind of damage,
data loss etc. The software is provided with no warranty.

**Remark**: I do not provide pre-builts, if you have found any pre-built of this software somewhere, I **do not** endorse it.

**Last but not least**: Sed quis custodiet ipsos Custodes? The code is open and auditable and it always will be.
I believe this is the best paradigm for certain information security software. Anyone with disposition, knowledge
and concerned about privacy is encouraged to join this project by reviewing, escrutinizing, proposing and always
improving it on.
