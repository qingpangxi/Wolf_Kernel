/* linux/arch/arm/mach-exynos4/include/mach/regs-clock-4212.h
 *
 * Copyright (c) 2010-2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * EXYNOS4 - 4212 Clock register definitions
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __ASM_ARCH_REGS_CLOCK_4212_H
#define __ASM_ARCH_REGS_CLOCK_4212_H __FILE__

#define S5P_CLKGATE_BUS_LEFTBUS		S5P_CLKREG(0x04700)
#define S5P_CLKGATE_BUS_IMAGE		S5P_CLKREG(0x04730)

#define S5P_CLKGATE_IP_IMAGE_4212	S5P_CLKREG(0x04930)

#define S5P_CLKGATE_BUS_RIGHTBUS	S5P_CLKREG(0x08700)
#define S5P_CLKGATE_BUS_PERIR		S5P_CLKREG(0x08760)
#define S5P_CLKGATE_IP_PERIR_4212	S5P_CLKREG(0x08960)

#define S5P_EPLL_CON2			S5P_CLKREG(0x0C118)
#define S5P_VPLL_CON2			S5P_CLKREG(0x0C128)

#define S5P_CLKSRC_ISP			S5P_CLKREG(0x0C238)
#define S5P_CLKSRC_CAM1			S5P_CLKREG(0x0C258)

#define S5P_CLKSRC_MASK_ISP		S5P_CLKREG(0x0C338)

#define S5P_CLKDIV_ISP			S5P_CLKREG(0x0C538)
#define S5P_CLKDIV_CAM1			S5P_CLKREG(0x0C568)

#define S5P_CLKDIV_STAT_TOP		S5P_CLKREG(0x0C610)
#define S5P_CLKDIV_STAT_MFC		S5P_CLKREG(0x0C628)

#define S5P_CLKGATE_IP_ISP		S5P_CLKREG(0x0C938)
#define S5P_CLKGATE_IP_MAUDIO		S5P_CLKREG(0x0C93C)

#define S5P_MPLL_LOCK_4212		S5P_CLKREG(0x10008)
#define S5P_MPLL_CON0_4212		S5P_CLKREG(0x10108)
#define S5P_MPLL_CON1_4212		S5P_CLKREG(0x1010C)

#define S5P_CLKGATE_BUS_DMC0		S5P_CLKREG(0x10700)
#define S5P_CLKGATE_BUS_DMC1		S5P_CLKREG(0x10704)
#define S5P_CLKGATE_SCLK_DMC		S5P_CLKREG(0x10800)
#define S5P_CLKGATE_IP_DMC1		S5P_CLKREG(0x10904)

#endif /* __ASM_ARCH_REGS_CLOCK_4212_H */
