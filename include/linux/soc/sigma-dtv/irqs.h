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

#ifndef __ARCH_SIGMA_DTV_IRQS_H__
#define __ARCH_SIGMA_DTV_IRQS_H__

enum irq_type{
	GIC_IRQ_TYPE_SPI = 0,
	GIC_IRQ_TYPE_PPI,
};

#if IS_REACHABLE(CONFIG_TRIX_DRV_HELPER)
/*
 * Create mapping for GIC
 * @irq:   the interrupt number.SPI interrupts are in the range [0-987]
 *         PPI interrupts are in the range [0-15]
 * @type:  interrupt type; 0 for SPI interrupts, 1 for PPI
 *         interrupts.
 * @flags: trigger type and level flags
 *
 * see Documentation/devicetree/bindings/arm/gic(-v3).txt for the details
 * of gic bindings
 */
extern unsigned int trix_gic_create_mapping(unsigned int irq,
				unsigned int type, unsigned int flags);

#else

static inline
unsigned int trix_gic_create_mapping(unsigned int irq, unsigned int type, unsigned int flags)
{
	return 0;
}
#endif /* CONFIG_TRIX_DRV_HELPER */

#define DECLARE_SPI(irq)    trix_gic_create_mapping((irq), GIC_IRQ_TYPE_SPI, 0)
#define DECLARE_PPI(irq)    trix_gic_create_mapping((irq), GIC_IRQ_TYPE_PPI, 0)

/*
 * mapping SPI IRQ numbers for interrupt handler in kernel modules
 */

#if defined(CONFIG_SIGMA_SOC_SX8_OF)
#include "gic-spi-sx8.h"
#elif defined(CONFIG_SIGMA_SOC_UNION)
#include "gic-spi-union.h"
#else
#error "unknown chip when search for SPIs"
#endif

#endif /*__ARCH_SIGMA_DTV_IRQS_H__*/
