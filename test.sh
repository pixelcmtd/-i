#!/bin/sh
set -eu

d=$(mktemp -d)
trap 'rm -rf "$d"' 0

echo one > "$d/file"
chmod 640 "$d/file"
./-i "$d/file" tr o O
echo One | cmp - "$d/file"
test -n "$(find "$d/file" -perm 0640 -print)"

if ./-i "$d/file" sh -c 'echo bad; exit 7'; then
    exit 1
else
    test "$?" -eq 7
fi
echo One | cmp - "$d/file"

(exec 1>&-; ./-i "$d/file" tr O o)
echo one | cmp - "$d/file"
