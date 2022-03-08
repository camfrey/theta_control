/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file  xtmrctr_pwm_example.c
*
* This file contains a design example using the timer counter driver
* and hardware device using interrupt mode. The example demonstrates
* the use of PWM feature of axi timers. PWM is configured to operate at specific
* duty cycle and after every N cycles the duty cycle is incremented until a
* specific duty cycle is achieved. No software validation of duty cycle is
* undergone in the example.
*
* This example assumes that the interrupt controller is also present as a part
* of the system.
*
*
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date	 Changes
* ----- ---- -------- -----------------------------------------------
* 1.00b cjp  03/28/18 First release
*</pre>
******************************************************************************/

/***************************** Include Files *********************************/
#include "xparameters.h"
#include "xil_exception.h"
#include "xsysmon.h"
#include "xintc.h"
#include "xil_printf.h"
#include "pwm_gen.h"

/************************** Constant Definitions *****************************/
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are only defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef TESTAPP_GEN
#define SYSMON_DEVICE_ID	XPAR_SYSMON_0_DEVICE_ID
#endif

#ifdef XPAR_INTC_0_DEVICE_ID	/* Interrupt Controller */
#define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
#define INTR_ID			XPAR_INTC_0_SYSMON_0_VEC_ID
#else	/* SCUGIC Interrupt Controller */
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define INTR_ID		XPAR_FABRIC_SYSTEM_MANAGEMENT_WIZ_0_IP2INTC_IRPT_INTR
#endif /* XPAR_INTC_0_DEVICE_ID */

#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC		XIntc
#define INTC_HANDLER	XIntc_InterruptHandler
#else
#define INTC		XScuGic
#define INTC_HANDLER	XScuGic_InterruptHandler
#endif

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int XADCInit(XSysMon* SysMonInstPtr, u16 SysMonDeviceId);


/************************** Variable Definitions *****************************/
static XSysMon SysMonInst; 	  /* System Monitor driver instance */


/*****************************************************************************/
/**
* This function is the main function of the Tmrctr PWM example.
*
* @param	None.
*
* @return	XST_SUCCESS to indicate success, else XST_FAILURE to indicate a
*		Failure.
*
* @note		None.
*
	Status=  XSysMon_SetSingleChParams(SysMonInstPtr, XSM_CH_AUX_MIN+3,
******************************************************************************/
int main(void)
{
	int Status;

	Status = XADCInit(&SysMonInst,SYSMON_DEVICE_ID);

	if (Status != XST_SUCCESS) {
		xil_printf("Tmrctr PWM Example Failed\r\n");
		return XST_FAILURE;
	}

	while(1){
		PWM_GEN_mWriteReg(XPAR_PWM_GEN_0_S_AXI_BASEADDR,PWM_GEN_S_AXI_SLV_REG0_OFFSET,0);
		PWM_GEN_mWriteReg(XPAR_PWM_GEN_0_S_AXI_BASEADDR,PWM_GEN_S_AXI_SLV_REG1_OFFSET,250);
		PWM_GEN_mWriteReg(XPAR_PWM_GEN_0_S_AXI_BASEADDR,PWM_GEN_S_AXI_SLV_REG2_OFFSET,500);
		PWM_GEN_mWriteReg(XPAR_PWM_GEN_0_S_AXI_BASEADDR,PWM_GEN_S_AXI_SLV_REG3_OFFSET,750);
		PWM_GEN_mWriteReg(XPAR_PWM_GEN_0_S_AXI_BASEADDR,PWM_GEN_S_AXI_SLV_REG4_OFFSET,1000);
		PWM_GEN_mWriteReg(XPAR_PWM_GEN_0_S_AXI_BASEADDR,PWM_GEN_S_AXI_SLV_REG5_OFFSET,1250);
		PWM_GEN_mWriteReg(XPAR_PWM_GEN_0_S_AXI_BASEADDR,PWM_GEN_S_AXI_SLV_REG6_OFFSET,1500);
		PWM_GEN_mWriteReg(XPAR_PWM_GEN_0_S_AXI_BASEADDR,PWM_GEN_S_AXI_SLV_REG7_OFFSET,1750);
	}

	//xil_printf("Successfully ran Tmrctr PWM Example\r\n");
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function runs a test on the System Monitor/ADC device using the
* driver APIs.
*
* The function does the following tasks:
*	- Initiate the System Monitor/ADC device driver instance
*	- Run self-test on the device
*	- Reset the device
*	- Set up alarm fQuick Accessor VCCINT
*	- Set up the configuration registers for single channel continuous mode
*	for VCCINT channel
*	- Setup interrupt system
*	- Enable interrupts
*	- Wait until the VCCINT alarm interrupt occurs
*
* @param	IntcInstancePtr is a pointer to the Interrupt Controller
* @param	SysMonInstPtr is a pointer to the XSysMon driver Instance.
* @param	SysMonDeviceId is the XPAR_<SYSMON_ADC_instance>_DEVICE_ID value
*		from xparameters.h.
*
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
* @note		This function may never return if no interrupt occurs.
*
****************************************************************************/
int XADCInit(XSysMon* SysMonInstPtr, u16 SysMonDeviceId)
{
	int Status;
	XSysMon_Config *ConfigPtr;

	/*
	 * Initialize the SysMon driver.
	 */
	ConfigPtr = XSysMon_LookupConfig(SysMonDeviceId);
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}
	XSysMon_CfgInitialize(SysMonInstPtr, ConfigPtr, ConfigPtr->BaseAddress);

	/*
	 * Self Test the System Monitor/ADC device.
	 */
	Status = XSysMon_SelfTest(SysMonInstPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XSysMon_SetSequencerMode(SysMonInstPtr, XSM_SEQ_MODE_SINGCHAN);

	/*
	 * Set the configuration registers for single channel continuous mode
	 * of operation for the VCCINT channel.
	 */
	Status=  XSysMon_SetSingleChParams(SysMonInstPtr, XSM_CH_AUX_MIN + 4,
						FALSE, FALSE, FALSE);
	if(Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XSysMon_SetAlarmEnables(SysMonInstPtr, 0x0);

	return XST_SUCCESS;
}
