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

#ifndef __ASM_ARCH_TRIX_MACH_SPI_TRIX_UNION_H__
#define __ASM_ARCH_TRIX_MACH_SPI_TRIX_UNION_H__

/**UNION SPIs**/
#define TRIHIDTV_AVMIPS_RQST_0_IRQ                    (DECLARE_SPI(0))
#define TRIHIDTV_AVMIPS_RSPNS_0_IRQ                   (DECLARE_SPI(1))
#define TRIHIDTV_AVMIPS_RQST_1_IRQ                    (DECLARE_SPI(2))
#define TRIHIDTV_AVMIPS_RSPNS_1_IRQ                   (DECLARE_SPI(3))

#define TRIHIDTV_CIP_MSP_TSA_INTERRUPT                (DECLARE_SPI(4))
#define TRIHIDTV_CIP_MSP_MCX_INTERRUPT                (DECLARE_SPI(5))
#define TRIHIDTV_CIP_MSP_TSP_INTERRUPT                (DECLARE_SPI(6))
#define TRIHIDTV_CIP_MSP_TSR_INTERRUPT                (DECLARE_SPI(7))
#define TRIHIDTV_CIP_MSP_TSX_INTERRUPT                (DECLARE_SPI(8))
#define TRIHIDTV_CIP_MSP_VD0_INTERRUPT                (DECLARE_SPI(9))
#define TRIHIDTV_CIP_MSP_VD1_INTERRUPT                (DECLARE_SPI(10))
#define TRIHIDTV_CIP_MSP_IPC_INTERRUPT                (DECLARE_SPI(11))

#define TRIHIDTV_GE2D_INTERRUPT                       (DECLARE_SPI(12))

#define TRIHIDTV_DISP_MIPS_REQUEST_INTERRUPT_0        (DECLARE_SPI(13))
#define TRIHIDTV_DISP_MIPS_REQUEST_INTERRUPT_1        (DECLARE_SPI(14))
#define TRIHIDTV_DISP_MIPS_RESPONSE_INTERRUPT_0       (DECLARE_SPI(15))
#define TRIHIDTV_DISP_MIPS_RESPONSE_INTERRUPT_1       (DECLARE_SPI(16))

#define TRIHIDTV_GC_INTERRUPT                         (DECLARE_SPI(17))

#define TRIHIDTV_USB_0_INTERRUPT                      (DECLARE_SPI(18))
#define TRIHIDTV_USB_1_INTERRUPT                      (DECLARE_SPI(19))

#define TRIHIDTV_PMAN_HUB_0_INTERRUPT                 (DECLARE_SPI(20))
#define TRIHIDTV_PMAN_HUB_0_MONITOR_INTERRUPT         (DECLARE_SPI(22))

#define TRIHIDTV_TIMER_0_INTERRUPT                    (DECLARE_SPI(24))
#define TRIHIDTV_TIMER_1_INTERRUPT                    (DECLARE_SPI(25))

#define TRIHIDTV_FLASH_INTERRUPT                      (DECLARE_SPI(26))
#define TRIHIDTV_SDIO_1_INTERRUPT                     (DECLARE_SPI(27))
#define TRIHIDTV_CA_INTERRUPT                         (DECLARE_SPI(28))
#define TRIHIDTV_ETHERNET_INTERRUPT                   (DECLARE_SPI(29))
#define TRIHIDTV_SDIO_0_INTERRUPT                     (DECLARE_SPI(30))

#define TRIHIDTV_MDISP_RES_CH1                        (DECLARE_SPI(31))
#define TRIHIDTV_GPIO_INTERRUPT                       (DECLARE_SPI(32))
#define TRIHIDTV_PLF_DCS_INTERRUPT                    (DECLARE_SPI(33))
#define TRIHIDTV_SPI_CONTROLLER_0_INTERRUPT           (DECLARE_SPI(34))
#define TRIHIDTV_SPI_CONTROLLER_1_INTERRUPT           (DECLARE_SPI(35))
#define TRIHIDTV_MDISP_RES_CH2                        (DECLARE_SPI(36))

#define TRIHIDTV_DEMUX_TSA_IRQ                        (DECLARE_SPI(37))
#define TRIHIDTV_DEMUX_MCX_IRQ                        (DECLARE_SPI(38))
#define TRIHIDTV_DEMUX_TSP_IRQ                        (DECLARE_SPI(39))
#define TRIHIDTV_DEMUX_TSR_IRQ                        (DECLARE_SPI(40))
#define TRIHIDTV_DEMUX_TSX_IRQ                        (DECLARE_SPI(41))
#define TRIHIDTV_DEMUX_VD0_IRQ                        (DECLARE_SPI(42))
#define TRIHIDTV_DEMUX_VD1_IRQ                        (DECLARE_SPI(43))

#define TRIHIDTV_TURING_CKL_IRQ                       (DECLARE_SPI(44))
#define TRIHIDTV_TURING_CRYPTO_IRQ                    (DECLARE_SPI(45))
#define TRIHIDTV_TURING_ACC_IRQ                       (DECLARE_SPI(46))
#define TRIHIDTV_TURING_SAM_IRQ                       (DECLARE_SPI(47))

#define TRIHIDTV_STANDBY_CPU_REQUEST_INTERRUPT        (DECLARE_SPI(48))
#define TRIHIDTV_STANDBY_CPU_RESPONSE_INTERRUPT       (DECLARE_SPI(49))
#define TRIHIDTV_MASTER_I2C_0_INTERRUPT               (DECLARE_SPI(50))
#define TRIHIDTV_MASTER_I2C_1_INTERRUPT               (DECLARE_SPI(51))
#define TRIHIDTV_MASTER_I2C_2_INTERRUPT               (DECLARE_SPI(52))

#define TRIHIDTV_PLF_UART_0_INTERRUPT                 (DECLARE_SPI(53))
#define TRIHIDTV_PLF_UART_1_INTERRUPT                 (DECLARE_SPI(54))
#define TRIHIDTV_PLF_UART_2_INTERRUPT                 (DECLARE_SPI(55))

#define TRIHIDTV_DEMOD_INTERRUPT                      (DECLARE_SPI(57))
#define TRIHIDTV_SMART_CARD_INTERRUPT                 (DECLARE_SPI(58))
#define TRIHIDTV_ETHERNET_PMT_INTERRUPT               (DECLARE_SPI(59))

#define TRIHIDTV_GE3D_BROTHER_INTERRUPT               (DECLARE_SPI(60))

#define TRIHIDTV_HEADPHONE_DETECT_INTERRUPT           (DECLARE_SPI(63))


#define TRIHIDTV_MPEGDISP_CHANNEL_1_INTERRUPT         (DECLARE_SPI(66))
#define TRIHIDTV_DEMUX_IPC_INTERRUPT                  (DECLARE_SPI(68))

#define TRIHIDTV_ABP_DTV_INTERRUPT                    (DECLARE_SPI(69))
#define TRIHIDTV_ABP_MSP_INTERRUPT                    (DECLARE_SPI(70))

#define TRIHIDTV_AUDIO_BRIDGE_INTRRUPT                (DECLARE_SPI(71))
#define TRIHIDTV_MPEGDISP_CHANNEL_2_INTERRUPT         (DECLARE_SPI(72))
#define TRIHIDTV_VDMA_W_INTERRUPT                     (DECLARE_SPI(73))

#define TRIHIDTV_V_PROC_INTERRUPT                     (DECLARE_SPI(74))
#define TRIHIDTV_V_AFE_INTERRUPT                      (DECLARE_SPI(75))
#define TRIHIDTV_V_BLEND_INTERRUPT_VIDEO              (DECLARE_SPI(76))
#define TRIHIDTV_V_DEITN_INTERRUPT                    (DECLARE_SPI(77))
#define TRIHIDTV_V_INCAP_1_INTERRUPT                  (DECLARE_SPI(80))
#define TRIHIDTV_V_PANEL_INTERRUPT                    (DECLARE_SPI(81))

#define TRIHIDTV_HDMI_RX_INTERRUPT                    (DECLARE_SPI(83))
#define TRIHIDTV_V_BLENDER_INTERRUPT_OSD              (DECLARE_SPI(84))

#define TRIHIDTV_MASTER_I2C_3_INTERRUPT               (DECLARE_SPI(85))

#define TRIHIDTV_SPI_CONTROLLER_0_DMAC_INTERRUPT      (DECLARE_SPI(86))
#define TRIHIDTV_SPI_CONTROLLER_1_DMAC_INTERRUPT      (DECLARE_SPI(87))

#define TRIHIDTV_V_PANEL_NEW_INTERRUPT                (DECLARE_SPI(88))
#define TRIHIDTV_V_INCAP_2_INTERRUPT                  (DECLARE_SPI(89))


#define TRIHIDTV_VP9_INTERRUPT                        (DECLARE_SPI(91))
#define TRIHIDTV_HDMI_TX_INTERRUPT                    (DECLARE_SPI(92))
#define TRIHIDTV_PLF_SEC_TIMER_INTERRUPT              (DECLARE_SPI(95))

#define TRIHIDTV_A53_EXT_ERROR_INTERRUPT              (DECLARE_SPI(128))
#define TRIHIDTV_A53_INT_ERROR_INTERRUPT              (DECLARE_SPI(129))
#define TRIHIDTV_A53_COMMTX_0_INTERRUPT               (DECLARE_SPI(130))
#define TRIHIDTV_A53_COMMTX_1_INTERRUPT               (DECLARE_SPI(131))
#define TRIHIDTV_A53_COMMRX_0_INTERRUPT               (DECLARE_SPI(132))
#define TRIHIDTV_A53_COMMRX_1_INTERRUPT               (DECLARE_SPI(133))

#define TRIHIDTV_RESERVED_IRQ			      (DECLARE_SPI(134))
#endif /*__ASM_ARCH_TRIX_MACH_SPI_TRIX_UNION_H__*/
