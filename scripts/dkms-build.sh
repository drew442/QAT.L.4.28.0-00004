#!/bin/sh
set -eu

kernel_source_dir="${1:-${kernel_source_dir:-}}"
kernelver="${2:-${kernelver:-$(uname -r)}}"

if [ -z "$kernel_source_dir" ]; then
	kernel_source_dir="/lib/modules/$kernelver/build"
fi

if [ ! -d "$kernel_source_dir" ]; then
	echo "QAT DKMS: kernel source directory not found: $kernel_source_dir" >&2
	exit 1
fi

configure_flags="${QAT_DKMS_CONFIGURE_FLAGS:---enable-kapi --enable-qat-lkcf}"
build_output="${QAT_DKMS_BUILD_OUTPUT:-$PWD/build}"
param_check="${QAT_DKMS_PARAM_CHECK:-y}"

case "$param_check" in
	y|n)
		;;
	*)
		echo "QAT DKMS: QAT_DKMS_PARAM_CHECK must be y or n" >&2
		exit 1
		;;
esac

case "$configure_flags" in
	*--enable-param-check*)
		;;
	*)
		configure_flags="$configure_flags --enable-param-check=$param_check"
		;;
esac

echo "QAT DKMS: configuring against kernel source $kernel_source_dir"
# shellcheck disable=SC2086
sh ./configure $configure_flags \
	ICP_ROOT="$PWD" \
	ICP_BUILD_OUTPUT="$build_output" \
	KERNEL_SOURCE_ROOT="$kernel_source_dir"

echo "QAT DKMS: building kernel API and QAT 1.x modules"
make \
	ICP_ROOT="$PWD" \
	ICP_BUILD_OUTPUT="$build_output" \
	ICP_PARAM_CHECK="$param_check" \
	KERNEL_SOURCE_ROOT="$kernel_source_dir" \
	qat-driver-all quickassist-all

for module in \
	intel_qat \
	qat_api \
	usdm_drv \
	qat_dh895xcc \
	qat_c62x \
	qat_dh895xccvf \
	qat_c62xvf
do
	if [ ! -r "$build_output/$module.ko" ]; then
		echo "QAT DKMS: expected module missing: $build_output/$module.ko" >&2
		exit 1
	fi
done

artifact_root="${QAT_DKMS_ARTIFACT_ROOT:-/usr/src/qat-4.28.0-00004}"
if [ "$artifact_root" != "$PWD" ]; then
	rm -rf "$artifact_root/build"
	mkdir -p "$artifact_root/build"
	cp -a "$build_output/." "$artifact_root/build/"

	for symvers in \
		quickassist/lookaside/access_layer/src/Module.symvers \
		quickassist/qat/Module.symvers
	do
		if [ -r "$PWD/$symvers" ]; then
			mkdir -p "$artifact_root/$(dirname "$symvers")"
			cp -a "$PWD/$symvers" "$artifact_root/$symvers"
		fi
	done
fi
