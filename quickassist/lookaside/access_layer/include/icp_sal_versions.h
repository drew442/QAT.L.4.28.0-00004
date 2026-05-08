/***************************************************************************
 *
 * This file is provided under a dual BSD/GPLv2 license.  When using or
 *   redistributing this file, you may do so under either license.
 * 
 *   GPL LICENSE SUMMARY
 * 
 *   Copyright(c) 2007-2024 Intel Corporation. All rights reserved.
 * 
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of version 2 of the GNU General Public License as
 *   published by the Free Software Foundation.
 * 
 *   This program is distributed in the hope that it will be useful, but
 *   WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   General Public License for more details.
 * 
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *   The full GNU General Public License is included in this distribution
 *   in the file called LICENSE.GPL.
 * 
 *   Contact Information:
 *   Intel Corporation
 * 
 *   BSD LICENSE
 * 
 *   Copyright(c) 2007-2024 Intel Corporation. All rights reserved.
 *   All rights reserved.
 * 
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 * 
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 * 
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * 
 *  version: QAT.L.4.28.0-00004
 *
 ***************************************************************************/

/**
 ***************************************************************************
 * @file icp_sal_versions.h
 *
 * @defgroup SalVersions
 *
 * @ingroup SalVersions
 *
 * API and structures definition for obtaining software and hardware versions
 *
 ***************************************************************************/

#ifndef _ICP_SAL_VERSIONS_H_
#define _ICP_SAL_VERSIONS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "icp_sal_dev_versions.h"

/* Part name and number of the accelerator device  */
#define SAL_INFO2_DRIVER_SW_VERSION_MAJ_NUMBER 4
#define SAL_INFO2_DRIVER_SW_VERSION_MIN_NUMBER 28
#define SAL_INFO2_DRIVER_SW_VERSION_PATCH_NUMBER 0

#define VERSION_STRING(ver) #ver

#define VERSION_CONCAT(ver, ...) VERSION_STRING(ver) #__VA_ARGS__

#define QAT_SW_VERSION(s1, s2, s3, s4) s1 s2 s3 s4

static const char *__attribute__((used)) qat_sw_version =
    QAT_SW_VERSION(VERSION_CONCAT(QAT_SOFTWARE_VERSION, =),
                   VERSION_CONCAT(SAL_INFO2_DRIVER_SW_VERSION_MAJ_NUMBER, .),
                   VERSION_CONCAT(SAL_INFO2_DRIVER_SW_VERSION_MIN_NUMBER, .),
                   VERSION_CONCAT(SAL_INFO2_DRIVER_SW_VERSION_PATCH_NUMBER));

#ifdef __cplusplus
} /* close the extern "C" { */
#endif

#endif
