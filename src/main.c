/*
	Copyright (C) 2010  Commtech, Inc.
	
	This file is part of fscc-linux.

	fscc-linux is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	fscc-linux is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with fscc-linux.  If not, see <http://www.gnu.org/licenses/>.
	
*/

#include <linux/poll.h> /* poll_wait, POLL* */
#include "card.h" /* struct fscc_card */
#include "port.h" /* struct fscc_port */
#include "config.h" /* DEVICE_NAME, DEFAULT_* */
#include "utils.h" /* DEVICE_NAME, DEFAULT_* */

static int fscc_major_number;
static struct class *fscc_class = 0;
unsigned memory_cap = DEFAULT_MEMORY_CAP_VALUE;
unsigned hot_plug = DEFAULT_HOT_PLUG_VALUE;

wait_queue_head_t output_queue;
LIST_HEAD(fscc_cards);

struct pci_device_id fscc_id_table[] __devinitdata = {
	{ COMMTECH_VENDOR_ID, FSCC_ID, PCI_ANY_ID, 0, 0, 0 },
	{ COMMTECH_VENDOR_ID, FSCC_232_ID, PCI_ANY_ID, 0, 0, 0 },
	{ COMMTECH_VENDOR_ID, FSCC_4_ID, PCI_ANY_ID, 0, 0, 0 },
	{ COMMTECH_VENDOR_ID, SFSCC_ID, PCI_ANY_ID, 0, 0, 0 },
	{ COMMTECH_VENDOR_ID, SFSCC_4_ID, PCI_ANY_ID, 0, 0, 0 },
	{ COMMTECH_VENDOR_ID, SFSCC_4_LVDS_ID, PCI_ANY_ID, 0, 0, 0 },
	{ COMMTECH_VENDOR_ID, SFSCCe_4_ID, PCI_ANY_ID, 0, 0, 0 },
	{ 0, },
};

int fscc_open(struct inode *inode, struct file *file)
{
	struct fscc_port *current_port = 0;
	
	current_port = container_of(inode->i_cdev, struct fscc_port, cdev);	
	file->private_data = current_port;
	
	return 0;
}

unsigned fscc_memory_usage(void)
{	
	struct fscc_card *current_card = 0;
	unsigned memory = 0;
	
	list_for_each_entry(current_card, &fscc_cards, list) {	
		memory += fscc_card_get_memory_usage(current_card);
	}
	
	return memory;
}

//Returns -ENOBUFS if read size is smaller than next frame
//Returns -EOPNOTSUPP if in async mode
ssize_t fscc_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
	struct fscc_port *port = 0;
	ssize_t read_count = 0;
	
	port = file->private_data;
		
	if (fscc_port_using_async(port)) {
		dev_warn(port->device, "use /dev/ttySx nodes while in async mode\n");
		return -EOPNOTSUPP;
	}
	
	if (down_interruptible(&port->read_semaphore))
		return -ERESTARTSYS;
		
	while (!fscc_port_has_iframes(port)) {
		up(&port->read_semaphore);
		
		if (file->f_flags & O_NONBLOCK)
			return -EAGAIN;
			
		if (wait_event_interruptible(port->input_queue, 
		                             fscc_port_has_iframes(port))) {
			return -ERESTARTSYS;
		}
			
		if (down_interruptible(&port->read_semaphore))
			return -ERESTARTSYS;
	}
	
	read_count = fscc_port_read(port, buf, count);
	up(&port->read_semaphore);
	
	return read_count;
}

ssize_t fscc_write(struct file *file, const char *buf, size_t count, 
                   loff_t *ppos)
{
	struct fscc_port *port = 0;
	
	port = file->private_data;
		
	if (fscc_port_using_async(port)) {
		dev_warn(port->device, "use /dev/ttySx nodes while in async mode\n");
		return -EOPNOTSUPP;
	}
	
	if (down_interruptible(&port->write_semaphore))
		return -ERESTARTSYS;
		
	while (fscc_memory_usage() + count > memory_cap) {
		up(&port->write_semaphore);
		
		if (file->f_flags & O_NONBLOCK)
			return -EAGAIN;
			
		if (wait_event_interruptible(output_queue, 
		                          fscc_memory_usage() + count <= memory_cap)) {
			return -ERESTARTSYS;
		}
			
		if (down_interruptible(&port->write_semaphore))
			return -ERESTARTSYS;
	}
		
	fscc_port_write(port, buf, count);
	
	up(&port->write_semaphore);
	
	return count;
}

unsigned fscc_poll(struct file *file, struct poll_table_struct *wait)
{
	struct fscc_port *port = 0;
	unsigned mask = 0;
	
	port = file->private_data;
	
	down(&port->poll_semaphore);
	
	poll_wait(file, &port->input_queue, wait);
	poll_wait(file, &output_queue, wait);
	
	if (fscc_port_has_iframes(port))
		mask |= POLLIN | POLLRDNORM;
		
	if (fscc_memory_usage() < memory_cap)
		mask |= POLLOUT | POLLWRNORM;
	
	up(&port->poll_semaphore);
	
	return mask;
}

int fscc_ioctl(struct inode *inode, struct file *file, unsigned int cmd, 
               unsigned long arg)
{
	struct fscc_port *port = 0;
	
	port = file->private_data;
	
	switch (cmd) {
	case FSCC_GET_REGISTERS:
		fscc_port_get_registers(port, (struct fscc_registers *)arg);
		break;

	case FSCC_SET_REGISTERS:
		fscc_port_set_registers(port, (struct fscc_registers *)arg);
		break;
			
	case FSCC_FLUSH_TX:
		fscc_port_flush_tx(port);
		break;
			
	case FSCC_FLUSH_RX:
		fscc_port_flush_rx(port);
		break;
		
	case FSCC_ENABLE_APPEND_STATUS:
		fscc_port_set_append_status(port, 1);
		break;
		
	case FSCC_DISABLE_APPEND_STATUS:
		fscc_port_set_append_status(port, 0);
		break;
		
	case FSCC_SET_CLOCK_BITS:
		fscc_port_set_clock_bits(port, (char *)arg);
		break;
			
	default:
		dev_dbg(port->device, "unknown ioctl 0x%x\n", cmd);
		return -ENOTTY;			
	}
	
	return 0;
}

static struct file_operations fscc_fops = {
	.owner = THIS_MODULE,
	.open = fscc_open,
	.read = fscc_read,
	.write = fscc_write,
	.poll = fscc_poll,
	.ioctl = fscc_ioctl,
};

static int __devinit fscc_probe(struct pci_dev *pdev, 
                                const struct pci_device_id *id)
{
	struct fscc_card *new_card = 0;
	
	if (pci_enable_device(pdev))
		return -EIO;
	
	new_card = fscc_card_new(pdev, fscc_major_number, fscc_class,
                                 &fscc_fops);
		
	if (new_card)                         
		list_add_tail(&new_card->list, &fscc_cards);
	
	return 0;
}

static void __devexit fscc_remove(struct pci_dev *pdev)
{
	struct fscc_card *card = 0;
	
	card = fscc_card_find(pdev, &fscc_cards);
	
	if (card == 0)
		return;
	
	list_del(&card->list);
	fscc_card_delete(card);	
	pci_disable_device(pdev);
}

static int fscc_suspend(struct pci_dev *pdev, pm_message_t state)
{
	struct fscc_card *card = 0;
		
	card = fscc_card_find(pdev, &fscc_cards);
	
	dev_dbg(fscc_card_get_device(card), "suspending\n");
	
	fscc_card_suspend(card);

	pci_save_state(pdev);
	pci_set_power_state(pdev, pci_choose_state(pdev, state));
	
	return 0;
}

static int fscc_resume(struct pci_dev *pdev)
{
	struct fscc_card *card = 0;	
	
	card = fscc_card_find(pdev, &fscc_cards);
	
	dev_dbg(fscc_card_get_device(card), "resuming\n");

	pci_set_power_state(pdev, PCI_D0);
	pci_restore_state(pdev);
		
	fscc_card_resume(card);

	return 0;
}

struct pci_driver fscc_pci_driver = {
	.name = DEVICE_NAME,
	.probe = fscc_probe,
	.remove = fscc_remove,
	.suspend = fscc_suspend,
	.resume = fscc_resume,
	.id_table = fscc_id_table,
};

static int __init fscc_init(void)
{
	unsigned registered_devices = 0;
	unsigned num_devices = 0;
	int error_code = 0;
	
	fscc_class = class_create(THIS_MODULE, DEVICE_NAME);
	
	if (IS_ERR(fscc_class)) {
		printk(KERN_ERR DEVICE_NAME " class_create failed\n");
		return PTR_ERR(fscc_class);
	}
	
	fscc_major_number = error_code = register_chrdev(0, "fscc", &fscc_fops);
	
	if (fscc_major_number < 0) {
		printk(KERN_ERR DEVICE_NAME " register_chrdev failed\n");
		class_destroy(fscc_class);
		return error_code;
	}
	
	registered_devices = error_code = pci_register_driver(&fscc_pci_driver);
	
	if (registered_devices < 0) {
		printk(KERN_ERR DEVICE_NAME " pci_register_driver failed\n");
		unregister_chrdev(fscc_major_number, "fscc");
		class_destroy(fscc_class);
		return error_code;
	}

	if (hot_plug == 0 && registered_devices == 0) {	
		struct pci_dev *pdev = NULL;	
		
		pdev = pci_get_device(COMMTECH_VENDOR_ID, PCI_ANY_ID, pdev);
		
		while (pdev != NULL) {
			if (is_fscc_device(pdev))
				++num_devices;  
			
			pdev = pci_get_device(COMMTECH_VENDOR_ID, PCI_ANY_ID, pdev);
		}
	
		if (num_devices == 0) {
			pci_unregister_driver(&fscc_pci_driver);
			unregister_chrdev(fscc_major_number, "fscc");
			class_destroy(fscc_class);
			return -ENODEV;
		}		
	}

	if (memory_cap != DEFAULT_MEMORY_CAP_VALUE)
		printk(KERN_INFO DEVICE_NAME " changing the memory cap from %u => %u (bytes)\n", 
		       DEFAULT_MEMORY_CAP_VALUE, memory_cap);
	
	init_waitqueue_head(&output_queue);
	
	return 0;
}

static void __exit fscc_exit(void)
{
	pci_unregister_driver(&fscc_pci_driver);
	unregister_chrdev(fscc_major_number, DEVICE_NAME);
	class_destroy(fscc_class);	
}

MODULE_DEVICE_TABLE(pci, fscc_id_table);

MODULE_LICENSE("GPL");
MODULE_VERSION("2.0 beta");
MODULE_AUTHOR("William Fagan <willf@commtech-fastcom.com>");

MODULE_DESCRIPTION("Driver for the FSCC series of cards from Commtech, Inc."); 

module_param(memory_cap, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(memory_cap, "The maximum user data (in bytes) the driver will allow in it's buffer.");

module_param(hot_plug, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(hot_plug, "Let's the driver load even if no devices exist.");

module_init(fscc_init);
module_exit(fscc_exit); 
 
