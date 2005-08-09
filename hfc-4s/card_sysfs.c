#include <linux/kernel.h>
#include <linux/spinlock.h>

#include "card.h"
#include "card_inline.h"
#include "fifo.h"
#include "fifo_inline.h"

static ssize_t hfc_show_output_level(
	struct device *device,
	char *buf)
{
	struct pci_dev *pci_dev = to_pci_dev(device);
	struct hfc_card *card = pci_get_drvdata(pci_dev);

	return snprintf(buf, PAGE_SIZE, "%02x\n", card->output_level);
}

static ssize_t hfc_store_output_level(
	struct device *device,
	const char *buf,
	size_t count)
{
	struct pci_dev *pci_dev = to_pci_dev(device);
	struct hfc_card *card = pci_get_drvdata(pci_dev);
	
	int value;
	if (sscanf(buf, "%x", &value) < 1)
		return -EINVAL;

	if (value < 0 || value > 0xff)
		return -EINVAL;

	unsigned long flags;
	spin_lock_irqsave(&card->lock, flags);
	hfc_outb(card, hfc_R_PWM1, value);
	spin_unlock_irqrestore(&card->lock, flags);

	return count;
}

static DEVICE_ATTR(output_level, S_IRUGO | S_IWUSR,
		hfc_show_output_level,
		hfc_store_output_level);

//----------------------------------------------------------------------------

static ssize_t hfc_show_bert_mode(
	struct device *device,
	char *buf)
{
	struct pci_dev *pci_dev = to_pci_dev(device);
	struct hfc_card *card = pci_get_drvdata(pci_dev);

	return snprintf(buf, PAGE_SIZE, "%d\n", card->bert_mode);
}

static ssize_t hfc_store_bert_mode(
	struct device *device,
	const char *buf,
	size_t count)
{
	struct pci_dev *pci_dev = to_pci_dev(device);
	struct hfc_card *card = pci_get_drvdata(pci_dev);
	
	int mode;
	sscanf(buf, "%d", &mode);

	unsigned long flags;
	spin_lock_irqsave(&card->lock, flags);
	card->bert_mode = mode;
	hfc_update_bert_wd_md(card, 0);
	spin_unlock_irqrestore(&card->lock, flags);

	return count;
}

static DEVICE_ATTR(bert_mode, S_IRUGO | S_IWUSR,
		hfc_show_bert_mode,
		hfc_store_bert_mode);

//----------------------------------------------------------------------------

static ssize_t hfc_show_bert_err(
	struct device *device,
	char *buf)
{
	return -EOPNOTSUPP;
}

static ssize_t hfc_store_bert_err(
	struct device *device,
	const char *buf,
	size_t count)
{
	struct pci_dev *pci_dev = to_pci_dev(device);
	struct hfc_card *card = pci_get_drvdata(pci_dev);
	
	unsigned long flags;
	spin_lock_irqsave(&card->lock, flags);
	hfc_update_bert_wd_md(card, hfc_R_BERT_WD_MD_V_BERT_ERR);
	spin_unlock_irqrestore(&card->lock, flags);

	return count;
}

static DEVICE_ATTR(bert_err, S_IWUSR,
		hfc_show_bert_err,
		hfc_store_bert_err);

//----------------------------------------------------------------------------

static ssize_t hfc_show_bert_sync(
	struct device *device,
	char *buf)
{
	struct pci_dev *pci_dev = to_pci_dev(device);
	struct hfc_card *card = pci_get_drvdata(pci_dev);

	return snprintf(buf, PAGE_SIZE, "%d\n",
		(hfc_inb(card, hfc_R_BERT_STA) &
			hfc_R_BERT_STA_V_BERT_SYNC) ? 1 : 0);
}

static DEVICE_ATTR(bert_sync, S_IRUGO,
		hfc_show_bert_sync,
		NULL);

//----------------------------------------------------------------------------

static ssize_t hfc_show_bert_inv(
	struct device *device,
	char *buf)
{
	struct pci_dev *pci_dev = to_pci_dev(device);
	struct hfc_card *card = pci_get_drvdata(pci_dev);

	return snprintf(buf, PAGE_SIZE, "%d\n",
		(hfc_inb(card, hfc_R_BERT_STA) &
			hfc_R_BERT_STA_V_BERT_INV_DATA) ? 1 : 0);
}

static DEVICE_ATTR(bert_inv, S_IRUGO,
		hfc_show_bert_inv,
		NULL);

//----------------------------------------------------------------------------

static ssize_t hfc_show_bert_cnt(
	struct device *device,
	char *buf)
{
	struct pci_dev *pci_dev = to_pci_dev(device);
	struct hfc_card *card = pci_get_drvdata(pci_dev);

	int cnt;
	unsigned long flags;
	spin_lock_irqsave(&card->lock, flags);
	cnt = hfc_inb(card, hfc_R_BERT_ECL);
	cnt += hfc_inb(card, hfc_R_BERT_ECH) << 8;
	spin_unlock_irqrestore(&card->lock, flags);

	return snprintf(buf, PAGE_SIZE, "%d\n", cnt);
}

static DEVICE_ATTR(bert_cnt, S_IRUGO,
		hfc_show_bert_cnt,
		NULL);

//----------------------------------------------------------------------------

static ssize_t hfc_show_ramsize(
	struct device *device,
	char *buf)
{
	struct pci_dev *pci_dev = to_pci_dev(device);
	struct hfc_card *card = pci_get_drvdata(pci_dev);

	return snprintf(buf, PAGE_SIZE,
		"%d\n",
		card->ramsize);
}

static ssize_t hfc_store_ramsize(
	struct device *device,
	const char *buf,
	size_t count)
{
	struct pci_dev *pci_dev = to_pci_dev(device);
	struct hfc_card *card = pci_get_drvdata(pci_dev);
	
	unsigned long flags;

	unsigned int value;
	
	if (sscanf(buf, "%u", &value) < 1)
		return -EINVAL;

	if (value != 32 &&
	    value != 128 &&
	    value != 512)
		return -EINVAL;

	if (value != card->ramsize) {
		spin_lock_irqsave(&card->lock, flags);

		card->regs.ctrl &= ~hfc_R_CTRL_V_EXT_RAM;
		card->regs.ram_misc &= ~hfc_R_RAM_MISC_V_RAM_SZ_MASK;

		if (value == 32) {
			card->regs.ram_misc |= hfc_R_RAM_MISC_V_RAM_SZ_32K;
			hfc_configure_fifos(card, 0, 0, 0);
		} else if (value == 128) {
			card->regs.ctrl |= hfc_R_CTRL_V_EXT_RAM;
			card->regs.ram_misc |= hfc_R_RAM_MISC_V_RAM_SZ_128K;
			hfc_configure_fifos(card, 1, 0, 0);
		} else if (value == 512) {
			card->regs.ctrl |= hfc_R_CTRL_V_EXT_RAM;
			card->regs.ram_misc |= hfc_R_RAM_MISC_V_RAM_SZ_512K;
			hfc_configure_fifos(card, 2, 0, 0);
		}

		hfc_outb(card, hfc_R_CTRL, card->regs.ctrl);
		hfc_outb(card, hfc_R_RAM_MISC, card->regs.ram_misc);

		hfc_softreset(card);
		hfc_initialize_hw(card);

		card->ramsize = value;

		spin_unlock_irqrestore(&card->lock, flags);

		hfc_debug_card(card, 1, "RAM size set to %d\n", value);
	}

	return count;
}

static DEVICE_ATTR(ramsize, S_IRUGO | S_IWUSR,
		hfc_show_ramsize,
		hfc_store_ramsize);

//----------------------------------------------------------------------------
static ssize_t hfc_show_clock_source_config(
	struct device *device,
	char *buf)
{
	struct pci_dev *pci_dev = to_pci_dev(device);
	struct hfc_card *card = pci_get_drvdata(pci_dev);

	if (card->clock_source >= 0)
		return snprintf(buf, PAGE_SIZE, "%d\n", card->clock_source);
	else
		return snprintf(buf, PAGE_SIZE, "auto\n");
}

static ssize_t hfc_store_clock_source_config(
	struct device *device,
	const char *buf,
	size_t count)
{
	struct pci_dev *pci_dev = to_pci_dev(device);
	struct hfc_card *card = pci_get_drvdata(pci_dev);
	
	unsigned long flags;

	if (count >= 4 && !strncmp(buf, "auto", 4)) {
		card->clock_source = -1;

		hfc_debug_card(card, 1, "Clock source set to auto\n");
	} else if(count > 0) {
		int clock_source;
		sscanf(buf, "%d", &clock_source);

		card->clock_source = clock_source;

		hfc_debug_card(card, 1, "Clock source set to %d\n", clock_source);
	}

	spin_lock_irqsave(&card->lock, flags);
	hfc_update_st_sync(card);
	spin_unlock_irqrestore(&card->lock, flags);

	return count;
}

static DEVICE_ATTR(clock_source_config, S_IRUGO | S_IWUSR,
		hfc_show_clock_source_config,
		hfc_store_clock_source_config);

//----------------------------------------------------------------------------
static ssize_t hfc_show_clock_source_current(
	struct device *device,
	char *buf)
{
	struct pci_dev *pci_dev = to_pci_dev(device);
	struct hfc_card *card = pci_get_drvdata(pci_dev);

	return snprintf(buf, PAGE_SIZE, "%d\n",
		hfc_inb(card, hfc_R_BERT_STA) &
			hfc_R_BERT_STA_V_RD_SYNC_SRC_MASK);
}

static DEVICE_ATTR(clock_source_current, S_IRUGO,
		hfc_show_clock_source_current,
		NULL);

//----------------------------------------------------------------------------

static ssize_t hfc_show_dip_switches(
	struct device *device,
	char *buf)
{
	struct pci_dev *pci_dev = to_pci_dev(device);
	struct hfc_card *card = pci_get_drvdata(pci_dev);

	return snprintf(buf, PAGE_SIZE, "%02x\n",
		hfc_inb(card, hfc_R_GPIO_IN1) >> 5);
}

static DEVICE_ATTR(dip_switches, S_IRUGO,
		hfc_show_dip_switches,
		NULL);

//----------------------------------------------------------------------------

static ssize_t hfc_show_fifo_state(
	struct device *device,
	char *buf)
{
	struct pci_dev *pci_dev = to_pci_dev(device);
	struct hfc_card *card = pci_get_drvdata(pci_dev);

	int len = 0;
	len += snprintf(buf + len, PAGE_SIZE - len,
		"\n      Receive                 Transmit\n"
		"FIFO#  F1 F2   Z1   Z2 Used   F1 F2   Z1   Z2 Used Connected\n");

	int i;
	for (i=0; i<card->num_fifos; i++) {
//		if (!card->fifos[i][RX].used && !card->fifos[i][TX].used)
//			continue;

		struct hfc_fifo *fifo_rx = &card->fifos[i][RX];
		struct hfc_fifo *fifo_tx = &card->fifos[i][TX];

		unsigned long flags;
		spin_lock_irqsave(&card->lock, flags);

		len += snprintf(buf + len, PAGE_SIZE - len,
			"%2d   :", i);

		hfc_fifo_select(fifo_rx);

		union hfc_fgroup f;
		union hfc_zgroup z;

		f.f1f2 = hfc_inw(card, hfc_A_F12);
		z.z1z2 = hfc_inl(card, hfc_A_Z12);

		len += snprintf(buf + len, PAGE_SIZE - len,
			" %02x %02x %04x %04x %4d",
			f.f1, f.f2, z.z1, z.z2,
			hfc_fifo_used_rx(fifo_rx));

		hfc_fifo_select(fifo_tx);

		f.f1f2 = hfc_inw(card, hfc_A_F12);
		z.z1z2 = hfc_inl(card, hfc_A_Z12);

		len += snprintf(buf + len, PAGE_SIZE - len,
			"   %02x %02x %04x %04x %4d",
			f.f1, f.f2, z.z1, z.z2,
			hfc_fifo_used_tx(fifo_tx));

		if (fifo_rx->connected_chan) {
			len += snprintf(buf + len, PAGE_SIZE - len,
				" %d:%s",
				fifo_rx->connected_chan->chan->port->id,
				fifo_rx->connected_chan->chan->name);
		}

		if (fifo_tx->connected_chan) {
			len += snprintf(buf + len, PAGE_SIZE - len,
				" %d:%s",
				fifo_tx->connected_chan->chan->port->id,
				fifo_tx->connected_chan->chan->name);
		}

		len += snprintf(buf + len, PAGE_SIZE - len, "\n");

		spin_unlock_irqrestore(&card->lock, flags);
	}

	return len;
}

static DEVICE_ATTR(fifo_state, S_IRUGO,
		hfc_show_fifo_state,
		NULL);


int hfc_card_sysfs_create_files(
	struct hfc_card *card)
{
	int err;

	err = device_create_file(
		&card->pcidev->dev,
		&dev_attr_clock_source_config);
	if (err < 0)
		goto err_device_create_file_clock_source_config;

	err = device_create_file(
		&card->pcidev->dev,
		&dev_attr_clock_source_current);
	if (err < 0)
		goto err_device_create_file_clock_source_current;

	err = device_create_file(
		&card->pcidev->dev,
		&dev_attr_dip_switches);
	if (err < 0)
		goto err_device_create_file_dip_switches;

	err = device_create_file(
		&card->pcidev->dev,
		&dev_attr_ramsize);
	if (err < 0)
		goto err_device_create_file_ramsize;

	err = device_create_file(
		&card->pcidev->dev,
		&dev_attr_bert_mode);
	if (err < 0)
		goto err_device_create_file_bert_mode;

	err = device_create_file(
		&card->pcidev->dev,
		&dev_attr_bert_err);
	if (err < 0)
		goto err_device_create_file_bert_err;

	err = device_create_file(
		&card->pcidev->dev,
		&dev_attr_bert_sync);
	if (err < 0)
		goto err_device_create_file_bert_sync;

	err = device_create_file(
		&card->pcidev->dev,
		&dev_attr_bert_inv);
	if (err < 0)
		goto err_device_create_file_bert_inv;

	err = device_create_file(
		&card->pcidev->dev,
		&dev_attr_bert_cnt);
	if (err < 0)
		goto err_device_create_file_bert_cnt;

	err = device_create_file(
		&card->pcidev->dev,
		&dev_attr_output_level);
	if (err < 0)
		goto err_device_create_file_output_level;

	err = device_create_file(
		&card->pcidev->dev,
		&dev_attr_fifo_state);
	if (err < 0)
		goto err_device_create_file_fifo_state;

	return 0;

	device_remove_file(&card->pcidev->dev, &dev_attr_fifo_state);
err_device_create_file_fifo_state:
	device_remove_file(&card->pcidev->dev, &dev_attr_output_level);
err_device_create_file_output_level:
	device_remove_file(&card->pcidev->dev, &dev_attr_bert_cnt);
err_device_create_file_bert_cnt:
	device_remove_file(&card->pcidev->dev, &dev_attr_bert_inv);
err_device_create_file_bert_inv:
	device_remove_file(&card->pcidev->dev, &dev_attr_bert_sync);
err_device_create_file_bert_sync:
	device_remove_file(&card->pcidev->dev, &dev_attr_bert_err);
err_device_create_file_bert_err:
	device_remove_file(&card->pcidev->dev, &dev_attr_bert_mode);
err_device_create_file_bert_mode:
	device_remove_file(&card->pcidev->dev, &dev_attr_ramsize);
err_device_create_file_ramsize:
	device_remove_file(&card->pcidev->dev, &dev_attr_dip_switches);
err_device_create_file_dip_switches:
	device_remove_file(&card->pcidev->dev, &dev_attr_clock_source_current);
err_device_create_file_clock_source_current:
	device_remove_file(&card->pcidev->dev, &dev_attr_clock_source_config);
err_device_create_file_clock_source_config:

	return err;
}

void hfc_card_sysfs_delete_files(
	struct hfc_card *card)
{
	device_remove_file(&card->pcidev->dev, &dev_attr_fifo_state);

	device_remove_file(&card->pcidev->dev, &dev_attr_output_level);

	device_remove_file(&card->pcidev->dev, &dev_attr_bert_cnt);
	device_remove_file(&card->pcidev->dev, &dev_attr_bert_inv);
	device_remove_file(&card->pcidev->dev, &dev_attr_bert_sync);
	device_remove_file(&card->pcidev->dev, &dev_attr_bert_err);
	device_remove_file(&card->pcidev->dev, &dev_attr_bert_mode);

	device_remove_file(&card->pcidev->dev, &dev_attr_ramsize);
	device_remove_file(&card->pcidev->dev, &dev_attr_dip_switches);
	device_remove_file(&card->pcidev->dev, &dev_attr_clock_source_current);
	device_remove_file(&card->pcidev->dev, &dev_attr_clock_source_config);
}