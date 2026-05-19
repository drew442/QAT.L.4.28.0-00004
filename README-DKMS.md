# QAT 4.28 DKMS packaging

This branch adds initial DKMS packaging for the QAT 4.28 CE driver used by the OpenZFS QAT fork.

## Scope

- Target package: `qat/4.28.0-00004`.
- Default DKMS source path: `/usr/src/qat-4.28.0-00004`.
- Default configure flags: `--enable-kapi --enable-qat-lkcf`.
- Installed kernel modules are limited to the common driver, QAT kernel API support, and dh895xcc/C62x physical and VF modules.
- QAT 2.0+/Gen4 modules built by the upstream package are intentionally not installed by this DKMS configuration.

## Install

Run from the package root:

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

The installer intentionally excludes generated top-level configure output, generated build output, and the nested `openzfs` helper submodule from the DKMS source copy. Source `Makefile` files under `quickassist/` are retained because the QAT driver build requires them.

## OpenZFS lock-step build

The OpenZFS fork expects ZFS DKMS builds to use the same QAT source tree by default:

```sh
ICP_ROOT=/usr/src/qat-4.28.0-00004
```

Install or rebuild the QAT DKMS package before rebuilding ZFS DKMS so `${ICP_ROOT}/build`, `${ICP_ROOT}/quickassist/include`, and the QAT `Module.symvers` files match the kernel being built.
