/*
 * arch/arm/mach-tegra/board-nvodm.c
 *
 * Converts data from ODM query library into platform data
 *
 * Copyright (c) 2009-2010, NVIDIA Corporation.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/clk.h>
#include <linux/console.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/fsl_devices.h>
#include <linux/gpio.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/platform_device.h>
#include <linux/platform_data/tegra_usb.h>
#include <linux/pda_power.h>
#include <linux/regulator/machine.h>
#include <linux/reboot.h>
#include <linux/serial_8250.h>
#include <linux/i2c.h>
#include <linux/i2c-tegra.h>
#include <linux/spi-tegra.h>
#include <linux/spi/spi.h>
#include <linux/spi/cpcap.h>
#include <linux/tegra_uart.h>
#include <linux/nvhost.h>

#include <linux/usb/composite.h>
#include <linux/usb/gadget.h>
#include <linux/usb/f_accessory.h>
#include <linux/fsl_devices.h>

#include <asm/mach/time.h>
#include <asm/mach-types.h>

#include <mach/clk.h>
#include <mach/gpio.h>
#include <mach/io.h>
#include <mach/iomap.h>
#include <mach/irqs.h>
#include <mach/i2s.h>
#include <mach/kbc.h>
#include <mach/nand.h>
#include <mach/pinmux.h>
#include <mach/sdhci.h>
#include <mach/w1.h>
#include <mach/usb_phy.h>
#include <mach/olympus_usb.h>
#include <mach/nvmap.h>

#include "clock.h"
#include "devices.h"
#include "gpio-names.h"
#include "pm.h"
#include "board.h"
#include "hwrev.h"
#include "board-olympus.h"
#include <linux/mmc/host.h>

#ifdef CONFIG_USB_G_ANDROID
#include <linux/usb/android_composite.h>
#endif

#define BT_RESET 0
#define BT_SHUTDOWN 1

static struct platform_device tegra_camera = {
	.name = "tegra_camera",
	.id = -1,
};

static struct platform_device olympus_touch_xmegat = {
	.name = "qtouch-obp-ts",
	.id = -1,
};

static struct platform_device olympus_codec_cpcap = {
	.name   = "cpcap_audio",
        .id             = 0,
};

static struct platform_device *olympus_devices[] __initdata = {
	&tegra_pmu_device,
	&tegra_gart_device,
	&tegra_wdt_device,
//	&tegra_spi_slave_device1,
//	&tegra_spi_device1,
	&tegra_spi_device2,
	&tegra_spi_device3,
	&tegra_spi_device4,
	&tegra_pwfm1_device,
	&tegra_avp_device,
	&tegra_camera,
	&tegra_i2s_device1,
	&tegra_i2s_device2,
	&tegra_das_device,
	&tegra_pcm_device,
	&tegra_spdif_device,
	&olympus_codec_cpcap,
	&spdif_dit_device,
	&olympus_touch_xmegat,
};


static int tegra_reboot_notify(struct notifier_block *nb,
				unsigned long event, void *data)
{
	printk(KERN_INFO "pICS_%s: event = [%lu]",__func__, event);
	switch (event) {
	case SYS_RESTART:
	case SYS_HALT:
	case SYS_POWER_OFF:
		/* USB power rail must be enabled during boot */
		/*NvOdmEnableUsbPhyPowerRail(1);*/
		return NOTIFY_OK;
	}
	return NOTIFY_DONE;
}

static struct notifier_block tegra_reboot_nb = {
	.notifier_call = tegra_reboot_notify,
	.next = NULL,
	.priority = 0
};

static void olympus_reboot_init(void)
{
	int rc = register_reboot_notifier(&tegra_reboot_nb);
	if (rc)
		pr_err("%s: failed to regsiter platform reboot notifier\n",
			__func__);
}

static struct tegra_uart_platform_data ipc_olympus_pdata = 
{
	.uart_ipc = 1,
	.uart_wake_host = TEGRA_GPIO_PA0,
	.uart_wake_request = TEGRA_GPIO_PF1,
#ifdef CONFIG_MDM_CTRL
	.peer_register = olympus_mdm_ctrl_peer_register,
#endif
};

void __init olympus_devices_init()
{
//	struct clk *clk;

	olympus_i2c_reg();

	olympus_uart_init();

	platform_add_devices(olympus_devices, ARRAY_SIZE(olympus_devices));

	olympus_sdhci_init();

	olympus_usb_init();

	pm_power_off = tegra_system_power_off;
	
	olympus_reboot_init();

	tegra_uartd_device.dev.platform_data = &ipc_olympus_pdata;
}

