/*
 * Copyright 2017 NXP
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
* @file fsl_flexspi_nor_flash.c
* @brief support to define flexspi flash config
* @version 2.0 
* @author AIIT XUOS Lab
* @date 2022-03-22
*/

#include "fsl_flexspi_nor_flash.h"

/*******************************************************************************
 * Code
 ******************************************************************************/
#if defined(__CC_ARM) || defined(__GNUC__)
    __attribute__((section(".boot_hdr.conf")))
#elif defined(__ICCARM__)
#pragma location=".boot_hdr.conf"
#endif

const flexspi_nor_config_t Qspiflash_config =
{
    .memConfig =
    {
        .tag = FLEXSPI_CFG_BLK_TAG,
        .version = FLEXSPI_CFG_BLK_VERSION,
        .readSampleClkSrc = kFlexSPIReadSampleClk_LoopbackInternally,
        .csHoldTime = 3u,
        .csSetupTime = 3u,
        .deviceModeCfgEnable = true,
        .deviceModeType = 1,//Quad Enable command
        .deviceModeSeq.seqNum = 1,
        .deviceModeSeq.seqId = 4,				
        .deviceModeArg = 0x000200,//Set QE
        .deviceType = kFlexSpiDeviceType_SerialNOR,
        .sflashPadType = kSerialFlash_4Pads,
        .serialClkFreq = kFlexSpiSerialClk_60MHz,//80MHz for Winbond, 100MHz for GD, 133MHz for ISSI
        .sflashA1Size = 16u * 1024u * 1024u,//4MBytes
        .dataValidTime = {16u, 16u},
        .lookupTable =
        {
//         //Fast Read Sequence
//         [0]  = FLEXSPI_LUT_SEQ(CMD_SDR, FLEXSPI_1PAD, 0x0B, RADDR_SDR, FLEXSPI_1PAD, 0x18),
//         [1]  = FLEXSPI_LUT_SEQ(DUMMY_SDR, FLEXSPI_1PAD, 0x08, READ_SDR, FLEXSPI_1PAD, 0x08),
//         [2]  = FLEXSPI_LUT_SEQ(JMP_ON_CS, 0, 0, 0, 0, 0),
           //Quad Input/output read sequence
           [0] = FLEXSPI_LUT_SEQ(CMD_SDR, FLEXSPI_1PAD, 0xEB, RADDR_SDR, FLEXSPI_4PAD, 0x18),
           [1] = FLEXSPI_LUT_SEQ(DUMMY_SDR, FLEXSPI_4PAD, 0x06, READ_SDR, FLEXSPI_4PAD, 0x04),
           [2] = FLEXSPI_LUT_SEQ(0, 0, 0, 0, 0, 0),
           //Read Status
           [1*4] = FLEXSPI_LUT_SEQ(CMD_SDR, FLEXSPI_1PAD, 0x05, READ_SDR, FLEXSPI_1PAD, 0x04),
           //Write Enable
           [3*4] = FLEXSPI_LUT_SEQ(CMD_SDR, FLEXSPI_1PAD, 0x06, STOP, 0, 0),
           //Write status
           [4*4] = FLEXSPI_LUT_SEQ(CMD_SDR, FLEXSPI_1PAD, 0x01, WRITE_SDR, FLEXSPI_1PAD, 0x2),
	 },
    },
    .pageSize = 256u,
    .sectorSize = 4u * 1024u,
};
