# QAT 4.28 DKMS packaging

This branch adds initial DKMS packaging for the QAT 4.28 CE driver used by the OpenZFS QAT fork.

## Scope

- Target package: `qat/4.28.0-00004`.
- Default DKMS source path: `/usr/src/qat-4.28.0-00004`.
- Default configure flags: `--enable-kapi --enable-qat-lkcf`.
- Installed kernel modules are limited to the common driver, QAT kernel API support, and dh895xcc/C62x physical and VF modules.
- QAT 2.0+/Gen4 modules built by the upstream package are intentionally not installed by this DKMS configuration.

## Install

Run the installer from the package root or by repository-relative path:

```sh
sudo ./scripts/install-dkms.sh
```

To replace an existing local DKMS registration for the same package/version:

```sh
sudo QAT_DKMS_REPLACE=1 ./scripts/install-dkms.sh
```

Optional environment overrides:

- `QAT_DKMS_SOURCE_DIR`: source directory copied into DKMS. Defaults to `/usr/src/qat-4.28.0-00004`.
- `QAT_DKMS_CONFIGURE_FLAGS`: configure flags used during DKMS builds. Defaults to `--enable-kapi --enable-qat-lkcf`.
- `QAT_DKMS_BUILD_OUTPUT`: build output directory. Defaults to `<source>/build`.
- `QAT_DKMS_REQUIRE_RUNTIME_TOOLS`: set to `0` to skip runtime checks for `adf_ctl` and active `/etc` QAT config files. Defaults to `1`.
- `QAT_DKMS_PREFLIGHT_ONLY`: set to `1` to run installer preflight checks and exit before changing DKMS state. Defaults to `0`.

The installer preflights build tools, kernel headers, QAT service source files, `adf_ctl` source files, and dh895xcc/C62x config templates before registering the DKMS package. By default it also checks that the host has `adf_ctl` in `PATH` and at least one active `/etc/dh895xcc_dev*.conf` or `/etc/c6xx_dev*.conf` runtime config file.

To validate installer prerequisites without rebuilding:

```sh
sudo QAT_DKMS_PREFLIGHT_ONLY=1 ./scripts/install-dkms.sh
```

The installer intentionally excludes generated top-level configure output, generated build output, and the nested `openzfs` helper submodule from the DKMS source copy. Source `Makefile` files under `quickassist/` are retained because the QAT driver build requires them.

The installer uses `dkms install --force` so the DKMS-built `qat_api.ko` and `usdm_drv.ko` replace same-version modules that may already exist under the kernel module tree from an earlier manual QAT install.

During DKMS build, `scripts/dkms-build.sh` publishes the generated `build/` directory and QAT `Module.symvers` files back into `/usr/src/qat-4.28.0-00004`. OpenZFS uses that source path as `ICP_ROOT`, and its configure checks require those generated artifacts.

If older manual QAT modules under `/lib/modules/$(uname -r)/updates/drivers/crypto/qat` would shadow `/updates/dkms`, the installer archives them under `/var/backups/qat-dkms-shadowed-*`, runs `depmod`, and verifies every managed module resolves to `/updates/dkms`. It also moves any legacy `qat-dkms-shadowed-*` archive left under `/lib/modules/$(uname -r)/updates` out of the module search tree.

## OpenZFS lock-step build

The OpenZFS fork expects ZFS DKMS builds to use the same QAT source tree by default:

```sh
ICP_ROOT=/usr/src/qat-4.28.0-00004
```

Install or rebuild the QAT DKMS package before rebuilding ZFS DKMS so `${ICP_ROOT}/build`, `${ICP_ROOT}/quickassist/include`, and the QAT `Module.symvers` files match the kernel being built.

## DC timing instrumentation

This branch adds QAT DC timing counters for the traditional compression API
path used by OpenZFS. The counters are intended for OpenZFS QAT latency
profiling and report request construction time, transport enqueue time,
response-wait time, callback processing time, user callback time, total request
time, submit counts, callback counts, and TX retry/error counts.

The global aggregate read surfaces are:

```sh
cat /proc/qat_dc_timing
cat /sys/kernel/debug/qat_api/dc_timing
```

The output is a two-column `name value` table. Timing values are cumulative
nanoseconds since `qat_api.ko` loaded; benchmark tooling should read before and
after a workload and calculate deltas. The same counters are also included in
the existing per-compression-instance debug output when that legacy QAT debug
plumbing is available.

On initramfs-booted hosts, update initramfs after replacing the QAT DKMS module
or the next boot may load the stale `qat_api.ko` from the old initramfs:

```sh
sudo update-initramfs -u -k "$(uname -r)"
```
