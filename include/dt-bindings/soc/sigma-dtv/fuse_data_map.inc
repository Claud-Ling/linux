/*
 * Brief: DTV Fuse data mapping descriptor file
 * File: arch/arm/mach-trix/fuse_data_map.inc
 * Author: Tony He <tony_he@sigmadesigns.com>
 * Date: 2015-10-20
 *
 * This file is initialized for SX7.
 * refer to "ProSX7_revA_Fuse_Data_Map.20151013.xlsx" for detailed description
 * So it might need be revised when apply to other silicons.
 *
 * Here only exposes OTP data in which user might be interested.
 * To be noted, Fuse array are secure assets to be with secure monitor.
 *
 * Directives definitions
 * otp_fuse_entry(name, offset[, field[, field[, ...]]])
 *	used to describe directly accessible fuse values, 4 byte long each.
 *
 * otp_fuse_field(bstart, nbits, name)
 *	used to describe bit field for one entry.
 *
 * otp_fuse_generic(name, offset, nbytes)
 *	used to describe generic fuse data, especially for indirectly accessible only one.
 *
 *
 * History:
 *	10/20/2015	TonyHe	- Initial version
 *	07/11/2016	TonyHe	- Add SX8 support
 *
 */

otp_fuse_entry(FC_0,     0x004, otp_fuse_field(0, 6, DECODER_FORMAT_DIS),
				otp_fuse_field(7, 1, USB_SS_DIS),
				otp_fuse_field(8, 4, MELO_D_DIS),
				otp_fuse_field(12,1, GFX_3D_DIS),
#ifdef CONFIG_SIGMA_SOC_SX7
				otp_fuse_field(13,1, EDR_EN),
#else
				otp_fuse_field(13,1, EDR_DIS),
#endif
				otp_fuse_field(14,1, UHD_DIS),
				otp_fuse_field(19,1, DEMOD_DVBT_DIS),
				otp_fuse_field(20,1, DEMOD_ANALOG_DIS),
				otp_fuse_field(21,1, DEMOD_ATSC_DIS),
				otp_fuse_field(22,1, DEMOD_DVBC_DIS),
				otp_fuse_field(23,1, DEMOD_ISDBT_DIS),
				otp_fuse_field(24,1, AUDIO_SRS_DIS),
				otp_fuse_field(25,1, AUDIO_BBE_DIS),
				otp_fuse_field(26,1, AUDIO_SOUNDPROJECTOR_DIS),
#ifdef CONFIG_SIGMA_SOC_SX8
				otp_fuse_field(27,5, ENCODER_FORMAT_DIS),
#endif
				)

otp_fuse_entry(FC_1,     0x008, otp_fuse_field(0, 1, HEVC_DEC_DIS),
				otp_fuse_field(1, 1, MACROVISION_DETECT_DIS),
#ifdef CONFIG_SIGMA_SOC_SX8
				otp_fuse_field(2, 1, MEMC_DIS),
#endif
				otp_fuse_field(3, 1, WATERMARK_EN),
				otp_fuse_field(4, 1, MEM_ENC),
#ifdef CONFIG_SIGMA_SOC_SX8
				otp_fuse_field(5, 2, SELECT_ROOT_KEY),	/*0/1/2:OTP_KEY_3/4/5*/
#endif
				otp_fuse_field(7, 1, DEFAULT_DFT_TAP_ONLY_EN),
				otp_fuse_field(8, 2, CSCAN_MBIST_DBGPORT_PROT),
				otp_fuse_field(10,2, BSCAN_DBGPORT_PROT),
				otp_fuse_field(12,2, HOST_DBGPORT_PROT),
				otp_fuse_field(14,2, TSP_DBGPORT_PROT),
				otp_fuse_field(16,2, FIN_DEST_KL),
#ifdef CONFIG_SIGMA_SOC_SX7
				otp_fuse_field(18,1, SELECT_ROOT_KEY),
#endif
				otp_fuse_field(19,1, LOCK_ROOT_KEY),
				otp_fuse_field(20,1, DISABLE_KL),
				otp_fuse_field(21,1, THREE_LYR_KL),
				otp_fuse_field(22,1, KL_FINAL_KEY_ATT_FROM_OTP),
				otp_fuse_field(23,1, I2C_DEBUG_DIS),
				otp_fuse_field(24,1, WATERMARK_ID_FROM_OTP),
				otp_fuse_field(25,1, WATERMARK_EN_FROM_OTP),
				otp_fuse_field(26,1, WATERMARK_OPERATORID_FROM_OTP),
				otp_fuse_field(28,4, EMMC_CARD_DETECT_TIME))

otp_fuse_entry(FC_2,     0x00C, otp_fuse_field(0, 1, BOOT_FROM_ROM),
				otp_fuse_field(1, 1, SEC_BOOT_EN),
				otp_fuse_field(2, 5, BOOT_VAL_USER_ID),
				otp_fuse_field(7, 1, RSA_KEY_FLSH_VAL),
				otp_fuse_field(8, 1, FLASH_RES_RSA_KEY_GEN_CHECK),
				otp_fuse_field(9, 1, RSA_COMP),
				otp_fuse_field(10,1, FLASH_AUTH_SHUTDOWN_ON_FAIL),
				otp_fuse_field(11,1, BOOT_CODE_DBG_PRINT_EN),
				otp_fuse_field(12,1, NEW_NAND_CTRL_SEL),
				otp_fuse_field(15,10,EMMC_CLOCK_DIVIDER),
				otp_fuse_field(25,2, EMMC_BIT_WIDTH),
				otp_fuse_field(27,1, EMMC_HIGH_SPEED),
				otp_fuse_field(28,1, EMMC_DDR_MODE),
				otp_fuse_field(29,1, A9AXI_RESP_OK),
				otp_fuse_field(30,1, A9_SPIDEN_DIS),
				otp_fuse_field(31,1, A9_SPNIDEN_DIS))

otp_fuse_entry(FC_3,     0x010, otp_fuse_field(0, 1, SEC_DIS),
				otp_fuse_field(1, 1, BOOT_FROM_ROM_DIS))

otp_fuse_entry(DIE_ID_0, 0x038, otp_fuse_field(0, 8, DESIGN_ID),
				otp_fuse_field(8, 1, DTS_SOUND_EN),
				otp_fuse_field(9, 1, DTS_DECODE_EN))

#ifdef CONFIG_SIGMA_SOC_SX7
otp_fuse_generic(RSA_PUB_KEY, 0x290, 0x100)
#elif defined(CONFIG_SIGMA_SOC_SX8)
otp_fuse_generic(RSA_PUB_KEY, 0xd50, 0x100)
#endif
