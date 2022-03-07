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
#include "phase_calc.h"
//#include "print_functions.h" // for some reason get duplicate error

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

const double phase_Min=-18.36089378; // Min phase angle of the left most element at (45 degrees)
const double phase_Max=18.36089378; // Max phase angle of the right most element at (45 degrees)

const double pwm_Min=0;
const double pwm_Max=pow(2,11);


int main(void)
{
	int Status;

	Status = XADCInit(&SysMonInst,SYSMON_DEVICE_ID);

	if (Status != XST_SUCCESS) {
		xil_printf("Tmrctr PWM Example Failed\r\n");
		return XST_FAILURE;
	}
/*
 * Phase Testing Stuff
 */

double pos_x[]={-0.0567,-0.0405,-0.0243,-0.0081,0.0081,0.0243,0.0405,0.0567}; // position of each array column (in mm)

int pos_x_size = (sizeof pos_x/sizeof pos_x[0]);

xil_printf("Size: %d\n", pos_x_size); //debug

double phase_result[pos_x_size]; // Array of the phase calculation for each element

double pwm_result[pos_x_size]; // Array of the element values converted from phase to PWM

double desired_angle=10; // angle to steer the beam


memset(phase_result, 0.0, sizeof phase_result); // initializing all elements in phase_result to 0


calcPhase(&pos_x,&phase_result,pos_x_size,desired_angle); // calculate the phases for each array element


for(int i =0; i<pos_x_size;i++){ // debug to make sure phase calculations are being calculated and assigned correctly
	xil_printf("Phase element %d: ",i);
	print_float(phase_result[i]);
	xil_printf("\n");
}

for(int i=0;i<pos_x_size;i++){ // Map phase caluculations to PWM range (0-2^11)
pwm_result[i]=mapToPWM(phase_result[i],phase_Min,phase_Max,pwm_Min,pwm_Max);
}

for(int i =0; i<pos_x_size;i++){ // debug to make sure PWM calculations are being calculated and assigned correctly
	xil_printf("PWM element %d: ",i);
	print_float(pwm_result[i]);
	xil_printf("\n");
}

	while(1){
//		u16 adcData = 0;
//		adcData = XSysMon_GetAdcData(&SysMonInst,XSM_CH_AUX_MIN + 4);
//		xil_printf("%u\r\n",(adcData >> 4) & 0xFFF);
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
