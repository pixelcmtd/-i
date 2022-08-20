# `-i`

Have you ever tried to do something like `program < $file > $file`? Then you
know the pain. This tool tries to solve that problem by both providing the nicer
and “[dry](https://en.wikipedia.org/wiki/Don%27t_repeat_yourself)” syntax
`-i $file program` and making it actually work without ruining your file.

In addition, this tool should be useful as a replacement for
implementation-specific in-place flags. For example, write a script on FreeBSD
or Darwin/macOS using `sed` to mutate a file in-place. Now try running it on a
GNU system, or vice versa.
