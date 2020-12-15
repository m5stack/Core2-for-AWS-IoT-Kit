/**
 * \file
 * \brief Print hex dump utility function
 *
 * \copyright (c) 2017-2019 Microchip Technology Inc. and its subsidiaries.
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip software
 * and any derivatives exclusively with Microchip products. It is your
 * responsibility to comply with third party license terms applicable to your
 * use of third party software (including open source software) that may
 * accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
 * PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT,
 * SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE
 * OF ANY KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF
 * MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE
 * FORESEEABLE. TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL
 * LIABILITY ON ALL CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED
 * THE AMOUNT OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR
 * THIS SOFTWARE.
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "app.h"

#include "hex_dump.h"

/**
 * \brief Prints a hex dump of the information in the buffer.
 *
 * \param[in] buffer             The data buffer containing the information
 *                               to be printed
 * \param[in] length             The length, in bytes, of the data buffer
 * \param[in] display_address    Whether to display the data address information
 *                                 TRUE  - The data address information will be displayed
 *                                 FALSE - The data address information will not be displayed
 * \param[in] display_ascii      Whether to display the data ASCII information
 *                                 TRUE  - The data ASCII information will be displayed
 *                                 FALSE - The data ASCII information will not be displayed
 * \param[in] bytes_line         The number of bytes to display on each line
 */
void print_hex_dump(const void *buffer, size_t length, bool display_address, 
                    bool display_ascii, size_t bytes_line)
{
    uint8_t *data = (uint8_t*)buffer;
    size_t current_position = 0;
    char *ascii_buffer = NULL;
    int printable_character = 0;

    if ((buffer == NULL) || (length == 0))
    {
        return;
    }

    // Allocate the memory
    if (display_ascii == true)
    {
        ascii_buffer = (char*)malloc((bytes_line + 1));
        memset(ascii_buffer, ' ', (bytes_line + 1));
    }

    do
    {
        // Display the data address
        if (display_address == true)
        {
            APP_DebugPrintf("%08lX  ", (uint32_t)current_position);
        }

        for (uint8_t index = 0; index < bytes_line; index++)
        {
            // Add a space after every 8th byte of data
            if ((index > 0) && ((index % 8) == 0))
            {
                APP_DebugPrintf(" ");
            }

            if ((current_position + index) < length)
            {
                // Print the hex representation of the data
                APP_DebugPrintf("%02X ", data[(current_position + index)]);

                // Check for a printable character
                if (display_ascii == true)
                {
                    printable_character = isprint(data[(current_position + index)]);

                    if (printable_character != 0)
                    {
                        ascii_buffer[index] = data[(current_position + index)];
                    }
                    else
                    {
                        ascii_buffer[index] = '.';
                    }
                }
            }
            else
            {
                // Add spaces for the data
                APP_DebugPrintf("   ");
            }
        }

        // Print the ASCII representation of the data
        if (display_ascii == true)
        {
            ascii_buffer[bytes_line] = '\0';
            APP_DebugPrintf(" %s\r\n", ascii_buffer);
            memset(ascii_buffer, 0, bytes_line);
        }
        else
        {
            APP_DebugPrintf("\r\n");
        }

        // Increment the current position
        current_position += bytes_line;
    } while (current_position < length);

    // Free allocated memory
    if (display_ascii == true)
    {
        free(ascii_buffer);
    }
}
