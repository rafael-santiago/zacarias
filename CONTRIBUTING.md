# CONTRIBUTING

The best way of knowing what could be done is reading ``doc/todo.txt``.

This is the general ``Zacarias`` repository layout:

- ``doc``: Documentation.
    - ``man``: Unix zc's tool Unix manpage.
- ``etc``: Miscellaneous stuff.
- ``src``: All source code must be placed somewhere under this sub-directory.
    - ``build``: Build scripts.
    - ``cmd``: zc's command line tool source code.
        - ``test``: zc's unit tests and system tests.
    - ``dev``: Zacarias' device driver source code.
        - ``ctx``: Codes related to Zacarias structures used in or shared from kernel.
            - ``test``: Prelimiar user-mode unit tests from those kernel mode structures.
        - ``defs``: Definitions related to the device driver.
        - ``freebsd``: Platform dependent parts of the device driver for FreeBSD.
        - ``linux``: Platform dependent parts of the device driver for Linux.
        - ``windows``: Platform dependent parts of the device driver for Windows.
        - ``sec``: Codes directly related to security.
            - ``test``: Prelimiar user-mode unit tests from those security related stuff.
    - ``kbd``: Codes related to keyboard management.
        - ``test``: Unit tests for all keyboard management stuff.
    - ``lib``: Libraries artifacts and git-submodules are built/hosted here.
    - ``libc``: Codes related to our minimal libc replacements.
        - ``test``: Unit tests for our minimal libc replacements.
    - ``test``: Some general tests.
        - ``kernel``: Kernel-mode unit tests for essential parts used into device driver.

# Code style

There is no absolute truth (a.k.a unicorn) when talking about code style, anyway, this is the truth that I have
been taking into consideration here.

- Replace tabs with spaces. Each tab must be equal to eight spaces. Use one tab to indent to the next level;
- Avoid trailing spaces at the end of the lines;
- Your work must exist without having you around. Code it to make your ideas self-sufficient just by reading
  your code and its documentation let by you since the last time you have been working here;
- Comments are good;
- Tests are awesome (use the testing library used by the project <https://github.com/rafael-santiago/cutest>);
- If your test need a human to drive it, sorry but your test is a kind of crap;
- We like C (C, not C with sugar or anything similar);
- Do not use 'func()' when prototyping use 'func(void)';
- Arrays as function parameters are not welcome, use pointers;
- Pointers are good. Taking into consideration that you are a fluent and experienced C programmer;
- The project should be the most self-contained possible;
- Git submodules are ok, but must be hosted into 'src/lib' directory;
- This project is GPLv2. Always include the copyright disclaimer in new code files;
- Respect the credit from other people;
- Give credit to other people;
- Give credit to yourself;

# Code constructions

Always include the statmements between '{ ... }'.

This is considered bad here:

```c
        if (x < y) do_stuf();

        if (x < y)
            do_this();
        else
            do_that();
```

Macros are ok, however, "undef" it when it is not necessary anymore:

```c
    #define scoped_left_sh(x, s) ( (x) << (s) ) | ( (x) >> ( (sizeof(x) << 3) - (s) ) )

        ...

    #undef scoped_left_sh
```

When passing a string pass its size besides assuming it as a null terminated string.

When commenting some code use the following format:

```c
    // INFO(your name, or as people know you): This is an information.

    // WARN(your name, or as people know you): This is a warning.

    // FIXME(your name, or as people know you): I do not know how to solve it.

    // BUG(your name, or as people know you): I found a bug here, it should be fixed.

    // TODO(your name, or as people know you): To do item.

    // TIP(your name, or as people know you): You are giving the tips for people understand
    //                                        some craziness, weird code chunk.
```

Static functions even being static must be prototyped at the beginning of the implementation file.

Avoid using double quotes when including files in C implementation stuff (local includes). Headers
and implemenation files should be relative to the top-level src sub-directory.

Gotos are ok if it always move forward but never ever backward.

Avoid capital letters in function and variable names.

C Defines:

- while constants must be in upper case;
- while macros must be in lower case;
- while a DSL statement must be in upper case;

# Avoiding libc hook

Since ``zacarias`` is a password manager we need to mitigate as much as possible chances of password leakage.
I believe that the cornerstone of mitigating this kind of problem is avoiding library hooking.

``Zacarias`` try to reduce its dependencies in order to be able to control it in a more sane way. The ``zc``
command line tool tries to use only ``libc`` conveniences on ``unix-like`` but it also uses ``Xorg`` seeking
to be also useful to users of ``X`` based environments.

When stealing info from applications, the most targeted API functions are the memory handling functions from
``libc``: ``memcpy``, ``memset`` and ``memcmp``. When coding ``Zacarias`` you must avoid using directly
those functions. You must not avoid calling it from your code, but you need to take the care of passing
to the compiler the following macros:

- ``-Dmemcpy=zc_memcpy``
- ``-Dmemset=zc_memset``
- ``-Dmemcmp=zc_memcmp``

It will replace all original memory handling functions to our local implementations located at ``src/libc``.
Here, our ``memcmp`` (``zc_memcmp``) is also time attack resilient.

When using ``zc_memcpy``, ``zc_memset`` or ``zc_memcmp`` your code will depend on ``libzcc``. Thus you
need to indicate ``-lzcc`` when building your tests or even another final artifact besides ``zc``
command line tool.

Anyway, if you have added a new sub-module into ``src`` (e.g.: ``src/passwd_mind_transfer_proto``). All
your calls to ``memcpy``, ``memset`` or ``memcmp`` will be replaced automatically because your build has
inherited the compiler macros present within top-level ``src`` directory invocation file (``.ivk``).

However, it is necessary to add the following lines into the epilogue function of your build project:

```
    ...
    if (hefesto.sys.last_forge_result() == 0) {
        if (has_bad_funcs(hefesto.sys.lines_from_file("../BAD_FUNCS", ".*"), $src, $includes, $cflags)) {
            hefesto.project.abort(1);
        }
        ...
    }
```

The build function ``has_bad_funcs`` will look for "bad functions" (to our context) into your code by
breaking the build when at least one be found.

The file ``BAD_FUNCS`` is located at the top-level ``src`` directory. The function ``has_bad_funcs`` is
defined within ``build/toolsets.hsl``.

Doing it at least the most critical parts will be mitigated but what to do about ``ioctl`` and ``Xorg``
functions? You should use ``static link``. By the way, this is the default linking configuration used
by ``Zacarias``. Due to it by default you will demanded to indicate some ``Xorg`` library tarballs at the
first-time ``Zacarias'`` build.

The bad functions searching is an important mitigation for people that still prefer using a ``shared link``
version of ``Zacarias``. It will allow us to give some minimal level of password leaking mitigation for those
people, too.

# Use inclusive and neutral language

Always try to use inclusive and neutral words/terms in your source codes and documentations. If you find something that
for you seems to be not so correct, please let me know by opening an issue and suggesting improvements. Thank you in
advance.

In general avoid use colors to name what should be "good" or "bad". Outdated terms such as ``whitelist``/``blacklist``
are deprecated/banned here. You should use ``allowlist/denylist`` or anything more related to what you really are doing. Terms
like ``master/slave`` are out too. You could use ``main``, ``secondary``, ``next``, ``trunk``, ``current``, ``supervisor``,
``worker`` in replacement.

Do not use sexist and/or machist terms, too.
