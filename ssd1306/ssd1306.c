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


/* SSD1306 data buffer */
static u8 ssd1306_Buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];

/* Init sequence taken from the Adafruit SSD1306 Arduino library */
static void ssd1306_init_lcd(struct i2c_client *drv_client) {

    char m;
    char i;

    dev_info(dev, "ssd1306: Device init \n");
    	/* Init LCD */    
    i2c_smbus_write_byte_data(drv_client, 0x00, 0xAE); //display off
    i2c_smbus_write_byte_data(drv_client, 0x00, 0x20); //Set Memory Addressing Mode
    i2c_smbus_write_byte_data(drv_client, 0x00, 0x10); //00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
    i2c_smbus_write_byte_data(drv_client, 0x00, 0xB0); //Set Page Start Address for Page Addressing Mode,0-7
    i2c_smbus_write_byte_data(drv_client, 0x00, 0xC8); //Set COM Output Scan Direction
    i2c_smbus_write_byte_data(drv_client, 0x00, 0x00); //---set low column address
    i2c_smbus_write_byte_data(drv_client, 0x00, 0x10); //---set high column address
    i2c_smbus_write_byte_data(drv_client, 0x00, 0x40); //--set start line address
    i2c_smbus_write_byte_data(drv_client, 0x00, 0x81); //--set contrast control register
    i2c_smbus_write_byte_data(drv_client, 0x00, 0x0A);
    i2c_smbus_write_byte_data(drv_client, 0x00, 0xA1); //--set segment re-map 0 to 127
    i2c_smbus_write_byte_data(drv_client, 0x00, 0xA6); //--set normal display
    i2c_smbus_write_byte_data(drv_client, 0x00, 0xA8); //--set multiplex ratio(1 to 64)
    i2c_smbus_write_byte_data(drv_client, 0x00, 0x3F); //
    i2c_smbus_write_byte_data(drv_client, 0x00, 0xA4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
    i2c_smbus_write_byte_data(drv_client, 0x00, 0xD3); //-set display offset
    i2c_smbus_write_byte_data(drv_client, 0x00, 0x00); //-not offset

    i2c_smbus_write_byte_data(drv_client, 0x00, 0xD5); //--set display clock divide ratio/oscillator frequency
    i2c_smbus_write_byte_data(drv_client, 0x00, 0xa0); //--set divide ratio
    i2c_smbus_write_byte_data(drv_client, 0x00, 0xD9); //--set pre-charge period
    i2c_smbus_write_byte_data(drv_client, 0x00, 0x22); //

    i2c_smbus_write_byte_data(drv_client, 0x00, 0xDA); //--set com pins hardware configuration
    i2c_smbus_write_byte_data(drv_client, 0x00, 0x12);
    i2c_smbus_write_byte_data(drv_client, 0x00, 0xDB); //--set vcomh
    i2c_smbus_write_byte_data(drv_client, 0x00, 0x20); //0x20,0.77xVcc
    i2c_smbus_write_byte_data(drv_client, 0x00, 0x8D); //--set DC-DC enable
    i2c_smbus_write_byte_data(drv_client, 0x00, 0x14); //
    i2c_smbus_write_byte_data(drv_client, 0x00, 0xAF); //--turn on SSD1306 panel
    
    for (m = 0; m < 8; m++) {
        i2c_smbus_write_byte_data(drv_client, 0x00, 0xB0 + m);
        i2c_smbus_write_byte_data(drv_client, 0x00, 0x00);
        i2c_smbus_write_byte_data(drv_client, 0x00, 0x10);
        // Write multi data 
        /*
        for (i = 0; i < SSD1306_WIDTH; i++) {
            i2c_smbus_write_byte_data(drv_client, 0x40, 0xaa);
        }
        */
    }   
}

int ssd1306_UpdateScreen(struct ssd1306_data *drv_data) {

    struct i2c_client *drv_client;
    char m;
    char i;

    drv_client = drv_data->client;

    for (m = 0; m < 8; m++) {
        i2c_smbus_write_byte_data(drv_client, 0x00, 0xB0 + m);
        i2c_smbus_write_byte_data(drv_client, 0x00, 0x00);
        i2c_smbus_write_byte_data(drv_client, 0x00, 0x10);
        /* Write multi data */
        for (i = 0; i < SSD1306_WIDTH; i++) {
            i2c_smbus_write_byte_data(drv_client, 0x40, ssd1306_Buffer[SSD1306_WIDTH*m +i]);
        }   
    }

    return 0;
}



void ssd1306_clear(u8 color){

	memset(ssd1306_Buffer, color, sizeof(ssd1306_Buffer));

	//ssd1306_UpdateScreen(lcd);
}

static ssize_t clear_show(struct class *class,
	struct class_attribute *attr, char *buf)
{
	//int ret;
	ssize_t i = 0;

	i += sprintf(buf, "sys_lcd_clear\n");
	dev_info(dev, "%s\n", __FUNCTION__);
	
	ssd1306_clear(0);
    ssd1306_UpdateScreen(lcd);

	return i;
}


static ssize_t paint_show(struct class *class,
	struct class_attribute *attr, char *buf)
{
	ssize_t i = 0;
	//_Point center = {64, 32};

	i += sprintf(buf, "sys_lcd_paint\n");
	dev_info(dev, "%s\n", __FUNCTION__);

	ssd1306_clear(0xAA);

	ssd1306_UpdateScreen(lcd);

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


    ssd1306_init_lcd(drv_client);
    //ssd1306_UpdateScreen(lcd);

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
