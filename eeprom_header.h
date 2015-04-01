/* NAME : Ashwin Raman */
/*EEPROM INITIALIZATION HEADER FILE */
/* CONTAINS functions for
1) EEPROM initialization
2) GENERATE CLOCK FOR EEPROM
3) START AND STOP CONDTIONS FOR READING AND WRITING TO EEPROM
*/

#include <at89c51ed2.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

#define HEAP_SIZE 3200

#define SCL P1_6
#define SDA P1_7
#define NOP _asm NOP _endasm;

void NOP_delay(int no)
{
    int i;
    for (i=0;i<no;i++)
        NOP
}

void i2c_init()
{
    SCL = 1;
    SDA = 1;
    NOP_delay(5); //~5us delay after initialization for stable SDA and SCL highs
}

void SCL_clock() //57Khz
{
    SCL=1;
    NOP_delay(5); //~5us
    SCL=0;
    NOP_delay(5); //~5us
}

void i2c_start()//SDA goes low then SCL goes low
{
    SDA = 1;
    SCL = 1;

    NOP_delay(5);
    SDA = 0;
    NOP_delay(5); //minimum 4 us delay for start condition to be satisfied
    SCL = 0;
    NOP_delay(5);
}

void i2c_stop() //SCL goes high, then SDA goes high
{
    SDA = 0;
    SCL = 1;
    NOP_delay(5); //minimum
    SDA = 1;
    NOP_delay(5);
}

/*sbit i2c_ack()
{
    sbit ack = 0;

    NOP_delay(5);
    //SCL = 1;
    NOP_delay(5);
    ack = SDA;
    //SCL = 0;
    //NOP
    SDA = 0;
    NOP_delay(15);
    return ack;
}*/

void i2c_ack()
{
	SDA = 0;
	NOP_delay(5);
	SCL = 1;
	NOP_delay(5);
	SCL = 0;
	NOP_delay(5);
	SDA = 1;
	NOP_delay(5);
}

void i2c_nak()
{
	SDA = 1;
	NOP_delay(5);
	SCL = 1;
	NOP_delay(5);
	SCL = 0;
	NOP_delay(5);
	SDA = 1;
	NOP_delay(5);
}

unsigned char device_addressing(char page,char read_write)
{
    unsigned char device_addr = 0xA0;

    page = page << 1;
    device_addr = device_addr|page|read_write;

    return device_addr;
}

void write_byte(unsigned char byte)
{
        unsigned char i = 0;
        //SCL = 1;
        while(i<8)
        {
            if(byte&0x80) //if MSB is 1
                SDA = 1;
            else SDA = 0;         //if MSB is 0
            byte = byte<<1;
            i++;
            SCL_clock();
        }
        //SCL_clock();
        //SDA= 1;
}

unsigned char read_byte()
//Read bit from SDA only when SCL is high
{
        unsigned char i = 0;
        unsigned char read_data=0;

        while(i<8)
        {
            NOP_delay(5);
            SCL = 1;
            if(SDA)
                read_data = read_data|1;

            if(i<7)
                read_data<<=1;
            NOP_delay(5);
            SCL = 0;
            i++;
        }

        return read_data;
}

void eeprom_write_byte(unsigned int addr, unsigned char databyte)
{
    unsigned char page_no,offset_addr,device_addr;
    page_no = addr/256;

    /* condition for Page number */
    offset_addr = addr%256;
    device_addr = device_addressing(page_no,0);

    /* STEPS FOR WRITE BYTE */
    i2c_start();//START CONDITION IN MAIN

    write_byte(device_addr);
    i2c_ack(); //initiate acknowledgement

    write_byte(offset_addr);
    i2c_ack(); // initiate acknowledgement

    write_byte(databyte);
    i2c_ack();

    i2c_stop();//STOP CONDITION
}

unsigned char eeprom_read_byte(unsigned int addr) /* RANDOM READ */
{
    unsigned char page_no,offset_addr,device_addr;
    unsigned char buffer;

    page_no = addr/256;
    /* condition for Page number */
    offset_addr = addr%256;
    device_addr = device_addressing(page_no,0); // write condition

    /* procedure for Random Read */
    i2c_start();//START CONDITION IN MAIN

    write_byte(device_addr); //device address sent to intiate writing. LSB is 1.
    i2c_ack();  //initiate acknowledgement

    write_byte(offset_addr);
    i2c_ack(); //initiate acknowledgement

    /* Read byte from address */
    i2c_start();  //Initiate Read. <-  PROBLEM

    write_byte(device_addr | 0x01); //Device address to read. LSB is 1
    i2c_ack();

    buffer = read_byte();

    i2c_nak(); //no acknowledgement
    i2c_stop();//STOP CONDITION

    return buffer;
}
