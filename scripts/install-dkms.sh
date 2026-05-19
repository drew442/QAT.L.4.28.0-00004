#!/bin/sh
set -eu

package_name="qat"
package_version="4.28.0-00004"
source_dir="${QAT_DKMS_SOURCE_DIR:-/usr/src/${package_name}-${package_version}}"
kernelver="$(uname -r)"
module_names="intel_qat qat_api usdm_drv qat_dh895xcc qat_c62x qat_dh895xccvf qat_c62xvf"
script_dir="$(unset CDPATH; cd -- "$(dirname -- "$0")" && pwd)"
package_root="$(unset CDPATH; cd -- "$script_dir/.." && pwd)"

cd "$package_root"

die() {
	echo "QAT DKMS installer: $*" >&2
	exit 1
}

require_cmd() {
	command -v "$1" >/dev/null 2>&1 ||
		die "missing required command in PATH: $1"
}

require_file() {
	[ -r "$1" ] || die "missing required file: $1"
}

require_dir() {
	[ -d "$1" ] || die "missing required directory: $1"
}

preflight() {
	[ "$(id -u)" -eq 0 ] || die "must be run as root"

	for cmd in ar date depmod dkms find g++ gcc make modinfo nasm pkg-config tar; do
		require_cmd "$cmd"
	done

	require_dir "/lib/modules/$kernelver/build"
	require_file "/lib/modules/$kernelver/build/Makefile"
	require_file "./configure"
	require_file "./quickassist/Makefile"
	require_file "./quickassist/qat/Makefile"
	require_file "./quickassist/utilities/osal/include/Osal.h"
	require_file "./quickassist/build_system/build_files/qat.service"
	require_file "./quickassist/build_system/build_files/qat_service"
	require_file "./quickassist/utilities/adf_ctl/Makefile"
	require_file "./quickassist/utilities/adf_ctl/conf_files/dh895xcc_dev0.conf"
	require_file "./quickassist/utilities/adf_ctl/conf_files/c6xx_dev0.conf"

	if [ "${QAT_DKMS_REQUIRE_RUNTIME_TOOLS:-1}" = "1" ]; then
		require_cmd adf_ctl
		found_runtime_config=0
		for config in /etc/dh895xcc_dev*.conf /etc/c6xx_dev*.conf; do
			if [ -e "$config" ]; then
				found_runtime_config=1
				break
			fi
		done
		if [ "$found_runtime_config" -eq 0 ]; then
			die "missing active QAT runtime config under /etc/dh895xcc_dev*.conf or /etc/c6xx_dev*.conf"
		fi
	fi
}

preflight

if [ "${QAT_DKMS_PREFLIGHT_ONLY:-0}" = "1" ]; then
	echo "QAT DKMS preflight passed"
	exit 0
fi

if dkms status -m "$package_name" -v "$package_version" >/dev/null 2>&1; then
	if [ "${QAT_DKMS_REPLACE:-0}" = "1" ]; then
		dkms remove -m "$package_name" -v "$package_version" --all || true
	else
		die "package $package_name/$package_version already exists; set QAT_DKMS_REPLACE=1 to remove and rebuild it"
	fi
fi

if [ -z "$source_dir" ] || [ "$source_dir" = "/" ]; then
	die "refusing unsafe QAT_DKMS_SOURCE_DIR: $source_dir"
fi

if [ -d "$source_dir" ] &&
	[ -n "$(find "$source_dir" -mindepth 1 -maxdepth 1 -print -quit)" ]; then
	if [ "${QAT_DKMS_REPLACE:-0}" = "1" ]; then
		find "$source_dir" -mindepth 1 -maxdepth 1 -exec rm -rf -- {} +
	else
		die "source directory already exists and is not empty: $source_dir; set QAT_DKMS_REPLACE=1 to replace it"
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

shadow_root="/lib/modules/$kernelver/updates/drivers/crypto/qat"
archive_root="/var/backups/qat-dkms-shadowed-$kernelver-$(date -u +%Y%m%dT%H%M%SZ)"

for old_archive in /lib/modules/"$kernelver"/updates/qat-dkms-shadowed-*; do
	if [ -e "$old_archive" ]; then
		mkdir -p "$archive_root/legacy-updates-archive"
		mv "$old_archive" "$archive_root/legacy-updates-archive/"
	fi
done

for module in $module_names; do
	case "$module" in
		intel_qat)
			shadow_path="$shadow_root/qat_common/$module.ko"
			;;
		qat_*)
			shadow_path="$shadow_root/$module/$module.ko"
			;;
		*)
			continue
			;;
	esac

	if [ -e "$shadow_path" ]; then
		archive_path="$archive_root/${shadow_path#/lib/modules/"$kernelver"/updates/}"
		mkdir -p "$(dirname "$archive_path")"
		mv "$shadow_path" "$archive_path"
	fi
done

depmod -a "$kernelver"

for module in $module_names; do
	module_path="$(modinfo -k "$kernelver" -n "$module")"
	case "$module_path" in
		/lib/modules/"$kernelver"/updates/dkms/*)
			;;
		*)
			echo "QAT DKMS: $module resolves outside /updates/dkms: $module_path" >&2
			exit 1
			;;
	esac
done

echo "QAT DKMS source installed at $source_dir"
