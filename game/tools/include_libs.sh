#!/bin/bash

if [ $# -ne 3 ]; then
	echo "Usage: include_libs.sh <exe> <destination folder> <relative path>" 1>&2
	exit 1
fi

debug()
{
	if [ -n "$DEBUG" ]; then
		echo "$@"
	fi
}

exe="$1"
dst="$2"
relative="$3"

mkdir -p "$dst"

local_deps_of()
{
	otool -L "$1" | cut -f 2 | grep ^/usr/local | cut -f 1 -d ' '
}

local_frameworks_of()
{
	otool -L "$1" | cut -f 2 | grep '^@executable_path.*\.framework' | cut -f 1 -d ' '
}

old_to_new()
{
	echo "$1" | sed -e "s|/usr/local/lib|@executable_path/$relative|"
}

list=()
while read dep; do
	debug "Found library $dep"
	list[${#list[@]}]=$dep
done < <(local_deps_of "$exe")

# FIXME: technically, we should re-iterate over the collected libs recursively and get their deps as well.

for (( i = 0; i < ${#list[@]}; i++ )); do
	old="${list[$i]}"
	new=`old_to_new "$old"`
	base=`basename "$old"`
	debug "Including $base"
	cp "$old" "$dst/"
	debug "Changing id of $dst/$base to $new"
	install_name_tool -id "$new" "$dst/$base"
done

list[${#list[@]}]=$exe
for (( i = 0; i < ${#list[@]}; i++ )); do
	cur="${list[$i]}"
	if echo "$cur" | grep -q '^/usr/local'; then
		cur=`basename "$cur"`
		cur="$dst/$cur"
	fi
	debug "Checking dependencies of $cur"
	local_deps_of "$cur" | while read dep; do
		new=`old_to_new "$dep"`
		debug "Modifying dependency of $cur from $dep to $new"
		install_name_tool -change "$dep" "$new" "$cur"
	done
done


list=()
while read dep; do
	base=`echo "$dep" | sed -e 's@\(\.framework\)/.*@\1@'`
	base=`basename "$base"`
	debug "Found framework $base"
	list[${#list[@]}]=$base
done < <(local_frameworks_of "$exe")

for (( i = 0; i < ${#list[@]}; i++ )); do
	dep="${list[$i]}"
	debug "Including $dep"
	new="$dst/$dep"
	old="/Library/Frameworks/$dep"
	cp -a "$old" "$new/"
	find "$new" -type l       -print0 | xargs -0 rm
	find "$new" -name Headers -print0 | xargs -0 rm -rf
done

