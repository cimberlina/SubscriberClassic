//*******************************************************************
//*				1-Wire iButtons Dallas Control Functions		    *
//*******************************************************************
//*

#include "includes.h"

#define	IBUTTON_PORT		2
#define	IBUTTON_PIN			12


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// LookUp Table para el calculo del DOW CRC de iButtons Dallas
//
const uint8_t DOWCRC_Table[256] = {
	0, 94, 188,	226, 97, 63, 221, 131, 194, 156, 126, 32, 163, 253,	31,	65,
	157, 195, 33, 127, 252, 162, 64, 30, 95, 1, 227, 189, 62, 96, 130, 220,
	35, 125, 159, 193, 66, 28, 254, 160, 225, 191, 93, 3, 128, 222, 60, 98,
	190, 224, 2, 92, 223, 129, 99, 61, 124, 34, 192, 158, 29, 67, 161, 255,
	70, 24, 250, 164, 39, 121, 155, 197, 132, 218, 56, 102, 229, 187, 89, 7,
	219, 133, 103, 57, 186, 228, 6, 88, 25, 71, 165, 251, 120, 38, 196, 154,
	101, 59, 217, 135, 4, 90, 184, 230, 167, 249, 27, 69, 198, 152, 122, 36,
	248, 166, 68, 26, 153, 199, 37, 123, 58, 100, 134, 216, 91, 5, 231, 185,
	140, 210, 48, 110, 237, 179, 81, 15, 78, 16, 242, 172, 47, 113, 147, 205,
	17, 79, 173, 243, 112, 46, 204, 146, 211, 141, 111, 49, 178, 236, 14, 80,
	175, 241, 19, 77, 206, 144, 114, 44, 109, 51, 209, 143, 12, 82, 176, 238,
	50, 108, 142, 208, 83, 13, 239, 177, 240, 174, 76, 18, 145, 207, 45, 115,
	202, 148, 118, 40, 171, 245, 23, 73, 8, 86, 180, 234, 105, 55, 213, 139,
	87, 9, 235, 181, 54, 104, 138, 212, 149, 203, 41, 119, 244, 170, 72, 22,
	233, 183, 85, 11, 136, 214, 52, 106, 43, 117, 151, 201, 74, 20, 246, 168,
	116, 42, 200, 150, 21, 75, 169, 247, 182, 232, 10, 84, 215, 137, 107, 53
};

//const unsigned char BlockSelectorCodes[9][8] = {
//	{ 0x56,0x56,0x7F,0x51,0x57,0x5D,0x5A,0x7F },
//	{ 0x9A,0x9A,0xB3,0x9D,0x64,0x6E,0x69,0x4C },
//	{ 0x9A,0x9A,0x4C,0x62,0x9B,0x91,0x69,0x4C },
//	{ 0x9A,0x65,0xB3,0x62,0x9B,0x6E,0x96,0x4C },
//	{ 0x6A,0x6A,0x43,0x6D,0x6B,0x61,0x66,0x43 },
//	{ 0x95,0x95,0xBC,0x92,0x94,0x9E,0x99,0xBC },
//	{ 0x65,0x9A,0x4C,0x9D,0x64,0x91,0x69,0xB3 },
//	{ 0x65,0x65,0xB3,0x9D,0x64,0x6E,0x96,0xB3 },
//	{ 0x65,0x65,0x4C,0x62,0x9B,0x91,0x96,0xB3 }
//};

//#include "tabla_ibuttons0.h"
#include "tablasIbutton1.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// iButton_reset ---	Manda el pulso de reset y lee el pulso de presencia
//					del iButton, si hay pulso de presencia devuelve un 1
//

uint8_t iButton_reset( void )
{
	
	GPIO_SetDir(IBUTTON_PORT, (1 << IBUTTON_PIN), 1); GPIO_ClearValue(IBUTTON_PORT, 1<<IBUTTON_PIN);
	delay_us(700);
	GPIO_SetDir(IBUTTON_PORT, (1 << IBUTTON_PIN), 0); GPIO_SetValue(IBUTTON_PORT, 1<<IBUTTON_PIN);
	delay_us(100);

	if( GPIO_ReadValue(IBUTTON_PORT) & (1<<IBUTTON_PIN) )	{
		delay_us(500);	    //500
		return 0;
	} else	{
		delay_us(500);	   //500
		return 1;
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// iButton_w_bit --	Genera un time slot de escritura, escribiendo el 
//					valor del bit indicado.
//
void iButton_w_bit( int16_t bit )
{
	GPIO_SetDir(IBUTTON_PORT, (1 << IBUTTON_PIN), 1); GPIO_ClearValue(IBUTTON_PORT, 1<<IBUTTON_PIN);
	delay_us(3);
	if( bit )	{
		GPIO_SetDir(IBUTTON_PORT, (1 << IBUTTON_PIN), 0); GPIO_SetValue(IBUTTON_PORT, 1<<IBUTTON_PIN);
	}
	else	{
		delay_us(12);
	}

	delay_us(72);
	GPIO_SetDir(IBUTTON_PORT, (1 << IBUTTON_PIN), 0); GPIO_SetValue(IBUTTON_PORT, 1<<IBUTTON_PIN);
	delay_us(3);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// iButton_cmd ---	Envia un comando tipo ROM al iButton
//
uint8_t iButton_cmd( uint8_t cmd )
{
	uint8_t tempch1;
	int16_t i;

	tempch1 = cmd;
	if( iButton_reset() )	{
		for(i=0; i<8; i++)	{
			if(tempch1 & 0x01)
				iButton_w_bit(1);
			else
				iButton_w_bit(0);
			tempch1 >>= 1;
		}
		delay_us(500);
		return 1;
	} return 0;
}

void iButton_tx_byte ( uint8_t data )
{
	uint8_t i, temp;
	
	temp = data;
	for( i = 0; i < 8; i++ )	{
		if( temp & 0x01 )
			iButton_w_bit(1);
		else
			iButton_w_bit(0);
		temp >>= 1;
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// iButton_r_bit ---	Lee un bit desde el iButton, devuelve el estado del
//					bit leido.
//
uint8_t iButton_r_bit( void )
{
	uint8_t temp;
	
	GPIO_SetDir(IBUTTON_PORT, (1 << IBUTTON_PIN), 1); GPIO_ClearValue(IBUTTON_PORT, 1<<IBUTTON_PIN);
	delay_us(3);				// flanco de sincronizacion
	GPIO_SetDir(IBUTTON_PORT, (1 << IBUTTON_PIN), 0); GPIO_SetValue(IBUTTON_PORT, 1<<IBUTTON_PIN);
	delay_us(12); //delay12us();

	if(GPIO_ReadValue(IBUTTON_PORT) & (1<<IBUTTON_PIN))
		temp = 1;
	else temp = 0;

	delay_us(36);
	return( temp );
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// iButton_read ---	Lee los 64-bits del serial id del iButton, recibe
//					como argumento un pointer a un buffer de 8 bytes
//					donde almacenar este nro.
//
uint8_t iButton_read( uint8_t buffer[] )
{
	uint8_t i;
	
	CPU_SR_ALLOC();
	CPU_CRITICAL_ENTER();
	if( iButton_reset() )	{
		iButton_tx_byte( 0x33 );
		delay_us(300);		//5
		for( i = 0; i < 8; i++)
			buffer[i] = iButton_rx_byte();

	} else	{
		CPU_CRITICAL_EXIT();
        SystemFlag10 &= ~IBUTTONREAD;
		return 0;
	}

	CPU_CRITICAL_EXIT();
	if( ((buffer[0] == 0x01) || (buffer[0] == 0x02) || (buffer[0] == 0x04) )&& !DowCRC(buffer) )	{
		//--------------------------------
//		if( !(SystemFlag10 & IBUTTONREAD)) {
//            SystemFlag10 |= IBUTTONREAD;
//            CommSendString(DEBUG_COMM, "{0x");
//            printByte(buffer[0]);
//            CommSendString(DEBUG_COMM, ", ");
//            for (i = 6; i > 0; i--) {
//                CommSendString(DEBUG_COMM, "0x");
//                printByte(buffer[i]);
//                CommSendString(DEBUG_COMM, ", ");
//            }
//            CommSendString(DEBUG_COMM, "0x");
//            printByte(buffer[7]);
//            CommSendString(DEBUG_COMM, " },\n\r");
//        }
		//--------------------------------
		//WriteIbuttonLog(buffer);
		return 1;
	}
	else {
        SystemFlag10 &= ~IBUTTONREAD;
	    return 0;
	}
	
	
}


uint8_t iButton_rx_byte( void )
{
	uint8_t temp, i;
	
	temp = 0x00;
	for( i = 0; i < 8; i++ )	{
		temp >>= 1;
		if( iButton_r_bit() )
			temp |= 0x80;
		else
			temp &= 0x7F;
	}
	return temp;
}


uint8_t DowCRC( uint8_t * buffer )
{
	uint8_t CRC = 0;
	uint8_t temp;
	int16_t i;

	for( i=0; i<8; i++ )	{
		temp = CRC ^ buffer[i];
		CRC = DOWCRC_Table[temp];
	}
	return CRC;
}

//uint8_t check_id( uint8_t *idbuffer)
//{
//	uint8_t i, j, retval;
//	int error;
//
//	for( i = 0; i < 7; i++ )	{
//		if( idbuffer[i] != 0x00 )
//			break;
//	}
//	if( i == 7 )
//		return 0;
//
//	for( j = 0; j < 201; j++ )	{
//		for( i = 1; i < 7; i++ )	{
//			if( idbuffer[i] != ibutton_table[j][7-i] )
//				break;
//		}
//		if( i == 7 )	{
//			retval = EepromReadByte(ZONE_E2P_ADDR, &error);
//			if(j < IDZONE1)	{						//zona universal
//				return j;
//			} else
//			if((j >= IDZONE1) && (j < IDZONE3))	{	//zona 1 y 2
//				if((retval == 2) || (retval == 1))
//					return j;
//			} else
//			if((j >= IDZONE3) && (j < IDZONE4))	{	//zona 3
//				if(retval == 3)
//					return j;
//			} else
//			if((j >= IDZONE4) && (j < IDZONE5))	{	//zona 4
//				if(retval == 4)
//					return j;
//			} else
//			if((j >= IDZONE5) && (j < IDZONE6))	{	//zona 5
//				if(retval == 5)
//					return j;
//			} else return 0;
//		}
//	}
//	return 0;
//}

uint8_t check_id_2( uint8_t *idbuffer)
{
	int i, j, retval;
	int error;

	for( i = 0; i < 7; i++ )	{
		if( idbuffer[i] != 0x00 )
			break;
	}
	if( i == 7 )
		return 0;


	for( j = 0; ((IbuttonTable[j].usernumber != 0) && (IbuttonTable[j].zone != 999)); j++)	{
		for( i = 1; i < 7; i++ )	{
			if( idbuffer[i] != IbuttonTable[j].ibutton_data[7-i] )
				break;
		}
		if( i == 7 )	{
			retval = EepromReadByte(ZONE_E2P_ADDR, &error);
			if(IbuttonTable[j].zone == 0)	{
				return j;
			} else
			if((retval == 2) || (retval == 1))	{
				if((IbuttonTable[j].zone == 1) || (IbuttonTable[j].zone == 2))
					return j;
			} else
			if( retval == IbuttonTable[j].zone )	{
				return j;
			} else
				return 0;
		}
	}
	return 0;

}
