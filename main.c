/** \file
* Inicjalizacja, sekwencja powitalna
*/

#include "MKL46Z4.h"
#include "bluetooth.h"
#include <string.h>
#include <stdio.h>
#include "fifo.h"

int main (void){
	bluetoothInit();
	FIFO_Init(&U2Tx);
	FIFO_Init(&U2Rx);
	
	UART2_Put('W');
	UART2_Put('i');
	UART2_Put('t');
	UART2_Put('a');
	UART2_Put('j');
	UART2_Put('!');
	UART2_Put('!');
	UART2_Put('!');
	UART2_Put(10); //enter
	
		
	for(;;){}
}
