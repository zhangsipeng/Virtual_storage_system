/**
********************************************************************************
\file   console-xiuos.c

\brief  Console input/output implementation for XiUOS

This file contains the console input/output implementation for XiUOS.

\ingroup module_console
*******************************************************************************/

/*------------------------------------------------------------------------------
Copyright (c) 2016, B&R Industrial Automation GmbH
Copyright (c) 2020, AIIT XUOS Lab
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holders nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
------------------------------------------------------------------------------*/

//------------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------------
#include <dev_serial.h>

//============================================================================//
//            P U B L I C   F U N C T I O N S                                 //
//============================================================================//

//------------------------------------------------------------------------------
/**
\brief  Get character from console

This function reads a character from the console input. It uses the
termios library.

\return The function returns the read character.

\ingroup module_console
*/
//------------------------------------------------------------------------------
int console_getch(void)
{
    char ch, len;

    extern char userShellRead(char *data);

    do {
        len = userShellRead(&ch);
    } while (!len);

    return ch;
}

//------------------------------------------------------------------------------
/**
\brief  Detecting a keystroke

The function checks the console for a keystroke.

\return The function returns 0 if no key has been pressed or 1 if a key has
        been pressed.

\ingroup module_console
*/
//------------------------------------------------------------------------------
int console_kbhit(void)
{
    extern HardwareDevType console;
    struct SerialHardwareDevice *serial_dev = (struct SerialHardwareDevice *)console;
    int ret;

    ret = KSemaphoreObtain(serial_dev->haldev.dev_sem, 0);

    if (ret == EOK) {
        KSemaphoreAbandon(serial_dev->haldev.dev_sem);
        return 1;
    } else {
        return 0;
    }
}
