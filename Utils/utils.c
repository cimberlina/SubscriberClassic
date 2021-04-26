/*
 * utils.c
 *
 *  Created on: Oct 31, 2011
 *      Author: IronMan
 */

#include "includes.h"

void fsm_optoinp_dbnc( void );

OS_TCB		FsmLedsTask_TCB;
CPU_STK		FsmLedsTask_Stk[FsmLedsTask_STK_SIZE];

uint8_t bledShadow, bledShadow2;
Led_Struct led_dcb[8];
Led_Struct led_swp[8];

Led_Struct led_screen[12][8];
uint8_t SLedShadow[12];

Led_Struct AP_Aper_led_dcb;
Led_Struct Buzzer_dcb;

uint16_t DiagEventFlags;
#define	DIAGEVENT 923

uint8_t FSM_OPTOINP_state[8];
#define FSM_OPTOINP_IDLE	0x10
#define FSM_OPTOINP_ONWAIT	0x15
#define FSM_OPTOINP_ON		0x20
#define FSM_OPTOINP_OFFWAIT	0x25
#define FSM_OPTOINP_OFF		0x30

uint8_t FSM_OPTOINP_timer[8];
uint8_t	OptoInDbnc;

void SetLeds( uint8_t imageled )
{
	uint8_t i;

	if(!(SysFlag_AP_Apertura & AP_APR_VALID)) {
		GPIO_ClearValue( 2, 1 << 3);
		GPIO_ClearValue( 2, 1 << 4);
		GPIO_ClearValue( 0, 1 << 0);
		GPIO_ClearValue( 0, 1 << 1);
		GPIO_ClearValue( 0, 1 << 5);
		GPIO_ClearValue( 0, 1 << 22);
		GPIO_ClearValue( 0, 1 << 26);
		GPIO_ClearValue( 1, 1 << 23);
		return;
	}
	for( i = 0; i < 8; i++ )	{
		if( imageled & (1<<i) )
			switch( i )	{
				case 0:
					GPIO_SetValue( 2, 1 << 3);
					break;
				case 1:
					GPIO_SetValue( 2, 1 << 4);
					break;
				case 2:
					GPIO_SetValue( 0, 1 << 0);
					break;
				case 3:
					GPIO_SetValue( 0, 1 << 1);
					break;
				case 4:
					GPIO_SetValue( 0, 1 << 5);
					break;
				case 5:
					GPIO_SetValue( 0, 1 << 22);
					break;
				case 6:
					GPIO_SetValue( 0, 1 << 26);
					break;
				case 7:
					GPIO_SetValue( 1, 1 << 23);
					break;

			}
		else
			switch( i )	{
				case 0:
					GPIO_ClearValue( 2, 1 << 3);
					break;
				case 1:
					GPIO_ClearValue( 2, 1 << 4);
					break;
				case 2:
					GPIO_ClearValue( 0, 1 << 0);
					break;
				case 3:
					GPIO_ClearValue( 0, 1 << 1);
					break;
				case 4:
					GPIO_ClearValue( 0, 1 << 5);
					break;
				case 5:
					GPIO_ClearValue( 0, 1 << 22);
					break;
				case 6:
					GPIO_ClearValue( 0, 1 << 26);
					break;
				case 7:
					GPIO_ClearValue( 1, 1 << 23);
					break;

			}
	}
}

#define	SOUND_03_bflag	0x01

void FsmLedsTask( void )
{
	int this, i, j;
	OS_ERR	os_err;
	static int timer_rotura = 0;
//	uint8_t bledShadow;
	static uint8_t ledcount, ledtimer, optodbnc1, optodbnc2, bzflags;

	uint8_t current_rtu, err;
	time_t timeout;

	

	ledcount = 0;
	ledtimer = 20;
	optodbnc1 = 0;
	optodbnc2 = 0;

	OptoInDbnc = 0x00;

	for(i = 0; i < 8; i++)	{
		FSM_OPTOINP_state[i] = FSM_OPTOINP_IDLE;
		FSM_OPTOINP_timer[i] = 0;
	}

	while(DEF_ON)	{
		OSTimeDlyHMSM(0, 0, 0, 100,
			           OS_OPT_TIME_HMSM_STRICT,
			           &os_err);
		fsm_console_enter();
		openptm_process();
		ptm_group_replica();
		fsm_optoinp_dbnc();


		//-------------------------------------------------------------------------------------------------
		optodbnc1 = optodbnc2;
         optodbnc2 = OptoInputs;
		if(optodbnc1 != optodbnc2)	{
			ledcount = 0;
			ledtimer = 20;
		}
		switch(OptoInputs)	{
		case 0x84:
			if(ledtimer ==  20)	{
				//--------------------------------------------------------
				//para debug de 232
				CommPutChar(DEBUG_COMM, 0x06, 0);
//				OSTimeDlyHMSM(0, 0, 0, 100, OS_OPT_TIME_HMSM_STRICT, &os_err);
				current_rtu = 0x00;


				timeout = MSEC_TIMER + 100;
				while(CommIsEmpty(DEBUG_COMM) != TRUE)	{
					current_rtu = CommGetChar(DEBUG_COMM, 10, &err);
					if(current_rtu == 0x06)
						break;
					if( MSEC_TIMER > timeout)
						break;
				}

				if(current_rtu != 0x06)	{
					Rot485_flag |= ROT232_FLAG;
				} else	{
					Rot485_flag &= ~ROT232_FLAG;
				}
				//--------------------------------------------------------
				ledtimer = 0;
			} else	{
				ledtimer++;
			}
			break;
		case 0x90:
			if(ledtimer ==  20)	{
				//--------------------------------------------------------
				//para debug de 232
				CommPutChar(DEBUG_COMM, 0x06, 0);
				timeout = MSEC_TIMER + 50;
				while( CommIsEmpty(DEBUG_COMM) != TRUE)	{
					current_rtu = CommGetChar(DEBUG_COMM, 1, &err);
					if( MSEC_TIMER > timeout)
						break;
				}
				if(current_rtu != 0x06)	{
					Rot485_flag |= ROT232_FLAG;
				} else	{
					Rot485_flag &= ~ROT232_FLAG;
				}
				//--------------------------------------------------------
				ledtimer = 0;
				for( i = 3; i < 8; i++)	{
					if(BaseAlarmPkt_estado_dispositivos & (1 << i))	{
						ScreenLedBlink(10, 7-i, 255, 255, BLINK_FOREVER);
					} else	{
						ScreenLedBlink(10, 7-i, 1, 25, BLINK_FOREVER);
					}
				}
			} else	{
				ledtimer++;
			}
			break;
		case 0x80:
		case 0x40:
		case 0x20:
		case 0x10:
		case 0x08:
		case 0x04:
		case 0x02:
		case 0x01:
			if(ledtimer ==  20)	{
				ledtimer = 0;
				for(i = 0; i < 12; i++)	{
					if( (i > 7) && (i < 12))
						continue;

					ScreenLedBlink(i, 0, 0, 0, 0);
					ScreenLedBlink(i, 1, 0, 0, 0);
					ScreenLedBlink(i, 2, 0, 0, 0);
					ScreenLedBlink(i, 3, 0, 0, 0);
					ScreenLedBlink(i, 4, 0, 0, 0);
					ScreenLedBlink(i, 5, 0, 0, 0);
					ScreenLedBlink(i, 6, 0, 0, 0);
					ScreenLedBlink(i, 7, 0, 0, 0);

					if( i == 6 )	{
						switch(ledcount)	{
						case 0:
							current_rtu = 70;
							break;
						case 1:
							current_rtu = 80;
							break;
						//case 2:
						//	current_rtu = 90;
						//	break;
						default:
							break;
						}
					} 
					if( i == 7 )	{
						current_rtu = ledcount*10 + 19;
					} else	{
						current_rtu = (i+1)*10 + (ledcount + 1);
					}

					if( get_array_bit( current_rtu, devfound) )	{
						ScreenLedBlink(i, ledcount, 1, 3, 5);
						if(OptoInputs & (1 << (7 - i)))	{
							Buzzer_dcb.led_cad = 1*0x100 + 3;
							Buzzer_dcb.led_state = LED_IDLE;
							Buzzer_dcb.led_blink = 5;
						}
					} else	{
						if(habi485[i] & (1 << (ledcount)))	{
							if(diag485[i] & (1 << (ledcount)))	{
								ScreenLedBlink(i, ledcount, 20, 5, 1);
								if(OptoInputs & (1 << (7 - i)))	{
									Buzzer_dcb.led_cad = 17*0x100 + 1;
									Buzzer_dcb.led_state = LED_IDLE;
									Buzzer_dcb.led_blink = 1;
								}
							} else	{
								ScreenLedBlink(i, ledcount, 4, 4, BLINK_FOREVER);
								if(OptoInputs & (1 << (7 - i)))	{
									Buzzer_dcb.led_cad = 6*0x100 + 6;
									Buzzer_dcb.led_state = LED_IDLE;
									Buzzer_dcb.led_blink = 2;
								}
							}
						} else	{
							ScreenLedBlink(i, ledcount, 2, 18, BLINK_FOREVER);
	//						Buzzer_dcb.led_cad = 0;
	//						Buzzer_dcb.led_state = LED_IDLE;
	//						Buzzer_dcb.led_blink = 0;
						}
					}
				}
				ledcount++;
				if(ledcount == 8)
					ledcount = 0;
			} else	{
				ledtimer++;
			}
			break;
		default :
			break;
		}

		//-------------------------------------------------------------------------------------------------
		//actualizo flags que deben actuar cada 100ms
		if( timer_rotura < 7)
			timer_rotura++;
		else	{
			SysFlag0 |= TIMROT_flag;
			timer_rotura = 0;
		}
		//manejo el timer de prevencion de polling rf
		if(preve_timer > 0)
			preve_timer--;
		else	{
			SysFlag1 |= PREVE_CENTRAL_RX;
			//preve_timer = TIEMPO_PREVE;
		}

		//actualizo los timers de los leds cada 100ms
		for( this = 0; this < 8; this++ )	{
			if( led_dcb[this].led_tim_off )
				led_dcb[this].led_tim_off--;
			if( led_dcb[this].led_tim_on )
				led_dcb[this].led_tim_on--;
		}
		for( this = 0; this < 8; this++ )	{
			if( led_swp[this].led_tim_off )
				led_swp[this].led_tim_off--;
			if( led_swp[this].led_tim_on )
				led_swp[this].led_tim_on--;
		}
		for( j = 0; j < 8; j++)	{
			for( i = 0; i < 11; i++)	{
				if( led_screen[i][j].led_tim_off )
					led_screen[i][j].led_tim_off--;
				if( led_screen[i][j].led_tim_on )
					led_screen[i][j].led_tim_on--;
			}
		}

		if( AP_Aper_led_dcb.led_tim_off )
			AP_Aper_led_dcb.led_tim_off--;
		if( AP_Aper_led_dcb.led_tim_on )
			AP_Aper_led_dcb.led_tim_on--;

		if( Buzzer_dcb.led_tim_off )
			Buzzer_dcb.led_tim_off--;
		if( Buzzer_dcb.led_tim_on )
			Buzzer_dcb.led_tim_on--;
		fsm_buzzer();

		for( j = 0; j < 8; j++)	{
			for( i = 0; i < 11; i++)	{
				//-----------------------------------------------------------------------------------------------------
				switch( led_screen[i][j].led_state )	{
					case LED_IDLE :
						SLedShadow[i] &= ~led_screen[i][j].led_mask;
						if(led_screen[i][j].led_cad == (255*0x100 + 0))	{
							led_screen[i][j].led_flags |= 0x01;
						} else if(led_screen[i][j].led_cad == 0) {
							led_screen[i][j].led_flags &= ~0x01;
						}
						if( led_screen[i][j].led_cad )	{
							SLedShadow[i] |= led_screen[i][j].led_mask;
							led_screen[i][j].led_state = LED_ON;
							led_screen[i][j].led_tim_on = (led_screen[i][j].led_cad >> 8) & 0x00FF;
						} else	{
							SLedShadow[i] &= ~led_screen[i][j].led_mask;
							led_screen[i][j].led_tim_off = 0;
							led_screen[i][j].led_tim_on = 0;
						}
						break;
					case LED_ON :
						SLedShadow[i] |= led_screen[i][j].led_mask;
						if( (!(led_screen[i][j].led_tim_on)) && (led_screen[i][j].led_cad & 0x00FF) )	{
							SLedShadow[i] &= ~led_screen[i][j].led_mask;
							led_screen[i][j].led_state = LED_OFF;
							led_screen[i][j].led_tim_off = led_screen[i][j].led_cad & 0x00FF;
						}
						break;
					case LED_OFF :
						SLedShadow[i] &= ~led_screen[i][j].led_mask;
						if( !(led_screen[i][j].led_tim_off) )	{
							SLedShadow[i] |= led_screen[i][j].led_mask;
							led_screen[i][j].led_state = LED_ON;
							led_screen[i][j].led_tim_on = (led_screen[i][j].led_cad >> 8) & 0x00FF;

							if( (led_screen[i][j].led_blink < BLINK_FOREVER) && (led_screen[i][j].led_blink > 0) )	{
								led_screen[i][j].led_blink--;
								if( !led_screen[i][j].led_blink )	{
									led_screen[i][j].led_state = LED_IDLE;
									SLedShadow[i] &= ~led_screen[i][j].led_mask;
									led_screen[i][j].led_cad = 0;
									if(led_screen[i][j].led_flags & 0x01)	{
										led_screen[i][j].led_cad = (255*0x100 + 0);
										SLedShadow[i] |= led_screen[i][j].led_mask;
										led_screen[i][j].led_state = LED_ON;
										led_screen[i][j].led_tim_on = (led_screen[i][j].led_cad >> 8) & 0x00FF;
									}
								}
							}
						}
						break;
					default :
						led_screen[i][j].led_state = LED_IDLE;
						break;
				}
				//-----------------------------------------------------------------------------------------
			}
		}

		//ejecuto la maquina propiamente dicha
		if(!(SysFlag1 & AP_APERLED_CTRL))	{

			for( this = 0; this < 8; this++ )	{
					//-----------------------------------------------------------------------------------------------------
					switch( led_dcb[this].led_state )	{
						case LED_IDLE :
							bledShadow &= ~led_dcb[this].led_mask;
							if(led_dcb[this].led_cad == (255*0x100 + 0))	{
								led_dcb[this].led_flags |= 0x01;
							} else if(led_dcb[this].led_cad == 0) {
								led_dcb[this].led_flags &= ~0x01;
							}
							if( led_dcb[this].led_cad )	{
								bledShadow |= led_dcb[this].led_mask;
								led_dcb[this].led_state = LED_ON;
								led_dcb[this].led_tim_on = (led_dcb[this].led_cad >> 8) & 0x00FF;
							} else	{
								bledShadow &= ~led_dcb[this].led_mask;
								led_dcb[this].led_tim_off = 0;
								led_dcb[this].led_tim_on = 0;
							}
							break;
						case LED_ON :
							bledShadow |= led_dcb[this].led_mask;
							if( (!(led_dcb[this].led_tim_on)) && (led_dcb[this].led_cad & 0x00FF) )	{
								bledShadow &= ~led_dcb[this].led_mask;
								led_dcb[this].led_state = LED_OFF;
								led_dcb[this].led_tim_off = led_dcb[this].led_cad & 0x00FF;
							}
							break;
						case LED_OFF :
							bledShadow &= ~led_dcb[this].led_mask;
							if( !(led_dcb[this].led_tim_off) )	{
								bledShadow |= led_dcb[this].led_mask;
								led_dcb[this].led_state = LED_ON;
								led_dcb[this].led_tim_on = (led_dcb[this].led_cad >> 8) & 0x00FF;

								if( (led_dcb[this].led_blink < BLINK_FOREVER) && (led_dcb[this].led_blink > 0) )	{
									led_dcb[this].led_blink--;
									if( !led_dcb[this].led_blink )	{
										led_dcb[this].led_state = LED_IDLE;
										bledShadow &= ~led_dcb[this].led_mask;
										led_dcb[this].led_cad = 0;
										if(led_dcb[this].led_flags & 0x01)	{
											led_dcb[this].led_cad = (255*0x100 + 0);
											bledShadow |= led_dcb[this].led_mask;
											led_dcb[this].led_state = LED_ON;
											led_dcb[this].led_tim_on = (led_dcb[this].led_cad >> 8) & 0x00FF;
										}
									}
								}
							}
							break;
						default :
							led_dcb[this].led_state = LED_IDLE;
							break;
					}
					//-----------------------------------------------------------------------------------------
					switch( led_swp[this].led_state )	{
						case LED_IDLE :
							if( led_swp[this].led_cad )	{
								bledShadow2 |= led_swp[this].led_mask;
								led_swp[this].led_state = LED_ON;
								led_swp[this].led_tim_on = (led_swp[this].led_cad >> 8) & 0x00FF;
							} else	{
								bledShadow2 &= ~led_swp[this].led_mask;
								led_swp[this].led_tim_off = 0;
								led_swp[this].led_tim_on = 0;
							}
							break;
						case LED_ON :
							if( (!(led_swp[this].led_tim_on)) && (led_swp[this].led_cad & 0x00FF) )	{
								bledShadow2 &= ~led_swp[this].led_mask;
								led_swp[this].led_state = LED_OFF;
								led_swp[this].led_tim_off = led_swp[this].led_cad & 0x00FF;
							}
							break;
						case LED_OFF :
							if( !(led_swp[this].led_tim_off) )	{
								bledShadow2 |= led_swp[this].led_mask;
								led_swp[this].led_state = LED_ON;
								led_swp[this].led_tim_on = (led_swp[this].led_cad >> 8) & 0x00FF;

								if( (led_swp[this].led_blink < BLINK_FOREVER) && (led_swp[this].led_blink > 0) )	{
									led_swp[this].led_blink--;
									if( !led_swp[this].led_blink )	{
										led_swp[this].led_state = LED_IDLE;
										bledShadow2 &= ~led_swp[this].led_mask;
										led_swp[this].led_cad = 0;
									}
								}
							}
							break;
						default :
							led_swp[this].led_state = LED_IDLE;
							break;
					}
					//-----------------------------------------------------------------------------------------
			}

			if(SysFlag1 & PREVE_CENTRAL_TX)	{
				ScreenLedBlink(8, 2, 255, 255, BLINK_FOREVER);
				ScreenLedBlink(8, 3, 255, 255, BLINK_FOREVER);
				//SLedShadow[8] |= 0x30;
			} else	{
				ScreenLedBlink(8, 2, 0, 0, 0);
				ScreenLedBlink(8, 3, 0, 0, 0);
				//SLedShadow[8] &= ~0x30;
			}

			if(SysFlag1 & PREVE_CENTRAL_RX)	{
				ScreenLedBlink(8, 4, 255, 255, BLINK_FOREVER);
				ScreenLedBlink(8, 5, 255, 255, BLINK_FOREVER);
				//SLedShadow[8] |= 0x0C;
			} else	{
				ScreenLedBlink(8, 4, 0, 0, 0);
				ScreenLedBlink(8, 5, 0, 0, 0);
				//SLedShadow[8] &= ~0x0C;
			}
#ifdef USAR_IRIDIUM

			if(IRIDIUM_flag & IRI_MDMALIVE_FLAG)	{
				//ScreenLedBlink(9, 7, 20, 5, BLINK_FOREVER);
				led_screen[9][7].led_blink = BLINK_FOREVER;
				led_screen[9][7].led_cad = 50*0x100 + 5;
			} else 	{
				//ScreenLedBlink(9, 7, 6, 8, BLINK_FOREVER);
				led_screen[9][7].led_blink = BLINK_FOREVER;
				led_screen[9][7].led_cad = 6*0x100 + 5;
			}
#endif

			if(Rot485_flag & ROT491_FLAG)	{
				ScreenLedBlink(9, 0, 255, 255, BLINK_FOREVER);
				ScreenLedBlink(9, 1, 255, 255, BLINK_FOREVER);
				//SLedShadow[8] |= 0x30;
			} else	{
				ScreenLedBlink(9, 0, 0, 0, 0);
				ScreenLedBlink(9, 1, 0, 0, 0);
				//SLedShadow[8] &= ~0x30;
			}
			if(Rot485_flag & ROT232_FLAG)	{
				ScreenLedBlink(9, 2, 255, 255, BLINK_FOREVER);
				ScreenLedBlink(9, 3, 255, 255, BLINK_FOREVER);
				//SLedShadow[8] |= 0x30;
			} else	{
				ScreenLedBlink(9, 2, 0, 0, 0);
				ScreenLedBlink(9, 3, 0, 0, 0);
				//SLedShadow[8] &= ~0x30;
			}

			switch(OptoInDbnc)	{
			case 0x80:
				if((!(DiagEventFlags & (1 << 0))) && (BaseAlarmPkt_alarm & bitpat[APER_bit]))	{
					DiagEventFlags = 0x0000;
					DiagEventFlags |= (1 << 0);
					logCidEvent(account, 1, DIAGEVENT, 0, 981);
                    warm_reset_mem_alrm(ASAL_bit);
                    warm_reset_mem_alrm(TESO_bit);
                    warm_reset_mem_alrm(INCE_bit);
                    warm_reset_mem_alrm(ROTU_bit);
				}
				SetLeds( SLedShadow[0] );
				break;
			case 0x40:
				if((!(DiagEventFlags & (1 << 1))) && (BaseAlarmPkt_alarm & bitpat[APER_bit]))	{
					DiagEventFlags = 0x0000;
					DiagEventFlags |= (1 << 1);
					logCidEvent(account, 1, DIAGEVENT, 0, 982);
				}
				SetLeds( SLedShadow[1] );
				break;
			case 0x20:
				if((!(DiagEventFlags & (1 << 2))) && (BaseAlarmPkt_alarm & bitpat[APER_bit]))	{
					DiagEventFlags = 0x0000;
					DiagEventFlags |= (1 << 2);
					logCidEvent(account, 1, DIAGEVENT, 0, 983);
				}
				SetLeds( SLedShadow[2] );
				break;
			case 0x10:
				if((!(DiagEventFlags & (1 << 3))) && (BaseAlarmPkt_alarm & bitpat[APER_bit]))	{
					DiagEventFlags = 0x0000;
					DiagEventFlags |= (1 << 3);
					logCidEvent(account, 1, DIAGEVENT, 0, 984);
				}
				SetLeds( SLedShadow[3] );
				break;
			case 0x08:
				if((!(DiagEventFlags & (1 << 4))) && (BaseAlarmPkt_alarm & bitpat[APER_bit]))	{
					DiagEventFlags = 0x0000;
					DiagEventFlags |= (1 << 4);
					logCidEvent(account, 1, DIAGEVENT, 0, 985);
				}
				SetLeds( SLedShadow[4] );
				break;
			case 0x04:
				if((!(DiagEventFlags & (1 << 5))) && (BaseAlarmPkt_alarm & bitpat[APER_bit]))	{
					DiagEventFlags = 0x0000;
					DiagEventFlags |= (1 << 5);
					logCidEvent(account, 1, DIAGEVENT, 0, 986);
				}
				SetLeds( SLedShadow[5] );
				break;
			case 0x02:
				if((!(DiagEventFlags & (1 << 6))) && (BaseAlarmPkt_alarm & bitpat[APER_bit]))	{
					DiagEventFlags = 0x0000;
					DiagEventFlags |= (1 << 6);
					logCidEvent(account, 1, DIAGEVENT, 0, 987);
				}
				SetLeds( SLedShadow[6] );
				break;
			case 0x01:
				if((!(DiagEventFlags & (1 << 7))) && (BaseAlarmPkt_alarm & bitpat[APER_bit]))	{
					DiagEventFlags = 0x0000;
					DiagEventFlags |= (1 << 7);
					logCidEvent(account, 1, DIAGEVENT, 0, 999);
				}
				SetLeds( SLedShadow[7] );
				break;
			case 0x81:
				if((!(DiagEventFlags & (1 << 8))) && (BaseAlarmPkt_alarm & bitpat[APER_bit]))	{
					DiagEventFlags = 0x0000;
					DiagEventFlags |= (1 << 8);
					logCidEvent(account, 1, DIAGEVENT, 0, 991);
				}
				SetLeds( SLedShadow[8] );
				break;
			case 0x84:
				if((!(DiagEventFlags & (1 << 9))) && (BaseAlarmPkt_alarm & bitpat[APER_bit]))	{
					DiagEventFlags = 0x0000;
					DiagEventFlags |= (1 << 9);
					logCidEvent(account, 1, DIAGEVENT, 0, 994);
				}
				SetLeds( SLedShadow[9] );
				break;
			case 0x90:
				if((!(DiagEventFlags & (1 << 10))) && (BaseAlarmPkt_alarm & bitpat[APER_bit]))	{
					DiagEventFlags = 0x0000;
					DiagEventFlags |= (1 << 10);
					logCidEvent(account, 1, DIAGEVENT, 0, 996);
				}
				SetLeds( SLedShadow[10]);
				break;
			case 0x03:
				if( !(bzflags & SOUND_03_bflag) )	{
					Buzzer_dcb.led_cad = 10*0x100 + 5;
					Buzzer_dcb.led_blink = 1;
					logCidEvent(account, 1, 916, 0, IbuttonTable[ibuttonid].usernumber + 500);
					bzflags |= SOUND_03_bflag;
				}
				if( (Buzzer_dcb.led_cad != (10*0x100 + 5)) || (Buzzer_dcb.led_blink != 1) )	{
					Buzzer_dcb.led_cad = 10*0x100 + 5;
					Buzzer_dcb.led_blink = 1;
					//bzflags |= SOUND_03_bflag;
				}
				SetLeds(ptm_pwd);
				break;
			case 0x00:
				if(BaseAlarmPkt_alarm & bitpat[APER_bit])
					DiagEventFlags = 0x0000;
			default:
				bzflags &= ~SOUND_03_bflag;
				if(SerialConsoleFlag & INCONFBOARD_flag)	{
					SetLeds( bledShadow2 );
				} else	{
					SetLeds( bledShadow );
				}
				break;
			}
		} else if(SysFlag1 & AP_APERLED_CTRL)	{
			fsm_AP_aper_led();
		}
	}
}

void ScreenLedBlink(uint8_t screen, uint8_t led, uint8_t ton, uint8_t toff, uint8_t blink)
{
	led_screen[screen][led].led_blink = blink;
	led_screen[screen][led].led_cad = ton*0x100 + toff;
	led_screen[screen][led].led_state = LED_IDLE;
}




void fsm_optoinp_dbnc( void )
{
	int i;

	for( i = 0; i < 8; i++)	{
		if(FSM_OPTOINP_timer[i] > 0)
			FSM_OPTOINP_timer[i]--;
	}

	for( i = 0; i < 8; i++)	{
		switch(FSM_OPTOINP_state[i])	{
			case FSM_OPTOINP_IDLE :
				if(OptoInputs & (1 << i))	{
					FSM_OPTOINP_state[i] = FSM_OPTOINP_ONWAIT;
					FSM_OPTOINP_timer[i] = 10;
				} else{
					FSM_OPTOINP_state[i] = FSM_OPTOINP_OFFWAIT;
					FSM_OPTOINP_timer[i] = 10;
				}
				break;
			case FSM_OPTOINP_ONWAIT :
				if(FSM_OPTOINP_timer[i] == 0)	{
					FSM_OPTOINP_state[i] = FSM_OPTOINP_ON;
					OptoInDbnc |= (1 << i);
				} else
				if(!(OptoInputs & (1 << i)))	{
					FSM_OPTOINP_state[i] = FSM_OPTOINP_OFFWAIT;
					FSM_OPTOINP_timer[i] = 10;
				}
				break;
			case FSM_OPTOINP_ON :
			if(!(OptoInputs & (1 << i)))	{
					FSM_OPTOINP_state[i] = FSM_OPTOINP_OFFWAIT;
					FSM_OPTOINP_timer[i] = 10;
				}
				break;
			case FSM_OPTOINP_OFFWAIT :
				if(FSM_OPTOINP_timer[i] == 0)	{
					FSM_OPTOINP_state[i] = FSM_OPTOINP_OFF;
					OptoInDbnc &= ~(1 << i);
				} else
				if(OptoInputs & (1 << i))	{
					FSM_OPTOINP_state[i] = FSM_OPTOINP_ONWAIT;
					FSM_OPTOINP_timer[i] = 10;
				}
				break;
			case FSM_OPTOINP_OFF :
				if(OptoInputs & (1 << i))	{
					FSM_OPTOINP_state[i] = FSM_OPTOINP_ONWAIT;
					FSM_OPTOINP_timer[i] = 10;
				}
				break;
			default:
				FSM_OPTOINP_state[i] = FSM_OPTOINP_IDLE;
				break;
		}
	}
}

void fsm_AP_aper_led( void )
{
	GPIO_ClearValue( 2, 1 << 3);
	GPIO_ClearValue( 2, 1 << 4);
	GPIO_ClearValue( 0, 1 << 0);
	GPIO_ClearValue( 0, 1 << 1);
	//GPIO_ClearValue( 0, 1 << 5);
	GPIO_ClearValue( 0, 1 << 22);
	GPIO_ClearValue( 0, 1 << 26);
	GPIO_ClearValue( 1, 1 << 23);

	switch( AP_Aper_led_dcb.led_state )	{
		case LED_IDLE :
			if( AP_Aper_led_dcb.led_cad )	{
				GPIO_SetValue( 0, 1 << 5);
				AP_Aper_led_dcb.led_state = LED_ON;
				AP_Aper_led_dcb.led_tim_on = (AP_Aper_led_dcb.led_cad >> 8) & 0x00FF;
			} else	{
				GPIO_ClearValue( 0, 1 << 5);
				AP_Aper_led_dcb.led_tim_off = 0;
				AP_Aper_led_dcb.led_tim_on = 0;
			}
			break;
		case LED_ON :
			if( (!(AP_Aper_led_dcb.led_tim_on)) && (AP_Aper_led_dcb.led_cad & 0x00FF) )	{
				GPIO_ClearValue( 0, 1 << 5);
				AP_Aper_led_dcb.led_state = LED_OFF;
				AP_Aper_led_dcb.led_tim_off = AP_Aper_led_dcb.led_cad & 0x00FF;
			}
			break;
		case LED_OFF :
			if( !(AP_Aper_led_dcb.led_tim_off) )	{
				GPIO_SetValue( 0, 1 << 5);
				AP_Aper_led_dcb.led_state = LED_ON;
				AP_Aper_led_dcb.led_tim_on = (AP_Aper_led_dcb.led_cad >> 8) & 0x00FF;

				if( (AP_Aper_led_dcb.led_blink < BLINK_FOREVER) && (AP_Aper_led_dcb.led_blink > 0) )	{
					AP_Aper_led_dcb.led_blink--;
					if( !AP_Aper_led_dcb.led_blink )	{
						AP_Aper_led_dcb.led_state = LED_IDLE;
						GPIO_ClearValue( 0, 1 << 5);
						AP_Aper_led_dcb.led_cad = 0;
					}
				}
			}
			break;
		default :
			AP_Aper_led_dcb.led_state = LED_IDLE;
			break;
	}

}


void fsm_buzzer( void )
{
	if(!(SysFlag_AP_Apertura & AP_APR_VALID)) 	{
		GPIO_ClearValue(0, (1 << 30));
		Buzzer_dcb.led_cad = 0;
		Buzzer_dcb.led_state = LED_IDLE;
		return;
	}
	switch( Buzzer_dcb.led_state )	{
		case LED_IDLE :
			if( Buzzer_dcb.led_cad )	{
				GPIO_SetValue(0, (1 << 30));
				Buzzer_dcb.led_state = LED_ON;
				Buzzer_dcb.led_tim_on = (Buzzer_dcb.led_cad >> 8) & 0x00FF;
			} else	{
				GPIO_ClearValue(0, (1 << 30));
				Buzzer_dcb.led_tim_off = 0;
				Buzzer_dcb.led_tim_on = 0;
			}
			break;
		case LED_ON :
			if( (!(Buzzer_dcb.led_tim_on)) && (Buzzer_dcb.led_cad & 0x00FF) )	{
				GPIO_ClearValue(0, (1 << 30));
				Buzzer_dcb.led_state = LED_OFF;
				Buzzer_dcb.led_tim_off = Buzzer_dcb.led_cad & 0x00FF;
			}
			break;
		case LED_OFF :
			if( !(Buzzer_dcb.led_tim_off) )	{
				GPIO_SetValue(0, (1 << 30));
				Buzzer_dcb.led_state = LED_ON;
				Buzzer_dcb.led_tim_on = (Buzzer_dcb.led_cad >> 8) & 0x00FF;

				if( (Buzzer_dcb.led_blink < BLINK_FOREVER) && (Buzzer_dcb.led_blink > 0) )	{
					Buzzer_dcb.led_blink--;
					if( !Buzzer_dcb.led_blink )	{
						Buzzer_dcb.led_state = LED_IDLE;
						GPIO_ClearValue(0, (1 << 30));
						Buzzer_dcb.led_cad = 0;
					}
				}
			}
			break;
		default :
			Buzzer_dcb.led_state = LED_IDLE;
			break;
	}

}


void printHexWord(unsigned int mydata)
{
	unsigned char data[4], i;

	data[0] = (unsigned char)((mydata >> 12) & 0x0F);
	data[1] = (unsigned char)((mydata >> 8) & 0x0F);
	data[2] = (unsigned char)((mydata >> 4) & 0x0F);
	data[3] = (unsigned char)(mydata & 0x0F);

	for( i = 0; i < 4; i++ )	{
		if(data[i] > 9)
			CommPutChar(DEBUG_COMM,('A' + data[i]- 10),0);
		else
			CommPutChar(DEBUG_COMM,('0' + data[i]),0);
	}
}

void conio_printHexWord(ConsoleState* state, unsigned int mydata)
{
	unsigned char data[5], i;

	data[0] = (unsigned char)((mydata >> 12) & 0x0F);
	data[1] = (unsigned char)((mydata >> 8) & 0x0F);
	data[2] = (unsigned char)((mydata >> 4) & 0x0F);
	data[3] = (unsigned char)(mydata & 0x0F);

	for( i = 0; i < 4; i++ )	{
		if(data[i] > 9)
            data[i] += 'A' - 10;
		else
            data[i] += '0';
	}
    data[4] = 0;
    state->conio->puts(data);
}

void conio_printByte(ConsoleState* state, unsigned char mydata)
{
	unsigned char data[3], i;

	data[0] = ((mydata >> 4) & 0x0F);
	data[1] = (mydata & 0x0F);

	for( i = 0; i < 2; i++ )	{
		if(data[i] > 9)
            data[i] += 'A' - 10;
		else
            data[i] += '0';
	}
    data[2] = 0;
}

void sprintHexWord(char buffer[], unsigned int mydata)
{
	unsigned char data[4], i;

	data[0] = (unsigned char)((mydata >> 12) & 0x0F);
	data[1] = (unsigned char)((mydata >> 8) & 0x0F);
	data[2] = (unsigned char)((mydata >> 4) & 0x0F);
	data[3] = (unsigned char)(mydata & 0x0F);

	for( i = 0; i < 4; i++ )	{
		if(data[i] > 9)
			buffer[i] = ('A' + data[i]- 10);
		else
			buffer[i] = ('0' + data[i]);
	}
	buffer[4] = 0;
}

void sprintHexDWord(char buffer[], unsigned int mydata)
{
    unsigned char data[8], i;

    data[0] = (unsigned char)((mydata >> 28) & 0x0F);
    data[1] = (unsigned char)((mydata >> 24) & 0x0F);
    data[2] = (unsigned char)((mydata >> 20) & 0x0F);
    data[3] = (unsigned char)((mydata >> 16) & 0x0F);
    data[4] = (unsigned char)((mydata >> 12) & 0x0F);
    data[5] = (unsigned char)((mydata >> 8) & 0x0F);
    data[6] = (unsigned char)((mydata >> 4) & 0x0F);
    data[7] = (unsigned char)(mydata & 0x0F);

    for( i = 0; i < 8; i++ )	{
        if(data[i] > 9)
            buffer[i] = ('A' + data[i]- 10);
        else
            buffer[i] = ('0' + data[i]);
    }
    buffer[8] = 0;
}

void sprintByte(char buffer[], unsigned char mydata)
{
	unsigned char data[2], i;

	data[0] = ((mydata >> 4) & 0x0F);
	data[1] = (mydata & 0x0F);

	for( i = 0; i < 2; i++ )	{
		if(data[i] > 9)
			buffer[i] = ('A' + data[i]- 10);
		else
			buffer[i] = ('0' + data[i]);
	}
	buffer[2] = 0;
}

void printByte(unsigned char mydata)
{
	unsigned char data[2], i;

	data[0] = ((mydata >> 4) & 0x0F);
	data[1] = (mydata & 0x0F);

	for( i = 0; i < 2; i++ )	{
		if(data[i] > 9)
			CommPutChar(DEBUG_COMM,('A' + data[i]- 10),0);
		else
			CommPutChar(DEBUG_COMM,('0' + data[i]),0);
	}
}

void printByte2(unsigned char mydata)
{
	unsigned char data[2], i;

	data[0] = ((mydata >> 4) & 0x0F);
	data[1] = (mydata & 0x0F);

	CommPutChar(DEBUG_COMM,'[',0);
	for( i = 0; i < 2; i++ )	{
		if(data[i] > 9)
			CommPutChar(DEBUG_COMM,('A' + data[i]- 10),0);
		else
			CommPutChar(DEBUG_COMM,('0' + data[i]),0);
	}
	CommPutChar(DEBUG_COMM,']',0);
	if(mydata == 0x0D)	{
		CommPutChar(DEBUG_COMM,'\n',0);
		CommPutChar(DEBUG_COMM,'\r',0);
	}
}

void printBuffByte( uint8_t *data, uint8_t len)
{
	int i;

	for( i = 0; i < len; i++)	{
		//CommPutChar(DEBUG_COMM,'[',0);
		//printByte(data[i]);
		CommPutChar(DEBUG_COMM,data[i],0);
		//CommPutChar(DEBUG_COMM,']',0);
	}
	CommPutChar(DEBUG_COMM,'\n',0);
	CommPutChar(DEBUG_COMM,'\r',0);
}

void printBuffByteRow( uint8_t *data, uint8_t len)
{
	int i;

	for( i = 0; i < len; i++)	{
		CommPutChar(DEBUG_COMM,'[',0);
		printByte(data[i]);
		//CommPutChar(DEBUG_COMM,data[i],0);
		CommPutChar(DEBUG_COMM,']',0);
	}
	CommPutChar(DEBUG_COMM,'\n',0);
	CommPutChar(DEBUG_COMM,'\r',0);
}

void dumpMemory( uint8_t *address, uint16_t len )
{
	uint8_t j;
	uint16_t i;

	CommSendString(DEBUG_COMM, "---------------------------------------------------\n\r");
	for( i = 0; i < len; i += 16 )	{

		printHexWord(i);
		CommPutChar(DEBUG_COMM,'\t',0);
		for( j = 0; j < 16; j++ )	{
			printByte(address[i+j]);
			CommPutChar(DEBUG_COMM,' ',0);
			if( j == 7 )
				CommSendString(DEBUG_COMM, "- ");
		}
		CommPutChar(DEBUG_COMM,'\t',0);
		for( j = 0; j < 16; j++ )	{
			if(address[i+j] >= 0x20)
				CommPutChar(DEBUG_COMM,address[i+j],0);
			else CommPutChar(DEBUG_COMM,'.',0);
		}
		CommSendString(DEBUG_COMM, "\n\r");
	}
}

int i2ctest_wr (void)
{
  uint32_t i;

  for (i = 3; i  < Master_Buffer_BUFSIZE; i++) I2CMasterBuffer[i] = 'a'+i-3;

  I2CWriteLength = Master_Buffer_BUFSIZE;
  I2CReadLength = 0;
  I2CMasterBuffer[0] = MC24LC64_ADDR;
  I2CMasterBuffer[1] = 0x00;		/* address MSB */
  I2CMasterBuffer[2] = 0x00;		/* address LSB */
//  I2CMasterBuffer[3] = 0x55;		/* Data0 */
//  I2CMasterBuffer[4] = 0xAA;		/* Data1 */
//  I2CMasterBuffer[5] = 0x12;		/* Data2 */
//  I2CMasterBuffer[6] = 0x34;		/* Data3 */
  uint32_t fin_state = I2CEngine();

  return 0;
}

int i2ctest_rd (void)
{
	  uint32_t i;

	  if ( I2CInit( (uint32_t)I2CMASTER ) == 0 )	/* initialize I2c */
	  {
		return 1;				/* Fatal error */
	  }

	  for ( i = 0; i < Slave_Buffer_BUFSIZE; i++ )
	  {
		I2CSlaveBuffer[i] = 0x00;
	  }
	  /* Write SLA(W), address, SLA(R), and read 4 bytes back. */

	  I2CWriteLength = 3;
	  I2CReadLength = Slave_Buffer_BUFSIZE;
	  I2CMasterBuffer[0] = MC24LC64_ADDR;
	  I2CMasterBuffer[1] = 0x00;		/* address MSB */
	  I2CMasterBuffer[2] = 0x00;		/* address LSB */
	  I2CMasterBuffer[3] = MC24LC64_ADDR | RD_BIT;

	  i=0;
	  while (I2CEngine() == I2CSTATE_SLA_NACK)
		  i++;

	  return 0;
}

void delay_us( uint32_t microseconds)
{
	uint32_t quantum, uscount, i, j;

	quantum = 14;
	for( uscount = 0; uscount < microseconds; uscount++)	{
		for( i = 0; i < quantum; i++)
			j = j + 1;
	}
}






#define INT_DIGITS 10		/* enough for 64 bit integer */

char *itoa(int i)
{
  /* Room for INT_DIGITS digits, - and '\0' */
  static char buf[INT_DIGITS + 2];
  char *p = buf + INT_DIGITS + 1;	/* points to terminating '\0' */
  if (i >= 0) {
    do {
      *--p = '0' + (i % 10);
      i /= 10;
    } while (i != 0);
    return p;
  }
  else {			/* i < 0 */
    do {
      *--p = '0' - (i % 10);
      i /= 10;
    } while (i != 0);
    *--p = '-';
  }
  return p;
}

uint8_t htoi_nibble( char data)
{
	if(data >= '0' && data <= '9')
		return (data - '0');
	else
	if(data >= 'A' && data <= 'F')
		return (data - 'A' + 10);
	else
	if(data >= 'a' && data <= 'f')
		return (data - 'a' + 10);
	else return 0;
}

uint16_t BCD_Word_to_int( uint16_t data)
{
	uint16_t temp, miles, centena, decena, unidad;

	unidad = data & 0x000F;
	decena = (data >> 4) & 0x000F;
	centena = (data >> 8) & 0x000F;
	miles = (data >> 12) & 0x000F;

	temp = miles*1000 + centena*100 + decena*10 + unidad;
	return temp;

}

int ChrIndexOf( uint8_t *string, uint8_t key,  int start)
{
	int i, len;

	len = sizeof(string);

	for( i = 0; i < len; i++ )	{
		if(string[start + i] == key)
			return i;
	}
	return -1;
}

uint8_t *SubString(uint8_t *string1, uint8_t *string2, int start, int end)
{
	int i, j;

	if((end - start) > sizeof(string2))	{
		string2[0] = '\0';
		return string2;
	}

	string2[0] = '\0';
	for( i = start, j = 0; i < end; i++, j++)	{
		string2[j] = string1[i];
	}
	string2[j] = '\0';

	return string2;
}

uint8_t *SubStringEnd(uint8_t *string1, uint8_t *string2, int start)
{
	int i, j, end;

	end = sizeof(string1);

	if((end - start) > sizeof(string2))	{
		string2[0] = '\0';
		return string2;
	}


	for( i = start, j = 0; i < end; i++, j++)	{
		string2[j] = string1[i];
	}
	string2[j] = '\0';

	return string2;
}


int str_starts_with(const char *str, const char *start)
{
	return !Str_Cmp_N (str, start, strlen(start));
}

char *strnext(char **sp, const char *delim)
{
	char *tk = *sp;
	char *p;

	if(*sp) {
		p = Str_Char((const  CPU_CHAR *)tk, *delim);
		if(p) {
			/* found delimiter */
			if(*p) {
				*sp = p+1;
			} else {
				*sp = 0; /* nothing ahead, so terminate */
			}
			*p = '\0';	/* terminate on the delimiter */
		} else {
			tk = 0;		/* not found */
		}
	}
	return tk;
}


int StrIndexOf(uint8_t s1[], uint8_t s2[])
{

	uint8_t *inptr;

	inptr = Str_Str( (const  CPU_CHAR *)s1, (const  CPU_CHAR *)s2);
	if(inptr)	{
		return (s1-inptr);
	} else
		return -1;

}

int StrLastIndexOf(uint8_t s1[], uint8_t s2[])
{
	uint8_t *inptr, *lastptr;
	int s2len, s1len;

	s2len = strlen(s2);
	s1len = strlen(s1);
	if(s2len > s1len)
		return -1;

	inptr = Str_Str( (const  CPU_CHAR *)s1, (const  CPU_CHAR *)s2);
	while(inptr)	{
		lastptr = Str_Str( (const  CPU_CHAR *)(inptr + s2len), (const  CPU_CHAR *)s2);
		if(lastptr)	{
			inptr = lastptr;
		}
		else
			break;
	}
	if(inptr)	{
		return (inptr - s1);
	} else
		return -1;

}




//int atoi(const char *s)
//{
//	static const char digits[] = "0123456789";  /* legal digits in order */
//	unsigned int val = 0;         /* value we're accumulating */
//	int neg = 0;              /* set to true if we see a minus sign */
//
//	 /* skip whitespace */
//	 while (*s==' ' || *s=='\t')	{
//			 s++;
//	 }
//
//	 /* check for sign */
//	 if (*s=='-') {
//			 neg=1;
//			 s++;
//	 }
//	 else if (*s=='+') {
//			 s++;
//	 }
//
//	 /* process each digit */
//	 while (*s) {
//		 const char *where;
//		 unsigned digit;
//
//		 /* look for the digit in the list of digits */
//		 where = strchr(digits, *s);
//		 if (where==NULL) {
//				 /* not found; not a digit, so stop */
//				 break;
//		}
//
//		 /* get the index into the digit list, which is the value */
//		 digit = (where - digits);
//
//		 /* could (should?) check for overflow here */
//
//		 /* shift the number over and add in the new digit */
//		 val = val*10 + digit;
//
//		 /* look at the next character */
//		s++;
//	 }
//
//	 /* handle negative numbers */
//	 if (neg) {
//			 return -val;
//	 }
//
//	 /* done */
//	 return val;
//}


void BuffPutHex (char *buffer, uint8_t hexnum)
{
	uint8_t nibble;

	nibble = (hexnum >> 4) & 0x0F;
	buffer[0] = (nibble > 9) ? ('A' + nibble - 10) : ('0' + nibble);
	nibble = hexnum & 0x0F;
	buffer[1] = (nibble > 9) ? ('A' + nibble - 10) : ('0' + nibble);
	buffer[2] = '\0';

}

uint16_t atoh(char *String)
{
    uint16_t Value = 0, Digit;
    char c;

    while ((c = *String++)) {

        if (c >= '0' && c <= '9')
            Digit = (uint16_t) (c - '0');
        else if (c >= 'a' && c <= 'f')
            Digit = (uint16_t) (c - 'a') + 10;
        else if (c >= 'A' && c <= 'F')
            Digit = (uint16_t) (c - 'A') + 10;
        else
            break;

        Value = (Value << 4) + Digit;
    }

    return Value;
}

