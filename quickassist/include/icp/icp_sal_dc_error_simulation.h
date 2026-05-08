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
 * @file icp_sal_dc_error_simulation.h
 *
 * @defgroup icpSalDcErrorSimulation DC Error Simulation API
 *
 * @ingroup icpSal
 *
 * @description
 *      Sal functions for DC Error Simulation.
 *      It contains the APIs under the macro ICP_DC_ERROR_SIMULATION.
 *
 ***************************************************************************/

#ifndef ICP_SAL_DC_ERROR_SIMULATION_H
#define ICP_SAL_DC_ERROR_SIMULATION_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ICP_DC_ERROR_SIMULATION

#include "cpa_dc.h"

/*
 * icp_sal_dc_simulate_error
 *
 * @description:
 *  This function injects a simulated compression error for a defined
 *  number of compression requests
 *
 * @context
 *      This function is called from the user process context
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @reentrant
 *      No
 * @threadSafe
 *      No
 *
 * @param[in] numErrors              Num DC Errors
 *                                   0 - No Error injection
 *                                   1-0xFE - Num Errors to Inject
 *                                   0xFF - Always inject Error
 * @param[in] dcError                DC Error Type
 * @retval CPA_STATUS_SUCCESS        No error
 * @retval CPA_STATUS_FAIL           Operation failed
 * @retval CPA_STATUS_UNSUPPORTED    Unsupported function
 */
CpaStatus icp_sal_dc_simulate_error(Cpa8U numErrors, Cpa8S dcError);

/*
 * icp_sal_cnv_simulate_error
 *
 * @description:
 *  This function enables the CnVError injection for the
 *  session passed in. All Compression requests sent within
 *  the session are injected with CnV errors. This error injection
 *  is for the duration of the session. Resetting the session
 *  results in setting being cleared.
 *  CnV error injection does not apply to Data Plane API.
 *
 * @note Only applies when compressAndVerify is on and
 *  compressAndVerifyAndRecover is off.
 *
 * @context
 *      This function is called from the user process context
 * @assumptions
 *      The session has been initialized via cpaDcInitSession function
 * @sideEffects
 *      None
 * @reentrant
 *      No
 * @threadSafe
 *      No
 *
 * @param[in] dcInstance             Instance Handle
 * @param[in] pSessionHandle         Session Handle
 *
 * @retval CPA_STATUS_UNSUPPORTED    Unsupported feature
 * @retval CPA_STATUS_INVALID_PARAM  Invalid parameter passed in
 * @retval CPA_STATUS_SUCCESS        No error
 * @retval CPA_STATUS_UNSUPPORTED    Unsupported function
 *
 */
CpaStatus icp_sal_cnv_simulate_error(CpaInstanceHandle dcInstance,
                                     CpaDcSessionHandle pSessionHandle);

/*
 * icp_sal_ns_cnv_simulate_error
 *
 * @description:
 *  This function enables the CnVError injection for the
 *  No-Session case. All Compression requests sent
 *  to the dcInstance that is  passed in as a parameter,
 *  are injected with CnV errors. This CnV error injection
 *  does not apply to Data Plane API.
 *  This function is for GEN4 devices.
 * @context
 *      This function is called from the user process context
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @reentrant
 *      No
 * @threadSafe
 *      No
 *
 * @param[in] dcInstance             Instance Handle
 *
 * @retval CPA_STATUS_UNSUPPORTED    Unsupported feature
 * @retval CPA_STATUS_INVALID_PARAM  Invalid parameter passed in
 * @retval CPA_STATUS_SUCCESS        No error
 * @retval CPA_STATUS_UNSUPPORTED    Unsupported function
 *
 */
CpaStatus icp_sal_ns_cnv_simulate_error(CpaInstanceHandle dcInstance);

/*
 * icp_sal_ns_cnv_reset_error
 *
 * @description:
 *  This function resets the CnVError injection for the
 *  specific dcInstance that is passed in as a parameter
 *  for the No-Session operations.
 * @context
 *      This function is called from the user process context
 * @assumptions
 *      None
 * @sideEffects
 *      None
 * @reentrant
 *      No
 * @threadSafe
 *      No
 *
 * @param[in] dcInstance             Instance Handle
 *
 * @retval CPA_STATUS_UNSUPPORTED    Unsupported feature
 * @retval CPA_STATUS_INVALID_PARAM  Invalid parameter passed in
 * @retval CPA_STATUS_SUCCESS        No error
 * @retval CPA_STATUS_UNSUPPORTED    Unsupported function
 *
 */
CpaStatus icp_sal_ns_cnv_reset_error(CpaInstanceHandle dcInstance);

#endif /* ICP_DC_ERROR_SIMULATION */

#ifdef __cplusplus
} /* close the extern "C" { */
#endif

#endif
