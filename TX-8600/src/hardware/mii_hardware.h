#ifndef __MII_HARDWARE_H
#define __MII_HARDWARE_H

/*
 //=============================================================================
 Ethernet Moudle IO Define For MII-interface
//=============================================================================*/
// MII Port

on tile[1]: in port p_rxclk = XS1_PORT_1D;
on tile[1]: in port p_rxer = XS1_PORT_1C;
on tile[1]: in port p_rxd = XS1_PORT_4E;
on tile[1]: in port p_rxdv = XS1_PORT_1L;
on tile[1]: in port p_txclk = XS1_PORT_1P;
on tile[1]: out port p_txen = XS1_PORT_1N;
on tile[1]: out port p_txd = XS1_PORT_4F;
on tile[1]: in port p_inmiit = XS1_PORT_1O;

// Internel ClockPort NoUse Extern
on tile[1]: port p_eth_dummy = XS1_PORT_4C;//

//Smi Port
on tile[1]: port p_smi_mdio = XS1_PORT_1K;
on tile[1]: port p_smi_mdc = XS1_PORT_1M;

//Eth Clk Set
on tile[1]: clock eth_rxclk = XS1_CLKBLK_1;
on tile[1]: clock eth_txclk = XS1_CLKBLK_2;

/*
	// MII Port
	on tile[1]: in port p_rxclk = XS1_PORT_1C;
	on tile[1]: in port p_rxer = XS1_PORT_1D;
	on tile[1]: in port p_rxd = XS1_PORT_4E;
	on tile[1]: in port p_rxdv = XS1_PORT_1K;
	on tile[1]: in port p_txclk = XS1_PORT_1G;
	on tile[1]: out port p_txen = XS1_PORT_1L;
	on tile[1]: out port p_txd = XS1_PORT_4F;
	
	on tile[1]: out port p_eth_reset = XS1_PORT_1N;
	
	// Internel ClockPort NoUse Extern
	on tile[1]: port p_eth_dummy = XS1_PORT_4C;//
	
	//Smi Port
	on tile[1]: port p_smi_mdio = XS1_PORT_1O;
	on tile[1]: port p_smi_mdc = XS1_PORT_1P;
	
	//Eth Clk Set
	on tile[1]: clock eth_rxclk = XS1_CLKBLK_1;
	on tile[1]: clock eth_txclk = XS1_CLKBLK_2;
*/

#endif //__MII_HARDWARE_H


