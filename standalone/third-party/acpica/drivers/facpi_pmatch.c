/*
 * Copyright : (C) 2025 Phytium Information Technology, Inc.
 * All Rights Reserved.
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
 * Modified from NetBSD sys/lib/libkern/pmatch.c with NETBSD LICENSE
 *
 * FilePath: facpi_pmatch.c
 * Date: 2025-04-10 16:20:52
 * LastEditTime: 2025-04-10 16:20:52
 * Description:  This file is for acpi pmatch functions
 *
 * Modify History:
 *  Ver   Who        Date         Changes
 * ----- ------     --------    --------------------------------------
 * 1.0   zhugengyu  2025/7/30    init commit
 */
/***************************** Include Files *********************************/
#include "fdrivers_port.h"
#include "facpi.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define FACPI_DEBUG_TAG "FACPI"
#define FACPI_ERROR(format, ...) \
    FT_DEBUG_PRINT_E(FACPI_DEBUG_TAG, format, ##__VA_ARGS__)
#define FACPI_WARN(format, ...) FT_DEBUG_PRINT_W(FACPI_DEBUG_TAG, format, ##__VA_ARGS__)
#define FACPI_INFO(format, ...) FT_DEBUG_PRINT_I(FACPI_DEBUG_TAG, format, ##__VA_ARGS__)
#define FACPI_DEBUG(format, ...) \
    FT_DEBUG_PRINT_D(FACPI_DEBUG_TAG, format, ##__VA_ARGS__)
#define FACPI_VERBOS(format, ...) \
    FT_DEBUG_PRINT_V(FACPI_DEBUG_TAG, format, ##__VA_ARGS__)
/************************** Function Prototypes ******************************/

/*****************************************************************************/

/*
 * pmatch():
 *	Return 2 on exact match.
 *	Return 1 on substring match.
 *	Return 0 on no match.
 *	Return -1 on error.
 * *estr will point to the end of thelongest exact or substring match.
 */
int pmatch(const char *string, const char *pattern, const char **estr)
{
    u_char stringc, patternc, rangec;
    int match, negate_range;
    const char *oestr, *pestr, *testr;

    if (estr == NULL)
    {
        estr = &testr;
    }

    for (;; ++string)
    {
        stringc = *string;
        switch (patternc = *pattern++)
        {
            case 0:
                *estr = string;
                return stringc == '\0' ? 2 : 1;
            case '?':
                if (stringc == '\0')
                {
                    return 0;
                }
                *estr = string;
                break;
            case '*':
                if (!*pattern)
                {
                    while (*string)
                    {
                        string++;
                    }
                    *estr = string;
                    return 2;
                }
                oestr = *estr;
                pestr = NULL;

                do
                {
                    switch (pmatch(string, pattern, estr))
                    {
                        case -1:
                            return -1;
                        case 0:
                            break;
                        case 1:
                            pestr = *estr;
                            break;
                        case 2:
                            return 2;
                        default:
                            return -1;
                    }
                    *estr = string;
                } while (*string++);

                if (pestr)
                {
                    *estr = pestr;
                    return 1;
                }
                else
                {
                    *estr = oestr;
                    return 0;
                }

            case '[':
                match = 0;
                if ((negate_range = (*pattern == '^')) != 0)
                {
                    pattern++;
                }
                while ((rangec = *pattern++) != '\0')
                {
                    if (rangec == ']')
                    {
                        break;
                    }
                    if (match)
                    {
                        continue;
                    }
                    if (rangec == '-' && *(pattern - 2) != '[' && *pattern != ']')
                    {
                        match = stringc <= (u_char)*pattern && (u_char) * (pattern - 2) <= stringc;
                        pattern++;
                    }
                    else
                    {
                        match = (stringc == rangec);
                    }
                }
                if (rangec == 0)
                {
                    return -1;
                }
                if (match == negate_range)
                {
                    return 0;
                }
                *estr = string;
                break;
            default:
                if (patternc != stringc)
                {
                    return 0;
                }
                *estr = string;
                break;
        }
    }
}
