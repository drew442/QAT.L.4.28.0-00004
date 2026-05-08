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
 * @file dc_sw_fallback.c
 *
 * @defgroup dcDecompress_SwFallback DC Data Decompression
 *
 * @ingroup dcDecompress_SwFallback
 *
 * @description
 * 	Implementation of the software fallback feature.
 * 	Used when the hardware decompression returns an empty dynamic block.
 *
 *****************************************************************************/

/*
*******************************************************************************
* Include public/global header files
*******************************************************************************
*/
#include "cpa.h"
#include "cpa_dc_dp.h"

/*
*******************************************************************************
* Include private header files
*******************************************************************************
*/
#include "dc_datapath.h"
#include "lac_log.h"

#ifndef KERNEL_SPACE
#include <zlib.h>
#else
#include <linux/crc32.h>
#include <linux/zutil.h>
#include <linux/zlib.h>
#endif

#define CRC32_XOR_VALUE (0xffffffff)
#define MAX_ERR_CODE 11
#define MAX_STREAM_MSG_LEN 50

typedef struct option_s
{
    char e_string[MAX_STREAM_MSG_LEN];
    CpaDcReqStatus e_type;
} option_t;

option_t inflate_cpa_error_map[MAX_ERR_CODE] = {
    { "invalid block type", CPA_DC_INVALID_BLOCK_TYPE },
    { "invalid stored block lengths", CPA_DC_BAD_STORED_BLOCK_LEN },
    { "too many length or distance symbols", CPA_DC_TOO_MANY_CODES },
    { "invalid code lengths set", CPA_DC_INCOMPLETE_CODE_LENS },
    { "invalid bit length repeat", CPA_DC_REPEATED_LENS },
    { "invalid literal/lengths set", CPA_DC_BAD_LITLEN_CODES },
    { "invalid distances set", CPA_DC_BAD_DIST_CODES },
    { "invalid literal/length code", CPA_DC_INVALID_CODE },
    { "invalid distance code", CPA_DC_BAD_DIST_CODES },
    { "invalid distance too far back", CPA_DC_INVALID_DIST },
    { "incorrect data check", CPA_DC_CRC_INTEG_ERR },
};

static Cpa8S inflate_getCpaErrorCode(option_t *arr, char *searchStr)
{
    Cpa8S i = 0;
    Cpa8S ret = -1;
    for (i = 0; i < MAX_ERR_CODE; i++)
    {
        if (strstr(searchStr, arr[i].e_string))
        {
            ret = arr[i].e_type;
            break;
        }
    }
    return ret;
}

STATIC void inflate_computeChecksum(dc_session_desc_t *pSessionDesc,
                                    Cpa32U *checksumVal,
                                    Cpa8U *buffer,
                                    Cpa32U bufferLen)
{
#ifndef KERNEL_SPACE
    if (CPA_DC_CRC32 == pSessionDesc->checksumType)
    {
        *checksumVal = crc32(*checksumVal, buffer, bufferLen);
    }
    else if (CPA_DC_ADLER32 == pSessionDesc->checksumType)
    {
        *checksumVal = adler32(*checksumVal, buffer, bufferLen);
    }
#else
    if (CPA_DC_CRC32 == pSessionDesc->checksumType)
    {
        *checksumVal =
            crc32(*checksumVal ^ CRC32_XOR_VALUE, buffer, bufferLen) ^
            CRC32_XOR_VALUE;
    }
    else if (CPA_DC_ADLER32 == pSessionDesc->checksumType)
    {
        *checksumVal = zlib_adler32(*checksumVal, buffer, bufferLen);
    }
#endif
}

STATIC CpaStatus inflate_init(z_stream *stream)
{
    int ret = 0;

#ifndef KERNEL_SPACE
    ret = inflateInit2(stream, -MAX_WBITS);
    if (ret != Z_OK)
    {
        LAC_LOG1("Error in inflateInit2, ret = %d\n", ret);
        return CPA_STATUS_FAIL;
    }

    ret = inflateReset(stream);
    if (ret != Z_OK)
    {
        LAC_LOG1("Error in inflateReset, ret = %d\n", ret);
        return CPA_STATUS_FAIL;
    }
#else
    stream->workspace = kmalloc(zlib_inflate_workspacesize(), GFP_KERNEL);
    if (NULL == stream->workspace)
    {
        LAC_LOG("Could not allocate zlib workspace memory\n");
        return CPA_STATUS_FAIL;
    }
    memset(stream->workspace, 0, zlib_inflate_workspacesize());
    ret = zlib_inflateInit2(stream, -MAX_WBITS);
    if (ret != Z_OK)
    {
        LAC_LOG1("Error in zlib_inflateInit2, ret = %d\n", ret);
        return CPA_STATUS_FAIL;
    }

    ret = zlib_inflateReset(stream);
    if (ret != Z_OK)
    {
        LAC_LOG1("Error in zlib_inflateReset, ret = %d\n", ret);
        return CPA_STATUS_FAIL;
    }
#endif
    return CPA_STATUS_SUCCESS;
}

STATIC void inflate_destroy(struct z_stream_s *stream)
{
#ifndef KERNEL_SPACE
    inflateEnd(stream);
#else
    zlib_inflateEnd(stream);
    if (stream->workspace != NULL)
    {
        kfree(stream->workspace);
    }
#endif
}

CpaStatus dcDecompress_SwFallback(dc_session_desc_t *pSessionDesc,
                                  Cpa64U *pReqData,
                                  icp_qat_fw_comp_resp_t *pCompRespMsg)
{
    CpaStatus ret = CPA_STATUS_FAIL;
    Cpa32U checksumVal = 0;
    Cpa64U pSrcBufCount = 0;
    Cpa64U pDstBufCount = 0;
    Cpa64U i = 0;
    Cpa32U srcBufferMemSize = 0;
    Cpa32U dstBufferMemSize = 0;
    CpaBufferList *pSrcBuff = NULL;
    CpaBufferList *pDestBuff = NULL;
    CpaPhysBufferList *pSrcDpBuff = NULL;
    CpaPhysBufferList *pDstDpBuff = NULL;
    dc_compression_cookie_t *pCookie = NULL;
    CpaDcDpOpData *pOpData = NULL;
    struct z_stream_s strm = { 0 };

    if (pSessionDesc->isDcDp == CPA_TRUE)
    {
        pOpData = (CpaDcDpOpData *)pReqData;
        pSrcDpBuff = LAC_OS_PHYS_TO_VIRT_INTERNAL(pOpData->srcBuffer);
        pDstDpBuff = LAC_OS_PHYS_TO_VIRT_INTERNAL(pOpData->destBuffer);

        LAC_OS_MALLOC(&pSrcBuff, sizeof(CpaBufferList));
        LAC_OS_MALLOC(&pDestBuff, sizeof(CpaBufferList));

        srcBufferMemSize = pSrcDpBuff->numBuffers * sizeof(CpaFlatBuffer);
        dstBufferMemSize = pDstDpBuff->numBuffers * sizeof(CpaFlatBuffer);

        LAC_OS_MALLOC(&pSrcBuff->pBuffers, srcBufferMemSize);
        LAC_OS_MALLOC(&pDestBuff->pBuffers, dstBufferMemSize);

        pSrcBuff->numBuffers = pSrcDpBuff->numBuffers;
        pDestBuff->numBuffers = pDstDpBuff->numBuffers;

        for (i = 0; i < pSrcBuff->numBuffers; i++)
        {
            pSrcBuff->pBuffers[i].pData = LAC_OS_PHYS_TO_VIRT_INTERNAL(
                pSrcDpBuff->flatBuffers[i].bufferPhysAddr);
            pSrcBuff->pBuffers[i].dataLenInBytes =
                pSrcDpBuff->flatBuffers[i].dataLenInBytes;
        }

        for (i = 0; i < pDestBuff->numBuffers; i++)
        {
            pDestBuff->pBuffers[i].pData = LAC_OS_PHYS_TO_VIRT_INTERNAL(
                pDstDpBuff->flatBuffers[i].bufferPhysAddr);
            pDestBuff->pBuffers[i].dataLenInBytes =
                pDstDpBuff->flatBuffers[i].dataLenInBytes;
        }
    }
    else
    {
        pCookie = (dc_compression_cookie_t *)pReqData;
        pSrcBuff = pCookie->pUserSrcBuff;
        pDestBuff = pCookie->pUserDestBuff;
    }

    strm.next_in = pSrcBuff->pBuffers[0].pData;
    strm.avail_in = pSrcBuff->pBuffers[0].dataLenInBytes;
    strm.next_out = pDestBuff->pBuffers[0].pData;
    strm.avail_out = pDestBuff->pBuffers[0].dataLenInBytes;

    if (CPA_DC_CRC32 == pSessionDesc->checksumType)
        checksumVal = 0;
    else if (CPA_DC_ADLER32 == pSessionDesc->checksumType)
        checksumVal = 1;

    ret = inflate_init(&strm);
    if (ret != CPA_STATUS_SUCCESS)
    {
        LAC_LOG("Inflate init failed\n");
        goto exit;
    }

    while (pSrcBufCount < pSrcBuff->numBuffers)
    {
#ifndef KERNEL_SPACE
        ret = inflate(&strm, Z_SYNC_FLUSH);
#else
        ret = zlib_inflate(&strm, Z_SYNC_FLUSH);
#endif
        if (ret < Z_OK)
        {
            LAC_LOG1("Inflate failed with status %d", ret);
            LAC_OSAL_LOG_STRING(OSAL_LOG_LVL_ERROR,
                                OSAL_LOG_DEV_STDERR,
                                "Error message %s\n",
                                strm.msg);
            if (Z_DATA_ERROR == ret)
            {
                ret = inflate_getCpaErrorCode(inflate_cpa_error_map, strm.msg);
            }
            else
            {
                ret = CPA_STATUS_FAIL;
            }
            goto exit;
        }
        else if (ret == Z_STREAM_END)
        {
            inflate_computeChecksum(
                pSessionDesc,
                &checksumVal,
                pDestBuff->pBuffers[pDstBufCount].pData,
                pDestBuff->pBuffers[pDstBufCount].dataLenInBytes -
                    strm.avail_out);
            pCompRespMsg->comn_resp.comn_status =
                ICP_QAT_FW_COMN_STATUS_CMP_END_OF_LAST_BLK_FLAG_SET;
            ret = CPA_STATUS_SUCCESS;
            goto exit;
        }

        if (strm.avail_in == 0)
        {
            pSrcBufCount++;
            if (pSrcBufCount >= pSrcBuff->numBuffers)
            {
                inflate_computeChecksum(
                    pSessionDesc,
                    &checksumVal,
                    pDestBuff->pBuffers[pDstBufCount].pData,
                    pDestBuff->pBuffers[pDstBufCount].dataLenInBytes -
                        strm.avail_out);
                ret = CPA_STATUS_SUCCESS;
                goto exit;
            }
            strm.next_in = pSrcBuff->pBuffers[pSrcBufCount].pData;
            strm.avail_in = pSrcBuff->pBuffers[pSrcBufCount].dataLenInBytes;
        }
        if (strm.avail_out == 0)
        {
            inflate_computeChecksum(
                pSessionDesc,
                &checksumVal,
                pDestBuff->pBuffers[pDstBufCount].pData,
                pDestBuff->pBuffers[pDstBufCount].dataLenInBytes);
            pDstBufCount++;
            if (pDstBufCount >= pDestBuff->numBuffers)
            {
                if (pSrcBufCount < pSrcBuff->numBuffers)
                {
                    LAC_LOG("not enough output buffer!\n");
                    ret = CPA_STATUS_FAIL;
                    goto exit;
                }
                else
                {
                    ret = CPA_STATUS_SUCCESS;
                    goto exit;
                }
            }
            strm.next_out = pDestBuff->pBuffers[pDstBufCount].pData;
            strm.avail_out = pDestBuff->pBuffers[pDstBufCount].dataLenInBytes;
        }
    }

exit:
    pCompRespMsg->comp_resp_pars.input_byte_counter = strm.total_in;
    pCompRespMsg->comp_resp_pars.output_byte_counter = strm.total_out;
    if (ret == CPA_STATUS_SUCCESS)
    {
        if (CPA_DC_CRC32 == pSessionDesc->checksumType)
        {
            pCompRespMsg->comp_resp_pars.crc.legacy.curr_crc32 = checksumVal;
        }
        else if (CPA_DC_ADLER32 == pSessionDesc->checksumType)
        {
            pCompRespMsg->comp_resp_pars.crc.legacy.curr_adler_32 = checksumVal;
        }
    }

    if (pSessionDesc->isDcDp == CPA_TRUE)
    {
        LAC_OS_FREE(pSrcBuff->pBuffers);
        LAC_OS_FREE(pDestBuff->pBuffers);
        LAC_OS_FREE(pSrcBuff);
        LAC_OS_FREE(pDestBuff);
    }

    inflate_destroy(&strm);

    return ret;
}
