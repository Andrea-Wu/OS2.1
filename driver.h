#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kdev_t.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/splice.h>
#include <linux/slab.h>

MODULE_LICENSE("Dual BSD/GPL");
