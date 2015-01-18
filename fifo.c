/** \file
* Obsluga kolejki FIFO oraz COMM_Proc czyli obsluga poleceń
*/


#include "MKL46Z4.h"
#include "fifo.h"
#include <string.h>
#include <stddef.h>

/**@defgroup FIFO
*@{
*/

/**
*@brief Cykliczny bufor FIFO
*@var powoluje do zycia 2 bufory do wysylania i odbirania danych
*/

// Cykliczny bufor FIFO
FIFO_BufferTypeDef U2Tx, U2Rx; //powoluje do zycia 2 bufory do wysylania i odbirania danych

volatile uint32_t command = 0;
/**
*@brief resetuje bufor
*@param buffer to zmienna typu wskaznik na typ bufora, wiec bedzie to Tx lub Rx
*@retval brak
*/
void FIFO_Init(FIFO_BufferTypeDef* buffer){
	buffer->in = 0;
	buffer->out = 0;
	buffer->count = 0;
}

/**
*@brief zwraca czy bufor jest pusty czy nie
*@param buffer to zmienna typu wskaznik na typ bufora, wiec bedzie to Tx lub Rx
*@retval czy sie powiodlo; w naglowkowym zdefiniowany SUCCESS jako 0 i ERROR jako -1
*/
int FIFO_IsEmpty(FIFO_BufferTypeDef* buffer){
	UART2->C2 &= ~UART_C2_TIE_MASK;
	UART2->C2 &= ~UART_C2_RIE_MASK;
	if (buffer->count == 0){		//jesli nie ma absolutnie zadnych danych to jest pusty
		UART2->C2 |= UART_C2_RIE_MASK;
		UART2->C2 |= UART_C2_TIE_MASK;
		return SUCCESS;
	}
	else{
		UART2->C2 |= UART_C2_RIE_MASK;
		UART2->C2 |= UART_C2_TIE_MASK;
		return ERROR;
	}
}

/**
*@brief wpisuje jeden bajt danych do kolejki FIFO
*@param buffer to zmienna typu wskaznik na typ bufora, wiec bedzie to Tx lub Rx
*@param 8 bitowa dana do wpisania
*@retval czy sie powiodlo; w naglowkowym zdefiniowany SUCCESS jako 0 i ERROR jako -1
*/
int FIFO_Put(FIFO_BufferTypeDef* buffer, uint8_t data){
	UART2->C2 &= ~UART_C2_TIE_MASK;
	UART2->C2 &= ~UART_C2_RIE_MASK;
	if (buffer->count == FIFOBUFSIZE){ //jesli w tym rejestrze licznik danych jest rowny pojemnosci bufora, to jest pelny, nie da sie nic wpisac => error!
		UART2->C2 |= UART_C2_RIE_MASK;
		UART2->C2 |= UART_C2_TIE_MASK;
		return ERROR;
	}
	++(buffer->in); //+1 do liczby znajdujacych sie danych wejsciowych
	
	if (buffer->in == FIFOBUFSIZE) //jesli bufor doszedl do konca np. liczba 256, to musi wrocic do poczatku, bo FIFO dziala w kolko
		buffer->in = 0;
	++(buffer->count); //+1 do liczby zawartych danych, bo juz ja wpiszemy na pewno
	buffer->tablica_buf[buffer->in] = data; //w danym rejestrze do tablicy wpiszemy dane na miejsce wyznaczone wczesniej przez buffer->in
	UART2->C2 |= UART_C2_RIE_MASK;
	UART2->C2 |= UART_C2_TIE_MASK;
	return SUCCESS;
}

/**
*@brief odczytuje 1 bajt danych z kolejki FIFO
*@param buffer to zmienna typu wskaznik na typ bufora, wiec bedzie to Tx lub Rx
*@param wskaznik na 8 bitowa dana do odczytania
*@retval czy sie powiodlo; w naglowkowym zdefiniowany SUCCESS jako 0 i ERROR jako -1
*/
int FIFO_Get(FIFO_BufferTypeDef* buffer, uint8_t* data){
	UART2->C2 &= ~UART_C2_TIE_MASK;
	UART2->C2 &= ~UART_C2_RIE_MASK;
	if (buffer->count == 0){
		UART2->C2 |= UART_C2_RIE_MASK;
		UART2->C2 |= UART_C2_TIE_MASK;
		return ERROR;
	}
	++(buffer->out); //jesli jest cos do odczytania to to odczytam ;p
	
	if (buffer->out == FIFOBUFSIZE)	//jesli wskaznik jest ustawiony na ostatni element kolejki FIFO, to przenies na poczatek, bo tam wciaz moga byc elementy, bo on je zapisuje w kolko
		buffer->out = 0;
	*data = buffer->tablica_buf[buffer->out]; //moja dana wpisuje do tablicy na miejsce okreslone wyzej
	--(buffer->count); //odejmuj, bo przeciez odczytalas juz jedna dana
	UART2->C2 |= UART_C2_RIE_MASK;
	UART2->C2 |= UART_C2_TIE_MASK;
	return SUCCESS;
}

/**
*@brief usuwa ostatni element w buforze
*@param buffer to zmienna typu wskaznik na typ bufora, wiec bedzie to Tx lub Rx
*@retval czy sie powiodlo; w naglowkowym zdefiniowany SUCCESS jako 0 i ERROR jako -1
*/
int FIFO_Del(FIFO_BufferTypeDef* buffer){
	UART2->C2 &= ~UART_C2_TIE_MASK;
	UART2->C2 &= ~UART_C2_RIE_MASK;
	if (buffer->count == 0){ //musi cos byc by sie dalo to usunac
		UART2->C2 |= UART_C2_RIE_MASK;
		UART2->C2 |= UART_C2_TIE_MASK;
		return ERROR;
	}
	--(buffer->count);
	--(buffer->in);
	if (buffer->in > FIFOBUFSIZE)    
		buffer->in = FIFOBUFSIZE - 1;
	UART2->C2 |= UART_C2_RIE_MASK;
	UART2->C2 |= UART_C2_TIE_MASK;
	return SUCCESS;
}

/**
*@brief wkłada char do buforu pisania, włacza UART
*@param dane do wpisania
*@retval brak
*/
void UART2_Put(uint8_t data){
	FIFO_Put(&U2Tx, data); //dodaje dane do buforu
	UART2->C2 |= UART_C2_TIE_MASK;
}

/**
*@brief odczytuje string z bufora FIFO i sprawdza czy to byla komenda, zmienia zmienna globalna 'command'
*@param buffer to zmienna typu wskaznik na typ bufora, wiec bedzie to Tx lub Rx
*@retval brak
*/

void COMM_Proc(FIFO_BufferTypeDef* buffer) {
	char tmp[FIFOBUFSIZE];
	uint16_t i;
	int a;
	int h;
	
	for (i=0;; i++) {
		if (FIFO_IsEmpty(buffer) == SUCCESS){ // w pierwszym wolnym miejscu kolejki wpisuje 0 i wyskakuje z petli
			tmp[i] = 0;
			break;  // wyskkujemy z petli for
		}
		FIFO_Get(buffer, &tmp[i]);
	}
	//porownuje (s1,s2,n) n znakow s1 i s2; para po parze znakow porownuje do momentu gdy beda rozne, gdy w ktoryms napotkamy "\0" czyli wartosc 00 lub gdy zostanie porownane pierwszych n znakow obu napisow
	//zwracane wartosci to liczba calkowita wskazujaca na relacje miedzy napisami
	//0 to napisy sa identyczne, liczba dodatnia to znak w s1 ma wieksza wartosc, ujemna to w s2
	if (strncmp(tmp, "RedOn", 5) == 0) {
		ledRedOn(); }
	else if (strncmp(tmp, "RedOff", 6) == 0) {
		ledRedOff();  }	
	else if (strncmp(tmp, "GreenOn", 7) == 0) {
		ledGreenOn();  }
	else if (strncmp(tmp, "GreenOff", 8) == 0) {
		ledGreenOff();  }
	else if (strncmp(tmp, "ledsOff", 7) == 0) {
		ledsOff();  }
	else if (strncmp(tmp, "ledsOn", 6) == 0) {
		ledsOn();  }
	
	else if (strncmp(tmp, "nr", 2) == 0) {
		
		for (a=0; FIFOBUFSIZE; a++) {
			
			if (tmp[a] == 0){
				if (a>7){
					Error();
					break;
				}
				else {
					sLCD_set(0x10, 1); //wyzerowanie segmentu
					sLCD_set(0x10, 2);
					sLCD_set(0x10, 3);
					sLCD_set(0x10, 4);
					for(h=2; h<a-1; h++){
						sLCD_set(tmp[h]-48, h-1);
					}
					break;
				}				
			}
		}
	}
}

/**
*@}
*/
