/** \file
* Zawiera funkcje inicjalizujaca bluetooth oraz obsluge przerwania UART
*/

#include "MKL46Z4.h"
#include "bluetooth.h"
#include "fifo.h"

/**@defgroup Bluetooth
*@{
*/

#define UART_IRQ_NBR (IRQn_Type) 14

//Rx do Tx na plytce - PTE16
//Tx do Rx na plytce - PTE17

/**
*@brief Inicjalizuje modul bluetooth, wlacza przerwania dla UART
*@param void
*@retval void
*/
void bluetoothInit (void){
	SIM->SCGC4 |= SIM_SCGC4_UART2_MASK;
	SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;
	
	PORTE->PCR[16] = PORT_PCR_MUX(3);
	PORTE->PCR[17] = PORT_PCR_MUX(3);
	
	UART2->C2 &= ~(UART_C2_TE_MASK | UART_C2_RE_MASK);
	
	//SBR wyszedl 156 (=9C). 0 ustawiam by w ogole byl nastawiony - czy cos w ten desen
	UART2->BDH |= UART_BDH_SBR(0);
	UART2->BDL |= UART_BDL_SBR(0x9C);
	
	UART2->BDH &= ~UART_BDH_SBNS_MASK;
	
	UART2->C1 &= ~UART_C1_M_MASK;
	UART2->C1 &= ~UART_C1_PE_MASK;
	
	UART2->C2 |= UART_C2_RIE_MASK; //wlaczenie przerwan od odbiornika (od nadajnika nie moze byc ciagle wlaczone, bo sie inaczej ciagle uruchamia)
	
	UART2->C2 |= (UART_C2_TE_MASK | UART_C2_RE_MASK);
	
	//NVIC - kontroler przerwan
	NVIC_ClearPendingIRQ(UART_IRQ_NBR); //wyczysc ewentualne smieci, ma byc czysciutko
	NVIC_EnableIRQ(UART_IRQ_NBR); //pozwol im zglaszac przerwania, niech maja
	
	NVIC_SetPriority (UART_IRQ_NBR, 3); //ustawiam mu najwyzszy priorytet przerwan
}

// to sie baaardzo przydaje. Dziala samoistnie, nie trzeba tego wywolywac

/**
*@brief Wektor przerwania od UART, sprawdza czy przerwanie od nadajnika czy odbiornika. W przypadku nadajnika wysyla co ma w buforze, w przypadku odbiornika wykonuje instrukcje case w zaleznosci od ustawionego znaku konczacego
*@param brak
*@retval brak
*/
//WYSYLANIE DZIALA TAK: wpisujemy do FIFO za pomoca FIFO_Put, potem 
void UART2_IRQHandler (void){
	char bajt;
	uint8_t znak;
	uint8_t tmp;
	
	if(UART2->S1 & UART_S1_TDRE_MASK){	//nadajnik jest pusty, wiec moge zaczac wysylac to co jest w FIFO
		if(FIFO_Get(&U2Tx, &znak) == SUCCESS){
			UART2->D = znak;
		}else{
			UART2->C2 &= ~UART_C2_TIE_MASK;
			//UART2->D = 'W';
		}
	}
	
	if(UART2->S1 & UART_S1_RDRF_MASK){	//odbiornik pelny; wpisujemy jego zawartosc do danej 'bajt' i ja wysylamy bo chcemy byc echem
		bajt = UART2->D;
		tmp = bajt;
		FIFO_Put(&U2Tx, tmp);
		//UART2->D = bajt;
		
		if (tmp >= 0x20 && tmp < 0x7f) //32 - 10000000 oraz 127-11111111, bo do 31 sa znaki wazne do komunikacji, wiec jest zignoruj
			FIFO_Put(&U2Rx, tmp);
		
		switch(tmp) {
			case 0x0A:
				COMM_Proc(&U2Rx);  //0A = 10, wiec to znak nowej linii
				break;
			case 0x0d:
				COMM_Proc(&U2Rx);  //0d = 13, wiec to znak nowej linii (Carriage Return)
				break;
			case 0x26:
				COMM_Proc(&U2Rx);  //26 = &
				break;
			case 0x25:
				COMM_Proc(&U2Rx);  //25 = %
				break;
			case 0x7f: 
				FIFO_Del(&U2Rx);  //7f = 127, wiec to delete
				break;
			case 0x08: 
				FIFO_Del(&U2Rx);  //08 = 8, wiec to backspace
				break;
			default:
				break;

		} 
	}
	
	
}
/**
*@}
*/
