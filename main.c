/*
NAME  : ASHWIN RAMAN

FUNTION:
PROGRAM HAS MENU FOR THE USER TO ENTER LCD COMMANDS AND TO DISPLAY STRINGS/CHARACTERS ON LCD
USER HAS THE OPTION TO CHANGE THE POSITION OF THE CURSOR
ALL THE LIBRARY FUNCTIONS FOR LCD ARE WRITTEN IN lcd_header.h
*/
#include "lcd_header.h"
#include "eeprom_header.h"

unsigned char xdata heap[HEAP_SIZE];

 _sdcc_external_startup()  // HARDWARE INITIALIZATION
{
    //IE   = 0x8a;    //EA = 1, ET0=1, ET1 = 1
    TMOD = 0x20;    // TIMER 1 USED AS 8 BIT AUTO RELOAD TIMER, TIMER 0 USED AS 16BIT TIMER

    TL1  = 0xfd;     // TCON Bit7
    TH1  = 0xfd;
    AUXR = AUXR|0x0c;
    SCON = 0x53;    //serial port init register
    //PCON = PCON & 0x01;
    TR1  = 1;
    TI = 1;
    RI = 0;
    return 0;
}

void putchar (char c)            //Putting  character to terminal
{
    while(!TI); //wait for transmitter to be ready
    SBUF = c;   //write character to transmit buffer
    TI = 0;     //clear the TI flag of SCON Register
}

char getchar ()                 //getting character from terminal
{
	while (RI == 0);
	RI = 0;			// clear RI flag
	return SBUF;  	// return character from SBUF
}

unsigned int str_to_hex(unsigned char addr[])   //for converting a "hex" string to an integer value
{
        unsigned int address;
        unsigned char p[1];
        char i = strlen(addr)-1;
        unsigned int k = 1;

        address = 0;
		printf_tiny("\n\rsize %d \n\r",strlen(addr));

        while(i>=0)
        {
            if(addr[i]>='0'&&addr[i]<='9')
            {
                p[0] = addr[i];
                address = address + (atoi(p)*k);
            }
            else if(addr[i]>='A'&&addr[i]<='F')
                address = address + (k*(addr[i]-55));
            else if(addr[i]>='a'&&addr[i]<='f')
                address = address + (k*(addr[i]-87));
            else
                return 1;
            i--;
            k = k*16;
        }

        printf_tiny("**decimal value : %d **\n",address);
        return address;
}

void hex_to_str(unsigned int cc,unsigned char hex[],unsigned char status)
/* cc= address in hex format; hex[] will contain the address in text format */
/* adding hex characters to array "hex" from last to first */
/* status  = 1 means hex will contain data, data length = 3 (DD) ; status  = 0 hex will contain addr, hence addr length = 3 (AAA) */
{
    unsigned char i;
    unsigned char mod;

	if(status==1) i=2;
    else i=3;

    while(cc>0)
    {
        mod = cc%16;
        //printf_tiny("\n\rMOD : %d\n",mod);
        if(mod>9)
            hex[i-1]=mod+55;
        else
            hex[i-1]=mod+48;
        cc = cc/16;
        i--;
    }
	if(i==2){hex[0] = '0';hex[1]='0';}
	if(i==1){hex[0] = '0';}
//	printf_tiny("\n\rlength : %d %d\n\r",strlen(hex),i);
}

void main(void)
{
        char row_char[2],col_char[2];
        unsigned int address;
        unsigned char display_char,*char_ptr,row,column;
        char lcd_string[64];

        unsigned char addr[4]={0},e_data[3],print_addr[4];
        unsigned char hex[3]={0}; //size is 3 to store the extra null character
        unsigned char cmd,i;
        unsigned int cc,ee_addr,ee_data,ee_start,ee_end,temp,counter,diff;
        unsigned char lcd_line = 0;

		while(1)
        {
        printf_tiny("\n\r=====================MENU=================================\n\r");
        printf_tiny("1 : PROGRAM 1 BASIC LCD DISPLAY\n\r");
        printf_tiny("2 : PROGRAM 2 EEPROM READ WRITE\n\r");
        printf_tiny("==========================================================\n\r");

        printf_tiny("\n\rENTER COMMAND : ");
        cmd = getchar();
        putchar(cmd);
        printf_tiny("\n");

        switch(cmd)
        {

        case '2':   //PROGRAM FOR TEST EEPROM FUNCTIONALITY LCD FUNCTIONS
        lcd_init();

        i2c_init();

        printf_tiny("\n\r=====================MENU=================================\n\r");
        printf_tiny("1 : WRITE BYTE TO EEPROM\n\r");
        printf_tiny("2 : READ BYTE FROM EEPROM AND DISPLAY ON TERMINAL IN AAA:DD FORMAT (DISPLAYED IN HEX)\n\r");
        printf_tiny("3 : DISPLAY CONTENT AT SPECIFIED ADDRESS OF EEPROM IN AAA:DD FORMAT\n\r");
        printf_tiny("4 : CLEAR LCD DISPLAY\n\r");
        printf_tiny("5 : DISPLAY HEX DUMP\n\r");
        printf_tiny("6 : EXIT\n\r");
        printf_tiny("==========================================================\n\r");

        while(1)
        {
            printf_tiny("\n\rENTER COMMAND : ");
            cmd = getchar();
            putchar(cmd);
            printf_tiny("\n");

            switch(cmd)
            {
                case '1':   memset(addr,0,sizeof(addr));
                            printf_tiny("\rENTER EEPROM ADDRESS : 0x");
                            gets(addr);
                            ee_addr = str_to_hex(addr);

                            if (ee_addr>=2048)
                            {
                                printf_tiny("\r\nPLEASE RESTRICT THE ADDRESS TO LESS THAN 2048 BYTES \n");
                                break;
                            }

                            else if(ee_addr == 1)
                            {
                                printf_tiny("\r***** INCORRECT ADDRESS *******\n\r");
                                break;
                            }

                            printf_tiny("\rENTER THE DATA TO BE WRITTEN IN THE SPECIFIED ADDRESS (ENTER IN HEX) : 0x");
                            gets(e_data);
                            if (strlen(e_data)>2) printf_tiny("\rPLEASE LIMIT THE SIZE OF YOUR HEX DATA TO ONE BYTE");
                            ee_data = str_to_hex(e_data);

                            eeprom_write_byte(ee_addr,ee_data);
                            break;

                case '2':   //memset(addr,0,sizeof(addr));
                            //memset(hex,0,sizeof(hex));

                            printf_tiny("\rENTER EEPROM ADDRESS : 0x");
                            gets(addr);
                            printf_tiny("\rstrlen %d\n\r",strlen(addr));
                            ee_addr = str_to_hex(addr); //convert the read hex text to a number

                            if (ee_addr>=2048)
                            {
                                printf_tiny("\r\nPLEASE RESTRICT THE ADDRESS TO LESS THAN 2048 BYTES \n");
                                break;
                            }

                            NOP_delay(200);

                            //print the address and data in AAA:DD format
                            hex_to_str(ee_addr,hex,0);
                            printf_tiny("\n\r");
							for(i=0;i<strlen(hex);i++)
                                putchar(hex[i]);
                            putchar(':');

							memset(hex,0,sizeof(hex));
							//printf_tiny("\n\rAshwin Raman\n\r");
							cc = eeprom_read_byte(ee_addr); //cc will contain the data at the user specified address in decimal
							//printf_tiny("\n\r DATA AT ADDRESS %d(decimal) IS :  %d (decimal)\n\r",ee_addr,cc);
							hex_to_str(cc,hex,1); //converting the number value at address to hex text. 1 implies data (format = DD)

                            for(i=0;i<strlen(hex);i++)
                                putchar(hex[i]);

                            printf_tiny("\n\r");
                            break;

                case '3' :  memset(addr,0,sizeof(addr));
                            memset(hex,0,sizeof(hex));

                            printf_tiny("\rENTER EEPROM ADDRESS : 0x");
                            gets(addr);
                            ee_addr = str_to_hex(addr); //convert the hex text to a number

                            if (ee_addr>=2048)
                            {
                                printf_tiny("\r\nPLEASE RESTRICT THE ADDRESS TO LESS THAN 2048 BYTES \n");
                                break;
                            }

                            NOP_delay(200);

                            if(lcd_line == 4) lcd_line = 0;
                            lcd_goto_xy(lcd_line,0);//position cursor at the beginning of a new line

							hex_to_str(ee_addr,hex,0);
                            lcd_putstr(hex);
                            lcd_putstr(":");

							memset(hex,0,sizeof(hex));
							cc = eeprom_read_byte(ee_addr); //cc will contain the data at the user specified address in decimal
							//printf_tiny("\n\r DATA AT ADDRESS %d(decimal) IS :  %d (decimal)\n\r",ee_addr,cc);
							hex_to_str(cc,hex,1); //convert the data in eeprom stored in hex to corresponding text ; 1 = Data (DD format)

                            lcd_putstr(hex);

                            lcd_line++; //jump to next line on LCD
                            break;

                case '4' :  // LCD CLEAR
                            lcd_init_command(0x01);
                            lcd_busy_wait();
                            lcd_init();
                            lcd_busy_wait();
                            break;

                case '5' :  //HEX DUMP
                            memset(addr,0,sizeof(addr));
                            memset(hex,0,sizeof(hex));

                            printf_tiny("\rENTER START ADDRESS (Enter in Hex): 0x");
                            gets(addr);
                            ee_start = str_to_hex(addr); //convert the hex text to a number

                            if (ee_start>=2048)
                            {
                                printf_tiny("\r\nPLEASE RESTRICT THE ADDRESS TO LESS THAN 2048 BYTES \n");
                                break;
                            }

                            printf_tiny("\rENTER END ADDRESS (Enter in Hex): 0x");
                            gets(addr);
                            ee_end = str_to_hex(addr); //convert the hex text to a number

                            if (ee_end>=2048)
                            {
                                printf_tiny("\r\nPLEASE RESTRICT THE ADDRESS TO LESS THAN 2048 BYTES \n");
                                break;
                            }
                            else if(ee_end<ee_start)
                            {
                                printf_tiny("\r\nYOUR END ADDRESS IS LESS THAN START ADDRESS \n");
                                break;
                            }

                            NOP_delay(200);
                            temp = ee_start;
                            diff = ee_end - ee_start;
                            counter = 0;
                            putchar('\r');

                            printf_tiny("\n\n\r    : DD DD DD DD DD DD DD DD DD DD DD DD DD DD DD DD");
                            printf_tiny("\n\rAAA : +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F");
                            printf_tiny("\n\r=====================================================\n\r");

                            hex_to_str(ee_start,print_addr,0); //print the starting address in hext test. 1 = address hence format is AAA

							for(i=0;i<strlen(print_addr);i++)
                                    putchar(print_addr[i]);
                            putchar(' ');
                            putchar(':');
                            putchar(' ');

                            while(temp<=ee_end)
                            {
                                cc = eeprom_read_byte(temp); //cc will contain the data at the user specified address in decimal
                                counter++;
                                hex_to_str(cc,hex,1); //convert data in address "temp" to readable hex text; 1=DATA (DD format)

                                for(i=0;i<strlen(hex);i++)
                                    putchar(hex[i]);
                                putchar(' ');

                                if (counter==16)
                                {
                                    counter = 0;
                                    putchar('\n');
                                    putchar('\r');

                                    ee_start = ee_start + 16;
                                    hex_to_str(ee_start,print_addr,0);//0=address (AAA format), converting address to hex text

                                    for(i=0;i<strlen(print_addr);i++)
                                            putchar(print_addr[i]);
                                    putchar(' ');
                                    putchar(':');
                                    putchar(' ');
                                }
                                temp ++;
                            }

                            while(counter!=16)  // to fill the remaining line if the last line does not display 16 DD values
                            {
								if(temp >=2048) break;
                                cc = eeprom_read_byte(temp); //cc will contain the data at the user specified address in decimal
                                counter++;
                                hex_to_str(cc,hex,1); //converting data in address"temp" to hexadecimal text; 1=Data (DD format)

                                for(i=0;i<strlen(hex);i++)
                                    putchar(hex[i]);
                                putchar(' ');

                                if (counter==16)
                                {
                                    putchar('\n');
                                    putchar('\r');
                                }
                                temp ++;
                            }

                            break;

                case '6' : //CODE EXIT
                            printf_tiny("\n\r*** EXITING CURRENT CODE ***\n\r");
                            break;

            }
            if(cmd=='6')break;
        }
        break;

        //i2c_start();


        case '1':   //PROGRAM FOR BASIC LCD FUNCTIONS
        lcd_init();

        printf_tiny("\n\r=====================MENU=================================\n\r");
        printf_tiny("1 : CLEAR LCD SCREEN\n\r");
        printf_tiny("2 : DISPLAY A CHARACTER AT CURRENT CURSOR POSITION. DEFAULT (0,0)\n\r");
        printf_tiny("3 : DISPLAY A STRING FROM THE CURRENT CURSOR POSITION. DEFAULT(0,0)\n\r");
        printf_tiny("4 : POSITION CURSOR AT A SPECIFIC DDRAM ADDRESS\n\r");
        printf_tiny("5 : POSITION CURSOR AT GIVEN ROW AND COLUMN\n\r");
        printf_tiny("6 : EXIT\n\r ");
        printf_tiny("==========================================================\n\r");


        while(1)
        {
            printf_tiny("\n\rENTER COMMAND : ");
            cmd = getchar();
            putchar(cmd);
            printf_tiny("\n");

            switch(cmd) //refer to the menu for corresponding funcionalities of each switch case
            {
                case '1':   lcd_init_command(0x01);
                            lcd_busy_wait();
                            lcd_init();
                            lcd_busy_wait();
                            break;

                case '2':   printf_tiny("\n\rENTER CHARACHTER TO BE DISPLAYED ON LCD : ");
                            display_char = getchar();
                            putchar(display_char);
                            char_ptr = &display_char;
                            lcd_putstr(char_ptr);
                            break;

                case '3':   printf_tiny("\n\rENTER STRING TO BE DISPLAYED ON LCD : ");
                            gets(lcd_string);
                            lcd_putstr(lcd_string);
                            break;

                case '4':   printf_tiny("\n\rENTER ADDRESS (HEX VALUE): ");

                            gets(addr);
                            //addr[0] = getchar();
                            //putchar(addr[0]);

                            //addr[1] = getchar();
                            //putchar(addr[1]);

                            address = str_to_hex(addr);
                            if (address == 1) {
                                printf_tiny("\n\r **** INVALID ADDRESS *****");
                                break;
                            }
                            lcd_goto_addr(address);
                            break;

                case '5':   printf_tiny("\n\rENTER ROW : ");
                            gets(row_char);
                            row = atoi(row_char);
                            printf_tiny("\n\rENTER COLUMN : ");
                            gets(col_char);
                            column = atoi(col_char);

                            lcd_goto_xy(row,column);
                            break;

                case '6':   printf_tiny("\n\r****** CODE EXIT. PRESS RESET TO RESTART PROGRAM ******\n\n\n");
                            break;
            }
            if(cmd=='6') break;

        }
        break;
        }
        }
}
