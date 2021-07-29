# Internal Qt protocol extensions

The protocol extensions in this folder are considered implementation details of
Qt. I.e. they may removed, renamed or changed without warning.

## Suffixed  protocols

For protocols that have a version suffix, however, we will strive to not break
backwards compatibility without bumping the suffix (renaming the protocol).
E.g.: If your client sees a `zqt_key_v1` global, it can safely bind to it:
the key event will always take the same number of arguments, regardless of
compositor version.

This is important also within a Qt-only scope if there are multiple versions of
Qt on the system. Consider for instance an application statically linked to Qt
(such as Qt Creator) running against a Qt compositor installed by the distro).
In such cases we don't want the compositor and client to disagree on the
protocol definition.
