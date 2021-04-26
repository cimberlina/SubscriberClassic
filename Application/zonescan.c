/*
*********************************************************************************************************
*									Implementacion del modulo
* Filename		:	zonescan.c
* Version		:	V1.00
* Programmer(s)	:	CMI
*********************************************************************************************************
*/

/*
*********************************************************************************************************
* 											INCLUDE FILES
*********************************************************************************************************
*/
#include "includes.h"

/*
*********************************************************************************************************
* 											GLOBAL VARIABLES
*********************************************************************************************************
*/
uint32_t zvaluemed0[16], zvaluemed1[16], pp_med0, pp_med1, np_med0, np_med1, bat_med0, bat_med1;
uint8_t Status_Zonas[16];


/*
*********************************************************************************************************
* 												TASK related
*********************************************************************************************************
*/
OS_TCB		ZoneScanTask_TCB;
CPU_STK		ZoneScanTask_Stk[ZoneScanTask_STK_SIZE];

/*
*********************************************************************************************************
*********************************************************************************************************
* 											LOCAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/
void InitZoneScan( void)
{
	uint8_t i;

	for( i = 0; i < 16; i++)	{
		zvaluemed0[i] = V_NORMAL_MED;
		zvaluemed1[i] = V_NORMAL_MED;
		Status_Zonas[i] = ALRM_NORMAL;
	}
	pp_med0 = 0;
	pp_med1 = 0;
	MUX4051_Address(0);
}
const uint8_t MUX_ReMap_ADDRESS[8] = {2, 1, 0, 3, 4, 6, 7, 5};
//const uint8_t MUX_ReMap_ADDRESS[8] = {0, 1, 2, 3, 4, 5, 6, 7};
void MUX4051_Address(uint8_t zonenumber)
{
	uint32_t address;

	address = MUX_ReMap_ADDRESS[zonenumber];
	GPIO_ClearValue(1, (7 << 26));
	GPIO_SetValue( 1, address << 26);

}

void  ZoneScanTask  (void  *p_arg)
{
	uint8_t	i;
	OS_ERR	os_err;
	uint32_t adc_value_mux0, adc_value_mux1, adc_value_mux2;
	uint8_t sysinput0, sysinput1, sysinput2;

	(void)p_arg;
	np_med0 = 0;
	np_med1 = 0;

		OSTimeDlyHMSM(0, 0, TIME_STARTUP_SCAN, 0,
					  OS_OPT_TIME_HMSM_STRICT,
					  &os_err);
	while(DEF_ON)	{
		if(!(SysFlag0 & STARTUP_flag))	{
		if(!(SysFlag_AP_zvolt & AP_ZVOLT_MEAS_flag))	{
			//aca exploro las lineas de F220 y Apertura
			sysinput0 = sysinput1;
			sysinput1 = sysinput2;
			if ((!(GPIO_ReadValue(2) & (1<<9))) || (SystemFlag3 & NAPER_F220V))
				sysinput2 |= F220_sbit;
			else
				sysinput2 &= ~F220_sbit;
			
			if (!(GPIO_ReadValue(0) & (1<<28)))
				sysinput2 |= APER_sbit;
			else
				sysinput2 &= ~APER_sbit;

			if (!(GPIO_ReadValue(1) & (1<<29)))
				sysinput2 |= DHCPM_sbit;
			else
				sysinput2 &= ~DHCPM_sbit;

			SysInputs = sysinput0 & sysinput1 & sysinput2;

			//-----------------------------------------
			for( i = 0; i < 8; i++ )	{
				zvaluemed0[i] = zvaluemed1[i];
				zvaluemed0[8+i] = zvaluemed1[8+i];
				MUX4051_Address(i);
				//paso por cada zona cada 32msec (8zonas x 4msec)
				OSTimeDlyHMSM(0, 0, 0, 8,			//4
							  OS_OPT_TIME_HMSM_STRICT,
							  &os_err);

				adc_value_mux0 = (ADC_ChannelGetData(LPC_ADC,ADC_CHANNEL_0)) & 0x00000FFF;
				adc_value_mux1 = (ADC_ChannelGetData(LPC_ADC,ADC_CHANNEL_1)) & 0x00000FFF;

				//promediamos el valor leido
				zvaluemed1[i] =  adc_value_mux0;
				zvaluemed1[8+i] =  adc_value_mux1;

				//------------------------------------------------------------------
                MUX4051_Address(0);
                OSTimeDlyHMSM(0, 0, 0, 4,
                              OS_OPT_TIME_HMSM_STRICT,
                              &os_err);
                pp_med1 = (ADC_ChannelGetData(LPC_ADC,ADC_CHANNEL_2)) & 0x00000FFF;

                if(pp_med1 >= 1750)	{
                    SysFlag1 |= INPPON_flag;
                    //GPIO_SetValue(1, 1<<18);
                } else	{
                    SysFlag1 &= ~INPPON_flag;
                    //GPIO_ClearValue(1, 1<<18);
                }
                fsm_ppon_wdog();
                //-----------------------------------------------------------------
			}
			//mido el nivel de portadora
			MUX4051_Address(1);
			OSTimeDlyHMSM(0, 0, 0, 4,
						  OS_OPT_TIME_HMSM_STRICT,
						  &os_err);
			adc_value_mux2 = (ADC_ChannelGetData(LPC_ADC,ADC_CHANNEL_2)) & 0x00000FFF;
			np_med0 = np_med1;
			np_med1 = (np_med0 + adc_value_mux2) >> 1;

			//mido la bateria
			MUX4051_Address(2);
			OSTimeDlyHMSM(0, 0, 0, 4,
						  OS_OPT_TIME_HMSM_STRICT,
						  &os_err);
			adc_value_mux2 = (ADC_ChannelGetData(LPC_ADC,ADC_CHANNEL_2)) & 0x00000FFF;
			bat_med0 = bat_med1;
			bat_med1 = (bat_med0 + adc_value_mux2) >> 1;
			//mido el PP
			MUX4051_Address(0);
			OSTimeDlyHMSM(0, 0, 0, 4,
						  OS_OPT_TIME_HMSM_STRICT,
						  &os_err);
			adc_value_mux2 = (ADC_ChannelGetData(LPC_ADC,ADC_CHANNEL_2)) & 0x00000FFF;

//			pp_med0 = pp_med1;
//			pp_med1 = (pp_med0 + adc_value_mux2) >> 1;
//			if((pp_med1 > 0.1*pp_med0) && (pp_med1 < 1.1*pp_med0))	{
//				if(pp_med1 >= 1750)	{
//					SysFlag1 |= INPPON_flag;
//					GPIO_SetValue(1, 1<<18);
//				} else	{
//					SysFlag1 &= ~INPPON_flag;
//					GPIO_ClearValue(1, 1<<18);
//				}
//			}

			//-----------------------------------
            pp_med1 = adc_value_mux2;
            if(pp_med1 >= 1500)	{
                SysFlag1 |= INPPON_flag;
                //GPIO_SetValue(1, 1<<18);
            } else	{
                SysFlag1 &= ~INPPON_flag;
                //GPIO_ClearValue(1, 1<<18);
            }
			fsm_ppon_wdog();
			//-----------------------------------

			// detecto si la zona esta en evento, normal o rotura
			for( i = 0; i < 8; i++ )	{
				if((zvaluemed1[i] > 0.5*zvaluemed0[i]) && (zvaluemed1[i] < 1.1*zvaluemed0[i]))	{

					if((zvaluemed1[i] <= V_ALARMA_HIGH)&&(zvaluemed1[i] >= V_ALARMA_LOW) )	{
						Status_Zonas[i] = ALRM_EVENTO;
					} else
					if((zvaluemed1[i] <= V_NORMAL_HIGH)&&(zvaluemed1[i] >= V_NORMAL_LOW) )	{
						Status_Zonas[i] = ALRM_NORMAL;
					} else
					if((zvaluemed1[i] <= V_SABOTAGE_HIGH)&&(zvaluemed1[i] >= V_SABOTAGE_LOW) )	{
						Status_Zonas[i] = ALRM_INDEF;
					} else
					if((zvaluemed1[i] < V_ALARMA_LOW) || (zvaluemed1[i] >= V_ROTURA_LOW) )	{
						Status_Zonas[i] = ALRM_ROTU;
					}
				} else if((zvaluemed1[i] < V_ALARMA_LOW) || (zvaluemed1[i] >= V_ROTURA_LOW) )	{
					Status_Zonas[i] = ALRM_ROTU;
				}
			}
			if((Status_Zonas[0] == ALRM_ROTU) && (Status_Zonas[1] == ALRM_ROTU) && (Status_Zonas[2] == ALRM_ROTU))	{
				Status_Zonas[0] = ALRM_EVENTO;
				Status_Zonas[1] = ALRM_EVENTO;
				Status_Zonas[2] = ALRM_EVENTO;
				Status_Zonas[7] = ALRM_ROTU;
				if(!(SystemFlag3 & ADC_EMERGENCY))	{
					SystemFlag3 |= ADC_EMERGENCY;
					logCidEvent(account, 1, 941, 0, 0);
				}
			}

			for( i = 8; i < 16; i++ )	{
				if((zvaluemed1[i] > 0.5*zvaluemed0[i]) && (zvaluemed1[i] < 1.1*zvaluemed0[i]))	{

					if(zvaluemed1[i] <= V_ALARMA_HIGH )	{
						Status_Zonas[i] = ALRM_EVENTO;
					} else
						Status_Zonas[i] = ALRM_NORMAL;
				}
			}

			for( i = 8; i < 16; i++ )	{
				if(Status_Zonas[i] != ALRM_NORMAL )	{
					OptoInputs |= bitpat[15 - i];			//bitpat[i - 8]
				} else OptoInputs &= ~bitpat[15 - i];		//bitpat[i - 8]
			}
		}
                }
                else
		  	OSTimeDlyHMSM(0, 0, 0, 4,
						  OS_OPT_TIME_HMSM_STRICT,
						  &os_err);
		
	}
}
