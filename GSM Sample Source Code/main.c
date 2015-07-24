//==========================================================================
//	Author					: CYTRON	
//	Project					: SK40C GSM
//	Project description		: GSM Enabled Smart Home.
//							  This sample source code is valid for 
//							  20MHz crystal.
//
//==========================================================================

//	include
//==========================================================================
#include <pic.h>
#include <string.h>

//	configuration
//==========================================================================
__CONFIG ( 0x3F32 );				//configuration for the  microcontroller

//	define
//==========================================================================
#define	rs			RB4				//RS pin of the LCD display
#define	e			RB5				//E pin of the LCD display

#define	lcd_data	PORTD			//LCD 8-bit data PORT

#define	SW1			RB0			
#define	SW2			RB1			

#define	LED1		RB6			
#define	LED2		RB7				

//	function prototype				(every function must have a function prototype)
//==========================================================================

void delay(unsigned long data);			

void send_config(unsigned char data);
void send_char(unsigned char data);
void lcd_goto(unsigned char data);
void lcd_clr(void);
void send_string(const char *s);

unsigned char uart_rec(void);			//receive uart value
void uart_send(unsigned char data);
void uart_str(const char *s);

void gsm_send_command(const char *command);
void gsm_read_line(char *buffer);


//	global variable
//==========================================================================

char gsm_response[20];
char command[10];


//	main function					(main fucntion of the program)
//==========================================================================
void main()
{
	char read_sms_command[] = "AT+CMGR=1";
	char delete_sms_command[] = "AT+CMGD=1";
	
	
	//set I/O input output
	TRISB = 0b00000011;					//configure PORTB I/O direction
	TRISD = 0b00000000;					//configure PORTD I/O direction
	TRISA = 0b00000111;					//configure PORTA I/O direction
	
	LED1=0;								// OFF LED1
	LED2=0;								// OFF LED2

	//Configure UART
	SPBRG=10;			//set baud rate as 115200 baud
	BRGH=1;				//baud rate high speed option
	TXEN=1;				//enable transmission
	TX9 =0;				//8-bit transmission
	RX9 =0;				//8-bit reception	
	CREN=1;				//enable reception
	SPEN=1;				//enable serial port

	//setup ADC
	ADCON1 = 0b00000110;				//set ADx pin digital I/O
	
	//configure lcd
	send_config(0b00000001);			//clear display at lcd
	send_config(0b00000010);			//lcd return to home 
	send_config(0b00000110);			//entry mode-cursor increase 1
	send_config(0b00001100);			//display on, cursor off and cursor blink off
	send_config(0b00111000);			//function set

	//display startup message	
	lcd_clr();							//clear lcd
	send_string("Cytron Tech.");		//display "Cytron Tech."
	lcd_goto(20);						//set the lcd cursor to location 20
	send_string("Smart Home");			//display "Smart Home"

	// Delay for a while.
	delay(100000);
	
	
	
	// Clear the LCD and display the new message.
	lcd_clr();
	send_string("Testing Com...");
	
	// Make sure we can communicate with the GSM modem.
	gsm_send_command("AT");
	
	// Read the response.
	gsm_read_line(&gsm_response);
	
	// We should receive "OK" from the GSM modem.
	// If we don't, display "Error".
	if (memcmp("OK", &gsm_response, 2) != 0)
	{
		lcd_clr();
		send_string("Error...");
		while(1);
	}	
	
	
	
	// Clear the LCD and display the new message.
	lcd_clr();
	send_string("Waiting for");
	lcd_goto(20);
	send_string("Call Ready");
	
	// Waiting for the GSM modem to search for the network.
	// We will receive a blank line before receiving "Call Ready".
	gsm_read_line(&gsm_response);
	gsm_read_line(&gsm_response);
	
	// We should receive "Call Ready" from the GSM modem.
	// If we don't, display "Error".
	if (memcmp("Call Ready", &gsm_response, 10) != 0)
	{
		lcd_clr();
		send_string("Error...");
		while(1);
	}	
	
	
	
	// Clear the LCD and display the new message.
	lcd_clr();
	send_string("Setting Text");
	lcd_goto(20);
	send_string("Mode...");
	
	// Set text mode for SMS.
	gsm_send_command("AT+CMGF=1");
	
	// Read the "OK".
	gsm_read_line(&gsm_response);
	
	
	
	// Clear the LCD and display the new message.
	lcd_clr();
	send_string("Ready");
	
	

	while(1)
	{
		// Check whether there is new data from the GSM modem.
		if (RCIF == 1)
		{
			// Check is there a new SMS?
			gsm_read_line(&gsm_response);
			if (memcmp("+CMTI: \"SM\",", &gsm_response, 12) == 0)
			{
				// Get the SMS index.
				// The command array is already initialized as "AT+CMGR=1" and "AT+CMGD=1",
				// we need to changed the index to the actual one.
				read_sms_command[8] = gsm_response[12];
				delete_sms_command[8] = gsm_response[12];
				
				// Clear the LCD and display the new message.
				lcd_clr();
				send_string("Reading SMS...");
				
				// Send command to read the SMS.
				gsm_send_command(&read_sms_command);
				
				// Read the response.
				// The first line is the SMS info which we don't need.
				// We only need the second line which is the message content.
				gsm_read_line(&gsm_response);
				gsm_read_line(&command);
				
				// Read the newline and "OK".
				gsm_read_line(&gsm_response);
				gsm_read_line(&gsm_response);
				
				
				
				// Switch on the light if we received "on".
				if (memcmp("on", &command, 2) == 0)
				{
					LED1 = 1;
					LED2 = 1;
				}	
				
				// Switch off the light if we received "off".
				else if (memcmp("off", &command, 3) == 0)
				{
					LED1 = 0;
					LED2 = 0;
				}	
				
				
				
				// Send command to delete the SMS.
				gsm_send_command(&delete_sms_command);
				
				// Read the "OK".
				gsm_read_line(&gsm_response);
				
				
				
				// Clear the LCD and display the new message.
				lcd_clr();
				send_string("Ready...");
			}
		}	
		
		
		
		// Check whether SW1 is pressed.
		if (SW1 == 0)
		{
			// Clear the LCD and display the new message.
			lcd_clr();
			send_string("Sending SMS...");
			
			// Send the SMS to notify the owner.
			// Please change this to your own number.
			gsm_send_command("AT+CMGS=\"0123456789\"");
			
			// We should receive '>' from the modem.
			// If we don't, display "Error".
			if (uart_rec() != '>')
			{
				lcd_clr();
				send_string("Error...");
				while(1);
			}	
			
			// Send the message.
			uart_str("Warning: Switch 1 has been triggered !");
			
			// Send <CTRL+Z>.
			uart_send(0x1a);
			
			
			
			// Read the newline and response.
			gsm_read_line(&gsm_response);
			gsm_read_line(&gsm_response);
			
			// We should receive "+CMGS: <Message ID>" from the GSM modem.
			// If we don't, display "Error".
			if (memcmp("+CMGS:", &gsm_response, 6) != 0)
			{
				lcd_clr();
				send_string("Error...");
				while(1);
			}
			
			
			
			// Read the newline and "OK".
			gsm_read_line(&gsm_response);
			gsm_read_line(&gsm_response);
			
			
			
			// Clear the LCD and display the new message.
			lcd_clr();
			send_string("Ready...");
			
			
			
			// Wait until SW1 is released so that we don't send duplicated message.
			while (SW1 == 0);
			
		}	
	}
}

//	functions
//==========================================================================
void delay(unsigned long data)			//delay function, the delay time
{										//depend on the given value
	for( ;data>0;data--);
}

void send_config(unsigned char data)	//send lcd configuration 
{
	rs=0;								//set lcd to configuration mode
	lcd_data=data;						//lcd data port = data
	e=1;								//pulse e to confirm the data
	delay(50);
	e=0;
	delay(50);
}

void send_char(unsigned char data)		//send lcd character
{
 	rs=1;								//set lcd to display mode
	lcd_data=data;						//lcd data port = data
	e=1;								//pulse e to confirm the data
	delay(10);
	e=0;
	delay(10);
}

void lcd_goto(unsigned char data)		//set the location of the lcd cursor
{										//if the given value is (0-15) the 
 	if(data<16)							//cursor will be at the upper line
	{									//if the given value is (20-35) the 
	 	send_config(0x80+data);			//cursor will be at the lower line
	}									//location of the lcd cursor(2X16):
	else								// -----------------------------------------------------
	{									// | |00|01|02|03|04|05|06|07|08|09|10|11|12|13|14|15| |
	 	data=data-20;					// | |20|21|22|23|24|25|26|27|28|29|30|31|32|33|34|35| |
		send_config(0xc0+data);			// -----------------------------------------------------	
	}
}

void lcd_clr(void)						//clear the lcd
{
 	send_config(0x01);
	delay(600);	
}

void send_string(const char *s)			//send a string to display in the lcd
{          
  	while (s && *s)send_char (*s++);
}

unsigned char uart_rec(void)			//receive uart value
{
	unsigned char rec_data;
	while(RCIF==0);						//wait for data
	rec_data = RCREG;				
	return rec_data;					//return the data received
}

void uart_send(unsigned char data)
{	
	while(TXIF==0);				//only send the new data after 
	TXREG=data;					//the previous data finish sent
}


void uart_str(const char *s)
{
	while(*s)uart_send(*s++);
}


void gsm_send_command(const char *command)
{
	unsigned char rec_data;
	
	while (*command != 0)
	{
		uart_send(*command++);		// Send the AT command.
		rec_data = uart_rec();	// Read the echo.
	}	
	
	// Send <ENTER>.
	uart_str("\r\n");
	
	// Read the echo until Line Feed character is received.
	do {
		rec_data = uart_rec();
	} while (rec_data != '\n');	
}	



void gsm_read_line(char *buffer)
{
	unsigned char rec_data;
	
	// Read the data until Line Feed character is received.
	do {
		rec_data = uart_rec();
		*buffer++ = rec_data;
	} while (rec_data != '\n');	
}	
