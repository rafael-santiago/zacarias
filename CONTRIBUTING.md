# CONTRIBUTING

The best way of knowing what could be done is reading 'doc/todo.txt'.

# Code style

There is no absolute truth (a.k.a unicorn) when talking about code style, anyway, this is the truth that I have
been taking into consideration here.

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
and implemenation files should be relative to the toplevel src subdirectory.

Gotos are ok if it always move forward but never ever backward.

Avoid capital letters in function and variable names.

C Defines:

- while constants must be in upper case;
- while macros must be in lower case;
- while a DSL statement must be in upper case;

# Use inclusive and neutral language

Always try to use inclusive and neutral words/terms in your source codes and documentations. If you find something that
for you seems to be not so correct, please let me known by opening an issue and suggesting improvements. Thank you in
advance.

In general avoid use colors to name what should be "good" or "bad". Outdated terms such as ``whitelist``/``blacklist``
are deprecated/banned here. You should use ``allowlist/denylist`` or anything more related to what you really are doing. Terms
like ``master/slave`` are out too. You could use ``main``, ``secondary``, ``next``, ``trunk``, ``current``, ``supervisor``,
``worker`` in replacement.

Do not use sexist and machist terms, too.
