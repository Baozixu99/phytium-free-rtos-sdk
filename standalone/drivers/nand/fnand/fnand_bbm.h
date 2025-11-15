/*
 * Copyright (C) 2022, Phytium Technology Co., Ltd.   All Rights Reserved.
 *
 * Licensed under the BSD 3-Clause License (the "License"); you may not use
 * this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *
 * FilePath: fnand_bbm.h
 * Date: 2022-02-10 14:53:42
 * LastEditTime: 2022-02-18 08:56:17
 * Description:  This file implements the Bad Block Management (BBM) functionality.
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   huanghe    2022/05/10    first release
 */


#ifndef FNAND_BBM_H
#define FNAND_BBM_H

#include "ftypes.h"
#include "fnand.h"


#ifdef __cplusplus
extern "C"
{
#endif

/************************** Constant Definitions *****************************/
/*
 * Block definitions for RAM based Bad Block Table (BBT)
 */
#define FNAND_BLOCK_GOOD              0x0 /* Block is good */
#define FNAND_BLOCK_BAD               0x1 /* Block is bad */
#define FNAND_BLOCK_RESERVED          0x2 /* Reserved block */
#define FNAND_BLOCK_FACTORY_BAD       0x3 /* Factory marked bad block */

/*
 * Block definitions for FLASH based Bad Block Table (BBT)
 */
#define FNAND_FLASH_BLOCK_GOOD        0x3 /* Block is good */
#define FNAND_FLASH_BLOCK_BAD         0x2 /* Block is bad */
#define FNAND_FLASH_BLOCK_RESERVED    0x1 /* Reserved block */
#define FNAND_FLASH_BLOCK_FACTORY_BAD 0x0 /* Factory marked bad block */

#define FNAND_BBT_SCAN_2ND_PAGE \
    0x00000001 /* Scan the \
                             second page               \
                             for bad block             \
                             information               \
*/
#define FNAND_BBT_DESC_PAGE_OFFSET \
    0 /* Page offset of Bad \
Block Table Desc */
#define FNAND_BBT_DESC_SIG_OFFSET \
    8 /* Bad Block Table \
signature offset */
#define FNAND_BBT_DESC_VER_OFFSET \
    12 /* Bad block Table \
version offset */
#define FNAND_BBT_DESC_SIG_LEN \
    4 /* Bad block Table \
signature length */
#define FNAND_BBT_DESC_MAX_BLOCKS \
    4 /* Bad block Table \
max blocks */

#define FNAND_BBT_BLOCK_SHIFT \
    2 /* Block shift value \
for a block in BBT */
#define FNAND_BBT_ENTRY_NUM_BLOCKS \
    4 /* Num of blocks in \
one BBT entry */
#define FNAND_BB_PATTERN_OFFSET_SMALL_PAGE \
    5 /* Bad block pattern \
offset in a page */
#define FNAND_BB_PATTERN_LENGTH_SMALL_PAGE \
    1 /* Bad block pattern \
length */
#define FNAND_BB_PATTERN_OFFSET_LARGE_PAGE \
    0 /* Bad block pattern \
                           offset in a large                      \
page */
#define FNAND_BB_PATTERN_LENGTH_LARGE_PAGE \
    2 /* Bad block pattern \
length */
#define FNAND_BB_PATTERN \
    0xFF                           /* Bad block pattern \
                               to search in a page                \
*/
#define FNAND_BLOCK_TYPE_MASK 0x03 /* Block type mask */
#define FNAND_BLOCK_SHIFT_MASK \
    0x06 /* Block shift mask \
                               for a Bad Block Table             \
entry byte */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/****************************************************************************/

#define FNAND_BBTBLOCKSHIFT(block) ((block * 2) & FNAND_BLOCK_SHIFT_MASK)


#ifdef __cplusplus
}
#endif

#endif
