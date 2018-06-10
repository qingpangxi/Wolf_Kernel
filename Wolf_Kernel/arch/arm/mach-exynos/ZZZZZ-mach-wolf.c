#include <linux/platform_device.h>
#include <linux/serial_core.h>
#include <linux/spi/spi.h>
#include <linux/spi/spi_gpio.h>
#include <linux/clk.h>
#include <linux/lcd.h>
#include <linux/gpio.h>
#include <linux/gpio_event.h>
#include <linux/i2c.h>
#include <linux/pwm_backlight.h>
#include <linux/input.h>
#include <linux/mmc/host.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/max8649.h>
#include <linux/regulator/fixed.h>
#include <linux/v4l2-mediabus.h>
#include <linux/memblock.h>
#include <linux/delay.h>
#if defined(CONFIG_S5P_MEM_CMA)
#include <linux/cma.h>
#endif
#ifdef CONFIG_ANDROID_PMEM
#include <linux/android_pmem.h>
#endif
#include <linux/smsc911x.h>
#include <linux/notifier.h>
#include <linux/reboot.h>
#include <asm/mach/arch.h>
#include <asm/mach-types.h>
#include <plat/regs-serial.h>
#include <plat/exynos4.h>
#include <plat/cpu.h>
#include <plat/clock.h>
#include <plat/keypad.h>
#include <plat/devs.h>
#include <plat/fb.h>
#include <plat/fb-s5p.h>
#include <plat/fb-core.h>
#include <plat/regs-fb-v4.h>
#include <plat/backlight.h>
#include <plat/gpio-cfg.h>
#include <plat/regs-adc.h>
#include <plat/adc.h>
#include <plat/iic.h>
#include <plat/pd.h>
#include <plat/sdhci.h>
#include <plat/mshci.h>
#include <plat/ehci.h>
#include <plat/usbgadget.h>
#include <plat/s3c64xx-spi.h>
#include <plat/tvout.h>
#include <plat/media.h>
#include <plat/regs-srom.h>
#include <plat/tv-core.h>
#include <media/exynos_flite.h>
#include <media/exynos_fimc_is.h>
#include <video/platform_lcd.h>
#include <mach/board_rev.h>
#include <mach/map.h>
#include <mach/spi-clocks.h>
#include <mach/exynos-ion.h>
#include <mach/regs-pmu.h>
#include <mach/map.h>
#include <mach/regs-pmu.h>
#ifdef CONFIG_EXYNOS4_CONTENT_PATH_PROTECTION
#include <mach/secmem.h>
#endif
#include <mach/dev.h>
#include <mach/ppmu.h>
#ifdef CONFIG_EXYNOS_C2C
#include <mach/c2c.h>
#endif
#include <plat/fimg2d.h>
#if defined(CONFIG_KERNEL_PANIC_DUMP)		//panic-dump
#include <mach/panic-dump.h>
#endif
#include <linux/mpu.h>
#include <linux/i2c-gpio.h>	
#ifdef CONFIG_REGULATOR_S5M8767
#include <linux/mfd/s5m87xx/s5m-core.h>
#include <linux/mfd/s5m87xx/s5m-pmic.h>
#endif

#define REG_INFORM4            (S5P_INFORM4)

/************************************************Start UART***************************************************/
#define SMDK4X12_UCON_DEFAULT	(S3C2410_UCON_TXILEVEL |	\
				 S3C2410_UCON_RXILEVEL |	\
				 S3C2410_UCON_TXIRQMODE |	\
				 S3C2410_UCON_RXIRQMODE |	\
				 S3C2410_UCON_RXFIFO_TOI |	\
				 S3C2443_UCON_RXERR_IRQEN)

#define SMDK4X12_ULCON_DEFAULT	S3C2410_LCON_CS8

#define SMDK4X12_UFCON_DEFAULT	(S3C2410_UFCON_FIFOMODE |	\
				 S5PV210_UFCON_TXTRIG4 |	\
				 S5PV210_UFCON_RXTRIG4)

static struct s3c2410_uartcfg smdk4x12_uartcfgs[] __initdata = {
	[0] = {
		.hwport		= 0,
		.flags		= 0,
		.ucon		= SMDK4X12_UCON_DEFAULT,
		.ulcon		= SMDK4X12_ULCON_DEFAULT,
		.ufcon		= SMDK4X12_UFCON_DEFAULT,
	},
	[1] = {
		.hwport		= 1,
		.flags		= 0,
		.ucon		= SMDK4X12_UCON_DEFAULT,
		.ulcon		= SMDK4X12_ULCON_DEFAULT,
		.ufcon		= SMDK4X12_UFCON_DEFAULT,
	},
	[2] = {
		.hwport		= 2,
		.flags		= 0,
		.ucon		= SMDK4X12_UCON_DEFAULT,
		.ulcon		= SMDK4X12_ULCON_DEFAULT,
		.ufcon		= SMDK4X12_UFCON_DEFAULT,
	},
	[3] = {
		.hwport		= 3,
		.flags		= 0,
		.ucon		= SMDK4X12_UCON_DEFAULT,
		.ulcon		= SMDK4X12_ULCON_DEFAULT,
		.ufcon		= SMDK4X12_UFCON_DEFAULT,
	},
};
/************************************************End UART***************************************************/

/************************************************Start Notifier***************************************************/
static int exynos4_notifier_call(struct notifier_block *this,
					unsigned long code, void *_cmd)
{
	int mode = 0;

	if ((code == SYS_RESTART) && _cmd)
		if (!strcmp((char *)_cmd, "recovery"))
			mode = 0xf;

	__raw_writel(mode, REG_INFORM4);

	return NOTIFY_DONE;
}

static struct notifier_block exynos4_reboot_notifier = {
	.notifier_call = exynos4_notifier_call,
};
/************************************************End Notifier***************************************************/

/************************************************Start MMC***************************************************/
#ifdef CONFIG_S3C_DEV_HSMMC1
static struct s3c_sdhci_platdata smdk4x12_hsmmc1_pdata __initdata = {
	.cd_type		= S3C_SDHCI_CD_INTERNAL,
	.clk_type		= S3C_SDHCI_CLK_DIV_EXTERNAL,
};
#endif

#ifdef CONFIG_S3C_DEV_HSMMC2
static struct s3c_sdhci_platdata smdk4x12_hsmmc2_pdata __initdata = {
	.cd_type		= S3C_SDHCI_CD_GPIO,//lisw sd    S3C_SDHCI_CD_INTERNAL,
	.ext_cd_gpio            =EXYNOS4_GPX0(7), //lisw sd
    	.ext_cd_gpio_invert     = 1,//lisw sd
	.clk_type		= S3C_SDHCI_CLK_DIV_EXTERNAL,
#ifdef CONFIG_EXYNOS4_SDHCI_CH2_8BIT
	.max_width		= 8,
	.host_caps		= MMC_CAP_8_BIT_DATA,
#endif
};
#endif

#ifdef CONFIG_S3C_DEV_HSMMC3
static struct s3c_sdhci_platdata smdk4x12_hsmmc3_pdata __initdata = {
	// SEMCO
	#if 0
	.cd_type		= S3C_SDHCI_CD_INTERNAL,
	.clk_type		= S3C_SDHCI_CLK_DIV_EXTERNAL,
	#else
	.cd_type		= S3C_SDHCI_CD_PERMANENT,
	.clk_type		= S3C_SDHCI_CLK_DIV_EXTERNAL,
	#endif
};
#endif

#ifdef CONFIG_S5P_DEV_MSHC
static struct s3c_mshci_platdata exynos4_mshc_pdata __initdata = {
	.cd_type		= S3C_MSHCI_CD_PERMANENT,
	.clk_type		= S3C_MSHCI_CLK_DIV_EXTERNAL, //lisw ms
	.has_wp_gpio		= true,
	.wp_gpio		= 0xffffffff,
#if defined(CONFIG_EXYNOS4_MSHC_8BIT) && \
	defined(CONFIG_EXYNOS4_MSHC_DDR)
	.max_width		= 8,
	.host_caps		= MMC_CAP_8_BIT_DATA | MMC_CAP_1_8V_DDR |
				  MMC_CAP_UHS_DDR50,
#elif defined(CONFIG_EXYNOS4_MSHC_8BIT)
	.max_width		= 8,
	.host_caps		= MMC_CAP_8_BIT_DATA,
#elif defined(CONFIG_EXYNOS4_MSHC_DDR)
	.host_caps		= MMC_CAP_1_8V_DDR | MMC_CAP_UHS_DDR50,
#endif
};
#endif
/************************************************End MMC***************************************************/

/************************************************Start 电源管理***************************************************/
static struct platform_device tc4_regulator_consumer = 
{	.name = "tc4-regulator-consumer",	
	.id = -1,
};

#ifdef CONFIG_REGULATOR_S5M8767
/* S5M8767 Regulator */
static int s5m_cfg_irq(void)
{
	/* AP_PMIC_IRQ: EINT15 */
	s3c_gpio_cfgpin(EXYNOS4_GPX1(7), S3C_GPIO_SFN(0xF));
	s3c_gpio_setpull(EXYNOS4_GPX1(7), S3C_GPIO_PULL_UP);
	return 0;
}
static struct regulator_consumer_supply s5m8767_ldo1_supply[] = {
	REGULATOR_SUPPLY("vdd_alive", NULL),
};

static struct regulator_consumer_supply s5m8767_ldo2_supply[] = {
	REGULATOR_SUPPLY("vddq_m12", NULL),
};

static struct regulator_consumer_supply s5m8767_ldo3_supply[] = {
	REGULATOR_SUPPLY("vddioap_18", NULL),
};

static struct regulator_consumer_supply s5m8767_ldo4_supply[] = {
	REGULATOR_SUPPLY("vddq_pre", NULL),
};

static struct regulator_consumer_supply s5m8767_ldo5_supply[] = {
	REGULATOR_SUPPLY("vdd18_2m", NULL),
};

static struct regulator_consumer_supply s5m8767_ldo6_supply[] = {
	REGULATOR_SUPPLY("vdd10_mpll", NULL),
};

static struct regulator_consumer_supply s5m8767_ldo7_supply[] = {
	REGULATOR_SUPPLY("vdd10_xpll", NULL),
};

static struct regulator_consumer_supply s5m8767_ldo8_supply[] = {
	REGULATOR_SUPPLY("vdd10_mipi", NULL),
};

static struct regulator_consumer_supply s5m8767_ldo9_supply[] = {
	REGULATOR_SUPPLY("vdd33_lcd", NULL),
};

static struct regulator_consumer_supply s5m8767_ldo10_supply[] = {
	REGULATOR_SUPPLY("vdd18_mipi", NULL),
};

static struct regulator_consumer_supply s5m8767_ldo11_supply[] = {
	REGULATOR_SUPPLY("vdd18_abb1", NULL),
};

static struct regulator_consumer_supply s5m8767_ldo12_supply[] = {
	REGULATOR_SUPPLY("vdd33_uotg", NULL),
};

static struct regulator_consumer_supply s5m8767_ldo13_supply[] = {
	REGULATOR_SUPPLY("vddioperi_18", NULL),
};

static struct regulator_consumer_supply s5m8767_ldo14_supply[] = {
	REGULATOR_SUPPLY("vdd18_abb02", NULL),
};

static struct regulator_consumer_supply s5m8767_ldo15_supply[] = {
	REGULATOR_SUPPLY("vdd10_ush", NULL),
};

static struct regulator_consumer_supply s5m8767_ldo16_supply[] = {
	REGULATOR_SUPPLY("vdd18_hsic", NULL),
};

static struct regulator_consumer_supply s5m8767_ldo17_supply[] = {
	REGULATOR_SUPPLY("vddioap_mmc012_28", NULL),
};
static struct regulator_consumer_supply s5m8767_ldo18_supply[] = {
	REGULATOR_SUPPLY("vddioperi_28", NULL),
};

static struct regulator_consumer_supply s5m8767_ldo19_supply[] = {
	REGULATOR_SUPPLY("dvdd25", NULL),
};


static struct regulator_consumer_supply s5m8767_ldo20_supply[] = {
	REGULATOR_SUPPLY("vdd28_cam", NULL),
};
static struct regulator_consumer_supply s5m8767_ldo21_supply[] = {
	REGULATOR_SUPPLY("vdd28_af", NULL),
};

static struct regulator_consumer_supply s5m8767_ldo22_supply[] = {
	REGULATOR_SUPPLY("vdda28_2m", NULL),
};

static struct regulator_consumer_supply s5m8767_ldo23_supply[] = {
	REGULATOR_SUPPLY("vdd_tf", NULL),
};


static struct regulator_consumer_supply s5m8767_ldo24_supply[] = {
	REGULATOR_SUPPLY("vdd33_a31", NULL),
};

static struct regulator_consumer_supply s5m8767_ldo25_supply[] = {
	REGULATOR_SUPPLY("vdd18_cam", NULL),
};

static struct regulator_consumer_supply s5m8767_ldo26_supply[] = {
	REGULATOR_SUPPLY("vdd18_a31", NULL),
};
static struct regulator_consumer_supply s5m8767_ldo27_supply[] = {
	REGULATOR_SUPPLY("gps_1v8", NULL),
};
static struct regulator_consumer_supply s5m8767_ldo28_supply[] = {
	REGULATOR_SUPPLY("dvdd12", NULL),
};


static struct regulator_consumer_supply s5m8767_buck1_consumer =
	REGULATOR_SUPPLY("vdd_mif", NULL);

static struct regulator_consumer_supply s5m8767_buck2_consumer =
	REGULATOR_SUPPLY("vdd_arm", NULL);

static struct regulator_consumer_supply s5m8767_buck3_consumer =
	REGULATOR_SUPPLY("vdd_int", NULL);

static struct regulator_consumer_supply s5m8767_buck4_consumer =
	REGULATOR_SUPPLY("vdd_g3d", NULL);

static struct regulator_consumer_supply s5m8767_buck5_consumer =
	REGULATOR_SUPPLY("vdd_m12", NULL);
static struct regulator_consumer_supply s5m8767_buck6_consumer =
	REGULATOR_SUPPLY("vdd12_5m", NULL);

static struct regulator_consumer_supply s5m8767_buck9_consumer =
	REGULATOR_SUPPLY("vddf28_emmc", NULL);



#define REGULATOR_INIT(_ldo, _name, _min_uV, _max_uV, _always_on, _ops_mask,\
		_disabled) \
	static struct regulator_init_data s5m8767_##_ldo##_init_data = {		\
		.constraints = {					\
			.name	= _name,				\
			.min_uV = _min_uV,				\
			.max_uV = _max_uV,				\
			.always_on	= _always_on,			\
			.boot_on	= _always_on,			\
			.apply_uV	= 1,				\
			.valid_ops_mask = _ops_mask,			\
			.state_mem	= {				\
				.disabled	= _disabled,		\
				.enabled	= !(_disabled),		\
			}						\
		},							\
		.num_consumer_supplies = ARRAY_SIZE(s5m8767_##_ldo##_supply),	\
		.consumer_supplies = &s5m8767_##_ldo##_supply[0],			\
	};

REGULATOR_INIT(ldo1, "VDD_ALIVE", 1100000, 1100000, 1,
		REGULATOR_CHANGE_STATUS, 0);
REGULATOR_INIT(ldo2, "VDDQ_M12", 1200000, 1200000, 1,
		REGULATOR_CHANGE_STATUS, 1);//sleep controlled by pwren
REGULATOR_INIT(ldo3, "VDDIOAP_18", 1800000, 1800000, 1,
		REGULATOR_CHANGE_STATUS, 0);
REGULATOR_INIT(ldo4, "VDDQ_PRE", 1800000, 1800000, 1,
		REGULATOR_CHANGE_STATUS, 1); //sleep controlled by pwren

REGULATOR_INIT(ldo5, "VDD18_2M", 1800000, 1800000, 0,
		REGULATOR_CHANGE_STATUS, 1);
REGULATOR_INIT(ldo6, "VDD10_MPLL", 1000000, 1000000, 1,
		REGULATOR_CHANGE_STATUS, 1);//sleep controlled by pwren
REGULATOR_INIT(ldo7, "VDD10_XPLL", 1000000, 1000000, 1,
		REGULATOR_CHANGE_STATUS, 1);//sleep controlled by pwren
REGULATOR_INIT(ldo8, "VDD10_MIPI", 1000000, 1000000, 1,
		REGULATOR_CHANGE_STATUS, 1);
REGULATOR_INIT(ldo9, "VDD33_LCD", 3300000, 3300000, 1,
		REGULATOR_CHANGE_STATUS, 1);


REGULATOR_INIT(ldo10, "VDD18_MIPI", 1800000, 1800000, 1,
		REGULATOR_CHANGE_STATUS, 1);
REGULATOR_INIT(ldo11, "VDD18_ABB1", 1800000, 1800000, 1,
		REGULATOR_CHANGE_STATUS, 0); //???
REGULATOR_INIT(ldo12, "VDD33_UOTG", 3300000, 3300000, 1,
		REGULATOR_CHANGE_STATUS, 0);
REGULATOR_INIT(ldo13, "VDDIOPERI_18", 1800000, 1800000, 1,
		REGULATOR_CHANGE_STATUS, 0);//???
REGULATOR_INIT(ldo14, "VDD18_ABB02", 1800000, 1800000, 1,
		REGULATOR_CHANGE_STATUS, 0); //???
REGULATOR_INIT(ldo15, "VDD10_USH", 1000000, 1000000, 1,
		REGULATOR_CHANGE_STATUS, 1);

//liang, VDD18_HSIC must be 1.8V, otherwise USB HUB 3503A can't be recognized
REGULATOR_INIT(ldo16, "VDD18_HSIC", 1800000, 1800000, 1,
		REGULATOR_CHANGE_STATUS, 1);
REGULATOR_INIT(ldo17, "VDDIOAP_MMC012_28", 2800000, 2800000, 1,
		REGULATOR_CHANGE_STATUS, 0); //???
REGULATOR_INIT(ldo18, "VDDIOPERI_28", 2800000, 2800000, 1,
		REGULATOR_CHANGE_STATUS, 0);//???
REGULATOR_INIT(ldo19, "DVDD25", 2500000, 2500000, 0,
		REGULATOR_CHANGE_STATUS, 1); //??
REGULATOR_INIT(ldo20, "VDD28_CAM", 2800000, 2800000, 0,
		REGULATOR_CHANGE_STATUS, 1);

REGULATOR_INIT(ldo21, "VDD28_AF", 2800000, 2800000, 0,
		REGULATOR_CHANGE_STATUS, 1);
REGULATOR_INIT(ldo22, "VDDA28_2M", 2800000, 2800000, 0,
		REGULATOR_CHANGE_STATUS, 1);
REGULATOR_INIT(ldo23, "VDD28_TF", 2800000, 2800000, 1,
		REGULATOR_CHANGE_STATUS, 1);//sleep controlled by pwren
REGULATOR_INIT(ldo24, "VDD33_A31", 3300000, 3300000, 1,
		REGULATOR_CHANGE_STATUS, 0);
REGULATOR_INIT(ldo25, "VDD18_CAM", 1800000, 1800000, 0,
		REGULATOR_CHANGE_STATUS, 1);
REGULATOR_INIT(ldo26, "VDD18_A31", 1800000, 1800000, 1,
		REGULATOR_CHANGE_STATUS, 0);
REGULATOR_INIT(ldo27, "GPS_1V8", 1800000, 1800000, 1,
		REGULATOR_CHANGE_STATUS, 1);
REGULATOR_INIT(ldo28, "DVDD12", 1200000, 1200000, 0,
		REGULATOR_CHANGE_STATUS, 1);


static struct regulator_init_data s5m8767_buck1_data = {
	.constraints	= {
		.name		= "vdd_mif range",
		.min_uV		= 900000,
		.max_uV		= 1100000,
		.valid_ops_mask	= REGULATOR_CHANGE_VOLTAGE |
				  REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled	= 1,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &s5m8767_buck1_consumer,
};

static struct regulator_init_data s5m8767_buck2_data = {
	.constraints	= {
		.name		= "vdd_arm range",
		.min_uV		=  850000,
		.max_uV		= 1450000,
		.valid_ops_mask	= REGULATOR_CHANGE_VOLTAGE |
				  REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled	= 1,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &s5m8767_buck2_consumer,
};

static struct regulator_init_data s5m8767_buck3_data = {
	.constraints	= {
		.name		= "vdd_int range",
		.min_uV		=  875000,
		.max_uV		= 1200000,
		.apply_uV	= 1,
		.valid_ops_mask	= REGULATOR_CHANGE_VOLTAGE |
				REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			//.uV		= 1100000,
			.mode		= REGULATOR_MODE_NORMAL,
			.disabled	= 1,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &s5m8767_buck3_consumer,
};

static struct regulator_init_data s5m8767_buck4_data = {
	.constraints	= {
		.name		= "vdd_g3d range",
		.min_uV		= 750000,
		.max_uV		= 1500000,
		.valid_ops_mask	= REGULATOR_CHANGE_VOLTAGE |
				REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled	= 1,
		},
	},
	.num_consumer_supplies = 1,
	.consumer_supplies = &s5m8767_buck4_consumer,
};

static struct regulator_init_data s5m8767_buck5_data = {
	.constraints	= {
		.name		= "vdd_m12 range",
		.min_uV		= 750000,
		.max_uV		= 1500000,
		.apply_uV	= 1,
		.boot_on	= 1,
		.valid_ops_mask	= REGULATOR_CHANGE_VOLTAGE |
				REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.enabled	= 1,
		},
	},
	.num_consumer_supplies = 1,
	.consumer_supplies = &s5m8767_buck5_consumer,
};
static struct regulator_init_data s5m8767_buck6_data = {
	.constraints	= {
		.name		= "vdd12_5m range",
		.min_uV		= 750000,
		.max_uV		= 1500000,
		.boot_on	= 0,
		.apply_uV	= 1,
		.valid_ops_mask	= REGULATOR_CHANGE_VOLTAGE |
				REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled	= 1,
		},
	},
	.num_consumer_supplies = 1,
	.consumer_supplies = &s5m8767_buck6_consumer,
};
static struct regulator_init_data s5m8767_buck9_data = {
	.constraints	= {
		.name		= "vddf28_emmc range",
		.min_uV		= 750000,
		.max_uV		= 3000000,
		.boot_on	= 1,
		.apply_uV	= 1,
		.valid_ops_mask	= REGULATOR_CHANGE_VOLTAGE |
				REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled	= 1,
		},
	},
	.num_consumer_supplies = 1,
	.consumer_supplies = &s5m8767_buck9_consumer,
};




static struct s5m_regulator_data pegasus_regulators[] = {
	{ S5M8767_BUCK1, &s5m8767_buck1_data },
	{ S5M8767_BUCK2, &s5m8767_buck2_data },
	{ S5M8767_BUCK3, &s5m8767_buck3_data },
	{ S5M8767_BUCK4, &s5m8767_buck4_data },
	{ S5M8767_BUCK5, &s5m8767_buck5_data },
	{ S5M8767_BUCK6, &s5m8767_buck6_data },
	{ S5M8767_BUCK9, &s5m8767_buck9_data },

	{ S5M8767_LDO1, &s5m8767_ldo1_init_data },
	{ S5M8767_LDO2, &s5m8767_ldo2_init_data },
	{ S5M8767_LDO3, &s5m8767_ldo3_init_data },
	{ S5M8767_LDO4, &s5m8767_ldo4_init_data },
	{ S5M8767_LDO5, &s5m8767_ldo5_init_data },
	{ S5M8767_LDO6, &s5m8767_ldo6_init_data },
	{ S5M8767_LDO7, &s5m8767_ldo7_init_data },
	{ S5M8767_LDO8, &s5m8767_ldo8_init_data },
	{ S5M8767_LDO9, &s5m8767_ldo9_init_data },
	{ S5M8767_LDO10, &s5m8767_ldo10_init_data },

	{ S5M8767_LDO11, &s5m8767_ldo11_init_data },
	{ S5M8767_LDO12, &s5m8767_ldo12_init_data },
	{ S5M8767_LDO13, &s5m8767_ldo13_init_data },
	{ S5M8767_LDO14, &s5m8767_ldo14_init_data },
	{ S5M8767_LDO15, &s5m8767_ldo15_init_data },
	{ S5M8767_LDO16, &s5m8767_ldo16_init_data },
	{ S5M8767_LDO17, &s5m8767_ldo17_init_data },
	{ S5M8767_LDO18, &s5m8767_ldo18_init_data },
	{ S5M8767_LDO19, &s5m8767_ldo19_init_data },
	{ S5M8767_LDO20, &s5m8767_ldo20_init_data },

	{ S5M8767_LDO21, &s5m8767_ldo21_init_data },
	{ S5M8767_LDO22, &s5m8767_ldo22_init_data },
	{ S5M8767_LDO23, &s5m8767_ldo23_init_data },
	{ S5M8767_LDO24, &s5m8767_ldo24_init_data },
	{ S5M8767_LDO25, &s5m8767_ldo25_init_data },
	{ S5M8767_LDO26, &s5m8767_ldo26_init_data },
	{ S5M8767_LDO27, &s5m8767_ldo27_init_data },
	{ S5M8767_LDO28, &s5m8767_ldo28_init_data },
	
	
};

static struct s5m_platform_data exynos4_s5m8767_pdata = {
	.device_type		= S5M8767X,
	.irq_base		= IRQ_BOARD_START,
	.num_regulators		= ARRAY_SIZE(pegasus_regulators),
	.regulators		= pegasus_regulators,
	.cfg_pmic_irq		= s5m_cfg_irq,

	.buck2_voltage[0]	= 1250000,
	.buck2_voltage[1]	= 1200000,
	.buck2_voltage[2]	= 1200000,
	.buck2_voltage[3]	= 1200000,
	.buck2_voltage[4]	= 1200000,
	.buck2_voltage[5]	= 1200000,
	.buck2_voltage[6]	=  1200000,
	.buck2_voltage[7]	=  1200000,

	.buck3_voltage[0]	= 1100000,
	.buck3_voltage[1]	= 1100000,
	.buck3_voltage[2]	= 1100000,
	.buck3_voltage[3]	= 1100000,
	.buck3_voltage[4]	= 1100000,
	.buck3_voltage[5]	= 1100000,
	.buck3_voltage[6]	= 1100000,
	.buck3_voltage[7]	= 1100000,

	.buck4_voltage[0]	= 1200000,
	.buck4_voltage[1]	= 1200000,
	.buck4_voltage[2]	= 1200000,
	.buck4_voltage[3]	= 1200000,
	.buck4_voltage[4]	= 1200000,
	.buck4_voltage[5]	= 1200000,
	.buck4_voltage[6]	= 1200000,
	.buck4_voltage[7]	= 1200000,

	.buck_default_idx	= 3,
	.buck_gpios[0]		= EXYNOS4_GPX2(5),
	.buck_gpios[1]		= EXYNOS4_GPX2(6),
	.buck_gpios[2]		= EXYNOS4_GPX2(7),

	.buck_ramp_delay        = 10,
	.buck2_ramp_enable      = true,
	.buck3_ramp_enable      = true,
	.buck4_ramp_enable      = true,
};
/* End of S5M8767 */
#endif
/************************************************End 电源管理***************************************************/

/************************************************Start i2c***************************************************/
/* i2c devs mapping: 
 * i2c0 : HDMI
 * i2c1 : max8997: PMIC & RTC & motor
 * i2c2 : not used
 * i2c3 : touch
 * i2c4 : max8997 fuel gauge & wm8994
 * i2c5 : sensor: MPU3050
 * i2c6 : camera & HSIC
 * i2c7 : light sensor
 */

static struct i2c_board_info i2c_devs1[] __initdata = {
	{
#ifdef CONFIG_REGULATOR_S5M8767

		I2C_BOARD_INFO("s5m87xx", 0xCC >> 1),
		.platform_data = &exynos4_s5m8767_pdata,
		.irq		= IRQ_EINT(15),

#else
		I2C_BOARD_INFO(MAX8997_I2C_NAME, MAX8997_ADDR),
#ifdef CONFIG_REGULATOR_MAX8997
		.platform_data = &exynos4_max8997_info,
#endif
#endif
	},
#ifdef CONFIG_VIBRATOR

#ifndef CONFIG_REGULATOR_S5M8767

	{
		I2C_BOARD_INFO(MOTOR8997_I2C_NAME, MOTOR8997_ADDR),
	},
#endif

#endif

#ifdef CONFIG_RTC_MAX8997
	{
		I2C_BOARD_INFO(RTC8997_I2C_NAME, RTC8997_I2C_ADDR),
	},
#endif

#ifdef CONFIG_BATTERY_MAX17040
	{
		I2C_BOARD_INFO(MAX17040_I2C_NAME, MAX17040_I2C_ADDR),
	},
#endif

};
/************************************************End i2c***************************************************/

/************************************************Start Memory***************************************************/

/************************************************End Memory***************************************************/

/************************************************Start PMIC***************************************************/
#ifdef CONFIG_ANDROID_PMEM
static struct android_pmem_platform_data pmem_pdata = {
	.name		= "pmem",
	.no_allocator	= 1,
	.cached		= 0,
	.start		= 0,
	.size		= 0
};
static struct android_pmem_platform_data pmem_gpu1_pdata = {
	.name		= "pmem_gpu1",
	.no_allocator	= 1,
	.cached		= 0,
	.start		= 0,
	.size		= 0,
};
static struct platform_device pmem_device = {
	.name	= "android_pmem",
	.id	= 0,
	.dev	= {
		.platform_data = &pmem_pdata
	},
};
static struct platform_device pmem_gpu1_device = {
	.name	= "android_pmem",
	.id	= 1,
	.dev	= {
		.platform_data = &pmem_gpu1_pdata
	},
};
static void __init android_pmem_set_platdata(void)
{
#if defined(CONFIG_S5P_MEM_CMA)
	pmem_pdata.size = CONFIG_ANDROID_PMEM_MEMSIZE_PMEM * SZ_1K;
	pmem_gpu1_pdata.size = CONFIG_ANDROID_PMEM_MEMSIZE_PMEM_GPU1 * SZ_1K;
#endif
}
#endif
/* s5p-pmic interface */
static struct resource s5p_pmic_resource[] = {

};


struct platform_device s5p_device_pmic = {
  .name             = "s5p-pmic",
  .id               = -1,
  .num_resources    = ARRAY_SIZE(s5p_pmic_resource),
  .resource         = s5p_pmic_resource,
};

EXPORT_SYMBOL(s5p_device_pmic);
/************************************************End PMIC***************************************************/

/************************************************Start 电源管理GPIO***************************************************/
static void __init smdk4x12_gpio_power_init(void)
{
	int err = 0;

	err = gpio_request_one(EXYNOS4_GPX0(0), 0, "GPX0");
	if (err) {
		printk(KERN_ERR "failed to request GPX0 for "
				"suspend/resume control\n");
		return;
	}
	s3c_gpio_setpull(EXYNOS4_GPX0(0), S3C_GPIO_PULL_NONE);

	gpio_free(EXYNOS4_GPX0(0));
}
/************************************************End 电源管理GPIO***************************************************/

#ifdef CONFIG_WAKEUP_ASSIST
static struct platform_device wakeup_assist_device = {
	.name   = "wakeup_assist",
};
#endif

#ifdef CONFIG_VIDEO_FIMG2D
static struct fimg2d_platdata fimg2d_data __initdata = {
	.hw_ver = 0x41,
	.parent_clkname = "mout_g2d0",
	.clkname = "sclk_fimg2d",
	.gate_clkname = "fimg2d",
	.clkrate = 267 * 1000000,	/* 266 Mhz */
};
#endif

#ifdef CONFIG_EXYNOS_C2C
struct exynos_c2c_platdata smdk4x12_c2c_pdata = {
	.setup_gpio	= NULL,
	.shdmem_addr	= C2C_SHAREDMEM_BASE,
	.shdmem_size	= C2C_MEMSIZE_64,
	.ap_sscm_addr	= NULL,
	.cp_sscm_addr	= NULL,
	.rx_width	= C2C_BUSWIDTH_16,
	.tx_width	= C2C_BUSWIDTH_16,
	.clk_opp100	= 400,
	.clk_opp50	= 266,
	.clk_opp25	= 0,
	.default_opp_mode	= C2C_OPP50,
	.get_c2c_state	= NULL,
	.c2c_sysreg	= S5P_VA_CMU + 0x12000,
};
#endif

#ifdef CONFIG_BUSFREQ_OPP
/* BUSFREQ to control memory/bus*/
static struct device_domain busfreq;
#endif

static struct platform_device exynos4_busfreq = {
	.id = -1,
	.name = "exynos-busfreq",
};

// SEMCO
/* The sdhci_s3c_sdio_card_detect function is used for detecting
   the WiFi/BT module when the menu for enabling the WiFi is
   selected.
   The semco_a31_detection function is called by ar6000's probe function.

   The call sequence is

   ar6000_pm_probe() -> plat_setup_power_for_onoff() -> detect_semco_wlan_for_onoff()
   -> setup_semco_wlan_power_onoff() -> semco_a31_detection()

   The mmc_semco_a31_sdio_remove function is used for removing the mmc driver
   when the menu for disabling the WiFi is selected.
   The semco_a31_removal function is called by ar6000's remove function.

   The call sequence is

   ar6000_pm_remove() -> plat_setup_power_for_onoff() -> detect_semco_wlan_for_onoff()
   -> setup_semco_wlan_power_onoff() -> semco_a31_removal()

   The setup_semco_wlan_power function is only used for sleep/wakeup. It controls only 
   the power of A31 module only(Do not card detection/removal function)
*/
   
extern void sdhci_s3c_sdio_card_detect(struct platform_device *pdev);
void semco_a31_detection(void)
{
	sdhci_s3c_sdio_card_detect(&s3c_device_hsmmc3);
}
EXPORT_SYMBOL(semco_a31_detection);


extern void mmc_semco_a31_sdio_remove(void);
void semco_a31_removal(void)
{
	mmc_semco_a31_sdio_remove();
}
EXPORT_SYMBOL(semco_a31_removal);

static struct platform_device s3c_wlan_ar6000_pm_device = {
        .name           = "wlan_ar6000_pm_dev",
        .id             = 1,
        .num_resources  = 0,
        .resource       = NULL,
};

static struct platform_device bt_sysfs = {
        .name = "bt-sysfs",
        .id = -1,
};
// END SEMCO

static struct platform_device *smdk4412_devices[] __initdata = {
	&s3c_device_adc,
};

static struct platform_device *smdk4x12_devices[] __initdata = {
#ifdef CONFIG_ANDROID_PMEM
	&pmem_device,
	&pmem_gpu1_device,
#endif
	/* Samsung Power Domain */
	&exynos4_device_pd[PD_MFC],
	&exynos4_device_pd[PD_G3D],
	&exynos4_device_pd[PD_LCD0],
	&exynos4_device_pd[PD_CAM],
	&exynos4_device_pd[PD_TV],
	&exynos4_device_pd[PD_GPS],
	&exynos4_device_pd[PD_GPS_ALIVE],
#ifdef CONFIG_VIDEO_EXYNOS_FIMC_IS
	&exynos4_device_pd[PD_ISP],
#endif
#ifdef CONFIG_FB_MIPI_DSIM
	&s5p_device_mipi_dsim,
#endif

	&s3c_device_wdt,
	&s3c_device_rtc,

	&s3c_device_i2c1,

	&tc4_regulator_consumer,

// SEMCO
    &s3c_wlan_ar6000_pm_device,
    &bt_sysfs,
// END SEMCO

#ifdef CONFIG_S5P_DEV_MSHC
	&s3c_device_mshci,//lisw sd mshci should be probe before hsmmc
#endif
#ifdef CONFIG_S3C_DEV_HSMMC
//	&s3c_device_hsmmc0,
#endif
#ifdef CONFIG_S3C_DEV_HSMMC1
//	&s3c_device_hsmmc1,
#endif
#ifdef CONFIG_S3C_DEV_HSMMC2
	&s3c_device_hsmmc2,
#endif
#ifdef CONFIG_S3C_DEV_HSMMC3
	&s3c_device_hsmmc3,
#endif

#ifdef CONFIG_S5P_SYSTEM_MMU

#endif /* CONFIG_S5P_SYSTEM_MMU */

#ifdef CONFIG_VIDEO_EXYNOS_FIMC_LITE
	&exynos_device_flite0,
	&exynos_device_flite1,
#endif

#ifdef CONFIG_WAKEUP_ASSIST
	&wakeup_assist_device,
#endif
#ifdef CONFIG_EXYNOS_C2C
	&exynos_device_c2c,
#endif

	&exynos4_busfreq
};

#if defined(CONFIG_S5P_MEM_CMA)
static void __init exynos4_cma_region_reserve(
			struct cma_region *regions_normal,
			struct cma_region *regions_secure)
{
	struct cma_region *reg;
	phys_addr_t paddr_last = 0xFFFFFFFF;

	for (reg = regions_normal; reg->size != 0; reg++) {
		phys_addr_t paddr;

		if (!IS_ALIGNED(reg->size, PAGE_SIZE)) {
			pr_err("S5P/CMA: size of '%s' is NOT page-aligned\n",
								reg->name);
			reg->size = PAGE_ALIGN(reg->size);
		}


		if (reg->reserved) {
			pr_err("S5P/CMA: '%s' alread reserved\n", reg->name);
			continue;
		}

		if (reg->alignment) {
			if ((reg->alignment & ~PAGE_MASK) ||
				(reg->alignment & ~reg->alignment)) {
				pr_err("S5P/CMA: Failed to reserve '%s': "
						"incorrect alignment 0x%08x.\n",
						reg->name, reg->alignment);
				continue;
			}
		} else {
			reg->alignment = PAGE_SIZE;
		}

		if (reg->start) {
			if (!memblock_is_region_reserved(reg->start, reg->size)
			    && (memblock_reserve(reg->start, reg->size) == 0))
				reg->reserved = 1;
			else
				pr_err("S5P/CMA: Failed to reserve '%s'\n",
								reg->name);

			continue;
		}

		paddr = memblock_find_in_range(0, MEMBLOCK_ALLOC_ACCESSIBLE,
						reg->size, reg->alignment);
		if (paddr != MEMBLOCK_ERROR) {
			if (memblock_reserve(paddr, reg->size)) {
				pr_err("S5P/CMA: Failed to reserve '%s'\n",
								reg->name);
				continue;
			}

			reg->start = paddr;
			reg->reserved = 1;
		} else {
			pr_err("S5P/CMA: No free space in memory for '%s'\n",
								reg->name);
		}

		if (cma_early_region_register(reg)) {
			pr_err("S5P/CMA: Failed to register '%s'\n",
								reg->name);
			memblock_free(reg->start, reg->size);
		} else {
			paddr_last = min(paddr, paddr_last);
		}
	}

	if (regions_secure && regions_secure->size) {
		size_t size_secure = 0;
		size_t align_secure, size_region2, aug_size, order_region2;

		for (reg = regions_secure; reg->size != 0; reg++)
			size_secure += reg->size;

		reg--;

		/* Entire secure regions will be merged into 2
		 * consecutive regions. */
		align_secure = 1 <<
			(get_order((size_secure + 1) / 2) + PAGE_SHIFT);
		/* Calculation of a subregion size */
		size_region2 = size_secure - align_secure;
		order_region2 = get_order(size_region2) + PAGE_SHIFT;
		if (order_region2 < 20)
			order_region2 = 20; /* 1MB */
		order_region2 -= 3; /* divide by 8 */
		size_region2 = ALIGN(size_region2, 1 << order_region2);

		aug_size = align_secure + size_region2 - size_secure;
		if (aug_size > 0)
			reg->size += aug_size;

		size_secure = ALIGN(size_secure, align_secure);

		if (paddr_last >= memblock.current_limit) {
			paddr_last = memblock_find_in_range(0,
					MEMBLOCK_ALLOC_ACCESSIBLE,
					size_secure, reg->alignment);
		} else {
			paddr_last -= size_secure;
			paddr_last = round_down(paddr_last, align_secure);
		}

		if (paddr_last) {
			while (memblock_reserve(paddr_last, size_secure))
				paddr_last -= align_secure;

			do {
				reg->start = paddr_last;
				reg->reserved = 1;
				paddr_last += reg->size;

				if (cma_early_region_register(reg)) {
					memblock_free(reg->start, reg->size);
					pr_err("S5P/CMA: "
					"Failed to register secure region "
					"'%s'\n", reg->name);
				} else {
					size_secure -= reg->size;
				}
			} while (reg-- != regions_secure);

			if (size_secure > 0)
				memblock_free(paddr_last, size_secure);
		} else {
			pr_err("S5P/CMA: Failed to reserve secure regions\n");
		}
	}
}

static void __init exynos4_reserve_mem(void)
{
	static struct cma_region regions[] = {
#ifdef CONFIG_ANDROID_PMEM_MEMSIZE_PMEM
		{
			.name = "pmem",
			.size = CONFIG_ANDROID_PMEM_MEMSIZE_PMEM * SZ_1K,
			.start = 0,
		},
#endif
#ifdef CONFIG_ANDROID_PMEM_MEMSIZE_PMEM_GPU1
		{
			.name = "pmem_gpu1",
			.size = CONFIG_ANDROID_PMEM_MEMSIZE_PMEM_GPU1 * SZ_1K,
			.start = 0,
		},
#endif
#ifndef CONFIG_VIDEOBUF2_ION
#ifdef CONFIG_VIDEO_SAMSUNG_MEMSIZE_TV
		{
			.name = "tv",
			.size = CONFIG_VIDEO_SAMSUNG_MEMSIZE_TV * SZ_1K,
			.start = 0
		},
#endif
#ifdef CONFIG_VIDEO_SAMSUNG_MEMSIZE_JPEG
		{
			.name = "jpeg",
			.size = CONFIG_VIDEO_SAMSUNG_MEMSIZE_JPEG * SZ_1K,
			.start = 0
		},
#endif
#ifdef CONFIG_AUDIO_SAMSUNG_MEMSIZE_SRP
		{
			.name = "srp",
			.size = CONFIG_AUDIO_SAMSUNG_MEMSIZE_SRP * SZ_1K,
			.start = 0,
		},
#endif
#ifdef CONFIG_VIDEO_SAMSUNG_MEMSIZE_FIMG2D
		{
			.name = "fimg2d",
			.size = CONFIG_VIDEO_SAMSUNG_MEMSIZE_FIMG2D * SZ_1K,
			.start = 0
		},
#endif
#ifdef CONFIG_VIDEO_SAMSUNG_MEMSIZE_FIMD
		{
			.name = "fimd",
			.size = CONFIG_VIDEO_SAMSUNG_MEMSIZE_FIMD * SZ_1K,
			.start = 0
		},
#endif
#ifdef CONFIG_VIDEO_SAMSUNG_MEMSIZE_FIMC0
		{
			.name = "fimc0",
			.size = CONFIG_VIDEO_SAMSUNG_MEMSIZE_FIMC0 * SZ_1K,
			.start = 0
		},
#endif
#ifdef CONFIG_VIDEO_SAMSUNG_MEMSIZE_FIMC2
		{
			.name = "fimc2",
			.size = CONFIG_VIDEO_SAMSUNG_MEMSIZE_FIMC2 * SZ_1K,
			.start = 0
		},
#endif
#if !defined(CONFIG_EXYNOS4_CONTENT_PATH_PROTECTION) && \
	defined(CONFIG_VIDEO_SAMSUNG_MEMSIZE_FIMC3)
		{
			.name = "fimc3",
			.size = CONFIG_VIDEO_SAMSUNG_MEMSIZE_FIMC3 * SZ_1K,
		},
#endif
#ifdef CONFIG_VIDEO_SAMSUNG_MEMSIZE_FIMC1
		{
			.name = "fimc1",
			.size = CONFIG_VIDEO_SAMSUNG_MEMSIZE_FIMC1 * SZ_1K,
			.start = 0
		},
#endif
#ifdef CONFIG_VIDEO_SAMSUNG_MEMSIZE_MFC1
		{
#ifdef CONFIG_EXYNOS4_CONTENT_PATH_PROTECTION
			.name = "mfc-normal",
#else
			.name = "mfc1",
#endif
			.size = CONFIG_VIDEO_SAMSUNG_MEMSIZE_MFC1 * SZ_1K,
			{ .alignment = 1 << 17 },
		},
#endif
#if !defined(CONFIG_EXYNOS4_CONTENT_PATH_PROTECTION) && \
	defined(CONFIG_VIDEO_SAMSUNG_MEMSIZE_MFC0)
		{
			.name = "mfc0",
			.size = CONFIG_VIDEO_SAMSUNG_MEMSIZE_MFC0 * SZ_1K,
			{ .alignment = 1 << 17 },
		},
#endif
#ifdef CONFIG_VIDEO_SAMSUNG_MEMSIZE_MFC
		{
			.name = "mfc",
			.size = CONFIG_VIDEO_SAMSUNG_MEMSIZE_MFC * SZ_1K,
			{ .alignment = 1 << 17 },
		},
#endif
#ifdef CONFIG_VIDEO_EXYNOS_FIMC_IS
		{
			.name = "fimc_is",
			.size = CONFIG_VIDEO_EXYNOS_MEMSIZE_FIMC_IS * SZ_1K,
			{
				.alignment = 1 << 26,
			},
			.start = 0
		},
#ifdef CONFIG_VIDEO_EXYNOS_FIMC_IS_BAYER
		{
			.name = "fimc_is_isp",
			.size = CONFIG_VIDEO_EXYNOS_MEMSIZE_FIMC_IS_ISP * SZ_1K,
			.start = 0
		},
#endif
#endif
#if !defined(CONFIG_EXYNOS4_CONTENT_PATH_PROTECTION) && \
	defined(CONFIG_VIDEO_SAMSUNG_S5P_MFC)
		{
			.name		= "b2",
			.size		= 32 << 20,
			{ .alignment	= 128 << 10 },
		},
		{
			.name		= "b1",
			.size		= 32 << 20,
			{ .alignment	= 128 << 10 },
		},
		{
			.name		= "fw",
			.size		= 1 << 20,
			{ .alignment	= 128 << 10 },
		},
#endif
#else /* !CONFIG_VIDEOBUF2_ION */
#ifdef CONFIG_FB_S5P
#error CONFIG_FB_S5P is defined. Select CONFIG_FB_S3C, instead
#endif
		{
			.name	= "ion",
			.size	= CONFIG_ION_EXYNOS_CONTIGHEAP_SIZE * SZ_1K,
		},
#endif /* !CONFIG_VIDEOBUF2_ION */
		{
			.size = 0
		},
	};
#ifdef CONFIG_EXYNOS4_CONTENT_PATH_PROTECTION
	static struct cma_region regions_secure[] = {
#ifdef CONFIG_VIDEO_SAMSUNG_MEMSIZE_FIMD_VIDEO
		{
			.name = "video",
			.size = CONFIG_VIDEO_SAMSUNG_MEMSIZE_FIMD_VIDEO * SZ_1K,
		},
#endif
#ifdef CONFIG_VIDEO_SAMSUNG_MEMSIZE_FIMC3
		{
			.name = "fimc3",
			.size = CONFIG_VIDEO_SAMSUNG_MEMSIZE_FIMC3 * SZ_1K,
		},
#endif
#ifdef CONFIG_VIDEO_SAMSUNG_MEMSIZE_MFC0
		{
			.name = "mfc-secure",
			.size = CONFIG_VIDEO_SAMSUNG_MEMSIZE_MFC0 * SZ_1K,
		},
#endif
		{
			.name = "sectbl",
			.size = SZ_1M,
			{
				.alignment = SZ_64M,
			},
		},
		{
			.size = 0
		},
	};
#else /* !CONFIG_EXYNOS4_CONTENT_PATH_PROTECTION */
	struct cma_region *regions_secure = NULL;
#endif
	static const char map[] __initconst =
#ifdef CONFIG_EXYNOS_C2C
		"samsung-c2c=c2c_shdmem;"
#endif
		"android_pmem.0=pmem;android_pmem.1=pmem_gpu1;"
		"s3cfb.0/fimd=fimd;exynos4-fb.0/fimd=fimd;"
#ifdef CONFIG_EXYNOS4_CONTENT_PATH_PROTECTION
		"s3cfb.0/video=video;exynos4-fb.0/video=video;"
#endif
		"s3c-fimc.0=fimc0;s3c-fimc.1=fimc1;s3c-fimc.2=fimc2;s3c-fimc.3=fimc3;"
		"exynos4210-fimc.0=fimc0;exynos4210-fimc.1=fimc1;exynos4210-fimc.2=fimc2;exynos4210-fimc.3=fimc3;"
#ifdef CONFIG_VIDEO_MFC5X
		"s3c-mfc/A=mfc0,mfc-secure;"
		"s3c-mfc/B=mfc1,mfc-normal;"
		"s3c-mfc/AB=mfc;"
#endif
#ifdef CONFIG_VIDEO_SAMSUNG_S5P_MFC
		"s5p-mfc/f=fw;"
		"s5p-mfc/a=b1;"
		"s5p-mfc/b=b2;"
#endif
		"samsung-rp=srp;"
		"s5p-jpeg=jpeg;"
		"exynos4-fimc-is/f=fimc_is;"
#ifdef CONFIG_VIDEO_EXYNOS_FIMC_IS_BAYER
		"exynos4-fimc-is/i=fimc_is_isp;"
#endif
		"s5p-mixer=tv;"
		"s5p-fimg2d=fimg2d;"
		"ion-exynos=ion,fimd,fimc0,fimc1,fimc2,fimc3,fw,b1,b2;"
#ifdef CONFIG_EXYNOS4_CONTENT_PATH_PROTECTION
		"s5p-smem/video=video;"
		"s5p-smem/sectbl=sectbl;"
#endif
		"s5p-smem/mfc=mfc0,mfc-secure;"
		"s5p-smem/fimc=fimc3;"
		"s5p-smem/mfc-shm=mfc1,mfc-normal;";

	cma_set_defaults(NULL, map);

	exynos4_cma_region_reserve(regions, regions_secure);
}
#endif

static void __init smdk4x12_map_io(void)
{
	clk_xusbxti.rate = 24000000;
	s5p_init_io(NULL, 0, S5P_VA_CHIPID);
	s3c24xx_init_clocks(24000000);
	s3c24xx_init_uarts(smdk4x12_uartcfgs, ARRAY_SIZE(smdk4x12_uartcfgs));

#if defined(CONFIG_S5P_MEM_CMA)
	exynos4_reserve_mem();
#endif
}

static void __init smdk4x12_smsc911x_init(void)
{
	u32 cs1;

	/* configure nCS1 width to 16 bits */
	cs1 = __raw_readl(S5P_SROM_BW) &
		~(S5P_SROM_BW__CS_MASK << S5P_SROM_BW__NCS1__SHIFT);
	cs1 |= ((1 << S5P_SROM_BW__DATAWIDTH__SHIFT) |
		(1 << S5P_SROM_BW__WAITENABLE__SHIFT) |
		(1 << S5P_SROM_BW__BYTEENABLE__SHIFT)) <<
		S5P_SROM_BW__NCS1__SHIFT;
	__raw_writel(cs1, S5P_SROM_BW);

	/* set timing for nCS1 suitable for ethernet chip */
	__raw_writel((0x1 << S5P_SROM_BCX__PMC__SHIFT) |
		     (0x9 << S5P_SROM_BCX__TACP__SHIFT) |
		     (0xc << S5P_SROM_BCX__TCAH__SHIFT) |
		     (0x1 << S5P_SROM_BCX__TCOH__SHIFT) |
		     (0x6 << S5P_SROM_BCX__TACC__SHIFT) |
		     (0x1 << S5P_SROM_BCX__TCOS__SHIFT) |
		     (0x1 << S5P_SROM_BCX__TACS__SHIFT), S5P_SROM_BC1);
}

// zsb: wait for i2c5 bus idle before software reset
extern int wait_for_i2c_idle(struct platform_device *pdev);
static void smdk4x12_power_off(void)
{
	int ret = 0;
	
//shengliang
	gpio_set_value(EXYNOS4_GPC0(0), 0);// MD_PWON low
	msleep(10);
	gpio_set_value(EXYNOS4_GPC0(2), 0);// MD_RSTN low
	gpio_set_value(EXYNOS4_GPL2(1), 0);// MD_RESETBB low
	msleep(10);
	#ifdef CONFIG_TC4_EVT
	if (is_charger_online())
	#else
	if(0) //Robin, For TC4 DVT, when usb plug-in, the pmic still can power off...
	#endif
	{    
		//Turn off some LDO&BUCKs, Will Implment later.
		//max8997_pmic_off();//TBD,Robin Wang
		ret = wait_for_i2c_idle(&s3c_device_i2c5);
		if (ret != 0) 
			printk(KERN_EMERG "%s : i2c5 bus is busy.\n", __func__);

		writel(1,S5P_SWRESET);
	}    
	else 
	{    
		
	}    
	while(1);

	printk(KERN_EMERG "%s : should not reach here!\n", __func__);
}

#if 1
extern void (*s3c_config_sleep_gpio_table)(void);
#include <plat/gpio-core.h>

int s3c_gpio_slp_cfgpin(unsigned int pin, unsigned int config)
{
	struct s3c_gpio_chip *chip = s3c_gpiolib_getchip(pin);
	void __iomem *reg;
	unsigned long flags;
	int offset;
	u32 con;
	int shift;

	if (!chip)
		return -EINVAL;

	if ((pin >= EXYNOS4_GPX0(0)) && (pin <= EXYNOS4_GPX3(7)))
		return -EINVAL;

	if (config > S3C_GPIO_SLP_PREV)
		return -EINVAL;

	reg = chip->base + 0x10;

	offset = pin - chip->chip.base;
	shift = offset * 2;

	local_irq_save(flags);

	con = __raw_readl(reg);
	con &= ~(3 << shift);
	con |= config << shift;
	__raw_writel(con, reg);

	local_irq_restore(flags);
	return 0;
}

int s3c_gpio_slp_setpull_updown(unsigned int pin, unsigned int config)
{
	struct s3c_gpio_chip *chip = s3c_gpiolib_getchip(pin);
	void __iomem *reg;
	unsigned long flags;
	int offset;
	u32 con;
	int shift;

	if (!chip)
		return -EINVAL;

	if ((pin >= EXYNOS4_GPX0(0)) && (pin <= EXYNOS4_GPX3(7)))
		return -EINVAL;

	if (config > S3C_GPIO_PULL_UP)
		return -EINVAL;

	reg = chip->base + 0x14;

	offset = pin - chip->chip.base;
	shift = offset * 2;

	local_irq_save(flags);

	con = __raw_readl(reg);
	con &= ~(3 << shift);
	con |= config << shift;
	__raw_writel(con, reg);

	local_irq_restore(flags);

	return 0;
}

static void config_sleep_gpio_table(int array_size, unsigned int (*gpio_table)[3])
{
        u32 i, gpio;

        for (i = 0; i < array_size; i++) {
                gpio = gpio_table[i][0];
                s3c_gpio_slp_cfgpin(gpio, gpio_table[i][1]);
                s3c_gpio_slp_setpull_updown(gpio, gpio_table[i][2]);
        }
}

/*sleep gpio table for TC4*/
static unsigned int tc4_sleep_gpio_table[][3] = {
#if 1 //zhangdong reduce sleep current
	{ EXYNOS4_GPA0(0),  S3C_GPIO_SLP_PREV,	S3C_GPIO_PULL_NONE}, //BT_TXD
	{ EXYNOS4_GPA0(1),  S3C_GPIO_SLP_PREV,	S3C_GPIO_PULL_NONE}, //BT_RXD
	{ EXYNOS4_GPA0(2),  S3C_GPIO_SLP_PREV,	S3C_GPIO_PULL_NONE}, //BT_RTS
	{ EXYNOS4_GPA0(3),  S3C_GPIO_SLP_PREV,	S3C_GPIO_PULL_NONE}, //BT_CTS
	
	{ EXYNOS4_GPA0(4),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE}, //AC100_TXD,SMM6260
	{ EXYNOS4_GPA0(5),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE}, //AC100_RXD
	{ EXYNOS4_GPA0(6),	S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_NONE}, //AC100_RTS
	{ EXYNOS4_GPA0(7),	S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_NONE}, //AC100_CTS


	{ EXYNOS4_GPA1(0),  S3C_GPIO_SLP_PREV,	S3C_GPIO_PULL_NONE}, //DEBUG
	{ EXYNOS4_GPA1(1),  S3C_GPIO_SLP_PREV,	S3C_GPIO_PULL_NONE}, //DEBUG
	{ EXYNOS4_GPA1(2),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},  //I2C_SDA3
	{ EXYNOS4_GPA1(3),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE}, //I2C_SCL3
#ifdef CONFIG_TC4_EVT	
	{ EXYNOS4_GPA1(4),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE}, //TP1_RST
	{ EXYNOS4_GPA1(5),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN}, //TestPoint
#endif
#ifdef CONFIG_TC4_DVT
	{ EXYNOS4_GPA1(4),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE}, //GPS_TXD
	{ EXYNOS4_GPA1(5),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN}, //GPS_RXD
#endif

	{ EXYNOS4_GPB(0),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE}, //I2C_SDA4
	{ EXYNOS4_GPB(1),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE}, //I2C_SCL4
	{ EXYNOS4_GPB(2),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE}, //I2C_SDA5
	{ EXYNOS4_GPB(3),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE}, //I2C_SCL5
	{ EXYNOS4_GPB(4),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN}, //GPS_RST
	{ EXYNOS4_GPB(5),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN}, //PMIC_SET1
	{ EXYNOS4_GPB(6),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN}, //PMIC_SET2
	{ EXYNOS4_GPB(7),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN}, //PMIC_SET3


	{ EXYNOS4_GPC0(0),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN}, //MD_PWON
	{ EXYNOS4_GPC0(1),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN}, //VLED_ON
	{ EXYNOS4_GPC0(2),  S3C_GPIO_SLP_PREV,	S3C_GPIO_PULL_NONE}, //MD_RSTN
	{ EXYNOS4_GPC0(3),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN}, //AP_SLEEP
	{ EXYNOS4_GPC0(4),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN}, //AP_WAKEUP_MD
	
	{ EXYNOS4_GPC1(0),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE}, //UART_SW  config as hp  out1??
	{ EXYNOS4_GPC1(1),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE}, //LED_EN18
	{ EXYNOS4_GPC1(2),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN}, //VLED_EN
	{ EXYNOS4_GPC1(3),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE}, //I2C6_SDA
	{ EXYNOS4_GPC1(4),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE}, //I2C6_SCL

	{ EXYNOS4_GPD0(0),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE}, // MOTOR-PWM
	{ EXYNOS4_GPD0(1),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE}, //XPWMOUT1
	{ EXYNOS4_GPD0(2),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},  //I2C7_SDA
	{ EXYNOS4_GPD0(3),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE}, //I2C7_SCL

	{ EXYNOS4_GPD1(0),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE}, //I2C0_SDA
	{ EXYNOS4_GPD1(1),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE}, //I2C0_SCL
	{ EXYNOS4_GPD1(2),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE}, //I2C1_SDA
	{ EXYNOS4_GPD1(3),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE}, //I2C1_SCL

	{ EXYNOS4_GPF0(0),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},	//LCD_HSYNC
	{ EXYNOS4_GPF0(1),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},	//LCD_VSYNC
	{ EXYNOS4_GPF0(2),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},	//LCD_VDEN
	{ EXYNOS4_GPF0(3),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},	//LCD_VCLK
	{ EXYNOS4_GPF0(4),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//CAM2M_RST
	{ EXYNOS4_GPF0(5),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//CAM2M_PWDN
	{ EXYNOS4_GPF0(6),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},	//LCD_D2
	{ EXYNOS4_GPF0(7),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},	//LCD_D3

	{ EXYNOS4_GPF1(0),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},	//LCD_D4
	{ EXYNOS4_GPF1(1),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},	//LCD_D5
	{ EXYNOS4_GPF1(2),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},	//D6
	{ EXYNOS4_GPF1(3),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},	//D7
	{ EXYNOS4_GPF1(4),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//CAM5M_RST
	{ EXYNOS4_GPF1(5),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//CAM5M_PWDN
	{ EXYNOS4_GPF1(6),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},	//D10
	{ EXYNOS4_GPF1(7),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},	//D11

	{ EXYNOS4_GPF2(0),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},	//D12
	{ EXYNOS4_GPF2(1),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},	//D13
	{ EXYNOS4_GPF2(2),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},	//D14
	{ EXYNOS4_GPF2(3),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},	//D15
	{ EXYNOS4_GPF2(4),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//NC
	{ EXYNOS4_GPF2(5),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//NC
	{ EXYNOS4_GPF2(6),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},	//D18
	{ EXYNOS4_GPF2(7),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},	//D19
	
	{ EXYNOS4_GPF3(0),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},	//D20
	{ EXYNOS4_GPF3(1),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},	//D21
	{ EXYNOS4_GPF3(2),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},	//D22
	{ EXYNOS4_GPF3(3),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},	//D23
	{ EXYNOS4_GPF3(4),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//NC
	{ EXYNOS4_GPF3(5),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//MD_G15

	{ EXYNOS4212_GPJ0(0),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//CAM_CLK
	{ EXYNOS4212_GPJ0(1),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//CAM_VSYNC
	{ EXYNOS4212_GPJ0(2),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//CAM_HREF
	{ EXYNOS4212_GPJ0(3),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//DATA0
	{ EXYNOS4212_GPJ0(4),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//DATA1
	{ EXYNOS4212_GPJ0(5),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//DATA2
	{ EXYNOS4212_GPJ0(6),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//DATA3
	{ EXYNOS4212_GPJ0(7),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//DATA4

	{ EXYNOS4212_GPJ1(0),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//DATA5
	{ EXYNOS4212_GPJ1(1),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//DATA6
	{ EXYNOS4212_GPJ1(2),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//DATA7
	{ EXYNOS4212_GPJ1(3),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},	//CAM_CLK_OUT
	{ EXYNOS4212_GPJ1(4),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//NC

	{ EXYNOS4_GPK0(0),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//eMMC_CLK
	{ EXYNOS4_GPK0(1),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//eMMC_CMD
	{ EXYNOS4_GPK0(2),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//eMMC_CDn
	{ EXYNOS4_GPK0(3),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//eMMC_DATA0
	{ EXYNOS4_GPK0(4),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//eMMC_DATA1
	{ EXYNOS4_GPK0(5),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//eMMC_DATA2
	{ EXYNOS4_GPK0(6),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//eMMC_DATA3
	#ifdef CONFIG_TC4_EVT
	{ EXYNOS4_GPK1(0),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//NC
	{ EXYNOS4_GPK1(1),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//NC
	#endif
	#ifdef CONFIG_TC4_DVT
	{ EXYNOS4_GPK1(0),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//ANX7805_PD
	{ EXYNOS4_GPK1(1),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//VDD50_EN
	#endif
	
	{ EXYNOS4_GPK1(2),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//NC
	{ EXYNOS4_GPK1(3),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//eMMC_DATA4
	{ EXYNOS4_GPK1(4),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//eMMC_DATA5
	{ EXYNOS4_GPK1(5),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//eMMC_DATA6
	{ EXYNOS4_GPK1(6),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//eMMC_DATA7

	{ EXYNOS4_GPK2(0),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//TF_CLK
	{ EXYNOS4_GPK2(1),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//TF_CMD
	{ EXYNOS4_GPK2(2),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//ANX7805_RSTN
	{ EXYNOS4_GPK2(3),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//TF_DATA0
	{ EXYNOS4_GPK2(4),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//TF_DATA1
	{ EXYNOS4_GPK2(5),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//TF_DATA2
	{ EXYNOS4_GPK2(6),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//TF_DATA3

	{ EXYNOS4_GPK3(0),  S3C_GPIO_SLP_PREV,	S3C_GPIO_PULL_NONE},	//WIFI_CLK
	{ EXYNOS4_GPK3(1),  S3C_GPIO_SLP_PREV,	S3C_GPIO_PULL_NONE},	//WIFI_CMD
	{ EXYNOS4_GPK3(2),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//HUB_CONNECT
	{ EXYNOS4_GPK3(3),  S3C_GPIO_SLP_PREV,	S3C_GPIO_PULL_NONE},	//WIFI_DATA0
	{ EXYNOS4_GPK3(4),  S3C_GPIO_SLP_PREV,	S3C_GPIO_PULL_NONE},	//WIFI_DATA1
	{ EXYNOS4_GPK3(5),  S3C_GPIO_SLP_PREV,	S3C_GPIO_PULL_NONE},	//WIFI_DATA2
	{ EXYNOS4_GPK3(6),  S3C_GPIO_SLP_PREV,	S3C_GPIO_PULL_NONE},	//WIFI_DATA3

	{ EXYNOS4_GPL0(0),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//BUCK6_EN
	#ifdef CONFIG_TC4_EVT
	{ EXYNOS4_GPL0(1),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//VDD50_EN
	#endif
	#ifdef CONFIG_TC4_DVT
	{ EXYNOS4_GPL0(1),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//6260_GPIO3
	#endif
	
	{ EXYNOS4_GPL0(2), /* S3C_GPIO_SLP_PREV*/S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},	//TP1_EN
	{ EXYNOS4_GPL0(3),  S3C_GPIO_SLP_OUT1,	S3C_GPIO_PULL_NONE},	//NFC_EN1  out1
	
	{ EXYNOS4_GPL0(4),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//CHG_EN
	{ EXYNOS4_GPL0(5),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//NO THIS PIN
	{ EXYNOS4_GPL0(6),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//HDMI_IIC_EN
	{ EXYNOS4_GPL0(7),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//NC

	{ EXYNOS4_GPL1(0),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE}, 	//LVDS_PWDN  out0
	{ EXYNOS4_GPL1(1),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//NC
	{ EXYNOS4_GPL1(2),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//NC

	{ EXYNOS4_GPL2(0),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//KP_COL0
	{ EXYNOS4_GPL2(1),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//MD_RESETBB
	{ EXYNOS4_GPL2(2),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//HUB_RESET
	{ EXYNOS4_GPL2(3),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//NFC_SCL
	{ EXYNOS4_GPL2(4),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//NFC_SDA
	{ EXYNOS4_GPL2(5),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//NFC_GPIO4
	#ifdef CONFIG_TC4_EVT
	{ EXYNOS4_GPL2(6),	S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//ANX7805_PWON
	{ EXYNOS4_GPL2(7),	S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//TP
	#endif

	#ifdef CONFIG_TC4_DVT
	//GPM4(2) --ISP_SCL1
	//GPM4(3)--ISP_SDA1
	//GPM3(5)--PMIC_DS2
	//GPM3(6)--PMIC_DS3
	//GPM3(7)--PMIC_DS4
	#endif
	

#if 1
	{ EXYNOS4_GPY0(0),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//NC
	{ EXYNOS4_GPY0(1),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//NC

	{ EXYNOS4_GPY0(2),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY0(3),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY0(4),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY0(5),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	/* NC */

	{ EXYNOS4_GPY1(0),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY1(1),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY1(2),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY1(3),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	/* NC */

	{ EXYNOS4_GPY2(0),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY2(1),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY2(2),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY2(3),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY2(4),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY2(5),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	/* NC */

	{ EXYNOS4_GPY3(0),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//NC
       { EXYNOS4_GPY3(1),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//NC
	{ EXYNOS4_GPY3(2),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	/* MHL_SCL_1.8V */
	{ EXYNOS4_GPY3(3),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ EXYNOS4_GPY3(4),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ EXYNOS4_GPY3(5),  S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ EXYNOS4_GPY3(6),  S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ EXYNOS4_GPY3(7),  S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},

	{ EXYNOS4_GPY4(0),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ EXYNOS4_GPY4(1),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ EXYNOS4_GPY4(2),  S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN},
	{ EXYNOS4_GPY4(3),  S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ EXYNOS4_GPY4(4),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY4(5),  S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ EXYNOS4_GPY4(6),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ EXYNOS4_GPY4(7),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},

	{ EXYNOS4_GPY5(0),  S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY5(1),  S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY5(2),  S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY5(3),  S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY5(4),  S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY5(5),  S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY5(6),  S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY5(7),  S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN},	/* NC */

	{ EXYNOS4_GPY6(0),  S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY6(1),  S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY6(2),  S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY6(3),  S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY6(4),  S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY6(5),  S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY6(6),  S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY6(7),  S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN},	/* NC */
#endif 
	{ EXYNOS4_GPZ(0),  S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN},	//I2S0_SCLK
	{ EXYNOS4_GPZ(1),  S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN},	//I2S0_CDCLK
	{ EXYNOS4_GPZ(2),  S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN}, //I2S0_LRCK
	{ EXYNOS4_GPZ(3),  S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN}, //I2S0_SDI
	{ EXYNOS4_GPZ(4),  S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN}, //I2S0_SDO0
	{ EXYNOS4_GPZ(5),  S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN}, //WIFI_PWDN
	{ EXYNOS4_GPZ(6),  S3C_GPIO_SLP_PREV,	S3C_GPIO_PULL_NONE},	//BT_RST

#else
	{ EXYNOS4_GPA0(0),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN}, //BT_TXD
	{ EXYNOS4_GPA0(1),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN}, //BT_RXD
	{ EXYNOS4_GPA0(2),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN}, //BT_RTS
	{ EXYNOS4_GPA0(3),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN}, //BT_CTS
	
	{ EXYNOS4_GPA0(4),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE}, //AC100_TXD,SMM6260
	{ EXYNOS4_GPA0(5),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE}, //AC100_RXD
	{ EXYNOS4_GPA0(6),	S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_NONE}, //AC100_RTS
	{ EXYNOS4_GPA0(7),	S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_NONE}, //AC100_CTS


	{ EXYNOS4_GPA1(0),  S3C_GPIO_SLP_PREV,	S3C_GPIO_PULL_NONE}, //DEBUG
	{ EXYNOS4_GPA1(1),  S3C_GPIO_SLP_PREV,	S3C_GPIO_PULL_NONE}, //DEBUG
	{ EXYNOS4_GPA1(2),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},  //I2C_SDA3
	{ EXYNOS4_GPA1(3),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE}, //I2C_SCL3
#ifdef CONFIG_TC4_EVT	
	{ EXYNOS4_GPA1(4),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE}, //TP1_RST
	{ EXYNOS4_GPA1(5),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN}, //TestPoint
#endif
#ifdef CONFIG_TC4_DVT
	{ EXYNOS4_GPA1(4),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE}, //GPS_TXD
	{ EXYNOS4_GPA1(5),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN}, //GPS_RXD
#endif

	{ EXYNOS4_GPB(0),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE}, //I2C_SDA4
	{ EXYNOS4_GPB(1),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE}, //I2C_SCL4
	{ EXYNOS4_GPB(2),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE}, //I2C_SDA5
	{ EXYNOS4_GPB(3),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE}, //I2C_SCL5
	{ EXYNOS4_GPB(4),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN}, //GPS_RST
	{ EXYNOS4_GPB(5),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE}, //PMIC_SET1
	{ EXYNOS4_GPB(6),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE}, //PMIC_SET2
	{ EXYNOS4_GPB(7),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE}, //PMIC_SET3


	{ EXYNOS4_GPC0(0),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN}, //MD_PWON
	{ EXYNOS4_GPC0(1),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN}, //VLED_ON
	{ EXYNOS4_GPC0(2),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN}, //MD_RSTN
	{ EXYNOS4_GPC0(3),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE}, //AP_SLEEP
	{ EXYNOS4_GPC0(4),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE}, //AP_WAKEUP_MD
	
	{ EXYNOS4_GPC1(0),  S3C_GPIO_SLP_PREV,  S3C_GPIO_PULL_NONE}, //UART_SW  config as hp  out1??
	{ EXYNOS4_GPC1(1),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE}, //LED_EN18
	{ EXYNOS4_GPC1(2),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN}, //VLED_EN
	{ EXYNOS4_GPC1(3),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE}, //I2C6_SDA
	{ EXYNOS4_GPC1(4),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE}, //I2C6_SCL

	{ EXYNOS4_GPD0(0),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE}, // MOTOR-PWM
	{ EXYNOS4_GPD0(1),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE}, //XPWMOUT1
	{ EXYNOS4_GPD0(2),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},  //I2C7_SDA
	{ EXYNOS4_GPD0(3),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE}, //I2C7_SCL

	{ EXYNOS4_GPD1(0),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE}, //I2C0_SDA
	{ EXYNOS4_GPD1(1),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE}, //I2C0_SCL
	{ EXYNOS4_GPD1(2),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE}, //I2C1_SDA
	{ EXYNOS4_GPD1(3),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE}, //I2C1_SCL

	{ EXYNOS4_GPF0(0),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},	//LCD_HSYNC
	{ EXYNOS4_GPF0(1),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},	//LCD_VSYNC
	{ EXYNOS4_GPF0(2),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},	//LCD_VDEN
	{ EXYNOS4_GPF0(3),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},	//LCD_VCLK
	{ EXYNOS4_GPF0(4),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//CAM2M_RST
	{ EXYNOS4_GPF0(5),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//CAM2M_PWDN
	{ EXYNOS4_GPF0(6),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},	//LCD_D2
	{ EXYNOS4_GPF0(7),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},	//LCD_D3

	{ EXYNOS4_GPF1(0),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},	//LCD_D4
	{ EXYNOS4_GPF1(1),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},	//LCD_D5
	{ EXYNOS4_GPF1(2),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},	//D6
	{ EXYNOS4_GPF1(3),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},	//D7
	{ EXYNOS4_GPF1(4),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//CAM5M_RST
	{ EXYNOS4_GPF1(5),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//CAM5M_PWDN
	{ EXYNOS4_GPF1(6),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},	//D10
	{ EXYNOS4_GPF1(7),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},	//D11

	{ EXYNOS4_GPF2(0),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},	//D12
	{ EXYNOS4_GPF2(1),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},	//D13
	{ EXYNOS4_GPF2(2),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},	//D14
	{ EXYNOS4_GPF2(3),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},	//D15
	{ EXYNOS4_GPF2(4),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//NC
	{ EXYNOS4_GPF2(5),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//NC
	{ EXYNOS4_GPF2(6),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},	//D18
	{ EXYNOS4_GPF2(7),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},	//D19
	
	{ EXYNOS4_GPF3(0),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},	//D20
	{ EXYNOS4_GPF3(1),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},	//D21
	{ EXYNOS4_GPF3(2),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},	//D22
	{ EXYNOS4_GPF3(3),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},	//D23
	{ EXYNOS4_GPF3(4),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//NC
	{ EXYNOS4_GPF3(5),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//MD_G15

	{ EXYNOS4212_GPJ0(0),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//CAM_CLK
	{ EXYNOS4212_GPJ0(1),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//CAM_VSYNC
	{ EXYNOS4212_GPJ0(2),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//CAM_HREF
	{ EXYNOS4212_GPJ0(3),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//DATA0
	{ EXYNOS4212_GPJ0(4),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//DATA1
	{ EXYNOS4212_GPJ0(5),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//DATA2
	{ EXYNOS4212_GPJ0(6),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//DATA3
	{ EXYNOS4212_GPJ0(7),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//DATA4

	{ EXYNOS4212_GPJ1(0),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//DATA5
	{ EXYNOS4212_GPJ1(1),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//DATA6
	{ EXYNOS4212_GPJ1(2),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//DATA7
	{ EXYNOS4212_GPJ1(3),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},	//CAM_CLK_OUT
	{ EXYNOS4212_GPJ1(4),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//NC

	{ EXYNOS4_GPK0(0),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//eMMC_CLK
	{ EXYNOS4_GPK0(1),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//eMMC_CMD
	{ EXYNOS4_GPK0(2),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//eMMC_CDn
	{ EXYNOS4_GPK0(3),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//eMMC_DATA0
	{ EXYNOS4_GPK0(4),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//eMMC_DATA1
	{ EXYNOS4_GPK0(5),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//eMMC_DATA2
	{ EXYNOS4_GPK0(6),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//eMMC_DATA3
	#ifdef CONFIG_TC4_EVT
	{ EXYNOS4_GPK1(0),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//NC
	{ EXYNOS4_GPK1(1),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//NC
	#endif
	#ifdef CONFIG_TC4_DVT
	{ EXYNOS4_GPK1(0),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//ANX7805_PD
	{ EXYNOS4_GPK1(1),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//VDD50_EN
	#endif
	
	{ EXYNOS4_GPK1(2),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//NC
	{ EXYNOS4_GPK1(3),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//eMMC_DATA4
	{ EXYNOS4_GPK1(4),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//eMMC_DATA5
	{ EXYNOS4_GPK1(5),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//eMMC_DATA6
	{ EXYNOS4_GPK1(6),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//eMMC_DATA7

	{ EXYNOS4_GPK2(0),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//TF_CLK
	{ EXYNOS4_GPK2(1),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//TF_CMD
	{ EXYNOS4_GPK2(2),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//ANX7805_RSTN
	{ EXYNOS4_GPK2(3),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//TF_DATA0
	{ EXYNOS4_GPK2(4),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//TF_DATA1
	{ EXYNOS4_GPK2(5),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//TF_DATA2
	{ EXYNOS4_GPK2(6),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//TF_DATA3

	{ EXYNOS4_GPK3(0),  S3C_GPIO_SLP_PREV,	S3C_GPIO_PULL_NONE},	//WIFI_CLK
	{ EXYNOS4_GPK3(1),  S3C_GPIO_SLP_PREV,	S3C_GPIO_PULL_NONE},	//WIFI_CMD
	{ EXYNOS4_GPK3(2),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//HUB_CONNECT
	{ EXYNOS4_GPK3(3),  S3C_GPIO_SLP_PREV,	S3C_GPIO_PULL_NONE},	//WIFI_DATA0
	{ EXYNOS4_GPK3(4),  S3C_GPIO_SLP_PREV,	S3C_GPIO_PULL_NONE},	//WIFI_DATA1
	{ EXYNOS4_GPK3(5),  S3C_GPIO_SLP_PREV,	S3C_GPIO_PULL_NONE},	//WIFI_DATA2
	{ EXYNOS4_GPK3(6),  S3C_GPIO_SLP_PREV,	S3C_GPIO_PULL_NONE},	//WIFI_DATA3

	{ EXYNOS4_GPL0(0),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//BUCK6_EN
	#ifdef CONFIG_TC4_EVT
	{ EXYNOS4_GPL0(1),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//VDD50_EN
	#endif
	#ifdef CONFIG_TC4_DVT
	{ EXYNOS4_GPL0(1),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//6260_GPIO3
	#endif
	
	{ EXYNOS4_GPL0(2),  S3C_GPIO_SLP_PREV,	S3C_GPIO_PULL_NONE},	//TP1_EN
	{ EXYNOS4_GPL0(3),  S3C_GPIO_SLP_OUT1,	S3C_GPIO_PULL_NONE},	//NFC_EN1  out1
	
	{ EXYNOS4_GPL0(4),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//CHG_EN
	{ EXYNOS4_GPL0(5),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//NO THIS PIN
	{ EXYNOS4_GPL0(6),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//HDMI_IIC_EN
	{ EXYNOS4_GPL0(7),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//NC

	{ EXYNOS4_GPL1(0),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE}, 	//LVDS_PWDN  out0
	{ EXYNOS4_GPL1(1),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//NC
	{ EXYNOS4_GPL1(2),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//NC

	{ EXYNOS4_GPL2(0),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//KP_COL0
	{ EXYNOS4_GPL2(1),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//MD_RESETBB
	{ EXYNOS4_GPL2(2),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//HUB_RESET
	{ EXYNOS4_GPL2(3),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//NFC_SCL
	{ EXYNOS4_GPL2(4),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},	//NFC_SDA
	{ EXYNOS4_GPL2(5),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//NFC_GPIO4
	#ifdef CONFIG_TC4_EVT
	{ EXYNOS4_GPL2(6),	S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//ANX7805_PWON
	{ EXYNOS4_GPL2(7),	S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//TP
	#endif

	#ifdef CONFIG_TC4_DVT
	//GPM4(2) --ISP_SCL1
	//GPM4(3)--ISP_SDA1
	//GPM3(5)--PMIC_DS2
	//GPM3(6)--PMIC_DS3
	//GPM3(7)--PMIC_DS4
	#endif
	

#if 1
	{ EXYNOS4_GPY0(0),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//NC
	{ EXYNOS4_GPY0(1),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//NC

	{ EXYNOS4_GPY0(2),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY0(3),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY0(4),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY0(5),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	/* NC */

	{ EXYNOS4_GPY1(0),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY1(1),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY1(2),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY1(3),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	/* NC */

	{ EXYNOS4_GPY2(0),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY2(1),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY2(2),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY2(3),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY2(4),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY2(5),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	/* NC */

	{ EXYNOS4_GPY3(0),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//NC
       { EXYNOS4_GPY3(1),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	//NC
	{ EXYNOS4_GPY3(2),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	/* MHL_SCL_1.8V */
	{ EXYNOS4_GPY3(3),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ EXYNOS4_GPY3(4),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ EXYNOS4_GPY3(5),  S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ EXYNOS4_GPY3(6),  S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ EXYNOS4_GPY3(7),  S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},

	{ EXYNOS4_GPY4(0),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ EXYNOS4_GPY4(1),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ EXYNOS4_GPY4(2),  S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN},
	{ EXYNOS4_GPY4(3),  S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ EXYNOS4_GPY4(4),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY4(5),  S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ EXYNOS4_GPY4(6),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ EXYNOS4_GPY4(7),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},

	{ EXYNOS4_GPY5(0),  S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY5(1),  S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY5(2),  S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY5(3),  S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY5(4),  S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY5(5),  S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY5(6),  S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY5(7),  S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN},	/* NC */

	{ EXYNOS4_GPY6(0),  S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY6(1),  S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY6(2),  S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY6(3),  S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY6(4),  S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY6(5),  S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY6(6),  S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN},	/* NC */
	{ EXYNOS4_GPY6(7),  S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN},	/* NC */
#endif 
	{ EXYNOS4_GPZ(0),  S3C_GPIO_SLP_PREV,  S3C_GPIO_PULL_NONE},	//I2S0_SCLK
	{ EXYNOS4_GPZ(1),  S3C_GPIO_SLP_PREV, S3C_GPIO_PULL_NONE},	//I2S0_CDCLK
	{ EXYNOS4_GPZ(2),  S3C_GPIO_SLP_PREV,  S3C_GPIO_PULL_NONE}, //I2S0_LRCK
	{ EXYNOS4_GPZ(3),  S3C_GPIO_SLP_PREV,  S3C_GPIO_PULL_NONE}, //I2S0_SDI
	{ EXYNOS4_GPZ(4),  S3C_GPIO_SLP_PREV,  S3C_GPIO_PULL_NONE}, //I2S0_SDO0
	{ EXYNOS4_GPZ(5),  S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN}, //WIFI_PWDN
	{ EXYNOS4_GPZ(6),  S3C_GPIO_SLP_INPUT, S3C_GPIO_PULL_DOWN},	//BT_RST
#endif
};
#ifdef CONFIG_TC4_DVT
	//GPX0(0) ---NC
	//GPX2(6) --6260_GPIO1
	//GPX3(2) --6260_GPIO2
#endif
static unsigned int tc4_sleep_alive_gpio_table[][4] =
{//ly 20111118 modified it for eint wakeup
	{EXYNOS4_GPX0(0), S3C_GPIO_INPUT,	S3C_GPIO_SETPIN_NONE, S3C_GPIO_PULL_NONE}, //ANX7805_INIT
	{EXYNOS4_GPX0(1), S3C_GPIO_SLP_OUT1,	S3C_GPIO_SETPIN_NONE, S3C_GPIO_PULL_NONE}, //ANX7805_PD
	//{EXYNOS4_GPX0(2), S3C_GPIO_OUTPUT,	S3C_GPIO_SETPIN_NONE, S3C_GPIO_PULL_NONE}, //ONO
	{EXYNOS4_GPX0(3), S3C_GPIO_INPUT,	S3C_GPIO_SETPIN_NONE, S3C_GPIO_PULL_NONE},	//CABLE_DET
	{EXYNOS4_GPX0(4), S3C_GPIO_INPUT,	S3C_GPIO_SETPIN_NONE, S3C_GPIO_PULL_NONE},	//TP1_INT
	{EXYNOS4_GPX0(5), S3C_GPIO_INPUT,	S3C_GPIO_SETPIN_NONE, S3C_GPIO_PULL_NONE},	//PIX_SDA
	{EXYNOS4_GPX0(6), S3C_GPIO_INPUT,	S3C_GPIO_SETPIN_NONE, S3C_GPIO_PULL_NONE}, //PIX_SCL
	//{EXYNOS4_GPX0(7), S3C_GPIO_SFN(0xf),	S3C_GPIO_SETPIN_NONE, S3C_GPIO_PULL_NONE}, //TF_CDN

	{EXYNOS4_GPX1(0), S3C_GPIO_INPUT,	S3C_GPIO_SETPIN_NONE, S3C_GPIO_PULL_NONE},  //CHG_FLT
	{EXYNOS4_GPX1(1), S3C_GPIO_INPUT,	S3C_GPIO_SETPIN_NONE, S3C_GPIO_PULL_NONE},	//NFC_INT
	{EXYNOS4_GPX1(2), S3C_GPIO_INPUT,	S3C_GPIO_SETPIN_NONE, S3C_GPIO_PULL_NONE},	//KP_LED
	{EXYNOS4_GPX1(3), S3C_GPIO_INPUT,	S3C_GPIO_SETPIN_NONE, S3C_GPIO_PULL_NONE}, //HOOK_DET
	{EXYNOS4_GPX1(4), S3C_GPIO_INPUT,	S3C_GPIO_SETPIN_NONE, S3C_GPIO_PULL_NONE},	//WIFI_WOW
	//{EXYNOS4_GPX1(5), S3C_GPIO_SFN(0xf),	S3C_GPIO_SETPIN_NONE, S3C_GPIO_PULL_NONE},	//CHG_UOK
	{EXYNOS4_GPX1(6), S3C_GPIO_INPUT,	S3C_GPIO_SETPIN_NONE, S3C_GPIO_PULL_NONE},	//MD_SLEEP_REQUEST
	{EXYNOS4_GPX1(7), S3C_GPIO_INPUT,	S3C_GPIO_SETPIN_NONE, S3C_GPIO_PULL_NONE}, //IRQ_PMIC


	{EXYNOS4_GPX2(0), S3C_GPIO_INPUT,	S3C_GPIO_SETPIN_NONE, S3C_GPIO_PULL_NONE},	//KP_ROW0
	{EXYNOS4_GPX2(1), S3C_GPIO_INPUT,	S3C_GPIO_SETPIN_NONE, S3C_GPIO_PULL_NONE},	//KP_ROW1
	{EXYNOS4_GPX2(2), S3C_GPIO_INPUT,	S3C_GPIO_SETPIN_NONE, S3C_GPIO_PULL_NONE},	//HP_DET
	{EXYNOS4_GPX2(3), S3C_GPIO_INPUT,	S3C_GPIO_SETPIN_NONE, S3C_GPIO_PULL_NONE},	//HUB_INIT_N
	{EXYNOS4_GPX2(4), S3C_GPIO_INPUT,	S3C_GPIO_SETPIN_NONE, S3C_GPIO_PULL_NONE}, //MD_B14
	//{EXYNOS4_GPX2(5), S3C_GPIO_SFN(0xf),	S3C_GPIO_SETPIN_NONE, S3C_GPIO_PULL_NONE}, //MD_WAKEUP_AP
	//{EXYNOS4_GPX2(6), S3C_GPIO_SFN(0xf),	S3C_GPIO_SETPIN_NONE, S3C_GPIO_PULL_NONE}, //CHG_DOK
	{EXYNOS4_GPX2(7), S3C_GPIO_INPUT,	S3C_GPIO_SETPIN_NONE, S3C_GPIO_PULL_NONE}, //CHG_COK

	{EXYNOS4_GPX3(0), S3C_GPIO_INPUT,	S3C_GPIO_SETPIN_NONE, S3C_GPIO_PULL_NONE},	//GM_INT1
	{EXYNOS4_GPX3(1), S3C_GPIO_INPUT,	S3C_GPIO_SETPIN_NONE, S3C_GPIO_PULL_NONE}, //GM_INI2
	//{EXYNOS4_GPX3(2), S3C_GPIO_SFN(0xf),	S3C_GPIO_SETPIN_NONE, S3C_GPIO_PULL_NONE}, //FUEL_ALRT
	{EXYNOS4_GPX3(3), S3C_GPIO_INPUT,	S3C_GPIO_SETPIN_NONE, S3C_GPIO_PULL_NONE},	//GYRO_INT
	{EXYNOS4_GPX3(4), S3C_GPIO_INPUT,	S3C_GPIO_SETPIN_NONE, S3C_GPIO_PULL_NONE}, //COMPASS_RDY
	{EXYNOS4_GPX3(5), S3C_GPIO_INPUT,	S3C_GPIO_SETPIN_NONE, S3C_GPIO_PULL_NONE}, //MD_L15
	{EXYNOS4_GPX3(6), S3C_GPIO_INPUT,	S3C_GPIO_SETPIN_NONE, S3C_GPIO_PULL_NONE}, //HDMI_CEC
	{EXYNOS4_GPX3(7), S3C_GPIO_INPUT,	S3C_GPIO_SETPIN_NONE, S3C_GPIO_PULL_NONE},	//HDMI_HPD

};


//added by yulu
static void tc4_config_sleep_gpio_table(void)
{
	int i,gpio;
/*
	[Fix Me] Below codes are sample GPIO initialization and review codes
	for target platform if needed.
*/
#if 0
	for (i = 0; i < ARRAY_SIZE(tc4_sleep_alive_gpio_table); i++)
	  {
		  gpio = tc4_sleep_alive_gpio_table[i][0];
	
		  s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(tc4_sleep_alive_gpio_table[i][1]));
		  if (tc4_sleep_alive_gpio_table[i][2] != S3C_GPIO_SETPIN_NONE)
		  {
			  gpio_set_value(gpio, tc4_sleep_alive_gpio_table[i][2]);
		  }
		  s3c_gpio_setpull(gpio, tc4_sleep_alive_gpio_table[i][3]);
	  }

	
#endif
	config_sleep_gpio_table(ARRAY_SIZE(tc4_sleep_gpio_table),
			tc4_sleep_gpio_table); ///yulu
}

#endif
#define SMDK4412_REV_0_0_ADC_VALUE 0
#define SMDK4412_REV_0_1_ADC_VALUE 443
int samsung_board_rev;

static int get_samsung_board_rev(void)
{
	int		ret = 0;
#if 0 //It's only for smdk
	int 		adc_val = 0;
	struct clk	*adc_clk;
	struct resource	*res;
	void __iomem	*adc_regs;
	unsigned int	con;
	int		ret;

	if ((soc_is_exynos4412() && samsung_rev() < EXYNOS4412_REV_1_0) ||
		(soc_is_exynos4212() && samsung_rev() < EXYNOS4212_REV_1_0))
		return SAMSUNG_BOARD_REV_0_0;

	adc_clk = clk_get(NULL, "adc");
	if (unlikely(IS_ERR(adc_clk)))
		return SAMSUNG_BOARD_REV_0_0;

	clk_enable(adc_clk);

	res = platform_get_resource(&s3c_device_adc, IORESOURCE_MEM, 0);
	if (unlikely(!res))
		goto err_clk;

	adc_regs = ioremap(res->start, resource_size(res));
	if (unlikely(!adc_regs))
		goto err_clk;

	writel(S5PV210_ADCCON_SELMUX(3), adc_regs + S5PV210_ADCMUX);

	con = readl(adc_regs + S3C2410_ADCCON);
	con &= ~S3C2410_ADCCON_MUXMASK;
	con &= ~S3C2410_ADCCON_STDBM;
	con &= ~S3C2410_ADCCON_STARTMASK;
	con |=  S3C2410_ADCCON_PRSCEN;

	con |= S3C2410_ADCCON_ENABLE_START;
	writel(con, adc_regs + S3C2410_ADCCON);

	udelay (50);

	adc_val = readl(adc_regs + S3C2410_ADCDAT0) & 0xFFF;
	writel(0, adc_regs + S3C64XX_ADCCLRINT);

	iounmap(adc_regs);
err_clk:
	clk_disable(adc_clk);
	clk_put(adc_clk);

	ret = (adc_val < SMDK4412_REV_0_1_ADC_VALUE/2) ?
			SAMSUNG_BOARD_REV_0_0 : SAMSUNG_BOARD_REV_0_1;

	pr_info ("SMDK MAIN Board Rev 0.%d (ADC value:%d)\n", ret, adc_val);
#endif
	return ret;
}

static void __init smdk4x12_machine_init(void)
{
	pm_power_off = smdk4x12_power_off;
	s3c_config_sleep_gpio_table = tc4_config_sleep_gpio_table;
	samsung_board_rev = get_samsung_board_rev();

#if defined(CONFIG_EXYNOS_DEV_PD) && defined(CONFIG_PM_RUNTIME)
	exynos_pd_disable(&exynos4_device_pd[PD_MFC].dev);
	exynos_pd_disable(&exynos4_device_pd[PD_G3D].dev);
	exynos_pd_disable(&exynos4_device_pd[PD_LCD0].dev);
	exynos_pd_disable(&exynos4_device_pd[PD_CAM].dev);
	exynos_pd_disable(&exynos4_device_pd[PD_TV].dev);
	exynos_pd_disable(&exynos4_device_pd[PD_GPS].dev);
	exynos_pd_disable(&exynos4_device_pd[PD_GPS_ALIVE].dev);
	exynos_pd_disable(&exynos4_device_pd[PD_ISP].dev);
#elif defined(CONFIG_EXYNOS_DEV_PD)
	/*
	 * These power domains should be always on
	 * without runtime pm support.
	 */
	exynos_pd_enable(&exynos4_device_pd[PD_MFC].dev);
	exynos_pd_enable(&exynos4_device_pd[PD_G3D].dev);
	exynos_pd_enable(&exynos4_device_pd[PD_LCD0].dev);
	exynos_pd_enable(&exynos4_device_pd[PD_CAM].dev);
	exynos_pd_enable(&exynos4_device_pd[PD_TV].dev);
	exynos_pd_enable(&exynos4_device_pd[PD_GPS].dev);
	exynos_pd_enable(&exynos4_device_pd[PD_GPS_ALIVE].dev);
	exynos_pd_enable(&exynos4_device_pd[PD_ISP].dev);
#endif

	s3c_i2c1_set_platdata(NULL);
	i2c_register_board_info(1, i2c_devs1, ARRAY_SIZE(i2c_devs1));

#ifdef CONFIG_ANDROID_PMEM
	android_pmem_set_platdata();
#endif
#ifdef CONFIG_S3C_DEV_HSMMC
	s3c_sdhci0_set_platdata(&smdk4x12_hsmmc0_pdata);
#endif
#ifdef CONFIG_S3C_DEV_HSMMC1
	s3c_sdhci1_set_platdata(&smdk4x12_hsmmc1_pdata);
#endif
#ifdef CONFIG_S3C_DEV_HSMMC2
	s3c_sdhci2_set_platdata(&smdk4x12_hsmmc2_pdata);
#endif
#ifdef CONFIG_S3C_DEV_HSMMC3
	s3c_sdhci3_set_platdata(&smdk4x12_hsmmc3_pdata);
#endif
#ifdef CONFIG_S5P_DEV_MSHC
	s3c_mshci_set_platdata(&exynos4_mshc_pdata);
#endif

	s3c_fimc3_set_platdata(NULL);

#ifdef CONFIG_EXYNOS4_CONTENT_PATH_PROTECTION
	secmem.parent = &exynos4_device_pd[PD_CAM].dev;
#endif

#ifdef CONFIG_BUSFREQ_OPP
	dev_add(&busfreq, &exynos4_busfreq.dev);
	ppmu_init(&exynos_ppmu[PPMU_DMC0], &exynos4_busfreq.dev);
	ppmu_init(&exynos_ppmu[PPMU_DMC1], &exynos4_busfreq.dev);
	ppmu_init(&exynos_ppmu[PPMU_CPU], &exynos4_busfreq.dev);
#endif
	register_reboot_notifier(&exynos4_reboot_notifier);
}

#ifdef CONFIG_EXYNOS_C2C
static void __init exynos_c2c_reserve(void)
{
	static struct cma_region region = {
			.name = "c2c_shdmem",
			.size = 64 * SZ_1M,
			{ .alignment	= 64 * SZ_1M },
			.start = C2C_SHAREDMEM_BASE
	};

	BUG_ON(cma_early_region_register(&region));
	BUG_ON(cma_early_region_reserve(&region));
}
#endif

#ifdef CONFIG_TC4_ICS
MACHINE_START(SMDK4212, "SMDK4X12")
	.boot_params	= S5P_PA_SDRAM + 0x100,
	.init_irq	= exynos4_init_irq,
	.map_io		= smdk4x12_map_io,
	.init_machine	= smdk4x12_machine_init,
	.timer		= &exynos4_timer,
	#if defined(CONFIG_KERNEL_PANIC_DUMP)		//mj for panic-dump
	.reserve		= reserve_panic_dump_area,
	#endif

#ifdef CONFIG_EXYNOS_C2C
	.reserve	= &exynos_c2c_reserve,
#endif
MACHINE_END

MACHINE_START(SMDK4412, "SMDK4X12")
	.boot_params	= S5P_PA_SDRAM + 0x100,
	.init_irq	= exynos4_init_irq,
	.map_io		= smdk4x12_map_io,
	.init_machine	= smdk4x12_machine_init,
	.timer		= &exynos4_timer,

	#if defined(CONFIG_KERNEL_PANIC_DUMP)		//mj for panic-dump
	.reserve		= reserve_panic_dump_area,
	#endif

#ifdef CONFIG_EXYNOS_C2C
	.reserve	= &exynos_c2c_reserve,
#endif
MACHINE_END
#endif

