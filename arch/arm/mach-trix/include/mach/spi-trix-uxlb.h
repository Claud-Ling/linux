/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *  Author: Tony He, 2016.
 */

#ifndef __ASM_ARCH_TRIX_MACH_SPI_TRIX_UXLB_H__
#define __ASM_ARCH_TRIX_MACH_SPI_TRIX_UXLB_H__

/**UXLB SPIs**/
#define TRIHIDTV_AVMIPS_RQST_IRQ                      (TRIHIDTV_EIC_IRQ_BASE + 0)
#define TRIHIDTV_AVMIPS_RSPNS_IRQ                     (TRIHIDTV_EIC_IRQ_BASE + 1)
#define TRIHIDTV_STBY_RQST_IRQ                        (TRIHIDTV_EIC_IRQ_BASE + 2)
#define TRIHIDTV_STBY_RSPNS_IRQ                       (TRIHIDTV_EIC_IRQ_BASE + 3)

#define TRIHIDTV_PLF_UART_0_INTERRUPT                 (TRIHIDTV_EIC_IRQ_BASE + 4)
#define TRIHIDTV_PLF_UART_1_INTERRUPT                 (TRIHIDTV_EIC_IRQ_BASE + 5)
#define TRIHIDTV_PLF_UART_2_INTERRUPT                 (TRIHIDTV_EIC_IRQ_BASE + 6)

#define TRIHIDTV_WATCHDOG_IRQ                         (TRIHIDTV_EIC_IRQ_BASE + 7)
#define TRIHIDTV_WATCHDOG_AV_IRQ                      (TRIHIDTV_EIC_IRQ_BASE + 8)
#define TRIHIDTV_TIMER_0_INTERRUPT                    (TRIHIDTV_EIC_IRQ_BASE + 9)
#define TRIHIDTV_TIMER_1_INTERRUPT                    (TRIHIDTV_EIC_IRQ_BASE + 10)

#define TRIHIDTV_MI2C0_IRQ                            (TRIHIDTV_EIC_IRQ_BASE + 11)
#define TRIHIDTV_MI2C1_IRQ                            (TRIHIDTV_EIC_IRQ_BASE + 12)
#define TRIHIDTV_MI2C2_IRQ                            (TRIHIDTV_EIC_IRQ_BASE + 13)

#define TRIHIDTV_CPUIF_ADDR_GAP_IRQ                   (TRIHIDTV_EIC_IRQ_BASE + 14)
#define TRIHIDTV_SECTION_FILTER_IRQ                   (TRIHIDTV_EIC_IRQ_BASE + 15)

#define TRIHIDTV_CVBSOUT_IRQ                          (TRIHIDTV_EIC_IRQ_BASE + 16)
#define TRIHIDTV_DEMOD_IRQ                            (TRIHIDTV_EIC_IRQ_BASE + 17)

#define TRIHIDTV_GE_IRQ                               (TRIHIDTV_EIC_IRQ_BASE + 18)
#define TRIHIDTV_GE3D_IRQ                             (TRIHIDTV_EIC_IRQ_BASE + 19)
#define TRIHIDTV_CA_IRQ                               (TRIHIDTV_EIC_IRQ_BASE + 20)

#define TRIHIDTV_FLASH_DMA_IRQ                        (TRIHIDTV_EIC_IRQ_BASE + 21)
#define TRIHIDTV_FLASH_IRQ                            (TRIHIDTV_EIC_IRQ_BASE + 22)
#define TRIHIDTV_FLASH_INTERRUPT                      TRIHIDTV_FLASH_IRQ/*alias*/

#define TRIHIDTV_USB_IRQ                              (TRIHIDTV_EIC_IRQ_BASE + 23)
#define TRIHIDTV_USB2_IRQ                             (TRIHIDTV_EIC_IRQ_BASE + 24)
#define TRIHIDTV_USB_0_INTERRUPT                      TRIHIDTV_USB_IRQ	/*alias*/
#define TRIHIDTV_USB_1_INTERRUPT                      TRIHIDTV_USB2_IRQ	/*alias*/


#define TRIHIDTV_ETHERNET_IRQ                         (TRIHIDTV_EIC_IRQ_BASE + 25)
#define TRIHIDTV_UMAC_IRQ                             (TRIHIDTV_EIC_IRQ_BASE + 26)

#define TRIHIDTV_HP_DETECT_IRQ                        (TRIHIDTV_EIC_IRQ_BASE + 27)

#define TRIHIDTV_SDIO_1_IRQ                           (TRIHIDTV_EIC_IRQ_BASE + 28)
#define TRIHIDTV_GPIO_IRQ                             (TRIHIDTV_EIC_IRQ_BASE + 29)
#define TRIHIDTV_SDIO_0_IRQ                           (TRIHIDTV_EIC_IRQ_BASE + 30)
#define TRIHIDTV_SDIO_0_INTERRUPT                     TRIHIDTV_SDIO_0_IRQ	/*alias*/
#define TRIHIDTV_SDIO_1_INTERRUPT                     TRIHIDTV_SDIO_1_IRQ	/*alias*/

#define TRIHIDTV_AES_IRQ                              (TRIHIDTV_EIC_IRQ_BASE + 31)
#define TRIHIDTV_SMARTCARD_IRQ                        (TRIHIDTV_EIC_IRQ_BASE + 32)

#define TRIHIDTV_HSUART_IRQ                           (TRIHIDTV_EIC_IRQ_BASE + 33)
#define TRIHIDTV_HDMI_IRQ                             (TRIHIDTV_EIC_IRQ_BASE + 34)
#define TRIHIDTV_LVDS_IRQ                             (TRIHIDTV_EIC_IRQ_BASE + 35)

#define TRIHIDTV_PVR_DUMP_IRQ                         (TRIHIDTV_EIC_IRQ_BASE + 36)
#define TRIHIDTV_PVRSI_IRQ                            (TRIHIDTV_EIC_IRQ_BASE + 37)

#define TRIHIDTV_SHA_IRQ                              (TRIHIDTV_EIC_IRQ_BASE + 38)

#define TRIHIDTV_VIDEO_IRQ                            (TRIHIDTV_EIC_IRQ_BASE + 39)
#define TRIHIDTV_VIDEO_AUTOPHASE_IRQ                  (TRIHIDTV_EIC_IRQ_BASE + 40)
#define TRIHIDTV_VIDEO_CRTC_VS_IRQ                    (TRIHIDTV_EIC_IRQ_BASE + 41)
#define TRIHIDTV_VIDEO_VS_CAPPIP_IRQ                  (TRIHIDTV_EIC_IRQ_BASE + 42)
#define TRIHIDTV_VIDEO_VS_CAPMP_IRQ                   (TRIHIDTV_EIC_IRQ_BASE + 43)
#define TRIHIDTV_VIDEO_VDE_DFI_IRQ                    (TRIHIDTV_EIC_IRQ_BASE + 44)
#define TRIHIDTV_VIDEO_VS_DFI_IRQ                     (TRIHIDTV_EIC_IRQ_BASE + 45)
#define TRIHIDTV_VIDEO_VDE_MC_IRQ                     (TRIHIDTV_EIC_IRQ_BASE + 46)
#define TRIHIDTV_VIDEO_VS_MC_IRQ                      (TRIHIDTV_EIC_IRQ_BASE + 47)
#define TRIHIDTV_VIDEO_CAP_MODE_CHG_PIP_IRQ           (TRIHIDTV_EIC_IRQ_BASE + 48)
#define TRIHIDTV_VIDEO_CAP_MODE_CHG_MP_IRQ            (TRIHIDTV_EIC_IRQ_BASE + 49)
#define TRIHIDTV_VIDEO_VDE_SRC_IRQ                    (TRIHIDTV_EIC_IRQ_BASE + 50)
#define TRIHIDTV_VIDEO_VDE_CRTC_IRQ                   (TRIHIDTV_EIC_IRQ_BASE + 51)

#define TRIHIDTV_PWM0_IRQ                             (TRIHIDTV_EIC_IRQ_BASE + 52)
#define TRIHIDTV_PWM1_IRQ                             (TRIHIDTV_EIC_IRQ_BASE + 53)

#define TRIHIDTV_DCSN_IRQ                             (TRIHIDTV_EIC_IRQ_BASE + 54)

/*HOLE: 55 ~ 63, RESERVED*/

#define TRIHIDTV_A9_DMA_CONTROLLER_INTERRUPT          (TRIHIDTV_EIC_IRQ_BASE + 64)
#define TRIHIDTV_A9_CPU0_COMMRX_INTERRUPT             (TRIHIDTV_EIC_IRQ_BASE + 65)
#define TRIHIDTV_A9_CPU0_COMMTX_INTERRUPT             (TRIHIDTV_EIC_IRQ_BASE + 66)
#define TRIHIDTV_A9_CPU0_CTI_INTERRUPT                (TRIHIDTV_EIC_IRQ_BASE + 67)
#define TRIHIDTV_A9_CPU0_PMU_INTERRUPT                (TRIHIDTV_EIC_IRQ_BASE + 68)

/*HOLE: 69 ~ 95, RESERVED*/
#define TRIHIDTV_RESERVED_IRQ                         (TRIHIDTV_EIC_IRQ_BASE + 69)

#endif /*__ASM_ARCH_TRIX_MACH_SPI_TRIX_UXLB_H__*/
