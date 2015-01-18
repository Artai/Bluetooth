/** \file
* Referencje funkcji, struktura bufora,makra dla ledów
*/

#ifndef fifo_h
#define fifo_h
#include <inttypes.h> //zawiera typy uint8_t itd - bez znaku integer 8 bit

#define FIFOBUFSIZE ((uint16_t)250)

#define ERROR -1
#define SUCCESS 0

#define RED_ON 0x1
#define RED_OFF 0x2
#define GREEN_ON 0x4
#define GREEN_OFF 0x8
#define LEDS_ON 0x10
#define LEDS_OFF 0x20

//Fifo buffer structure
typedef volatile struct {
	uint8_t in;
	uint8_t out;
	uint16_t count;			//ilosc danych w buforze
	char tablica_buf[FIFOBUFSIZE];
} FIFO_BufferTypeDef;

//deklaracja bufferow UART2
extern FIFO_BufferTypeDef U2Rx, U2Tx;

// Deklaracja funkcji
void UART2_Put(uint8_t);
uint8_t UART2_Get(void);

int FIFO_Put(FIFO_BufferTypeDef* buffer, uint8_t data);
int FIFO_Get(FIFO_BufferTypeDef* buffer, uint8_t* data);
int FIFO_Del(FIFO_BufferTypeDef* buffer);
void FIFO_Init(FIFO_BufferTypeDef* buffer);
int FIFO_IsEmpty(FIFO_BufferTypeDef* buffer);

void COMM_Proc(FIFO_BufferTypeDef* buffer);

extern volatile uint32_t command;	//zmienna globalna z flagami komend

#endif
