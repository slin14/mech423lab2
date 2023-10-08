/* --COPYRIGHT--,BSD_EX
 * Copyright (c) 2012, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *******************************************************************************
 * 
 *                       MSP430 CODE EXAMPLE DISCLAIMER
 *
 * MSP430 code examples are self-contained low-level programs that typically
 * demonstrate a single peripheral function or device feature in a highly
 * concise manner. For this the code may rely on the device's power-on default
 * register values and settings such as the clock configuration and care must
 * be taken when combining code from several examples to avoid potential side
 * effects. Also see www.ti.com/grace for a GUI- and www.ti.com/msp430ware
 * for an API functional library-approach to peripheral configuration.
 *
 * --/COPYRIGHT--*/
//******************************************************************************
//   MSP430FR57xx Demo - Configure MCLK for 24MHz operation
//
//   Description: Configure ACLK = SMCLK = MCLK = 24MHz
//   IMPORTANT NOTE: While the FR5739 is capable of operation using a 24MHz 
//   MCLK; the throughput of the device is dependednt on accesses to FRAM. 
//   The maximum speed for accessing FRAM is limited to 8MHz*. 
//   Wait states are inserted automatically when the FRAM is accessed at 
//   speeds higher than 8MHz to prevent timing violations.
//   Refer to the FRAM Clock System chapter of the User's Guide.
//   * Refer device D/s for typical numbers
//
//           MSP430FR57x
//         ---------------
//     /|\|               |
//      | |               |
//      --|RST            |
//        |               |
//        |               |  
//        |          P2.0 |---> ACLK = MCLk = 24MHz
//
//   Priya Thanigai
//   Texas Instruments Inc.
//   August 2010
//   Built with IAR Embedded Workbench Version: 5.10 & Code Composer Studio V4.0
//******************************************************************************

#include <msp430.h>

int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT  

  CSCTL0_H = 0xA5;
  CSCTL1 |= DCORSEL + DCOFSEL0 + DCOFSEL1;   // Set max. DCO setting
  CSCTL2 = SELA_3 + SELS_3 + SELM_3;        // set ACLK = MCLK = DCO
  CSCTL3 = DIVA_0 + DIVS_0 + DIVM_0;        // set all dividers to 0
    

  P2OUT = 0;                                // output ACLK
  P2DIR |= BIT0;
  P2SEL1 |= BIT0; 
  P2SEL0 |= BIT0;
  
  while(1);
  
}