/***************************************************************************//**
 *   @file   adrv9002.h
 *   @brief  adrv9002 driver header.
 *   @author Darius Berghe (darius.berghe@analog.com)
********************************************************************************
 * Copyright 2020(c) Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  - Neither the name of Analog Devices, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *  - The use of this software may or may not infringe the patent rights
 *    of one or more patent holders.  This license does not release you
 *    from the requirement that you obtain separate licenses from these
 *    patent holders to use this software.
 *  - Use of the software either in source or binary form, must be run
 *    on or directly connected to an Analog Devices Inc. component.
 *
 * THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/
#ifndef IIO_TRX_ADRV9002_H_
#define IIO_TRX_ADRV9002_H_

#include "gpio.h"
#include "delay.h"

#include "adi_common_log.h"
#include "adi_adrv9001_user.h"
#include "adi_adrv9001_cals_types.h"
#include "adi_adrv9001_radio_types.h"
#include "adi_adrv9001_rx_gaincontrol_types.h"
#ifdef CONFIG_DEBUG_FS
#include "adi_adrv9001_ssi.h"
#endif
#include "adi_platform_types.h"

#define ADRV_ADDRESS(port, chan)	((port) << 8 | (chan))
#define ADRV_ADDRESS_PORT(addr)		((addr) >> 8)
#define ADRV_ADDRESS_CHAN(addr)		((addr) & 0xFF)

enum {
	ADRV9002_CHANN_1,
	ADRV9002_CHANN_2,
	ADRV9002_CHANN_MAX,
};

#ifdef ADI_COMMON_VERBOSE
/*
 * Setting logEnable enables error reports which are only available if
 * ADI_COMMON_VERBOSE is defined
 */
#define	adrv9002_set_loglevel(common, level)	\
	adi_common_LogLevelSet(common, level); \
	(common)->error.logEnable = true
#else
#define	adrv9002_set_loglevel(common, level)	\
	adi_common_LogLevelSet(common, level)
#endif

enum ad900x_device_id {
	ID_ADRV9002,
	ID_ADRV9002_RX2TX2,
};

enum adrv9002_clocks {
	RX1_SAMPL_CLK,
	RX2_SAMPL_CLK,
	TX1_SAMPL_CLK,
	TX2_SAMPL_CLK,
	NUM_ADRV9002_CLKS,
};

enum adrv9002_rx_ext_info {
	RX_QEC_FIC,
	RX_QEC_W_POLY,
	RX_AGC,
	RX_TRACK_BBDC,
	RX_HD2,
	RX_RSSI_CAL,
	RX_RFDC,
	RX_RSSI,
	RX_DECIMATION_POWER,
	RX_RF_BANDWIDTH,
	RX_POWERDOWN,
	RX_GAIN_CTRL_PIN_MODE,
	RX_ENSM_MODE,
	RX_NCO_FREQUENCY,
	RX_ADC_SWITCH,
	RX_BBDC,
};

enum adrv9002_tx_ext_info {
	TX_QEC,
	TX_LOL,
	TX_LB_PD,
	TX_PAC,
	TX_CLGC,
	TX_RF_BANDWIDTH,
	TX_POWERDOWN,
	TX_ATTN_CTRL_PIN_MODE,
	TX_ENSM_MODE,
	TX_NCO_FREQUENCY
};

struct adrv9002_clock {
	struct spi_device	*spi;
	struct adrv9002_rf_phy	*phy;
	unsigned long		rate;
	enum adrv9002_clocks	source;
};

struct adrv9002_chan {
	adi_adrv9001_ChannelState_e cached_state;
	adi_common_ChannelNumber_e number;
	uint32_t power;
	int nco_freq;
	uint8_t enabled;
};

struct adrv9002_rx_chan {
	struct adrv9002_chan channel;
	struct adi_adrv9001_GainControlCfg *agc;
	struct adi_adrv9001_RxGainControlPinCfg *pin_cfg;
#ifdef CONFIG_DEBUG_FS
	struct adi_adrv9001_RxSsiTestModeCfg ssi_test;
	struct adi_adrv9001_GainControlCfg debug_agc;
#endif
};

struct adrv9002_tx_chan {
	struct adrv9002_chan channel;
	struct adi_adrv9001_TxAttenuationPinControlCfg *pin_cfg;
	uint8_t dac_boost_en;
#ifdef CONFIG_DEBUG_FS
	struct adi_adrv9001_TxSsiTestModeCfg ssi_test;
#endif
};

struct adrv9002_gpio {
	struct adi_adrv9001_GpioCfg gpio;
	uint32_t signal;
};

#define to_clk_priv(_hw) container_of(_hw, struct adrv9002_clock, hw)

struct adrv9002_rf_phy {
	struct spi_device		*spi;
	struct iio_dev			*indio_dev;
	struct adrv9002_clock		clk_priv[NUM_ADRV9002_CLKS];
	struct adrv9002_rx_chan		rx_channels[ADRV9002_CHANN_MAX];
	struct adrv9002_tx_chan		tx_channels[ADRV9002_CHANN_MAX];
	struct adrv9002_gpio 		*adrv9002_gpios;
	struct adi_adrv9001_Device	adrv9001_device;
	struct adi_adrv9001_Device	*adrv9001;
	struct adi_hal_Cfg		hal;
	struct adi_adrv9001_Init	*curr_profile;
	struct adi_adrv9001_Init	profile;
	struct adi_adrv9001_InitCals	init_cals;
	int				spi_device_id;
	int				ngpios;
	uint8_t				rx2tx2;
#ifdef CONFIG_DEBUG_FS
	struct adi_adrv9001_SsiCalibrationCfg ssi_delays;
#endif
	struct axi_adc			*rx1_adc;
	struct axi_dac			*tx1_dac;
	struct axi_adc			*rx2_adc;
	struct axi_dac			*tx2_dac;
	struct axi_dmac			*rx1_dmac;
	struct axi_dmac			*tx1_dmac;
	struct axi_dmac			*rx2_dmac;
	struct axi_dmac			*tx2_dmac;
};

int adrv9002_hdl_loopback(struct adrv9002_rf_phy *phy, bool enable);
int adrv9002_register_axi_converter(struct adrv9002_rf_phy *phy);
int adrv9002_axi_interface_set(struct adrv9002_rf_phy *phy,
			       const uint8_t n_lanes,
			       const uint8_t ssi_intf, const bool cmos_ddr,
			       const int channel);
struct adrv9002_rf_phy *adrv9002_spi_to_phy(struct spi_device *spi);
int adrv9002_axi_intf_tune(struct adrv9002_rf_phy *phy, const bool tx,
			   const int chann,
			   const adi_adrv9001_SsiType_e ssi_type, uint8_t *clk_delay, uint8_t *data_delay);
void adrv9002_axi_interface_enable(struct adrv9002_rf_phy *phy, const int chan,
				   const bool en);
int adrv9002_spi_read(struct spi_device *spi, uint32_t reg);
int adrv9002_spi_write(struct spi_device *spi, uint32_t reg, uint32_t val);
void adrv9002_get_ssi_interface(struct adrv9002_rf_phy *phy, const int channel,
				uint8_t *ssi_intf, uint8_t *n_lanes, bool *cmos_ddr_en);
int adrv9002_ssi_configure(struct adrv9002_rf_phy *phy);
void adrv9002_cmos_default_set(void);
int adrv9002_intf_tuning(struct adrv9002_rf_phy *phy);
int adrv9002_intf_test_cfg(struct adrv9002_rf_phy *phy, const int chann,
			   const bool tx,
			   const bool stop, const adi_adrv9001_SsiType_e ssi_type);
int adrv9002_check_tx_test_pattern(struct adrv9002_rf_phy *phy, const int chann,
				   const adi_adrv9001_SsiType_e ssi_type);
int adrv9002_intf_change_delay(struct adrv9002_rf_phy *phy, const int channel,
			       uint8_t clk_delay,
			       uint8_t data_delay, const bool tx,
			       const adi_adrv9001_SsiType_e ssi_type);
adi_adrv9001_SsiType_e adrv9002_axi_ssi_type_get(struct adrv9002_rf_phy *phy);
int __adrv9002_dev_err(const struct adrv9002_rf_phy *phy,
		       const char *function, const int line);
#define adrv9002_dev_err(phy)	__adrv9002_dev_err(phy, __func__, __LINE__)
int adrv9002_post_setup(struct adrv9002_rf_phy *phy);
int adrv9002_setup(struct adrv9002_rf_phy *phy,
		   adi_adrv9001_Init_t *adrv9002_init);
/* get init structs */
struct adi_adrv9001_SpiSettings *adrv9002_spi_settings_get(void);
struct adi_adrv9001_Init *adrv9002_init_get(void);
struct adi_adrv9001_RadioCtrlInit *adrv9002_radio_ctrl_init_get(void);
struct adi_adrv9001_PlatformFiles *adrv9002_platform_files_get(void);

static inline void adrv9002_sync_gpio_toogle(const struct adrv9002_rf_phy *phy)
{
	if (phy->rx2tx2) {
		/* toogle ssi sync gpio */
		gpio_set_value(phy->hal.gpio_ssi_sync, 1);
		udelay(5000);
		gpio_set_value(phy->hal.gpio_ssi_sync, 0);
	}
}
#endif