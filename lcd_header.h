/* NAME : Ashwin Raman */
/*LCD INITIALIZATION HEADER FILE */
/* CONTAINS functions for
1) LCD initialization
2) displaying strings on LCD
3) adjusting the position of cursor on LCD
4) Printing a string from a given address on the DRAM of LCD */

#include <at89c51ed2.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

#define HEAP_SIZE 3200
#define DEBUG                // DEBUG PORT ENABLE STATEMENT

#define RS P1_4
#define RW P1_5

xdata at (0xf000) unsigned char BUFFER; //creates variable at address 0xf000
xdata unsigned char *buffer_ptr;   // pointer to refer to variable at address 0xf000
unsigned char cnt;
volatile unsigned char line_no;

void induce_delay(int multiplier)
{
      /*if(P1_1 == 1) P1_1 = 0;
      else P1_1 = 1;*/

      TMOD = TMOD|0x01;

      while(multiplier > 0)      // multiplier with 100us
      {
            TCON = TCON &0xDF;
            TH0 = 0xFC;        // for 1ms
            TL0 = 0x66;
            TR0 = 1;        // start timer 0

            while(TF0==1);

            multiplier = multiplier-1;
      }
}

void lcd_busy_wait(void)
 // Description: Polls the LCD busy flag. Function does not return
 // until the LCD controller is ready to accept another command.
{
	RS = 0;
	RW = 1;
	buffer_ptr = &BUFFER;
	while ((*buffer_ptr & 0x80) !=0);
}


void lcd_init_command(char cmd)
// will take commands or charachters depending on the values of RS and RW
{
    lcd_busy_wait();
    RS = 0;
    RW = 0;
    buffer_ptr = &BUFFER;
    *buffer_ptr = cmd;
    /*if(cmd==0x01) //clear screen command will take cursor back to 0,0
    {
        cnt = 0;  //counts number of characters in a row
        line_no = 0; //maintains line number
    }*/
}

void lcd_init(void)
 // Description: Initializes the LCD
{
    int i;
    char cmd[8]={0x30,0x30,0x30,0x38,0x08,0x0c,0x06,0x01}; //commands for LCD init written at address f000H
    char delay[4]={30,10,1}; // delay times corresponding to 30ms, 10ms, 1ms

    cnt = 0;  //counts number of characters in a row
    line_no = 0; //maintains line number

    for(i=0;i<8;i++)  // 8 steps for LCD inititialization
    {
        if(i<3)  induce_delay(delay[i]);
        lcd_busy_wait();
        lcd_init_command(cmd[i]);
    }
}

void lcd_putch(char cc)
 // Description: Writes the specified character to the current
 // LCD cursor position.
{
    lcd_busy_wait();
    RS = 1; // RS and RW configuration to print charachters
    RW = 0;
    buffer_ptr = &BUFFER;
    *buffer_ptr = cc;
}

void lcd_putstr(char *ss)
 // Description: Writes the specified null-terminated string to the LCD
 // starting at the current LCD cursor position. Automatically wraps
 // long strings to the next LCD line after the right edge of the
 // display screen has been reached.
{
    unsigned char start[4]={0,64,16,80};
    char *temp = ss; //ss always points to beginning of string
    while(*temp)
    {
        lcd_putch(*temp);
        cnt++; //counts no. of charachters in a row

        if(cnt>15)
        {
            cnt=0;
            line_no ++;
            if(line_no==4)
            {
                line_no = 0;
            }
            lcd_init_command(0x80 + start[line_no]); // row2 starts at address offset 0x10
        }
        temp++;
    }
}

void lcd_goto_addr(unsigned char addr)
 // Description: Sets the cursor to the specified LCD DDRAM address.
{
    if (addr<=15)
    {
        line_no=0;
        cnt = addr-0;
    }

    else if(addr>=64&&addr<=79)
    {
        line_no=1;
        cnt = addr-64;
    }
    else if(addr>=16&&addr<=31)
    {
        line_no=2;
        cnt = addr-16;
    }
    else if(addr>=80&&addr<=95)
    {
        line_no=3;
        cnt = addr-80;
    }

    addr = addr + 0x80;     //starting address of DRAM is 0x80
    lcd_init_command(addr);
    lcd_busy_wait();
    lcd_init_command(0x0d);
    lcd_busy_wait();
}

void lcd_goto_xy(unsigned char row, unsigned char column)
// Description: Sets the cursor to the LCD DDRAM address corresponding
// to the specified row and column. Location (0,0) is the top left
// corner of the LCD screen.
{
    /* 0x80 is the starting address of the DRAM */
    if(row==0)
        lcd_goto_addr(0x00 + column); // row0 starts at address offset 0x00

    else if(row==1)
        lcd_goto_addr(0x40 + column); // row1 starts at address offset 0x40

    else if(row==2)
        lcd_goto_addr(0x10 + column); // row2 starts at address offset 0x10


    else if(row==3)
        lcd_goto_addr(0x50 + column); // row3 starts at address offset 0x50

    line_no = row;
    cnt = column;
    printf_tiny("\rline: %d, column: %d\n",line_no,cnt);
    lcd_busy_wait();
}
