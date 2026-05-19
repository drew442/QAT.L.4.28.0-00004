#!/bin/sh
set -eu

package_name="qat"
package_version="4.28.0-00004"
source_dir="${QAT_DKMS_SOURCE_DIR:-/usr/src/${package_name}-${package_version}}"

if [ "$(id -u)" -ne 0 ]; then
	echo "QAT DKMS installer must be run as root" >&2
	exit 1
fi

if command -v dkms >/dev/null 2>&1; then
	:
else
	echo "QAT DKMS installer requires dkms in PATH" >&2
	exit 1
fi

if dkms status -m "$package_name" -v "$package_version" >/dev/null 2>&1; then
	if [ "${QAT_DKMS_REPLACE:-0}" = "1" ]; then
		dkms remove -m "$package_name" -v "$package_version" --all || true
	else
		echo "QAT DKMS package $package_name/$package_version already exists." >&2
		echo "Set QAT_DKMS_REPLACE=1 to remove and rebuild it." >&2
		exit 1
	fi
fi

if [ -z "$source_dir" ] || [ "$source_dir" = "/" ]; then
	echo "Refusing unsafe QAT_DKMS_SOURCE_DIR: $source_dir" >&2
	exit 1
fi

if [ -d "$source_dir" ] &&
	[ -n "$(find "$source_dir" -mindepth 1 -maxdepth 1 -print -quit)" ]; then
	if [ "${QAT_DKMS_REPLACE:-0}" = "1" ]; then
		find "$source_dir" -mindepth 1 -maxdepth 1 -exec rm -rf -- {} +
	else
		echo "QAT DKMS source directory already exists and is not empty: $source_dir" >&2
		echo "Set QAT_DKMS_REPLACE=1 to replace it." >&2
		exit 1
	fi
fi

mkdir -p "$source_dir"
tar \
	--exclude=.git \
	--exclude=build \
	--exclude=./Makefile \
	--exclude=config.log \
	--exclude=config.status \
	--exclude=openzfs \
	--exclude='*.ko' \
	--exclude='*.o' \
	-cf - . | tar -C "$source_dir" -xf -

dkms add -m "$package_name" -v "$package_version"
dkms build -m "$package_name" -v "$package_version"
dkms install --force -m "$package_name" -v "$package_version"

echo "QAT DKMS source installed at $source_dir"
