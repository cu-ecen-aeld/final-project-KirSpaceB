cat > leddriver.c << 'EOF'
/**
 * @file leddriver.c
 * @brief Linux kernel character device driver for GPIO LED control
 * @author Kirk Flores
 * @date 2026
 */

#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/uaccess.h>

#define DRIVER_NAME "leddriver"
#define LED_GPIO_PIN 17 /* BCM GPIO 17 */
#define BUF_SIZE 8

          MODULE_AUTHOR("Kirk Flores");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("GPIO LED character device driver");

/* Device struct */
struct led_dev {
  struct cdev cdev;
  int led_state; /* 0 = off, 1 = on */
};

static struct led_dev led_device;
static int led_major = 0;
static int led_minor = 0;

static int led_open(struct inode *inode, struct file *filp) {
  struct led_dev *dev;
  dev = container_of(inode->i_cdev, struct led_dev, cdev);
  filp->private_data = dev;
  printk(KERN_DEBUG "leddriver: open\n");
  return 0;
}

static int led_release(struct inode *inode, struct file *filp) {
  printk(KERN_DEBUG "leddriver: release\n");
  return 0;
}

static ssize_t led_read(struct file *filp, char __user *buf, size_t count,
                        loff_t *f_pos) {
  struct led_dev *dev = filp->private_data;
  char state_str[BUF_SIZE];
  size_t len;

  /* Return current LED state as string */
  if (dev->led_state)
    len = snprintf(state_str, BUF_SIZE, "on\n");
  else
    len = snprintf(state_str, BUF_SIZE, "off\n");

  /* EOF - already read */
  if (*f_pos >= len)
    return 0;

  if (copy_to_user(buf, state_str, len)) {
    return -EFAULT;
  }

  *f_pos += len;
  return len;
}

static ssize_t led_write(struct file *filp, const char __user *buf,
                         size_t count, loff_t *f_pos) {
  struct led_dev *dev = filp->private_data;
  char kbuf[BUF_SIZE];
  size_t copy_size;

  /* Don't overflow our buffer */
  copy_size = min(count, (size_t)(BUF_SIZE - 1));

  if (copy_from_user(kbuf, buf, copy_size)) {
    return -EFAULT;
  }
  kbuf[copy_size] = '\0';

  /* Compare and set GPIO */
  if (strncmp(kbuf, "on", 2) == 0) {
    dev->led_state = 1;
    gpio_set_value(LED_GPIO_PIN, 1);
    printk(KERN_DEBUG "leddriver: LED on\n");
  } else if (strncmp(kbuf, "off", 3) == 0) {
    dev->led_state = 0;
    gpio_set_value(LED_GPIO_PIN, 0);
    printk(KERN_DEBUG "leddriver: LED off\n");
  } else {
    printk(KERN_WARNING "leddriver: unknown command '%s'\n", kbuf);
    return -EINVAL;
  }

  return count;
}

static struct file_operations led_fops = {
    .owner = THIS_MODULE,
    .open = led_open,
    .release = led_release,
    .read = led_read,
    .write = led_write,
};

static int led_init_module(void) {
  dev_t dev = 0;
  int result;

  /* Allocate major number */
  result = alloc_chrdev_region(&dev, led_minor, 1, DRIVER_NAME);
  if (result < 0) {
    printk(KERN_WARNING "leddriver: can't get major number\n");
    return result;
  }
  led_major = MAJOR(dev);

  /* Request GPIO pin */
  result = gpio_request(LED_GPIO_PIN, DRIVER_NAME);
  if (result < 0) {
    printk(KERN_ERR "leddriver: failed to request GPIO %d\n", LED_GPIO_PIN);
    unregister_chrdev_region(dev, 1);
    return result;
  }

  /* Set GPIO as output, initially off */
  result = gpio_direction_output(LED_GPIO_PIN, 0);
  if (result < 0) {
    printk(KERN_ERR "leddriver: failed to set GPIO direction\n");
    gpio_free(LED_GPIO_PIN);
    unregister_chrdev_region(dev, 1);
    return result;
  }

  /* Initialize and add cdev */
  memset(&led_device, 0, sizeof(struct led_dev));
  cdev_init(&led_device.cdev, &led_fops);
  led_device.cdev.owner = THIS_MODULE;
  led_device.led_state = 0;

  result = cdev_add(&led_device.cdev, dev, 1);
  if (result) {
    printk(KERN_ERR "leddriver: error %d adding cdev\n", result);
    gpio_free(LED_GPIO_PIN);
    unregister_chrdev_region(dev, 1);
    return result;
  }

  printk(KERN_INFO "leddriver: initialized on GPIO %d, major %d\n",
         LED_GPIO_PIN, led_major);
  return 0;
}

static void led_cleanup_module(void) {
  dev_t devno = MKDEV(led_major, led_minor);

  /* Turn off LED and free GPIO */
  gpio_set_value(LED_GPIO_PIN, 0);
  gpio_free(LED_GPIO_PIN);

  cdev_del(&led_device.cdev);
  unregister_chrdev_region(devno, 1);

  printk(KERN_INFO "leddriver: removed\n");
}

module_init(led_init_module);
module_exit(led_cleanup_module);
EOF
