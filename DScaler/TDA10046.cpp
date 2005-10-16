/////////////////////////////////////////////////////////////////////////////
// $Id: TDA10046.cpp,v 1.2 2005-10-16 17:02:40 kelddamsbo Exp $
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2005 Keld Damsbo.  All rights reserved.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
//
/////////////////////////////////////////////////////////////////////////////
//
// Sections in this code were ported from video4linux project's "tda1004x.c"
// file.  Copyright is unspecified.
//
/////////////////////////////////////////////////////////////////////////////
//
// $Log: not supported by cvs2svn $
// Revision 1.1  2005/10/15 20:01:33  kelddamsbo
// Initial Upload
//
// Revision 1.0  2005/10/09 17:59:00  kdamsbo
// Initial upload.
//
/////////////////////////////////////////////////////////////////////////////

/**
* @file TDA10046.cpp CTDA10046 Implementation
*/

#include "stdafx.h"
#include "TDA10046.h"
#include "TDA8275.h"
#include "TDA8290.h"
#include "DebugLog.h"

BYTE CTDA10046::GetDefaultAddress() const
{
	return I2C_ADDR_TDA10046_1;
}

void CTDA10046::WriteTDA10046Initialization()
{
	// Init the tuner PLL
//	if (state->config->pll_init) {
//		tda1004x_enable_tuner_i2c(state);
//		if (state->config->pll_init(fe)) {
//			printk(KERN_ERR "tda1004x: pll init failed\n");
//			return 	-EIO;
//		}
//		tda1004x_disable_tuner_i2c(state);
//	}

	// TDA10046 Setup
	MaskDataByte(TDA10046_CONF_C4, 0, 0x20); // disable DSP watchdog timer
	WriteToSubAddress(TDA10046_AUTO, 0x07); // select HP stream
	WriteToSubAddress(TDA10046_CONF_C1, 0x08); // disable pulse killer

//	tda10046_init_plls(fe);
//	switch (state->config->agc_config) {

//	case TDA10046_AGC_DEFAULT:
//		WriteToSubAddress(TDA10046_AGC_CONF, 0x00); // AGC setup
//		WriteToSubAddress(TDA10046_CONF_POLARITY, 0x60); // set AGC polarities
//		break;
//	case TDA10046_AGC_IFO_AUTO_NEG:
//		WriteToSubAddress(TDA10046_AGC_CONF, 0x0A); // AGC setup
//		WriteToSubAddress(TDA10046_CONF_POLARITY, 0x60); // set AGC polarities
//		break;
//	case TDA10046_AGC_IFO_AUTO_POS:
//		WriteToSubAddress(TDA10046H_AGC_CONF, 0x0A); // AGC setup
//		WriteToSubAddress(TDA10046_CONF_POLARITY, 0x00); // set AGC polarities
//		break;
//	case TDA10046_AGC_TDA827X:
		WriteToSubAddress(TDA10046_AGC_CONF, 0x02);   // AGC setup
		WriteToSubAddress(TDA10046_AGC_THRESHOLD, 0x70);    // AGC Threshold
		WriteToSubAddress(TDA10046_AGC_RENORM, 0x0E); // Gain Renormalize
		WriteToSubAddress(TDA10046_CONF_POLARITY, 0x60); // set AGC polarities
//		break;
//	}
	WriteToSubAddress(TDA10046_CONF_TRISTATE1, 0x61); // Turn both AGC outputs on
	WriteToSubAddress(TDA10046_AGC_TUN_MIN, 0x00);	  // }
	WriteToSubAddress(TDA10046_AGC_TUN_MAX, 0xFF); // } AGC min/max values
	WriteToSubAddress(TDA10046_AGC_IF_MIN, 0x00);	  // }
	WriteToSubAddress(TDA10046_AGC_IF_MAX, 0xFF);  // }
	WriteToSubAddress(TDA10046_AGC_GAINS, 0x01); // IF gain 2, TUN gain 1
	WriteToSubAddress(TDA10046_CVBER_CTRL, 0x1A); // 10^6 VBER measurement bits
	WriteToSubAddress(TDA10046_CONF_TS1, 0x07); // MPEG2 interface config
	WriteToSubAddress(TDA10046_CONF_TS2, 0xC0); // MPEG2 interface config
	MaskDataByte(TDA10046_IC_MODE, 0x01, 0x80); // Invert OCLK

	WriteToSubAddress(TDA10046_CONF_TRISTATE2, 0xE1); // tristate setup
	WriteToSubAddress(TDA10046_GPIO_OUT_SEL, 0xCC); // GPIO output config
	WriteToSubAddress(TDA10046_GPIO_SELECT, 0x08); // GPIO select

	state->initialised = 1;
	return 0;
}


void CTDA10046::EnableTuner()
{

	Result = MaskDataByte(TDA10046_CONF_C4, 0x02, 0x02);
	sleep(1);
	return Result;
}

void CTDA10046::DisableTuner()
{
	return MaskDataByte(TDA10046_CONF_C4, 0x00, 0x02);
}









struct tda1004x_state {
	struct i2c_adapter* i2c;
	struct dvb_frontend_ops ops;
	const struct tda1004x_config* config;
	struct dvb_frontend frontend;

	/* private demod data */
	u8 initialised;
	enum tda1004x_demod demod_type;
};

static int tda1004x_write_buf(struct tda1004x_state *state, int reg, unsigned char *buf, int len)
{
	int i;
	int result;

	dprintk("%s: reg=0x%x, len=0x%x\n", __FUNCTION__, reg, len);

	result = 0;
	for (i = 0; i < len; i++) {
		result = tda1004x_write_byteI(state, reg + i, buf[i]);
		if (result != 0)
			break;
	}

	return result;
}

static int tda10046h_set_bandwidth(struct tda1004x_state *state,
				   fe_bandwidth_t bandwidth)
{
	static u8 bandwidth_6mhz[] = { 0x80, 0x15, 0xfe, 0xab, 0x8e };
	static u8 bandwidth_7mhz[] = { 0x6e, 0x02, 0x53, 0xc8, 0x25 };
	static u8 bandwidth_8mhz[] = { 0x60, 0x12, 0xa8, 0xe4, 0xbd };

	switch (bandwidth) {
	case BANDWIDTH_6_MHZ:
		tda1004x_write_buf(state, TDA10046H_TIME_WREF1, bandwidth_6mhz, sizeof(bandwidth_6mhz));
		if (state->config->if_freq == TDA10046_FREQ_045) {
			tda1004x_write_byteI(state, TDA10046H_FREQ_PHY2_MSB, 0x09);
			tda1004x_write_byteI(state, TDA10046H_FREQ_PHY2_LSB, 0x4f);
		}
		break;

	case BANDWIDTH_7_MHZ:
		tda1004x_write_buf(state, TDA10046H_TIME_WREF1, bandwidth_7mhz, sizeof(bandwidth_7mhz));
		if (state->config->if_freq == TDA10046_FREQ_045) {
			tda1004x_write_byteI(state, TDA10046H_FREQ_PHY2_MSB, 0x0a);
			tda1004x_write_byteI(state, TDA10046H_FREQ_PHY2_LSB, 0x79);
		}
		break;

	case BANDWIDTH_8_MHZ:
		tda1004x_write_buf(state, TDA10046H_TIME_WREF1, bandwidth_8mhz, sizeof(bandwidth_8mhz));
		if (state->config->if_freq == TDA10046_FREQ_045) {
			tda1004x_write_byteI(state, TDA10046H_FREQ_PHY2_MSB, 0x0b);
			tda1004x_write_byteI(state, TDA10046H_FREQ_PHY2_LSB, 0xa3);
		}
		break;

	default:
		return -EINVAL;
	}

	return 0;
}

static void tda10046_init_plls(struct dvb_frontend* fe)
{
	struct tda1004x_state* state = fe->demodulator_priv;

	tda1004x_write_byteI(state, TDA10046H_CONFPLL1, 0xf0);
	tda1004x_write_byteI(state, TDA10046H_CONFPLL2, 0x0a); // PLL M = 10
	if (state->config->xtal_freq == TDA10046_XTAL_4M ) {
		dprintk("%s: setting up PLLs for a 4 MHz Xtal\n", __FUNCTION__);
		tda1004x_write_byteI(state, TDA10046H_CONFPLL3, 0); // PLL P = N = 0
	} else {
		dprintk("%s: setting up PLLs for a 16 MHz Xtal\n", __FUNCTION__);
		tda1004x_write_byteI(state, TDA10046H_CONFPLL3, 3); // PLL P = 0, N = 3
	}
	tda1004x_write_byteI(state, TDA10046H_FREQ_OFFSET, 99);
	switch (state->config->if_freq) {
	case TDA10046_FREQ_3617:
		tda1004x_write_byteI(state, TDA10046H_FREQ_PHY2_MSB, 0xd4);
		tda1004x_write_byteI(state, TDA10046H_FREQ_PHY2_LSB, 0x2c);
		break;
	case TDA10046_FREQ_3613:
		tda1004x_write_byteI(state, TDA10046H_FREQ_PHY2_MSB, 0xd4);
		tda1004x_write_byteI(state, TDA10046H_FREQ_PHY2_LSB, 0x13);
		break;
	case TDA10046_FREQ_045:
		tda1004x_write_byteI(state, TDA10046H_FREQ_PHY2_MSB, 0x0b);
		tda1004x_write_byteI(state, TDA10046H_FREQ_PHY2_LSB, 0xa3);
		break;
	case TDA10046_FREQ_052:
		tda1004x_write_byteI(state, TDA10046H_FREQ_PHY2_MSB, 0x0c);
		tda1004x_write_byteI(state, TDA10046H_FREQ_PHY2_LSB, 0x06);
		break;
	}
	tda10046h_set_bandwidth(state, BANDWIDTH_8_MHZ); // default bandwidth 8 MHz
}

static int tda1004x_encode_fec(int fec)
{
	// convert known FEC values
	switch (fec) {
	case FEC_1_2:
		return 0;
	case FEC_2_3:
		return 1;
	case FEC_3_4:
		return 2;
	case FEC_5_6:
		return 3;
	case FEC_7_8:
		return 4;
	}

	// unsupported
	return -EINVAL;
}

static int tda1004x_decode_fec(int tdafec)
{
	// convert known FEC values
	switch (tdafec) {
	case 0:
		return FEC_1_2;
	case 1:
		return FEC_2_3;
	case 2:
		return FEC_3_4;
	case 3:
		return FEC_5_6;
	case 4:
		return FEC_7_8;
	}

	// unsupported
	return -1;
}

int tda1004x_write_byte(struct dvb_frontend* fe, int reg, int data)
{
	struct tda1004x_state* state = fe->demodulator_priv;

	return tda1004x_write_byteI(state, reg, data);
}

static int tda1004x_set_fe(struct dvb_frontend* fe,
			   struct dvb_frontend_parameters *fe_params)
{
	struct tda1004x_state* state = fe->demodulator_priv;
	int tmp;
	int inversion;

	dprintk("%s\n", __FUNCTION__);

	if (state->demod_type == TDA1004X_DEMOD_TDA10046) {
		// setup auto offset
		tda1004x_write_mask(state, TDA1004X_AUTO, 0x10, 0x10);
		tda1004x_write_mask(state, TDA1004X_IN_CONF1, 0x80, 0);
		tda1004x_write_mask(state, TDA1004X_IN_CONF2, 0xC0, 0);

		// disable agc_conf[2]
		tda1004x_write_mask(state, TDA10046H_AGC_CONF, 4, 0);
	}

	// set frequency
	tda1004x_enable_tuner_i2c(state);
	if (state->config->pll_set(fe, fe_params)) {
		printk(KERN_ERR "tda1004x: pll set failed\n");
		return 	-EIO;
	}
	tda1004x_disable_tuner_i2c(state);

	// Hardcoded to use auto as much as possible on the TDA10045 as it
	// is very unreliable if AUTO mode is _not_ used.
	if (state->demod_type == TDA1004X_DEMOD_TDA10045) {
		fe_params->u.ofdm.code_rate_HP = FEC_AUTO;
		fe_params->u.ofdm.guard_interval = GUARD_INTERVAL_AUTO;
		fe_params->u.ofdm.transmission_mode = TRANSMISSION_MODE_AUTO;
	}

	// Set standard params.. or put them to auto
	if ((fe_params->u.ofdm.code_rate_HP == FEC_AUTO) ||
	    (fe_params->u.ofdm.code_rate_LP == FEC_AUTO) ||
	    (fe_params->u.ofdm.constellation == QAM_AUTO) ||
	    (fe_params->u.ofdm.hierarchy_information == HIERARCHY_AUTO)) {
		tda1004x_write_mask(state, TDA1004X_AUTO, 1, 1);	// enable auto
		tda1004x_write_mask(state, TDA1004X_IN_CONF1, 0x03, 0);	// turn off constellation bits
		tda1004x_write_mask(state, TDA1004X_IN_CONF1, 0x60, 0);	// turn off hierarchy bits
		tda1004x_write_mask(state, TDA1004X_IN_CONF2, 0x3f, 0);	// turn off FEC bits
	} else {
		tda1004x_write_mask(state, TDA1004X_AUTO, 1, 0);	// disable auto

		// set HP FEC
		tmp = tda1004x_encode_fec(fe_params->u.ofdm.code_rate_HP);
		if (tmp < 0)
			return tmp;
		tda1004x_write_mask(state, TDA1004X_IN_CONF2, 7, tmp);

		// set LP FEC
		tmp = tda1004x_encode_fec(fe_params->u.ofdm.code_rate_LP);
		if (tmp < 0)
			return tmp;
		tda1004x_write_mask(state, TDA1004X_IN_CONF2, 0x38, tmp << 3);

		// set constellation
		switch (fe_params->u.ofdm.constellation) {
		case QPSK:
			tda1004x_write_mask(state, TDA1004X_IN_CONF1, 3, 0);
			break;

		case QAM_16:
			tda1004x_write_mask(state, TDA1004X_IN_CONF1, 3, 1);
			break;

		case QAM_64:
			tda1004x_write_mask(state, TDA1004X_IN_CONF1, 3, 2);
			break;

		default:
			return -EINVAL;
		}

		// set hierarchy
		switch (fe_params->u.ofdm.hierarchy_information) {
		case HIERARCHY_NONE:
			tda1004x_write_mask(state, TDA1004X_IN_CONF1, 0x60, 0 << 5);
			break;

		case HIERARCHY_1:
			tda1004x_write_mask(state, TDA1004X_IN_CONF1, 0x60, 1 << 5);
			break;

		case HIERARCHY_2:
			tda1004x_write_mask(state, TDA1004X_IN_CONF1, 0x60, 2 << 5);
			break;

		case HIERARCHY_4:
			tda1004x_write_mask(state, TDA1004X_IN_CONF1, 0x60, 3 << 5);
			break;

		default:
			return -EINVAL;
		}
	}

	// set bandwidth
	switch (state->demod_type) {
	case TDA1004X_DEMOD_TDA10045:
		tda10045h_set_bandwidth(state, fe_params->u.ofdm.bandwidth);
		break;

	case TDA1004X_DEMOD_TDA10046:
		tda10046h_set_bandwidth(state, fe_params->u.ofdm.bandwidth);
		break;
	}

	// set inversion
	inversion = fe_params->inversion;
	if (state->config->invert)
		inversion = inversion ? INVERSION_OFF : INVERSION_ON;
	switch (inversion) {
	case INVERSION_OFF:
		tda1004x_write_mask(state, TDA1004X_CONFC1, 0x20, 0);
		break;

	case INVERSION_ON:
		tda1004x_write_mask(state, TDA1004X_CONFC1, 0x20, 0x20);
		break;

	default:
		return -EINVAL;
	}

	// set guard interval
	switch (fe_params->u.ofdm.guard_interval) {
	case GUARD_INTERVAL_1_32:
		tda1004x_write_mask(state, TDA1004X_AUTO, 2, 0);
		tda1004x_write_mask(state, TDA1004X_IN_CONF1, 0x0c, 0 << 2);
		break;

	case GUARD_INTERVAL_1_16:
		tda1004x_write_mask(state, TDA1004X_AUTO, 2, 0);
		tda1004x_write_mask(state, TDA1004X_IN_CONF1, 0x0c, 1 << 2);
		break;

	case GUARD_INTERVAL_1_8:
		tda1004x_write_mask(state, TDA1004X_AUTO, 2, 0);
		tda1004x_write_mask(state, TDA1004X_IN_CONF1, 0x0c, 2 << 2);
		break;

	case GUARD_INTERVAL_1_4:
		tda1004x_write_mask(state, TDA1004X_AUTO, 2, 0);
		tda1004x_write_mask(state, TDA1004X_IN_CONF1, 0x0c, 3 << 2);
		break;

	case GUARD_INTERVAL_AUTO:
		tda1004x_write_mask(state, TDA1004X_AUTO, 2, 2);
		tda1004x_write_mask(state, TDA1004X_IN_CONF1, 0x0c, 0 << 2);
		break;

	default:
		return -EINVAL;
	}

	// set transmission mode
	switch (fe_params->u.ofdm.transmission_mode) {
	case TRANSMISSION_MODE_2K:
		tda1004x_write_mask(state, TDA1004X_AUTO, 4, 0);
		tda1004x_write_mask(state, TDA1004X_IN_CONF1, 0x10, 0 << 4);
		break;

	case TRANSMISSION_MODE_8K:
		tda1004x_write_mask(state, TDA1004X_AUTO, 4, 0);
		tda1004x_write_mask(state, TDA1004X_IN_CONF1, 0x10, 1 << 4);
		break;

	case TRANSMISSION_MODE_AUTO:
		tda1004x_write_mask(state, TDA1004X_AUTO, 4, 4);
		tda1004x_write_mask(state, TDA1004X_IN_CONF1, 0x10, 0);
		break;

	default:
		return -EINVAL;
	}

	// start the lock
	switch (state->demod_type) {
	case TDA1004X_DEMOD_TDA10045:
		tda1004x_write_mask(state, TDA1004X_CONFC4, 8, 8);
		tda1004x_write_mask(state, TDA1004X_CONFC4, 8, 0);
		break;

	case TDA1004X_DEMOD_TDA10046:
		tda1004x_write_mask(state, TDA1004X_AUTO, 0x40, 0x40);
		msleep(1);
		tda1004x_write_mask(state, TDA10046H_AGC_CONF, 4, 1);
		break;
	}

	msleep(10);

	return 0;
}

static int tda1004x_get_fe(struct dvb_frontend* fe, struct dvb_frontend_parameters *fe_params)
{
	struct tda1004x_state* state = fe->demodulator_priv;
	dprintk("%s\n", __FUNCTION__);

	// inversion status
	fe_params->inversion = INVERSION_OFF;
	if (tda1004x_read_byte(state, TDA1004X_CONFC1) & 0x20)
		fe_params->inversion = INVERSION_ON;
	if (state->config->invert)
		fe_params->inversion = fe_params->inversion ? INVERSION_OFF : INVERSION_ON;

	// bandwidth
	switch (state->demod_type) {
	case TDA1004X_DEMOD_TDA10045:
		switch (tda1004x_read_byte(state, TDA10045H_WREF_LSB)) {
		case 0x14:
			fe_params->u.ofdm.bandwidth = BANDWIDTH_8_MHZ;
			break;
		case 0xdb:
			fe_params->u.ofdm.bandwidth = BANDWIDTH_7_MHZ;
			break;
		case 0x4f:
			fe_params->u.ofdm.bandwidth = BANDWIDTH_6_MHZ;
			break;
		}
		break;

	case TDA1004X_DEMOD_TDA10046:
		switch (tda1004x_read_byte(state, TDA10046H_TIME_WREF1)) {
		case 0x60:
			fe_params->u.ofdm.bandwidth = BANDWIDTH_8_MHZ;
			break;
		case 0x6e:
			fe_params->u.ofdm.bandwidth = BANDWIDTH_7_MHZ;
			break;
		case 0x80:
			fe_params->u.ofdm.bandwidth = BANDWIDTH_6_MHZ;
			break;
		}
		break;
	}

	// FEC
	fe_params->u.ofdm.code_rate_HP =
	    tda1004x_decode_fec(tda1004x_read_byte(state, TDA1004X_OUT_CONF2) & 7);
	fe_params->u.ofdm.code_rate_LP =
	    tda1004x_decode_fec((tda1004x_read_byte(state, TDA1004X_OUT_CONF2) >> 3) & 7);

	// constellation
	switch (tda1004x_read_byte(state, TDA1004X_OUT_CONF1) & 3) {
	case 0:
		fe_params->u.ofdm.constellation = QPSK;
		break;
	case 1:
		fe_params->u.ofdm.constellation = QAM_16;
		break;
	case 2:
		fe_params->u.ofdm.constellation = QAM_64;
		break;
	}

	// transmission mode
	fe_params->u.ofdm.transmission_mode = TRANSMISSION_MODE_2K;
	if (tda1004x_read_byte(state, TDA1004X_OUT_CONF1) & 0x10)
		fe_params->u.ofdm.transmission_mode = TRANSMISSION_MODE_8K;

	// guard interval
	switch ((tda1004x_read_byte(state, TDA1004X_OUT_CONF1) & 0x0c) >> 2) {
	case 0:
		fe_params->u.ofdm.guard_interval = GUARD_INTERVAL_1_32;
		break;
	case 1:
		fe_params->u.ofdm.guard_interval = GUARD_INTERVAL_1_16;
		break;
	case 2:
		fe_params->u.ofdm.guard_interval = GUARD_INTERVAL_1_8;
		break;
	case 3:
		fe_params->u.ofdm.guard_interval = GUARD_INTERVAL_1_4;
		break;
	}

	// hierarchy
	switch ((tda1004x_read_byte(state, TDA1004X_OUT_CONF1) & 0x60) >> 5) {
	case 0:
		fe_params->u.ofdm.hierarchy_information = HIERARCHY_NONE;
		break;
	case 1:
		fe_params->u.ofdm.hierarchy_information = HIERARCHY_1;
		break;
	case 2:
		fe_params->u.ofdm.hierarchy_information = HIERARCHY_2;
		break;
	case 3:
		fe_params->u.ofdm.hierarchy_information = HIERARCHY_4;
		break;
	}

	return 0;
}

static int tda1004x_read_status(struct dvb_frontend* fe, fe_status_t * fe_status)
{
	struct tda1004x_state* state = fe->demodulator_priv;
	int status;
	int cber;
	int vber;

	dprintk("%s\n", __FUNCTION__);

	// read status
	status = tda1004x_read_byte(state, TDA1004X_STATUS_CD);
	if (status == -1)
		return -EIO;

	// decode
	*fe_status = 0;
	if (status & 4)
		*fe_status |= FE_HAS_SIGNAL;
	if (status & 2)
		*fe_status |= FE_HAS_CARRIER;
	if (status & 8)
		*fe_status |= FE_HAS_VITERBI | FE_HAS_SYNC | FE_HAS_LOCK;

	// if we don't already have VITERBI (i.e. not LOCKED), see if the viterbi
	// is getting anything valid
	if (!(*fe_status & FE_HAS_VITERBI)) {
		// read the CBER
		cber = tda1004x_read_byte(state, TDA1004X_CBER_LSB);
		if (cber == -1)
			return -EIO;
		status = tda1004x_read_byte(state, TDA1004X_CBER_MSB);
		if (status == -1)
			return -EIO;
		cber |= (status << 8);
		tda1004x_read_byte(state, TDA1004X_CBER_RESET);

		if (cber != 65535)
			*fe_status |= FE_HAS_VITERBI;
	}

	// if we DO have some valid VITERBI output, but don't already have SYNC
	// bytes (i.e. not LOCKED), see if the RS decoder is getting anything valid.
	if ((*fe_status & FE_HAS_VITERBI) && (!(*fe_status & FE_HAS_SYNC))) {
		// read the VBER
		vber = tda1004x_read_byte(state, TDA1004X_VBER_LSB);
		if (vber == -1)
			return -EIO;
		status = tda1004x_read_byte(state, TDA1004X_VBER_MID);
		if (status == -1)
			return -EIO;
		vber |= (status << 8);
		status = tda1004x_read_byte(state, TDA1004X_VBER_MSB);
		if (status == -1)
			return -EIO;
		vber |= ((status << 16) & 0x0f);
		tda1004x_read_byte(state, TDA1004X_CVBER_LUT);

		// if RS has passed some valid TS packets, then we must be
		// getting some SYNC bytes
		if (vber < 16632)
			*fe_status |= FE_HAS_SYNC;
	}

	// success
	dprintk("%s: fe_status=0x%x\n", __FUNCTION__, *fe_status);
	return 0;
}

static int tda1004x_read_signal_strength(struct dvb_frontend* fe, u16 * signal)
{
	struct tda1004x_state* state = fe->demodulator_priv;
	int tmp;
	int reg = 0;

	dprintk("%s\n", __FUNCTION__);

	// determine the register to use
	switch (state->demod_type) {
	case TDA1004X_DEMOD_TDA10045:
		reg = TDA10045H_S_AGC;
		break;

	case TDA1004X_DEMOD_TDA10046:
		reg = TDA10046H_AGC_IF_LEVEL;
		break;
	}

	// read it
	tmp = tda1004x_read_byte(state, reg);
	if (tmp < 0)
		return -EIO;

	*signal = (tmp << 8) | tmp;
	dprintk("%s: signal=0x%x\n", __FUNCTION__, *signal);
	return 0;
}

static int tda1004x_read_snr(struct dvb_frontend* fe, u16 * snr)
{
	struct tda1004x_state* state = fe->demodulator_priv;
	int tmp;

	dprintk("%s\n", __FUNCTION__);

	// read it
	tmp = tda1004x_read_byte(state, TDA1004X_SNR);
	if (tmp < 0)
		return -EIO;
	tmp = 255 - tmp;

	*snr = ((tmp << 8) | tmp);
	dprintk("%s: snr=0x%x\n", __FUNCTION__, *snr);
	return 0;
}

static int tda1004x_read_ucblocks(struct dvb_frontend* fe, u32* ucblocks)
{
	struct tda1004x_state* state = fe->demodulator_priv;
	int tmp;
	int tmp2;
	int counter;

	dprintk("%s\n", __FUNCTION__);

	// read the UCBLOCKS and reset
	counter = 0;
	tmp = tda1004x_read_byte(state, TDA1004X_UNCOR);
	if (tmp < 0)
		return -EIO;
	tmp &= 0x7f;
	while (counter++ < 5) {
		tda1004x_write_mask(state, TDA1004X_UNCOR, 0x80, 0);
		tda1004x_write_mask(state, TDA1004X_UNCOR, 0x80, 0);
		tda1004x_write_mask(state, TDA1004X_UNCOR, 0x80, 0);

		tmp2 = tda1004x_read_byte(state, TDA1004X_UNCOR);
		if (tmp2 < 0)
			return -EIO;
		tmp2 &= 0x7f;
		if ((tmp2 < tmp) || (tmp2 == 0))
			break;
	}

	if (tmp != 0x7f)
		*ucblocks = tmp;
	else
		*ucblocks = 0xffffffff;

	dprintk("%s: ucblocks=0x%x\n", __FUNCTION__, *ucblocks);
	return 0;
}

static int tda1004x_read_ber(struct dvb_frontend* fe, u32* ber)
{
	struct tda1004x_state* state = fe->demodulator_priv;
	int tmp;

	dprintk("%s\n", __FUNCTION__);

	// read it in
	tmp = tda1004x_read_byte(state, TDA1004X_CBER_LSB);
	if (tmp < 0)
		return -EIO;
	*ber = tmp << 1;
	tmp = tda1004x_read_byte(state, TDA1004X_CBER_MSB);
	if (tmp < 0)
		return -EIO;
	*ber |= (tmp << 9);
	tda1004x_read_byte(state, TDA1004X_CBER_RESET);

	dprintk("%s: ber=0x%x\n", __FUNCTION__, *ber);
	return 0;
}

static int tda1004x_sleep(struct dvb_frontend* fe)
{
	struct tda1004x_state* state = fe->demodulator_priv;

	switch (state->demod_type) {
	case TDA1004X_DEMOD_TDA10045:
		tda1004x_write_mask(state, TDA1004X_CONFADC1, 0x10, 0x10);
		break;

	case TDA1004X_DEMOD_TDA10046:
		if (state->config->pll_sleep != NULL) {
			tda1004x_enable_tuner_i2c(state);
			state->config->pll_sleep(fe);
			if (state->config->if_freq != TDA10046_FREQ_052) {
				/* special hack for Philips EUROPA Based boards:
				 * keep the I2c bridge open for tuner access in analog mode
				 */
				tda1004x_disable_tuner_i2c(state);
			}
		}
		tda1004x_write_mask(state, TDA1004X_CONFC4, 1, 1);
		break;
	}
	state->initialised = 0;

	return 0;
}

static int tda1004x_get_tune_settings(struct dvb_frontend* fe, struct dvb_frontend_tune_settings* fesettings)
{
	fesettings->min_delay_ms = 800;
	/* Drift compensation makes no sense for DVB-T */
	fesettings->step_size = 0;
	fesettings->max_drift = 0;
	return 0;
}

static void tda1004x_release(struct dvb_frontend* fe)
{
	struct tda1004x_state *state = fe->demodulator_priv;
	kfree(state);
}

static struct dvb_frontend_ops tda10046_ops = {
	.info = {
		.name = "Philips TDA10046H DVB-T",
		.type = FE_OFDM,
		.frequency_min = 51000000,
		.frequency_max = 858000000,
		.frequency_stepsize = 166667,
		.caps =
		    FE_CAN_FEC_1_2 | FE_CAN_FEC_2_3 | FE_CAN_FEC_3_4 |
		    FE_CAN_FEC_5_6 | FE_CAN_FEC_7_8 | FE_CAN_FEC_AUTO |
		    FE_CAN_QPSK | FE_CAN_QAM_16 | FE_CAN_QAM_64 | FE_CAN_QAM_AUTO |
		    FE_CAN_TRANSMISSION_MODE_AUTO | FE_CAN_GUARD_INTERVAL_AUTO
	},

	.release = tda1004x_release,

	.init = tda10046_init,
	.sleep = tda1004x_sleep,

	.set_frontend = tda1004x_set_fe,
	.get_frontend = tda1004x_get_fe,
	.get_tune_settings = tda1004x_get_tune_settings,

	.read_status = tda1004x_read_status,
	.read_ber = tda1004x_read_ber,
	.read_signal_strength = tda1004x_read_signal_strength,
	.read_snr = tda1004x_read_snr,
	.read_ucblocks = tda1004x_read_ucblocks,
};

struct dvb_frontend* tda10046_attach(const struct tda1004x_config* config,
				     struct i2c_adapter* i2c)
{
	struct tda1004x_state *state;

	/* allocate memory for the internal state */
	state = kmalloc(sizeof(struct tda1004x_state), GFP_KERNEL);
	if (!state)
		return NULL;

	/* setup the state */
	state->config = config;
	state->i2c = i2c;
	memcpy(&state->ops, &tda10046_ops, sizeof(struct dvb_frontend_ops));
	state->initialised = 0;
	state->demod_type = TDA1004X_DEMOD_TDA10046;

	/* check if the demod is there */
	if (tda1004x_read_byte(state, TDA1004X_CHIPID) != 0x46) {
		kfree(state);
		return NULL;
	}

	/* create dvb_frontend */
	state->frontend.ops = &state->ops;
	state->frontend.demodulator_priv = state;
	return &state->frontend;
}

module_param(debug, int, 0644);
MODULE_PARM_DESC(debug, "Turn on/off frontend debugging (default:off).");

MODULE_DESCRIPTION("Philips TDA10045H & TDA10046H DVB-T Demodulator");
MODULE_AUTHOR("Andrew de Quincey & Robert Schlabbach");
MODULE_LICENSE("GPL");

EXPORT_SYMBOL(tda10045_attach);
EXPORT_SYMBOL(tda10046_attach);
EXPORT_SYMBOL(tda1004x_write_byte);
