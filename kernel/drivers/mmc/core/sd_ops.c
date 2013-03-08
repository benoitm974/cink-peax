/*
 *  linux/drivers/mmc/core/sd_ops.h
 *
 *  Copyright 2006-2007 Pierre Ossman
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 */

#include <linux/slab.h>
#include <linux/types.h>
#include <linux/scatterlist.h>

#include <linux/mmc/host.h>
#include <linux/mmc/card.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/sd.h>

#include "core.h"
#include "sd_ops.h"

int mmc_app_cmd(struct mmc_host *host, struct mmc_card *card)
{
	int err;
	struct mmc_command cmd = {0};

	BUG_ON(!host);
	BUG_ON(card && (card->host != host));

	cmd.opcode = MMC_APP_CMD;

	if (card) {
		cmd.arg = card->rca << 16;
		cmd.flags = MMC_RSP_SPI_R1 | MMC_RSP_R1 | MMC_CMD_AC;
	} else {
		cmd.arg = 0;
		cmd.flags = MMC_RSP_SPI_R1 | MMC_RSP_R1 | MMC_CMD_BCR;
	}

	err = mmc_wait_for_cmd(host, &cmd, 0);
	if (err)
		return err;

	/* Check that card supported application commands */
	if (!mmc_host_is_spi(host) && !(cmd.resp[0] & R1_APP_CMD))
		return -EOPNOTSUPP;

	return 0;
}
EXPORT_SYMBOL_GPL(mmc_app_cmd);

/**
 *	mmc_wait_for_app_cmd - start an application command and wait for
 			       completion
 *	@host: MMC host to start command
 *	@card: Card to send MMC_APP_CMD to
 *	@cmd: MMC command to start
 *	@retries: maximum number of retries
 *
 *	Sends a MMC_APP_CMD, checks the card response, sends the command
 *	in the parameter and waits for it to complete. Return any error
 *	that occurred while the command was executing.  Do not attempt to
 *	parse the response.
 */
int mmc_wait_for_app_cmd(struct mmc_host *host, struct mmc_card *card,
	struct mmc_command *cmd, int retries)
{
	struct mmc_request mrq = {0};

	int i, err;

	BUG_ON(!cmd);
	BUG_ON(retries < 0);

	err = -EIO;

	/*
	 * We have to resend MMC_APP_CMD for each attempt so
	 * we cannot use the retries field in mmc_command.
	 */
	for (i = 0;i <= retries;i++) {
		err = mmc_app_cmd(host, card);
		if (err) {
			/* no point in retrying; no APP commands allowed */
			if (mmc_host_is_spi(host)) {
				if (cmd->resp[0] & R1_SPI_ILLEGAL_COMMAND)
					break;
			}
			continue;
		}

		memset(&mrq, 0, sizeof(struct mmc_request));

		memset(cmd->resp, 0, sizeof(cmd->resp));
		cmd->retries = 0;

		mrq.cmd = cmd;
		cmd->data = NULL;

		mmc_wait_for_req(host, &mrq);

		err = cmd->error;
		if (!cmd->error)
			break;

		/* no point in retrying illegal APP commands */
		if (mmc_host_is_spi(host)) {
			if (cmd->resp[0] & R1_SPI_ILLEGAL_COMMAND)
				break;
		}
	}

	return err;
}

EXPORT_SYMBOL(mmc_wait_for_app_cmd);

int mmc_app_set_bus_width(struct mmc_card *card, int width)
{
	int err;
	struct mmc_command cmd = {0};

	BUG_ON(!card);
	BUG_ON(!card->host);

	cmd.opcode = SD_APP_SET_BUS_WIDTH;
	cmd.flags = MMC_RSP_R1 | MMC_CMD_AC;

	switch (width) {
	case MMC_BUS_WIDTH_1:
		cmd.arg = SD_BUS_WIDTH_1;
		break;
	case MMC_BUS_WIDTH_4:
		cmd.arg = SD_BUS_WIDTH_4;
		break;
	default:
		return -EINVAL;
	}

	err = mmc_wait_for_app_cmd(card->host, card, &cmd, MMC_CMD_RETRIES);
	if (err)
		return err;

	return 0;
}

int mmc_send_app_op_cond(struct mmc_host *host, u32 ocr, u32 *rocr)
{
	struct mmc_command cmd = {0};
	int i, err = 0;

	BUG_ON(!host);

	cmd.opcode = SD_APP_OP_COND;
	if (mmc_host_is_spi(host))
		cmd.arg = ocr & (1 << 30); /* SPI only defines one bit */
	else
		cmd.arg = ocr;
	cmd.flags = MMC_RSP_SPI_R1 | MMC_RSP_R3 | MMC_CMD_BCR;

	for (i = 100; i; i--) {
		err = mmc_wait_for_app_cmd(host, NULL, &cmd, MMC_CMD_RETRIES);
		if (err)
			break;

		/* if we're just probing, do a single pass */
		if (ocr == 0)
			break;

		/* otherwise wait until reset completes */
		if (mmc_host_is_spi(host)) {
			if (!(cmd.resp[0] & R1_SPI_IDLE))
				break;
		} else {
			if (cmd.resp[0] & MMC_CARD_BUSY)
				break;
		}

		err = -ETIMEDOUT;

		mmc_delay(10);
	}

	if (rocr && !mmc_host_is_spi(host))
		*rocr = cmd.resp[0];

	return err;
}

int mmc_send_if_cond(struct mmc_host *host, u32 ocr)
{
	struct mmc_command cmd = {0};
	int err;
	static const u8 test_pattern = 0xAA;
	u8 result_pattern;

	/*
	 * To support SD 2.0 cards, we must always invoke SD_SEND_IF_COND
	 * before SD_APP_OP_COND. This command will harmlessly fail for
	 * SD 1.0 cards.
	 */
	cmd.opcode = SD_SEND_IF_COND;
	cmd.arg = ((ocr & 0xFF8000) != 0) << 8 | test_pattern;
	cmd.flags = MMC_RSP_SPI_R7 | MMC_RSP_R7 | MMC_CMD_BCR;

	err = mmc_wait_for_cmd(host, &cmd, 0);
	if (err)
		return err;

	if (mmc_host_is_spi(host))
		result_pattern = cmd.resp[1] & 0xFF;
	else
		result_pattern = cmd.resp[0] & 0xFF;

	if (result_pattern != test_pattern)
		return -EIO;

	return 0;
}

int mmc_send_relative_addr(struct mmc_host *host, unsigned int *rca)
{
	int err;
	struct mmc_command cmd = {0};

	BUG_ON(!host);
	BUG_ON(!rca);

	cmd.opcode = SD_SEND_RELATIVE_ADDR;
	cmd.arg = 0;
	cmd.flags = MMC_RSP_R6 | MMC_CMD_BCR;

	err = mmc_wait_for_cmd(host, &cmd, MMC_CMD_RETRIES);
	if (err)
		return err;

	*rca = cmd.resp[0] >> 16;

	return 0;
}

int mmc_app_send_scr(struct mmc_card *card, u32 *scr)
{
	int err;
	struct mmc_request mrq = {0};
	struct mmc_command cmd = {0};
	struct mmc_data data = {0};
	struct scatterlist sg;
	void *data_buf;

	BUG_ON(!card);
	BUG_ON(!card->host);
	BUG_ON(!scr);

	/* NOTE: caller guarantees scr is heap-allocated */

	err = mmc_app_cmd(card->host, card);
	if (err)
		return err;

	/* dma onto stack is unsafe/nonportable, but callers to this
	 * routine normally provide temporary on-stack buffers ...
	 */
	data_buf = kmalloc(sizeof(card->raw_scr), GFP_KERNEL);
	if (data_buf == NULL)
		return -ENOMEM;

	mrq.cmd = &cmd;
	mrq.data = &data;

	cmd.opcode = SD_APP_SEND_SCR;
	cmd.arg = 0;
	cmd.flags = MMC_RSP_SPI_R1 | MMC_RSP_R1 | MMC_CMD_ADTC;

	data.blksz = 8;
	data.blocks = 1;
	data.flags = MMC_DATA_READ;
	data.sg = &sg;
	data.sg_len = 1;

	sg_init_one(&sg, data_buf, 8);

	mmc_set_data_timeout(&data, card);

	mmc_wait_for_req(card->host, &mrq);

	memcpy(scr, data_buf, sizeof(card->raw_scr));
	kfree(data_buf);

	if (cmd.error)
		return cmd.error;
	if (data.error)
		return data.error;

	scr[0] = be32_to_cpu(scr[0]);
	scr[1] = be32_to_cpu(scr[1]);

	return 0;
}

int mmc_sd_switch(struct mmc_card *card, int mode, int group,
	u8 value, u8 *resp)
{
	struct mmc_request mrq = {0};
	struct mmc_command cmd = {0};
	struct mmc_data data = {0};
	struct scatterlist sg;

	BUG_ON(!card);
	BUG_ON(!card->host);

	/* NOTE: caller guarantees resp is heap-allocated */

	mode = !!mode;
	value &= 0xF;

	mrq.cmd = &cmd;
	mrq.data = &data;

	cmd.opcode = SD_SWITCH;
	cmd.arg = mode << 31 | 0x00FFFFFF;
	cmd.arg &= ~(0xF << (group * 4));
	cmd.arg |= value << (group * 4);
	cmd.flags = MMC_RSP_SPI_R1 | MMC_RSP_R1 | MMC_CMD_ADTC;

	data.blksz = 64;
	data.blocks = 1;
	data.flags = MMC_DATA_READ;
	data.sg = &sg;
	data.sg_len = 1;

	sg_init_one(&sg, resp, 64);

	mmc_set_data_timeout(&data, card);

	mmc_wait_for_req(card->host, &mrq);

	if (cmd.error)
		return cmd.error;
	if (data.error)
		return data.error;

	return 0;
}

int mmc_app_sd_status(struct mmc_card *card, void *ssr)
{
	int err;
	struct mmc_request mrq = {0};
	struct mmc_command cmd = {0};
	struct mmc_data data = {0};
	struct scatterlist sg;

	BUG_ON(!card);
	BUG_ON(!card->host);
	BUG_ON(!ssr);

	/* NOTE: caller guarantees ssr is heap-allocated */

	err = mmc_app_cmd(card->host, card);
	if (err)
		return err;

	mrq.cmd = &cmd;
	mrq.data = &data;

	cmd.opcode = SD_APP_SD_STATUS;
	cmd.arg = 0;
	cmd.flags = MMC_RSP_SPI_R2 | MMC_RSP_R1 | MMC_CMD_ADTC;

	data.blksz = 64;
	data.blocks = 1;
	data.flags = MMC_DATA_READ;
	data.sg = &sg;
	data.sg_len = 1;

	sg_init_one(&sg, ssr, 64);

	mmc_set_data_timeout(&data, card);

	mmc_wait_for_req(card->host, &mrq);

	if (cmd.error)
		return cmd.error;
	if (data.error)
		return data.error;

	return 0;
}
#ifdef TINNO_ANDROID_SD_SECURE
#define MMC_LOCK_MODE_ERASE		(1 << 3)
#define MMC_LOCK_MODE_UNLOCK		(0 << 2)
#define MMC_LOCK_MODE_LOCK		(1 << 2)
#define MMC_LOCK_MODE_CLR_PWD	(1 << 1)
#define MMC_LOCK_MODE_SET_PWD	(1 << 0)

int mmc_sd_lock_unlock(struct mmc_card *card, uint32_t mode, 
	uint8_t *passwd, int passwd_length)
{
	struct mmc_command cmd;
	struct mmc_data data;
	struct mmc_request mrq;
	struct scatterlist sg;
	struct mmc_host *host;
	unsigned long erase_timeout;
	int error = 0;
	int data_size;
	uint8_t *data_buf;
	struct sd_sec_info *sec_info = &card->sec_info;
	if(mode != MMC_LOCK_MODE_ERASE) 
		data_size = passwd_length + 2;
	else
		data_size = 1;
	data_buf = (uint8_t *)kzalloc(data_size, GFP_KERNEL);
	if (!data_buf) {
		printk("%s: no mem for data_buf--Liu\n", __func__);
		return -ENOMEM;
	}
	printk("+%s: mode=%x--Liu\n", __func__, mode);
	if(mode != MMC_LOCK_MODE_ERASE) {
		data_buf[0] = mode;
		data_buf[1] = passwd_length;
		memcpy((void *)(data_buf + 2), (void *)passwd, passwd_length);
	}
	else
		data_buf[0] = MMC_LOCK_MODE_ERASE;
	host = card->host;
	if(!host) {
		printk("%s: host is null--Liu\n", __func__);
		goto out;
	}
	//mmc_claim_host(host);
	
	/* CMD16, set block length */
	/* From Physical Layer Simplified Specification Version 3.01
	 *  In the case of a Standard Capacity SD Memory Card, this command sets the 
	 *  block length (in bytes) for all following block commands (read, write, lock).
	 *  Default block length is fixed to 512 Bytes. Set length is valid for memory
	 *  access commands only if partial block read operation are allowed in CSD.
	 *  In the case of SDHC and SDXC Cards, block length set by CMD16
	 *  command doen't affect memory read and write commands. Always 512
	 *  Bytes fixed block length is used. This command is effective for
	 *  LOCK_UNLOCK command. In both cases, if block length is set
	 *  larger than 512Bytes, the card sets the BLOCK_LEN_ERROR bit.
	 *  In DDR50 mode, data is sampled on both edges of the clock. Therefore,
	 *  block length shall always be even.
	*/
	memset(&cmd, 0, sizeof(struct mmc_command));
	cmd.opcode = MMC_SET_BLOCKLEN;
	cmd.arg = data_size;
	cmd.flags = MMC_RSP_R1 | MMC_CMD_AC;
	error = mmc_wait_for_cmd(card->host, &cmd, MMC_CMD_RETRIES);
	if (error) {
		printk("%s: set block length error=%d--Liu\n", __func__, error);
		goto failed;
	}
	
	/* CMD42 */
	/* command */
	memset(&cmd, 0, sizeof(struct mmc_command));
	cmd.opcode = MMC_LOCK_UNLOCK;
	cmd.arg = 0;
	cmd.flags = MMC_RSP_R1B | MMC_CMD_ADTC;
	/* data */
	memset(&data, 0, sizeof(struct mmc_data));
	mmc_set_data_timeout(&data, card);
	data.blksz = data_size;
	data.blocks = 1;
	data.flags = MMC_DATA_WRITE;
	data.sg = &sg;
	data.sg_len = 1;
	/* req */
	memset(&mrq, 0, sizeof(struct mmc_request));
	mrq.cmd = &cmd;
	mrq.data = &data;
	sg_init_one(&sg, data_buf, data_size);
	mmc_wait_for_req(card->host, &mrq);
	if (cmd.error) {
		printk("%s: set cmd42 cmd error=%d--Liu\n", __func__, cmd.error);
		goto failed;
	}
	if (data.error) {
		printk("%s: set cmd42 data error=%d--Liu\n", __func__, data.error);
		goto failed;
	}
	/* must have some sleep for card to process the password */
	msleep(50);
	/* CMD13, get card status */
	memset(&cmd, 0, sizeof(struct mmc_command));
	cmd.opcode = MMC_SEND_STATUS;
	cmd.arg = card->rca << 16;
	cmd.flags = MMC_RSP_R1 | MMC_CMD_AC;
	/* set timeout for forced erase operation to 3 min. (see MMC spec) */
	erase_timeout = jiffies + 180 * HZ;
	do {
		/* we cannot use "retries" here because the
		 * R1_LOCK_UNLOCK_FAILED bit is cleared by subsequent reads to
		 * the status register, hiding the error condition */
		error = mmc_wait_for_cmd(card->host, &cmd, 0);
		if (error) {
			printk("%s: get card status error=%d--Liu\n", __func__, error);
			break;
		}	
		/* the other modes don't need timeout checking */
		if (mode != MMC_LOCK_MODE_ERASE)
			continue;
		if (time_after(jiffies, erase_timeout)) {
			printk("%s: forced erase timed out--Liu\n", __func__);
			error = -ETIMEDOUT;
			break;
		}
	} while (!(cmd.resp[0] & R1_READY_FOR_DATA));
	if (cmd.resp[0] & R1_CARD_IS_LOCKED) {
		printk("%s: card is locked--Liu\n", __func__);
		sec_info->sd_card_locked = 1;
	}	
	else {
		printk("%s: card is unlocked--Liu\n", __func__);
		sec_info->sd_card_locked = 0;
	}	
	host->sdcard_locked = sec_info->sd_card_locked;
	if(cmd.resp[0] & R1_LOCK_UNLOCK_FAILED) {
		printk("%s: card lock_unlock error--Liu\n", __func__);
		host->passwd_error = 1;
	}
	else {
		printk("%s: card lock_unlock success--Liu\n", __func__);
		host->passwd_error = 0;
	}
failed:
	//mmc_release_host(host);
out:
	kfree(data_buf);
	printk("-%s: mode=%x--Liu\n", __func__, mode);
	return error;
}
EXPORT_SYMBOL(mmc_sd_lock_unlock);
#endif