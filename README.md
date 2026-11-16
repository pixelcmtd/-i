# `-i`

Have you ever tried to do something like `program < $file > $file`? Then you
know the pain. This tool tries to solve that problem by both providing the nicer
and “[dry](https://en.wikipedia.org/wiki/Don%27t_repeat_yourself)” syntax
`-i $file program` and making it actually work without ruining your file.

In addition, this tool should be useful as a replacement for
implementation-specific in-place flags. For example, write a script on FreeBSD
or Darwin/macOS using `sed` to mutate a file in-place. Now try running it on a
GNU system, or vice versa.

Build with `make`, then run `./-i file command [arg ...]`.

Symlinks become regular files; their targets stay unchanged. Other hard links
keep the old content.

Limitations:

- The file's directory must be writable. Editing fails if ownership cannot be
  preserved.
- ACLs and extended attributes are lost.

For future ACL/xattr support, investigate macOS `copyfile`/`fcopyfile` and Linux
`libacl` (`acl_get_fd`/`acl_set_fd`) and `libattr` (`attr_copy_fd`).
