#include <linux/module.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/completion.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/slab.h>
#include <umac.h>

#define DQS_UPD_REQ_MASK (0xf << 8)
#define DQS_UPD_REQ_00	(1<<8)
#define DQS_UPD_REQ_01	(1<<9)
#define DQS_UPD_REQ_10	(1<<10)
#define DQS_UPD_REQ_11	(1<<11)

#define DQ_CH_SEL_A	(0x3)
#define DQ_CH_SEL_B	(0xc)
#define DQ_RANK_SEL_0	(0x1)
#define DQ_RANK_SEL_1	(0x2)

#define DQSDQ_DRIFT_COMPENSATION(ch, rak) ({				\
	ctl->u2p_DQSDQCR_shad.bits.dir = 				\
		drift_dir(tDQS2DQ_CH##ch##_rank##rak##_ref,		\
			tDQS2DQ_CH##ch##_rank##rak##_cur);		\
	ctl->u2p_DQSDQCR_shad.bits.dlyoffs = 				\
		drift_time2_phase(tDQS2DQ_CH##ch##_rank##rak##_ref,	\
				tDQS2DQ_CH##ch##_rank##rak##_cur);	\
	ctl->u2p_DQSDQCR_shad.bits.dqsel = DQ_CH_SEL_##ch;		\
	ctl->u2p_DQSDQCR_shad.bits.ranksel = DQ_RANK_SEL_##rak;		\
})

#define CALC_INIT_DRIFT_TOL(ch, rak, pstol) ({				\
	ctl->u2p_WDQS_tol_##ch##rak.bits.max_pos_drift = 		\
		wdqs_tol_maxpos_drift(					\
			ctl->u2p_MR18_19_ref_CH##ch.bits.rank##rak,	\
			tDQS2DQ_CH##ch##_rank##rak##_ref, pstol);	\
	ctl->u2p_WDQS_tol_##ch##rak.bits.max_neg_drift = 		\
		wdqs_tol_maxneg_drift(					\
			ctl->u2p_MR18_19_ref_CH##ch.bits.rank##rak,	\
			tDQS2DQ_CH##ch##_rank##rak##_ref, pstol);	\
})

#define CALC_NEXT_DRIFT_TOL(ch, rak, pstol) ({				\
	ctl->u2p_WDQS_tol_##ch##rak.bits.max_pos_drift = 		\
		wdqs_tol_maxpos_drift(					\
			ctl->u2p_MR18_19_shad_CH##ch.bits.rank##rak,	\
			tDQS2DQ_CH##ch##_rank##rak##_cur, pstol);	\
	ctl->u2p_WDQS_tol_##ch##rak.bits.max_neg_drift = 		\
		wdqs_tol_maxneg_drift(					\
			ctl->u2p_MR18_19_shad_CH##ch.bits.rank##rak,	\
			tDQS2DQ_CH##ch##_rank##rak##_cur, pstol);	\
})

#define TRIGGER_MUPD()	({						\
	ctl->u2p_DQSDQCR_shad.bits.mupd	= 1;				\
})

#define WAIT_UPD_DONE()	({						\
	do {;} while(ctl->u2p_DQSDQCR_shad.bits.mupd);			\
})


#define CLEAR_UPD_REQ(rq)	({					\
	ctl->u2p_WDQS_CTL.val &= (~rq);					\
})

#define RD_DQS2DQ(ch, rak, md) ({					\
	uint32_t val = 0;						\
	val = run_time / (2* (ctl->u2p_MR18_19_##md##_##ch.bits.rak));	\
	val;								\
})

int dram_clk_mhz(void)
{
	//TODO: Get DRAM clock on real chip
	return (1600L);
}

int dram_clk_represent(void)
{
	/*
	 * According to MR23 description in JEDEC
	 *
	 * 00000000B: DQS interval timer stop via MPC Command (Default)
	 * 00000001B: DQS timer stops automatically at 16th clocks after timer start
	 * 00000010B: DQS timer stops automatically at 32nd clocks after timer start
	 * 00000011B: DQS timer stops automatically at 48th clocks after timer start
	 * 00000100B: DQS timer stops automatically at 64th clocks after timer start
	 * ... ...
	 * 00111111B: DQS timer stops automatically at (63X16)th clocks after timer start
	 * 01XXXXXXB: DQS timer stops automatically at 2048th clocks after timer start
	 * 10XXXXXXB: DQS timer stops automatically at 4096th clocks after timer start
	 * 11XXXXXXB: DQS timer stops automatically at 8192nd clocks after timer start
	 */

/* DQS interval timer run count */
#define DQS_IT_RUN_2048CNT	(2048)
#define DQS_IT_RUN_4096CNT	(4096)
#define DQS_IT_RUN_8192CNT	(8192)
	return DQS_IT_RUN_8192CNT;
}

int mhz_2_ps(int mhz)
{
	uint64_t ps = 1000000000000;
	uint64_t hz = (mhz * 1000000);

	/* 32bit width should be long enough */
	return (int)(ps/hz);
}

int dram_clk_period_ps(void)
{
	return mhz_2_ps(dram_clk_mhz());
}

uint16_t wdqs_tol_maxpos_drift(uint16_t ref, uint32_t dqs2dq_ref, uint32_t tol)
{
	return (uint16_t)(ref-(uint16_t)((ref*tol)/(dqs2dq_ref+tol)));
}

uint16_t wdqs_tol_maxneg_drift(uint16_t ref, uint32_t dqs2dq_ref, uint32_t tol)
{
	return (uint16_t)(ref+(uint16_t)((ref*tol)/(dqs2dq_ref-tol)));
}

uint8_t drift_time2_phase(uint32_t ref, uint32_t cur)
{
	uint32_t time = (ref > cur) ? (ref - cur) : (cur - ref);
	/*
	 * Due to we can calculate float value, so here let value multiply 100,
	 * So we can preserve 1e-2 value
	 */
	uint32_t phase_unit_x100 = (dram_clk_period_ps() * 100) / 128;
	uint32_t phase = (time * 100) / phase_unit_x100;
	uint32_t remainder = (time * 100) % phase_unit_x100;

	/*
	 * According DQSDQ_retraining_V1.0.docx, Page 13
	 * The example:
	 * 	Drift phase = 36ps/4.88ps = 8
	 *
	 *   The more precision value should be '7.37704'
	 * Seems, the decimal fraction '0.37704' less than '0.5'
	 * but still round up.
	 *
	 *   Thus, here follow this algorithm
	 */
	if (remainder)
		phase +=1;
	return phase;
}

uint8_t drift_dir(uint32_t ref, uint32_t cur)
{
	return ((cur > ref) ? 1 : 0);
}

irqreturn_t dolphin_dqs_upd_handler(int irq, void * data)
{
	struct umac_device *udev = (struct umac_device *)data;
	struct umac_ctl *ctl = udev->ctl;
	uint32_t run_time = dram_clk_period_ps() * dram_clk_represent();

	/*
	 * Read 'ref' value by each 'Channel' & 'rank'
	 */
	uint32_t tDQS2DQ_CHA_rank0_ref = RD_DQS2DQ(CHA, rank0, ref);
	uint32_t tDQS2DQ_CHA_rank1_ref = RD_DQS2DQ(CHA, rank1, ref);
	uint32_t tDQS2DQ_CHB_rank0_ref = RD_DQS2DQ(CHB, rank0, ref);
	uint32_t tDQS2DQ_CHB_rank1_ref = RD_DQS2DQ(CHB, rank1, ref);

	/*
	 * Read 'cur'(shad) value by each 'Channel' & 'rank'
	 */
	uint32_t tDQS2DQ_CHA_rank0_cur = RD_DQS2DQ(CHA, rank0, shad);
	uint32_t tDQS2DQ_CHA_rank1_cur = RD_DQS2DQ(CHA, rank1, shad);
	uint32_t tDQS2DQ_CHB_rank0_cur = RD_DQS2DQ(CHB, rank0, shad);
	uint32_t tDQS2DQ_CHB_rank1_cur = RD_DQS2DQ(CHB, rank1, shad);


	/* Check which channel need do compenstation */
	uint32_t upd_rq = ctl->u2p_WDQS_CTL.val & DQS_UPD_REQ_MASK;

	if (upd_rq & DQS_UPD_REQ_00) {
		DQSDQ_DRIFT_COMPENSATION(A, 0);
		TRIGGER_MUPD();
		WAIT_UPD_DONE();
		CLEAR_UPD_REQ(DQS_UPD_REQ_00);
		CALC_NEXT_DRIFT_TOL(A, 0, 30);

	}

	if (upd_rq & DQS_UPD_REQ_10) {
		DQSDQ_DRIFT_COMPENSATION(A, 1);
		TRIGGER_MUPD();
		WAIT_UPD_DONE();
		CLEAR_UPD_REQ(DQS_UPD_REQ_10);
		CALC_NEXT_DRIFT_TOL(A, 1, 30);
	}

	if (upd_rq & DQS_UPD_REQ_01) {
		DQSDQ_DRIFT_COMPENSATION(B, 0);
		TRIGGER_MUPD();
		WAIT_UPD_DONE();
		CLEAR_UPD_REQ(DQS_UPD_REQ_01);
		CALC_NEXT_DRIFT_TOL(B, 0, 30);
	}

	if (upd_rq & DQS_UPD_REQ_11) {
		DQSDQ_DRIFT_COMPENSATION(B, 1);
		TRIGGER_MUPD();
		WAIT_UPD_DONE();
		CLEAR_UPD_REQ(DQS_UPD_REQ_11);
		CALC_NEXT_DRIFT_TOL(B, 1, 30);
	}

	return IRQ_HANDLED;
}

int dolphin_compensation_init(struct umac_device *udev)
{

	int ret = -1;

	uint32_t tDQS2DQ_CHA_rank0_ref = 0;
	uint32_t tDQS2DQ_CHA_rank1_ref = 0;
	uint32_t tDQS2DQ_CHB_rank0_ref = 0;
	uint32_t tDQS2DQ_CHB_rank1_ref = 0;

	struct umac_ctl *ctl = udev->ctl;
	uint32_t run_time = dram_clk_period_ps() * dram_clk_represent();

	/* Wait MR18, MR19 ref register ready */
	while (ctl->u2p_WDQS_CTL.bits.restart);


	/* Mask all of derate irq */
	ctl->u2p_WDQS_CTL.bits.int_derate_mask = 0x0;

	/* Enable all of DQS irq */
	ctl->u2p_WDQS_CTL.bits.int_dqs_upd_mask = 0xf;

	/* Enable Interrupt */
	ctl->u2p_WDQS_CTL.bits.int_en = 1;

	/* Make sure PCTL bg task is running */
	BUG_ON(ctl->u2p_WDQS_CTL.bits.pctl_bg_task_en !=1);

	tDQS2DQ_CHA_rank0_ref = RD_DQS2DQ(CHA, rank0, ref);
	tDQS2DQ_CHA_rank1_ref = RD_DQS2DQ(CHA, rank1, ref);
	tDQS2DQ_CHB_rank0_ref = RD_DQS2DQ(CHB, rank0, ref);
	tDQS2DQ_CHB_rank1_ref = RD_DQS2DQ(CHB, rank1, ref);

	/* Calc & set each chanel max drift base on 30ps drift time */
	CALC_INIT_DRIFT_TOL(A, 0, 30);
	CALC_INIT_DRIFT_TOL(A, 1, 30);
	CALC_INIT_DRIFT_TOL(B, 0, 30);
	CALC_INIT_DRIFT_TOL(B, 1, 30);

	/* Register IRQ handler */
	if (udev->irq <= 0) {
		dev_err(udev->dev, "No IRQ resource, UMAC compensation service NOT WORK!!\n");
		ret = -ENODEV;
		goto err;
	}

	ret = devm_request_irq(udev->dev, udev->irq, dolphin_dqs_upd_handler, IRQF_SHARED,
									"umac-DQS-UPD", udev);

	if (ret) {
		dev_err(udev->dev, "Unable request irq %d, ret = %d\n", udev->irq, ret);
		goto err;
	}


	return 0;
err:
	return ret;
}

int umac_compensation_init(struct umac_device *udev)
{
	return dolphin_compensation_init(udev);
}

