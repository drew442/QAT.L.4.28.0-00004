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
 * @file icp_sal_qat_dbg.h
 *
 * @ingroup SalUser
 *
 * Functions under the macro ICP_QAT_DBG.
 *
 ***************************************************************************/

#ifndef ICP_SAL_QAT_DBG_H
#define ICP_SAL_QAT_DBG_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ICP_QAT_DBG
#include "icp_adf_dbg_log.h"

typedef void *(*icp_adf_dbg_phys2virt_callback)(CpaPhysicalAddr);

typedef icp_adf_dbg_phys2virt_callback icp_sal_dbg_phys2virt_callback;
/**< @ingroup SalUser
 *      Type definition for user physical to virtual addresses translation
 *      callback used by Debuggability.
 */

/*************************************************************************
 * @ingroup SalUser
 * @description
 *      This function sets provided by user callback for translating
 *      physical to virtual addresses for QAT Debuggability purposes.
 *      The callback will be used for Data Plane API requests with SGL
 *      provided.
 *
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @reentrant
 *      Yes
 * @threadSafe
 *      Yes
 *
 * @param[in]  instanceHandle      Instance handle
 * @param[in]  user_dbg_phys2virt  Function which will be translating
 *                                 physical addresses to virtual ones
 *
 * @retval CPA_STATUS_FAIL              Failed to extract transport handle
 * @retval CPA_STATUS_SUCCESS           User callback set successfully
 * @retval CPA_STATUS_UNSUPPORTED       Unsupported algorithm/feature
 *
 *************************************************************************/
CpaStatus icp_sal_userSetDbgPhysToVirtCallback(
    CpaInstanceHandle instanceHandle,
    icp_sal_dbg_phys2virt_callback user_dbg_phys2virt);

#endif /* ICP_QAT_DBG */

#ifdef __cplusplus
} /* close the extern "C" { */
#endif

#endif
