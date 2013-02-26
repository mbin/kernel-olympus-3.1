/*
 * arch/arm/mach-tegra/board-olympus-i2c.c
 *
 * Copyright (C) 2010 Google, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/resource.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/i2c-tegra.h>
#include <asm/mach-types.h>
#include <mach/irqs.h>
#include <mach/iomap.h>
#include <mach/pinmux.h>
#include <linux/qtouch_obp_ts.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/leds-lm3530.h>
#include <linux/leds-lm3532.h>
#include <linux/isl29030.h>

#include "board-olympus.h"
#include "gpio-names.h"
#include "devices.h"
#include "board.h"
#include "hwrev.h"


//#define XMEGAT_BL_I2C_ADDR		0x24
#define XMEGAT_BL_I2C_ADDR		0x4A
#define OLYMPUS_TOUCH_IRQ_GPIO 		45
#define OLYMPUS_TOUCH_RESET_GPIO 	44
#define OLYMPUS_COMPASS_IRQ_GPIO TEGRA_GPIO_PE2

#define TEGRA_BACKLIGHT_EN_GPIO 	32 /* TEGRA_GPIO_PE0 */
#define TEGRA_KEY_BACKLIGHT_EN_GPIO 	47 /* TEGRA_GPIO_PE0 */

static int disp_backlight_init(void)
{
    int ret;
    if ((ret = gpio_request(TEGRA_BACKLIGHT_EN_GPIO, "backlight_en"))) {
        pr_err("%s: gpio_request(%d, backlight_en) failed: %d\n",
            __func__, TEGRA_BACKLIGHT_EN_GPIO, ret);
        return ret;
    } else {
        pr_info("%s: gpio_request(%d, backlight_en) success!\n",
            __func__, TEGRA_BACKLIGHT_EN_GPIO);
    }
    if ((ret = gpio_direction_output(TEGRA_BACKLIGHT_EN_GPIO, 1))) {
        pr_err("%s: gpio_direction_output(backlight_en) failed: %d\n",
            __func__, ret);
        return ret;
    }
	if ((ret = gpio_request(TEGRA_KEY_BACKLIGHT_EN_GPIO,
			"key_backlight_en"))) {
		pr_err("%s: gpio_request(%d, key_backlight_en) failed: %d\n",
			__func__, TEGRA_KEY_BACKLIGHT_EN_GPIO, ret);
		return ret;
	} else {
		pr_info("%s: gpio_request(%d, key_backlight_en) success!\n",
			__func__, TEGRA_KEY_BACKLIGHT_EN_GPIO);
	}
	if ((ret = gpio_direction_output(TEGRA_KEY_BACKLIGHT_EN_GPIO, 1))) {
		pr_err("%s: gpio_direction_output(key_backlight_en) failed: %d\n",
			__func__, ret);
		return ret;
	}

    return 0;
}

static int disp_backlight_power_on(void)
{
    pr_info("%s: display backlight is powered on\n", __func__);
    gpio_set_value(TEGRA_BACKLIGHT_EN_GPIO, 1);
    return 0;
}

static int disp_backlight_power_off(void)
{
    pr_info("%s: display backlight is powered off\n", __func__);
    gpio_set_value(TEGRA_BACKLIGHT_EN_GPIO, 0);
    return 0;
}

struct lm3530_platform_data lm3530_pdata = {
    .init = disp_backlight_init,
    .power_on = disp_backlight_power_on,
    .power_off = disp_backlight_power_off,

    .ramp_time = 0,   /* Ramp time in milliseconds */
    .gen_config = 
        LM3530_26mA_FS_CURRENT | LM3530_LINEAR_MAPPING | LM3530_I2C_ENABLE,
    .als_config = 0,  /* We don't use ALS from this chip */
};

struct lm3532_platform_data lm3532_pdata = {
    .flags = LM3532_CONFIG_BUTTON_BL | LM3532_HAS_WEBTOP,
    .init = disp_backlight_init,
    .power_on = disp_backlight_power_on,
    .power_off = disp_backlight_power_off,

    .ramp_time = 0,   /* Ramp time in milliseconds */
    .ctrl_a_fs_current = LM3532_26p6mA_FS_CURRENT,
    .ctrl_b_fs_current = LM3532_8p2mA_FS_CURRENT,
    .ctrl_a_mapping_mode = LM3532_LINEAR_MAPPING,
    .ctrl_b_mapping_mode = LM3532_LINEAR_MAPPING,
	.ctrl_a_pwm = 0x82,
};

extern int MotorolaBootDispArgGet(unsigned int *arg);

static int olympus_touch_reset(void)
{
	gpio_set_value(OLYMPUS_TOUCH_RESET_GPIO, 0);
	msleep(10);
	gpio_set_value(OLYMPUS_TOUCH_RESET_GPIO, 1);
	msleep(100);

	return 0;
}
/*
static struct vkey sholes_touch_vkeys[] = {
	{
		.code		= KEY_BACK,
	},
	{
		.code		= KEY_MENU,
	},
	{
		.code		= KEY_HOME,
	},
	{
		.code		= KEY_SEARCH,
	},
};

static struct qtm_touch_keyarray_cfg sholes_key_array_data[] = {
	{
		.ctrl = 0,
		.x_origin = 0,
		.y_origin = 0,
		.x_size = 0,
		.y_size = 0,
		.aks_cfg = 0,
		.burst_len = 0,
		.tch_det_thr = 0,
		.tch_det_int = 0,
		.reserve9 = 0,
		.reserve10 = 0,
	},
	{
		.ctrl = 0,
		.x_origin = 0,
		.y_origin = 0,
		.x_size = 0,
		.y_size = 0,
		.aks_cfg = 0,
		.burst_len = 0,
		.tch_det_thr = 0,
		.tch_det_int = 0,
		.reserve9 = 0,
		.reserve10 = 0,
	},
};
*/

struct qtouch_ts_platform_data olympus_touch_data = {

	.flags		= (QTOUCH_SWAP_XY |
			   QTOUCH_USE_MULTITOUCH |
			   QTOUCH_CFG_BACKUPNV |
			   QTOUCH_EEPROM_CHECKSUM),
	.irqflags		= (IRQF_TRIGGER_LOW),
	.abs_min_x		= 0,
	.abs_max_x		= 1023,
	.abs_min_y		= 0,
	.abs_max_y		= 1023,
	.abs_min_p		= 0,
	.abs_max_p		= 255,
	.abs_min_w		= 0,
	.abs_max_w		= 15,
	.x_delta		= 400,
	.y_delta		= 250,
	.nv_checksum		= 0xc240,
	.fuzz_x			= 0,
	.fuzz_y			= 0,
	.fuzz_p			= 2,
	.fuzz_w			= 2,
	.boot_i2c_addr	= XMEGAT_BL_I2C_ADDR,
	.hw_reset		= olympus_touch_reset,
	.key_array = {
		.cfg		= NULL,
		.keys		= NULL,
		.num_keys	= 0,
	},
	.power_cfg	= {
		.idle_acq_int	= 0xff,
		.active_acq_int	= 0xff,
		.active_idle_to	= 0x01,
	},
	.acquire_cfg	= {
		.charge_time	= 0x06,
/*		.atouch_drift	= 0x00,*/
		.touch_drift	= 0x0a,
		.drift_susp	= 0x05,
		.touch_autocal	= 0x00,
/*		.sync		= 0,*/
		.atch_cal_suspend_time	= 0,
		.atch_cal_suspend_thres	= 0,
	},
	.multi_touch_cfg	= {
		.ctrl		= 0x0b,
		.x_origin	= 0,
		.y_origin	= 0,
		.x_size		= 0x13,
		.y_size		= 0x0b,
		.aks_cfg	= 0,
		.burst_len	= 0x41,
		.tch_det_thr	= 0x14,
		.tch_det_int	= 0x2,
		.orient		= 4,
		.mrg_to		= 0x19,
		.mov_hyst_init	= 0x05,
		.mov_hyst_next	= 0x05,
		.mov_filter	= 0,
		.num_touch	= 0x02,
		.merge_hyst	= 0x05,
		.merge_thresh	= 0x05,
		.amp_hyst       = 0,
		.x_res		= 0x0000,
		.y_res		= 0x0000,
		.x_low_clip	= 0x00,
		.x_high_clip	= 0x00,
		.y_low_clip	= 0x00,
		.y_high_clip	= 0x00,
		.x_edge_ctrl	= 0,
		.x_edge_dist	= 0,
		.y_edge_ctrl	= 0,
		.y_edge_dist	= 0,
		.jump_limit	= 0,
	},
	.linear_tbl_cfg = {
		.ctrl		= 0x00,
		.x_offset	= 0x0000,
		.x_segment = {
			0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00
		},
		.y_offset = 0x0000,
		.y_segment = {
			0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00
		},
	},
	.comms_config_cfg = {
		.ctrl		= 0,
		.command	= 0,
	},
	.gpio_pwm_cfg = {
		.ctrl			= 0,
		.report_mask		= 0,
		.pin_direction		= 0,
		.internal_pullup	= 0,
		.output_value		= 0,
		.wake_on_change		= 0,
		.pwm_enable		= 0,
		.pwm_period		= 0,
		.duty_cycle_0		= 0,
		.duty_cycle_1		= 0,
		.duty_cycle_2		= 0,
		.duty_cycle_3		= 0,
		.trigger_0		= 0,
		.trigger_1		= 0,
		.trigger_2		= 0,
		.trigger_3		= 0,
	},
	.grip_face_suppression_cfg = {
		.ctrl		= 0x00,
		.xlogrip	= 0x00,
		.xhigrip	= 0x00,
		.ylogrip	= 0x00,
		.yhigrip	= 0x00,
		.maxtchs	= 0x00,
		.reserve6	= 0x00,
		.szthr1		= 0x00,
		.szthr2		= 0x00,
		.shpthr1	= 0x00,
		.shpthr2	= 0x00,
		.supextto	= 0x00,
	},
	.noise_suppression_cfg = {
		.ctrl			= 0,
		.reserve1		= 0,
		.reserve2		= 0,
		.reserve3		= 0,
		.reserve4		= 0,
		.reserve5		= 0,
		.reserve6		= 0,
		.reserve7		= 0,
		.noise_thres		= 0,
		.reserve9		= 0,
		.freq_hop_scale		= 0,
		.burst_freq_0		= 0,
		.burst_freq_1           = 0,
		.burst_freq_2           = 0,
		.burst_freq_3           = 0,
		.burst_freq_4           = 0,
		.reserve16		= 0,
	},
	.touch_proximity_cfg = {
		.ctrl			= 0,
		.x_origin		= 0,
		.y_origin		= 0,
		.x_size			= 0,
		.y_size			= 0,
		.reserve5		= 0,
		.blen			= 0,
		.tch_thresh		= 0,
		.tch_detect_int		= 0,
		.average		= 0,
		.move_null_rate		= 0,
		.move_det_tresh		= 0,
	},
	.one_touch_gesture_proc_cfg = {
		.ctrl			= 0,
		.num_gestures		= 0,
		.gesture_enable		= 0,
		.pres_proc		= 0,
		.tap_time_out		= 0,
		.flick_time_out		= 0,
		.drag_time_out		= 0,
		.short_press_time_out	= 0,
		.long_press_time_out	= 0,
		.repeat_press_time_out	= 0,
		.flick_threshold	= 0,
		.drag_threshold		= 0,
		.tap_threshold		= 0,
		.throw_threshold	= 0,
	},
	.self_test_cfg = {
		.ctrl			= 0,
		.command		= 0,
		.high_signal_limit_0	= 0,
		.low_signal_limit_0	= 0,
		.high_signal_limit_1	= 0,
		.low_signal_limit_1	= 0,
		.high_signal_limit_2	= 0,
		.low_signal_limit_2	= 0,
	},
	.two_touch_gesture_proc_cfg = {
		.ctrl			= 0,
		.num_gestures		= 0,
		.reserve2		= 0,
		.gesture_enable		= 0,
		.rotate_threshold	= 0,
		.zoom_threshold		= 0,
	},
	.cte_config_cfg = {
		.ctrl			= 1,
		.command		= 0,
		.reserve2		= 3,
		.idle_gcaf_depth	= 4,
		.active_gcaf_depth	= 8,
		.voltage		= 0,
	},
	.noise1_suppression_cfg = {
		.ctrl		= 0x01,
		.version	= 0x01,
		.atch_thr	= 0x64,
		.duty_cycle	= 0x08,
		.drift_thr	= 0x00,
		.clamp_thr	= 0x00,
		.diff_thr	= 0x00,
		.adjustment	= 0x00,
		.average	= 0x0000,
		.temp		= 0x00,
		.offset = {
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
		},
		.bad_chan = {
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00
		},
		.x_short	= 0x00,
	},
	.vkeys			= {
		.count		= 0,
		.keys		= NULL,
	},
};

extern struct isl29030_platform_data isl29030_als_ir_data_Olympus;

static struct i2c_board_info __initdata olympus_i2c_bus1_board_info[] = {
	{ /* Display backlight */
		I2C_BOARD_INFO(LM3532_NAME, LM3532_I2C_ADDR),
		.platform_data = &lm3532_pdata,
		/*.irq = ..., */
	},
	{
		I2C_BOARD_INFO(QTOUCH_TS_NAME, XMEGAT_BL_I2C_ADDR),
		.platform_data = &olympus_touch_data,
		.irq = TEGRA_GPIO_TO_IRQ(OLYMPUS_TOUCH_IRQ_GPIO),
	},

	{
		/*  ISL 29030 (prox/ALS) driver */
		I2C_BOARD_INFO(LD_ISL29030_NAME, 0x44),
		.platform_data = &isl29030_als_ir_data_Olympus,
		.irq = 180,
	},

};

static struct tegra_i2c_platform_data olympus_i2c1_platform_data = {
	.adapter_nr	= 0,
	.bus_count	= 1,
	.bus_clk_rate	= { 400000, 0 },
	.scl_gpio	= {TEGRA_GPIO_PC4, 0},
	.sda_gpio	= {TEGRA_GPIO_PC5, 0},
	.arb_recovery = arb_lost_recovery,
	.slave_addr = 0xFC,
};

static struct tegra_i2c_platform_data olympus_i2c2_platform_data = {
	.adapter_nr	= 1,
	.bus_count	= 1,
	.bus_clk_rate	= { 400000, 400000 },
};

static struct tegra_i2c_platform_data olympus_i2c3_platform_data = {
	.adapter_nr	= 3,
	.bus_count	= 1,
	.bus_clk_rate	= { 400000, 0 },
	.scl_gpio	= {TEGRA_GPIO_PBB2, 0},
	.sda_gpio	= {TEGRA_GPIO_PBB3, 0},
	.arb_recovery = arb_lost_recovery,
	.slave_addr = 0xFC,
};

static struct tegra_i2c_platform_data olympus_dvc_platform_data = {
	.adapter_nr	= 4,
	.bus_count	= 1,
	.bus_clk_rate	= { 100000, 0 },
	.is_dvc		= true,
	.scl_gpio	= {TEGRA_GPIO_PZ6, 0},
	.sda_gpio	= {TEGRA_GPIO_PZ7, 0},
	.arb_recovery = arb_lost_recovery,
};

void olympus_i2c_reg(void)
{
	tegra_i2c_device1.dev.platform_data = &olympus_i2c1_platform_data;
	tegra_i2c_device2.dev.platform_data = &olympus_i2c2_platform_data;
	tegra_i2c_device3.dev.platform_data = &olympus_i2c3_platform_data;
	tegra_i2c_device4.dev.platform_data = &olympus_dvc_platform_data;

	platform_device_register(&tegra_i2c_device1);
	platform_device_register(&tegra_i2c_device2);
	platform_device_register(&tegra_i2c_device3);
	platform_device_register(&tegra_i2c_device4);
}

/* center: x: home: 55, menu: 185, back: 305, search 425, y: 835 */
static int vkey_size_olympus_p_1_42[4][4] = 
            { {67,900,134,80},    // KEY_MENU
              {200,900,134,80},    // KEY_HOME
              {337,900,134,80},    // KEY_BACK
              {472,900,134,80}};  // KEY_SEARCH

static int vkey_size_olympus_p_1_43[4][4] = 
            { {68,1024,76,76},    // KEY_MENU
              {203,1024,87,76},    // KEY_HOME
              {337,1024,87,76},    // KEY_BACK
              {472,1024,76,76}};  // KEY_SEARCH

static ssize_t mot_virtual_keys_show(struct kobject *kobj,
					struct kobj_attribute *attr, char *buf)
{
	/* keys are specified by setting the x,y of the center, the width,
	 * and the height, as such keycode:center_x:center_y:width:height */
	if (HWREV_TYPE_IS_PORTABLE(system_rev)  ||
	    HWREV_TYPE_IS_FINAL(system_rev) )
	{
		/* Olympus, P1C+ product */
		if (HWREV_REV(system_rev) >= HWREV_REV_1C )
		{
			return sprintf(buf, __stringify(EV_KEY) ":" __stringify(KEY_MENU) ":%d:%d:%d:%d:" 
                           __stringify(EV_KEY) ":" __stringify(KEY_HOME) ":%d:%d:%d:%d:"
	            __stringify(EV_KEY) ":" __stringify(KEY_BACK) ":%d:%d:%d:%d:"
                           __stringify(EV_KEY) ":" __stringify(KEY_SEARCH) ":%d:%d:%d:%d\n",
                           vkey_size_olympus_p_1_43[0][0],vkey_size_olympus_p_1_43[0][1],vkey_size_olympus_p_1_43[0][2],vkey_size_olympus_p_1_43[0][3],
                           vkey_size_olympus_p_1_43[1][0],vkey_size_olympus_p_1_43[1][1],vkey_size_olympus_p_1_43[1][2],vkey_size_olympus_p_1_43[1][3],
                           vkey_size_olympus_p_1_43[2][0],vkey_size_olympus_p_1_43[2][1],vkey_size_olympus_p_1_43[2][2],vkey_size_olympus_p_1_43[2][3],
                           vkey_size_olympus_p_1_43[3][0],vkey_size_olympus_p_1_43[3][1], vkey_size_olympus_p_1_43[3][2], vkey_size_olympus_p_1_43[3][3]);
		}
		else
		{
			return sprintf(buf, __stringify(EV_KEY) ":" __stringify(KEY_MENU) ":%d:%d:%d:%d:" 
                           __stringify(EV_KEY) ":" __stringify(KEY_HOME) ":%d:%d:%d:%d:"
	            __stringify(EV_KEY) ":" __stringify(KEY_BACK) ":%d:%d:%d:%d:"
                           __stringify(EV_KEY) ":" __stringify(KEY_SEARCH) ":%d:%d:%d:%d\n",
                           vkey_size_olympus_p_1_42[0][0],vkey_size_olympus_p_1_42[0][1],vkey_size_olympus_p_1_42[0][2],vkey_size_olympus_p_1_42[0][3],
                           vkey_size_olympus_p_1_42[1][0],vkey_size_olympus_p_1_42[1][1],vkey_size_olympus_p_1_42[1][2],vkey_size_olympus_p_1_42[1][3],
                           vkey_size_olympus_p_1_42[2][0],vkey_size_olympus_p_1_42[2][1],vkey_size_olympus_p_1_42[2][2],vkey_size_olympus_p_1_42[2][3],
                           vkey_size_olympus_p_1_42[3][0],vkey_size_olympus_p_1_42[3][1], vkey_size_olympus_p_1_42[3][2], vkey_size_olympus_p_1_42[3][3]);
		}
	}
	else
		return 0;
};

static struct kobj_attribute mot_virtual_keys_attr = {
	.attr = {
		.name = "virtualkeys.qtouch-obp-ts",
		.mode = S_IRUGO,
	},
	.show = &mot_virtual_keys_show,
};

static struct attribute *mot_properties_attrs[] = {
	&mot_virtual_keys_attr.attr,
	NULL,
};

static struct attribute_group mot_properties_attr_group = {
	.attrs = mot_properties_attrs,
};

static void olympus_touch_init(void)
{
	int ret = 0;
	struct kobject *properties_kobj = NULL;

	tegra_gpio_enable(OLYMPUS_TOUCH_IRQ_GPIO);
	gpio_request(OLYMPUS_TOUCH_IRQ_GPIO, "touch_irq");
	gpio_direction_input(OLYMPUS_TOUCH_IRQ_GPIO);

	tegra_gpio_enable(OLYMPUS_TOUCH_RESET_GPIO);
	gpio_request(OLYMPUS_TOUCH_RESET_GPIO, "touch_reset");
	gpio_direction_output(OLYMPUS_TOUCH_RESET_GPIO, 1);
	

	printk("\n%s: Updating i2c_bus_board_info with correct setup info for TS\n", __func__);
	/*
  	 * This is the information for the driver! Update platform_data field with
 	 * the pointer to the correct data based on the machine type and screen 
 	 * size
	 */
	properties_kobj = kobject_create_and_add("board_properties", NULL);
	if (properties_kobj)
		ret = sysfs_create_group(properties_kobj,
				 &mot_properties_attr_group);
	if (!properties_kobj || ret)
		pr_err("failed to create board_properties\n");

	printk("TOUCH: determining size of the screen\n");
#if 0
	/* Setup Olympus Mortable as a default */ 
	olympus_i2c_bus1_board_info->platform_data = 
		&ts_platform_olympus_m_1;
	if (HWREV_TYPE_IS_PORTABLE(system_rev)  ||
	    HWREV_TYPE_IS_FINAL(system_rev) )
	{
		/* Olympus product */
		if (HWREV_REV(system_rev) >= HWREV_REV_1C )
		{
			info->platform_data = 
				&ts_platform_olympus_p_1_43;
		}
		else
		{
			info->platform_data = 
				&ts_platform_olympus_p_1_37;
		}
	}
#endif	
}

static void olympus_lights_init(void)
{
	unsigned int disp_type = 0;
	int ret;

#ifdef CONFIG_LEDS_DISP_BTN_TIED
	lm3532_pdata.flags |= LM3532_DISP_BTN_TIED;
#endif
	if ((ret = MotorolaBootDispArgGet(&disp_type))) {
		pr_err("\n%s: unable to read display type: %d\n", __func__, ret);
		return;
	}
	if (disp_type & 0x100) {
		pr_info("\n%s: 0x%x ES2 display; will enable PWM in LM3532\n",
			__func__, disp_type);
		lm3532_pdata.ctrl_a_pwm = 0x86;
	} else {
		pr_info("\n%s: 0x%x ES1 display; will NOT enable PWM in LM3532\n",
			__func__, disp_type);
	}
}

void __init olympus_i2c_init(void)
{
	olympus_i2c_reg();
	olympus_touch_init();
	olympus_lights_init();
	
	printk("%s: registering i2c devices...\n", __func__);
	printk("bus 0: %d devices\n", ARRAY_SIZE(olympus_i2c_bus1_board_info));
	i2c_register_board_info(0, olympus_i2c_bus1_board_info, 
				ARRAY_SIZE(olympus_i2c_bus1_board_info));

}

