/*
 * eeprom_map.h
 *
 *  Created on: Dec 18, 2011
 *      Author: IronMan
 */

#ifndef EEPROM_MAP_H_
#define EEPROM_MAP_H_

#define	E2P_SYSCFG_BASEADDR			0x0000
#define	E2P_SYSCFG_OFFSET			0x0000
#define	E2P_IPCFG_OFFSET			0x0100
#define	E2P_PTMTBL_OFFSET			0x0400


//--- Area de configuracion de sistema ---
#define	RF_NUMABO_E2P_ADDR			E2P_SYSCFG_BASEADDR 	+ E2P_SYSCFG_OFFSET	//Numero del abonado para sistema de RF
#define	RF_ANSWER_TYPE				RF_NUMABO_E2P_ADDR		+	1				//Typo de respuesta o protocolo de RF
#define FMODEON_ADDR				RF_ANSWER_TYPE			+	1
#define	FMODELAY_ADDR				FMODEON_ADDR			+	1

#define	MON_NUMABO_E2P_ADDR			FMODELAY_ADDR			+	1				//Numero del abonado para los monitoreos
#define	RFNUMCEN_E2P_ADDR			MON_NUMABO_E2P_ADDR		+	1				//Numero de la central de RF asignada
#define	CIDACCOUNT1_E2P_ADDR		RFNUMCEN_E2P_ADDR		+	1				//Cuenta ContactID asignada a la sucursal
#define CIDACCOUNT2_E2P_ADDR		CIDACCOUNT1_E2P_ADDR	+	2
#define NOMSUC_E2P_ADDR				CIDACCOUNT2_E2P_ADDR	+	2				//Nombre de la sucursal
#define	ZONE_E2P_ADDR				NOMSUC_E2P_ADDR			+	33
#define PREVETIMER_E2P_ADDR			ZONE_E2P_ADDR			+	1
#define	R3KSENDMODE_E2P_ADDR		PREVETIMER_E2P_ADDR		+	1
#define	REDBOXHAB_E2P_ADDR			R3KSENDMODE_E2P_ADDR	+	1
#define	SERIALNUM_E2P_ADDR			REDBOXHAB_E2P_ADDR		+	1
#define	PAPTSLOT_E2P_ADDR			SERIALNUM_E2P_ADDR		+	8
#define	PTXORMASK_E2P_ADDR			PAPTSLOT_E2P_ADDR		+	2
#define PAPARST_TIMER_E2P_ADDR		PTXORMASK_E2P_ADDR		+	1
#define	R3KACCOUNT_E2P_ADDR			PAPARST_TIMER_E2P_ADDR	+	2
#define	SYSFLAG1_E2P_ADDR			R3KACCOUNT_E2P_ADDR		+	2
#define	WDEVOTIMER_E2P_ADDR			SYSFLAG1_E2P_ADDR		+	1
#define	WDEVOEVENT_E2P_ADDR			WDEVOTIMER_E2P_ADDR		+	2
#define	TACHOF220_E2P_ADDR			WDEVOEVENT_E2P_ADDR		+	2

#define	ENDSYSCFG_E2P_ADDR			TACHOF220_E2P_ADDR		+	1
#define	BOOTMARK1_E2P_ADDR			E2P_SYSCFG_BASEADDR     +   0xF0
#define	BOOTMARK2_E2P_ADDR			E2P_SYSCFG_BASEADDR     +   0xF1
#define FIDUMARK_E2P_ADDR           E2P_SYSCFG_BASEADDR     +   0xFE

//--- Area de configuracion de datos IP ---
#define	LOCAL_IP_E2P_ADDR			E2P_SYSCFG_BASEADDR 	+ E2P_IPCFG_OFFSET	//Direccion IP de la placa
#define	NETMASK_E2P_ADDR			LOCAL_IP_E2P_ADDR		+	16				//Mascara de red
#define	GATEWAY_E2P_ADDR			NETMASK_E2P_ADDR		+	16				//Direccion del Gateway
#define	SERVER1_E2P_ADDR			GATEWAY_E2P_ADDR		+	16				//Direccion de la receptora de monitoreo 1
#define	SERVER2_E2P_ADDR			SERVER1_E2P_ADDR		+	16				//Direccion de la receptora de monitoreo 2
#define	PORT1_E2P_ADDR				SERVER2_E2P_ADDR		+	16				//Puerto IP de la receptora 1
#define	PORT2_E2P_ADDR				PORT1_E2P_ADDR			+	2				//Puerto IP de la receptora 2
#define	MACADDR_E2P_ADDR			PORT2_E2P_ADDR			+	2				//MAC address de la placa
#define	HBTTIME1_E2P_ADDR			MACADDR_E2P_ADDR		+	6				//Tiempo de HearBeat para receptora 1
#define	HBTTIME2_E2P_ADDR			HBTTIME1_E2P_ADDR		+	2				//Tiempo de HearBeat para receptora 2
#define	PROTCOL1_E2P_ADDR			HBTTIME2_E2P_ADDR		+	2				//Protocolo de alarma para server1
#define	PROTCOL2_E2P_ADDR			PROTCOL1_E2P_ADDR		+	1				//Protocolo de alarma para server1
#define	INUSE1_E2P_ADDR				PROTCOL2_E2P_ADDR		+	1				//Bunker 1 activo
#define	INUSE2_E2P_ADDR				INUSE1_E2P_ADDR			+	1				//Bunker 2 activo
#define TASMODE_E2P_ADDR			INUSE2_E2P_ADDR			+	1				//Activacion de modo TAS
#define	WDEVOMODE_E2P_ADDR			TASMODE_E2P_ADDR		+	1
#define	RADARMODE_E2P_ADDR			WDEVOMODE_E2P_ADDR		+	1
#define	NPMEDMODE_E2P_ADDR			RADARMODE_E2P_ADDR		+	1
#define	ETHPHYMODE_E2P_ADDR			NPMEDMODE_E2P_ADDR		+	1
#define	RHBTIMER_E2P_ADDR			ETHPHYMODE_E2P_ADDR		+	1
#define	RHBCTIMER_E2P_ADDR			RHBTIMER_E2P_ADDR		+	2
#define IP150CHECK_E2P_ADDR         RHBCTIMER_E2P_ADDR      +   2

#define	ENDIPCFG_E2P_ADDR			IP150CHECK_E2P_ADDR 	+	2				//--- Fin de area


#define	DHCPIPADD_E2P_ADDR			0x200
#define	DHCPGWADD_E2P_ADDR			0x220
#define	DHCPNMASK_E2P_ADDR			0x240
#define	DHCPUSED_E2P_ADDR			0x260				// Uso 1 byte
#define	DHCPJUMPER_E2P_ADDR			0x261

#define	IPADDCHK_E2P_ADDR			0x280 
#define	GWADDCHK_E2P_ADDR			0x281 
#define	NMASKCHK_E2P_ADDR			0x282 
#define	MACADDCHK_E2P_ADDR			0x283 

#define	SRV1ACHK_E2P_ADDR			0x284
#define	SRV2ACHK_E2P_ADDR			0x285
 
#define	OPENPTM_E2P_ADDR			0x286
 
#define	ENDIPCHK_E2P_ADDR			0x287

// Area de la tabla de datos de los PTM por 485
#define	PTM00DCB_E2P_ADDR			E2P_PTMTBL_OFFSET
#define	ENDPTMDCB_E2P_ADDR			E2P_PTMTBL_OFFSET + 32*5

#endif /* EEPROM_MAP_H_ */
