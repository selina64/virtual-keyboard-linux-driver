
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/ioctl.h>
#include <linux/uaccess.h>		
#include <asm/io.h>
#include <linux/platform_device.h>
#include <linux/version.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/input.h>
#include <linux/bits.h>

#if LINUX_VERSION_CODE > KERNEL_VERSION(3, 3, 0)
        #include <asm/switch_to.h>
#else
        #include <asm/system.h>
#endif

// function prototype declaration
static ssize_t io_bin_cat_callback(struct device *dev,struct device_attribute *attr, char *buf)  ;
static ssize_t io_bin_echo_callback(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);


uint8_t key_state; // store the current state of the virtual keyboard (or key '0' exactly)
struct input_dev *kb_dev; // keyboard device, used to send key values to system and simulate the illusion that a key is pressed or released.
struct kobject *kobj; // used to create sysfs file
static DEVICE_ATTR(io_bin, 0660, io_bin_cat_callback, io_bin_echo_callback); // used to handle io of the sysfs file

// called when cat is performed on io_bin
static ssize_t io_bin_cat_callback(struct device *dev,struct device_attribute *attr, char *buf)  
{
	// buf is the string that will be output by cat
    printk(KERN_INFO "*********io_bin_cat_callback: key_state: %d\n", key_state);
	buf[0] = key_state;
    return 1;
}

// called when echo is performed on io_bin
static ssize_t io_bin_echo_callback(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	// is the string that was written in by echo
	key_state = buf[0]; 
    printk(KERN_INFO "*********io_bin_echo_callback: key_state: %d\n", key_state);
	if (buf[0] == 0x31)
	{
		input_report_key(kb_dev, KEY_0, 0X01); /* 按键按下 */
		input_sync(kb_dev);
	}
	else if (buf[0] == 0x30)
	{
		input_report_key(kb_dev, KEY_0, 0X00); /* 按键抬起 */
		input_sync(kb_dev);
	}
	printk("key value [%d]\r\n", buf[0]);
    return count;
}

// called on insmod
static int __init virtual_keyboard_drv_init(void)
{
    printk(KERN_INFO "*********virtual_keyboard_drv_init\n");

    int ret = -1;

	/* input device (keyboard) register begin */
	kb_dev = input_allocate_device();
	kb_dev->name = "virtual_keyboard";
	kb_dev->evbit[0] = BIT_MASK(EV_KEY)/* | BIT_MASK(EV_REP)*/; /* 不创建重复事件 */
	input_set_capability(kb_dev, EV_KEY, KEY_0);	/* 设置键值 */
	
	ret = input_register_device(kb_dev);
	if (ret != 0)
	{
		printk("input device register failed.\r\n");
		return ret;
	}
	/* input device (keyboard) register end */

	/* sysfs_create_file begin */
	kobj = kobject_create_and_add("virtual_keyboard", kernel_kobj);
	sysfs_create_file(kobj, &dev_attr_io_bin.attr); // file path is /sys/kernel/{kobj->name}/{dev_attr_io_bin.attr.name}
	/* sysfs_create_file end */
	
	return 0;
}

static void __exit virtual_keyboard_drv_exit(void)
{
    printk(KERN_INFO "*********virtual_keyboard_drv_exit\n");
	kobject_del(kobj);
    kobject_put(kobj);
}

module_init(virtual_keyboard_drv_init);
module_exit(virtual_keyboard_drv_exit);
MODULE_LICENSE("GPL");



