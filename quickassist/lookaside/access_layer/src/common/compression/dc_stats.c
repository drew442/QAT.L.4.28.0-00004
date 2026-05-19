/****************************************************************************
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
 *****************************************************************************
 * @file dc_stats.c
 *
 * @ingroup Dc_DataCompression
 *
 * @description
 *      Implementation of the Data Compression stats operations.
 *
 *****************************************************************************/

/*
 *******************************************************************************
 * Include public/global header files
 *******************************************************************************
 */
#include "cpa.h"
#include "cpa_dc.h"
#include "icp_accel_devices.h"
#include "icp_adf_debug.h"
/*
 *******************************************************************************
 * Include private header files
 *******************************************************************************
 */
#include "lac_common.h"
#include "icp_accel_devices.h"
#include "sal_statistics.h"
#include "dc_session.h"
#include "dc_datapath.h"
#include "lac_mem_pools.h"
#include "sal_service_state.h"
#include "sal_types_compression.h"
#include "dc_stats.h"

#ifdef KERNEL_SPACE
#include <linux/debugfs.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/version.h>

static atomic64_t qatDcTimingGlobalStats[QAT_DC_TIMING_NUM_STATS];
static struct dentry *qatDcTimingDebugfsDir;
static struct dentry *qatDcTimingDebugfsFile;
static struct proc_dir_entry *qatDcTimingProcFile;

static const char *const qatDcTimingStatNames[QAT_DC_TIMING_NUM_STATS] = {
    [QAT_DC_TIMING_SUBMITS] = "submits",
    [QAT_DC_TIMING_COMP_SUBMITS] = "comp_submits",
    [QAT_DC_TIMING_DECOMP_SUBMITS] = "decomp_submits",
    [QAT_DC_TIMING_CALLBACKS] = "callbacks",
    [QAT_DC_TIMING_COMP_CALLBACKS] = "comp_callbacks",
    [QAT_DC_TIMING_DECOMP_CALLBACKS] = "decomp_callbacks",
    [QAT_DC_TIMING_TX_RETRIES] = "tx_retries",
    [QAT_DC_TIMING_TX_ERRORS] = "tx_errors",
    [QAT_DC_TIMING_CREATE_NS] = "create_ns",
    [QAT_DC_TIMING_TRANS_PUT_NS] = "trans_put_ns",
    [QAT_DC_TIMING_RESPONSE_WAIT_NS] = "response_wait_ns",
    [QAT_DC_TIMING_CALLBACK_PROCESS_NS] = "callback_process_ns",
    [QAT_DC_TIMING_USER_CALLBACK_NS] = "user_callback_ns",
    [QAT_DC_TIMING_TOTAL_NS] = "total_ns",
};

static int qatDcTimingDebugfsShow(struct seq_file *seq, void *unused)
{
    int i;

    (void)unused;

    for (i = 0; i < QAT_DC_TIMING_NUM_STATS; i++)
    {
        seq_printf(seq,
                   "%s %lld\n",
                   qatDcTimingStatNames[i],
                   atomic64_read(&qatDcTimingGlobalStats[i]));
    }

    return 0;
}

static int qatDcTimingDebugfsOpen(struct inode *inode, struct file *file)
{
    return single_open(file, qatDcTimingDebugfsShow, inode->i_private);
}

static const struct file_operations qatDcTimingDebugfsOps = {
    .owner = THIS_MODULE,
    .open = qatDcTimingDebugfsOpen,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = single_release,
};

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0))
static const struct proc_ops qatDcTimingProcOps = {
    .proc_open = qatDcTimingDebugfsOpen,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};
#else
static const struct file_operations qatDcTimingProcOps = {
    .owner = THIS_MODULE,
    .open = qatDcTimingDebugfsOpen,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = single_release,
};
#endif

int dcTimingDebugfsInit(void)
{
    qatDcTimingProcFile =
        proc_create("qat_dc_timing", 0444, NULL, &qatDcTimingProcOps);

    qatDcTimingDebugfsDir = debugfs_create_dir("qat_api", NULL);
    if (IS_ERR_OR_NULL(qatDcTimingDebugfsDir))
    {
        qatDcTimingDebugfsDir = NULL;
        return 0;
    }

    qatDcTimingDebugfsFile = debugfs_create_file("dc_timing",
                                                 0444,
                                                 qatDcTimingDebugfsDir,
                                                 NULL,
                                                 &qatDcTimingDebugfsOps);
    if (IS_ERR_OR_NULL(qatDcTimingDebugfsFile))
    {
        debugfs_remove_recursive(qatDcTimingDebugfsDir);
        qatDcTimingDebugfsDir = NULL;
        qatDcTimingDebugfsFile = NULL;
    }

    return 0;
}

void dcTimingDebugfsExit(void)
{
    debugfs_remove_recursive(qatDcTimingDebugfsDir);
    proc_remove(qatDcTimingProcFile);
    qatDcTimingDebugfsDir = NULL;
    qatDcTimingDebugfsFile = NULL;
    qatDcTimingProcFile = NULL;
}
#endif

CpaStatus dcStatsInit(sal_compression_service_t *pService)
{
    CpaStatus status = CPA_STATUS_SUCCESS;

    status = LAC_OS_MALLOC(&(pService->pCompStatsArr),
                           COMPRESSION_NUM_STATS * sizeof(OsalAtomic));

    if (CPA_STATUS_SUCCESS == status)
    {
        COMPRESSION_STATS_RESET(pService);
        dcTimingStatsReset(pService);
    }

    return status;
}

void dcStatsFree(sal_compression_service_t *pService)
{
    if (NULL != pService->pCompStatsArr)
    {
        LAC_OS_FREE(pService->pCompStatsArr);
    }
}

void dcStatsReset(sal_compression_service_t *pService)
{
    COMPRESSION_STATS_RESET(pService);
    dcTimingStatsReset(pService);
}

void dcTimingStatsReset(sal_compression_service_t *pService)
{
    int i;

    for (i = 0; i < QAT_DC_TIMING_NUM_STATS; i++)
    {
        osalAtomicSet(0, &pService->dcTimingStats[i]);
    }
}

void dcTimingStatInc(qat_dc_timing_stat_t statistic,
                     sal_compression_service_t *pService)
{
    osalAtomicInc(&pService->dcTimingStats[statistic]);
#ifdef KERNEL_SPACE
    atomic64_inc(&qatDcTimingGlobalStats[statistic]);
#endif
}

void dcTimingStatAdd(qat_dc_timing_stat_t statistic,
                     Cpa64U value,
                     sal_compression_service_t *pService)
{
    osalAtomicAdd((INT64)value, &pService->dcTimingStats[statistic]);
#ifdef KERNEL_SPACE
    atomic64_add((s64)value, &qatDcTimingGlobalStats[statistic]);
#endif
}

CpaStatus cpaDcGetStats(CpaInstanceHandle dcInstance, CpaDcStats *pStatistics)
{
    sal_compression_service_t *pService = NULL;
    CpaInstanceHandle insHandle = NULL;

#ifdef ICP_TRACE
    LAC_LOG2("Called with params (0x%lx, 0x%lx)\n",
             (LAC_ARCH_UINT)dcInstance,
             (LAC_ARCH_UINT)pStatistics);
#endif

    if (CPA_INSTANCE_HANDLE_SINGLE == dcInstance)
    {
        insHandle = dcGetFirstHandle();
    }
    else
    {
        insHandle = dcInstance;
    }

    pService = (sal_compression_service_t *)insHandle;

#ifdef ICP_PARAM_CHECK
    LAC_CHECK_NULL_PARAM(insHandle);
    LAC_CHECK_NULL_PARAM(pStatistics);
#endif
    SAL_RUNNING_CHECK(insHandle);

#ifdef ICP_PARAM_CHECK
    SAL_CHECK_INSTANCE_TYPE(insHandle, SAL_SERVICE_TYPE_COMPRESSION);
#endif

    /* Retrieves the statistics for compression */
    COMPRESSION_STATS_GET(pStatistics, pService);

    return CPA_STATUS_SUCCESS;
}
