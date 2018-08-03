/*
 * Texas Instruments TMP103 SMBus temperature sensor driver
 * Copyright (C) 2014 Heiko Schocher <hs@denx.de>
 *
 * Based on:
 * Texas Instruments TMP102 SMBus temperature sensor driver
 *
 * Copyright (C) 2010 Steven King <sfking@fdwdc.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/err.h>


#define DEVICE_NAME	"lcd_ssd1306"
#define CLASS_NAME	"oled_display" 
#define BUS_NAME	"i2c_1"

#define SSD1306_WIDTH	128
#define SSD1306_HEIGHT	64


static struct device *dev;




struct ssd1306_data {
    struct i2c_client *client;
	struct class *sys_class;
    int status;

	int value;
};
static struct ssd1306_data *lcd;



static ssize_t clear_show(struct class *class,
	struct class_attribute *attr, char *buf)
{
	//int ret;
	ssize_t i = 0;

	i += sprintf(buf, "sys_lcd_clear\n");
	
	dev_info(dev, "sys_lcd_clear\n");

	return i;
}


static ssize_t paint_show(struct class *class,
	struct class_attribute *attr, char *buf)
{
	ssize_t i = 0;
	//_Point center = {64, 32};

	i += sprintf(buf, "sys_lcd_paint\n");

	dev_info(dev, "sys_lcd_paint\n");

	return i;
}



CLASS_ATTR_RO(clear);
CLASS_ATTR_RO(paint);


static void make_sysfs_entry(struct i2c_client *drv_client)
{
	struct device_node *np = drv_client->dev.of_node;
	const char *name;
	int res;

	struct class *sys_class;

	if (np) {

		/*
		if (!of_property_read_string(np, "label", &name))
			dev_info(dev, "label = %s\n", name);
		*/

		sys_class = class_create(THIS_MODULE, DEVICE_NAME);

		if (IS_ERR(sys_class)){
			dev_err(dev, "bad class create\n");
		}
		else{
			res = class_create_file(sys_class, &class_attr_clear);
			res = class_create_file(sys_class, &class_attr_paint);


			lcd->sys_class = sys_class;
		}
	}

}




static int ssd1306_probe(struct i2c_client *drv_client,
			const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter;

	dev = &drv_client->dev;

	dev_info(dev, "init I2C driver\n");


    lcd = devm_kzalloc(&drv_client->dev, sizeof(struct ssd1306_data),
                        GFP_KERNEL);
    if (!lcd)
        return -ENOMEM;

    lcd->client = drv_client;
    lcd->status = 0xABCD;
    lcd->value 	= 10;

    i2c_set_clientdata(drv_client, lcd);

    adapter = drv_client->adapter;

    if (!adapter)
    {
        dev_err(dev, "adapter indentification error\n");
        return -ENODEV;
    }

    dev_info(dev, "I2C client address %d \n", drv_client->addr);

    if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
            dev_err(dev, "operation not supported\n");
            return -ENODEV;
    }


	make_sysfs_entry(drv_client);

    dev_info(dev, "ssd1306 driver successfully loaded\n");

	return 0;
}



static int ssd1306_remove(struct i2c_client *client)
{
	struct class *sys_class;

	sys_class = lcd->sys_class;

	class_remove_file(sys_class, &class_attr_clear);
	class_remove_file(sys_class, &class_attr_paint);
	class_destroy(sys_class);

	dev_info(dev, "Goodbye, world!\n");
	return 0;
}



static const struct of_device_id ssd1306_match[] = {
	{ .compatible = "DAndy,lcd_ssd1306", },
	{ },
};
MODULE_DEVICE_TABLE(of, ssd1306_match);

static const struct i2c_device_id ssd1306_id[] = {
	{ "lcd_ssd1306", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, ssd1306_id);


static struct i2c_driver ssd1306_driver = {
	.driver = {
		.name	= DEVICE_NAME,
		.of_match_table = ssd1306_match,
	},
	.probe		= ssd1306_probe,
	.remove 	= ssd1306_remove,
	.id_table	= ssd1306_id,
};
module_i2c_driver(ssd1306_driver);

MODULE_AUTHOR("Devyatov Andrey <andrii.deviatov@globallogic.com>");
MODULE_DESCRIPTION("ssd1306 I2C");
MODULE_LICENSE("GPL");
