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
#include "xtmrctr.h"
#include "xparameters.h"
#include "xil_exception.h"
#include "xsysmon.h"
#include "xintc.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are only defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define TMRCTR_DEVICE_ID        XPAR_TMRCTR_0_DEVICE_ID

#ifdef __MICROBLAZE__
#define TMRCTR_INTERRUPT_ID     XPAR_INTC_0_TMRCTR_0_VEC_ID
#else
#define TMRCTR_INTERRUPT_ID     XPAR_FABRIC_TMRCTR_0_VEC_ID
#endif

#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC_DEVICE_ID          XPAR_INTC_0_DEVICE_ID
#define INTC                    XIntc
#define INTC_HANDLER            XIntc_InterruptHandler
#else
#define INTC_DEVICE_ID          XPAR_SCUGIC_SINGLE_DEVICE_ID
#define INTC                    XScuGic
#define INTC_HANDLER            XScuGic_InterruptHandler
#endif /* XPAR_INTC_0_DEVICE_ID */

#define PWM_PERIOD              500000000    /* PWM period in (500 ms) */
#define TMRCTR_0                0            /* Timer 0 ID */
#define TMRCTR_1                1            /* Timer 1 ID */
#define CYCLE_PER_DUTYCYCLE     10           /* Clock cycles per duty cycle */
#define MAX_DUTYCYCLE           100          /* Max duty cycle */
#define DUTYCYCLE_DIVISOR       4            /* Duty cycle Divisor */
#define WAIT_COUNT              PWM_PERIOD   /* Interrupt wait counter */

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
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
int TmrCtrInit(INTC *IntcInstancePtr, XTmrCtr *InstancePtr, u16 DeviceId,
								u16 IntrId);

int XADCInit(XSysMon* SysMonInstPtr, u16 SysMonDeviceId);

static void TimerCounterHandler(void *CallBackRef, u8 TmrCtrNumber);
static int TmrCtrSetupIntrSystem(INTC *IntcInstancePtr, XTmrCtr *InstancePtr,
						u16 DeviceId, u16 IntrId);
static void TmrCtrDisableIntr(INTC *IntcInstancePtr, u16 IntrId);

/************************** Variable Definitions *****************************/
INTC InterruptController;  /* The instance of the Interrupt Controller */
XTmrCtr TimerCounterInst;  /* The instance of the Timer Counter */

static XSysMon SysMonInst; 	  /* System Monitor driver instance */

/*
 * The following variables are shared between non-interrupt processing and
 * interrupt processing such that they must be global.
 */
static int   PeriodTimerHit = FALSE;
static int   HighTimerHit = FALSE;

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

	/* Run the Timer Counter PWM example */
	Status = TmrCtrInit(&InterruptController, &TimerCounterInst,
				  TMRCTR_DEVICE_ID, TMRCTR_INTERRUPT_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Tmrctr PWM Example Failed\r\n");
		return XST_FAILURE;
	}

	Status = XADCInit(&SysMonInst,SYSMON_DEVICE_ID);

	while(1){
//		u16 adcData = 0;
//		adcData = XSysMon_GetAdcData(&SysMonInst,XSM_CH_AUX_MIN + 4);
//		xil_printf("%u\r\n",(adcData >> 4) & 0xFFF);
	}

	TmrCtrDisableIntr(&InterruptController, TMRCTR_INTERRUPT_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Tmrctr PWM Example Failed\r\n");
		return XST_FAILURE;
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


/*****************************************************************************/
/**
* This function demonstrates the use of tmrctr PWM APIs.
*
* @param	IntcInstancePtr is a pointer to the Interrupt Controller
*		driver Instance
* @param	TmrCtrInstancePtr is a pointer to the XTmrCtr driver Instance
* @param	DeviceId is the XPAR_<TmrCtr_instance>_DEVICE_ID value from
*		xparameters.h
* @param	IntrId is XPAR_<INTC_instance>_<TmrCtr_instance>_INTERRUPT_INTR
*		value from xparameters.h
*
* @return	XST_SUCCESS if the Test is successful, otherwise XST_FAILURE
*
* @note		none.
*
*****************************************************************************/
int TmrCtrInit(INTC *IntcInstancePtr, XTmrCtr *TmrCtrInstancePtr,
						u16 DeviceId, u16 IntrId)
{
//	u8  DutyCycle;
//	u8  NoOfCycles;
//	u8  Div;
//	u32 Period;
//	u32 HighTime;
//	u64 WaitCount;
	int Status;

	/*
	 * Initialize the timer counter so that it's ready to use,
	 * specify the device ID that is generated in xparameters.h
	 */
	Status = XTmrCtr_Initialize(TmrCtrInstancePtr, DeviceId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to ensure that the hardware was built
	 * correctly. Timer0 is used for self test
	 */
	Status = XTmrCtr_SelfTest(TmrCtrInstancePtr, TMRCTR_0);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the timer counter to the interrupt subsystem such that
	 * interrupts can occur
	 */
	Status = TmrCtrSetupIntrSystem(IntcInstancePtr, TmrCtrInstancePtr,
							DeviceId, IntrId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Setup the handler for the timer counter that will be called from the
	 * interrupt context when the timer expires
	 */
	XTmrCtr_SetHandler(TmrCtrInstancePtr, TimerCounterHandler,
							TmrCtrInstancePtr);

	/* Enable the interrupt of the timer counter */
	XTmrCtr_SetOptions(TmrCtrInstancePtr, TMRCTR_0, XTC_INT_MODE_OPTION);
	XTmrCtr_SetOptions(TmrCtrInstancePtr, TMRCTR_1, XTC_INT_MODE_OPTION);

	/*
	 * We start with the fixed divisor and after every CYCLE_PER_DUTYCYCLE
	 * decrement the divisor by 1, as a result Duty cycle increases
	 * proportionally. This is done until duty cycle is reached upto
	 * MAX_DUTYCYCLE
	Div = DUTYCYCLE_DIVISOR;

	// Configure PWM
	do {
		//Fail check for 0 divisor
		if (!Div) {
			Status = XST_FAILURE;
			goto err;
		}

		//Disable PWM for reconfiguration
		XTmrCtr_PwmDisable(TmrCtrInstancePtr);

		//Configure PWM
		Period = PWM_PERIOD;
		HighTime = PWM_PERIOD / Div--;
		DutyCycle = XTmrCtr_PwmConfigure(TmrCtrInstancePtr, Period,
								HighTime);
		if (Status != XST_SUCCESS) {
			Status = XST_FAILURE;
			goto err;
		}

		xil_printf("PWM Configured for Duty Cycle = %d\r\n", DutyCycle);

		//Enable PWM
		XTmrCtr_PwmEnable(TmrCtrInstancePtr);

		NoOfCycles = 0;
		WaitCount = WAIT_COUNT;
		while (NoOfCycles < CYCLE_PER_DUTYCYCLE) {
			if (PeriodTimerHit == TRUE && HighTimerHit == TRUE) {
				PeriodTimerHit = FALSE;
				HighTimerHit = FALSE;
				WaitCount = WAIT_COUNT;
				NoOfCycles++;
			}

			//Interrupt did not occur as expected
			if (!(--WaitCount)) {
				return XST_FAILURE;
			}
		}
	} while (DutyCycle < MAX_DUTYCYCLE);

	Status = XST_SUCCESS;
err:
	//Disable PWM
	XTmrCtr_PwmDisable(TmrCtrInstancePtr);

	//Disable interrupts
	TmrCtrDisableIntr(IntcInstancePtr, DeviceId);
	 */
	return Status;
}

/*****************************************************************************/
/**
* This function is the handler which performs processing for the timer counter.
* It is called from an interrupt context.
*
* @param	CallBackRef is a pointer to the callback function
* @param	TmrCtrNumber is the number of the timer to which this
*		handler is associated with.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void TimerCounterHandler(void *CallBackRef, u8 TmrCtrNumber)
{
	/* Mark if period timer expired */
	if (TmrCtrNumber == TMRCTR_0) {
		PeriodTimerHit = TRUE;
	}

	/* Mark if high time timer expired */
	if (TmrCtrNumber == TMRCTR_1) {
		HighTimerHit = TRUE;
	}
}

/*****************************************************************************/
/**
* This function setups the interrupt system such that interrupts can occur
* for the timer counter. This function is application specific since the actual
* system may or may not have an interrupt controller.  The timer counter could
* be directly connected to a processor without an interrupt controller.  The
* user should modify this function to fit the application.
*
* @param	IntcInstancePtr is a pointer to the Interrupt Controller
*		driver Instance.
* @param	TmrCtrInstancePtr is a pointer to the XTmrCtr driver Instance.
* @param	DeviceId is the XPAR_<TmrCtr_instance>_DEVICE_ID value from
*		xparameters.h.
* @param	IntrId is XPAR_<INTC_instance>_<TmrCtr_instance>_VEC_ID
*		value from xparameters.h.
*
* @return	XST_SUCCESS if the Test is successful, otherwise XST_FAILURE.
*
* @note		none.
*
******************************************************************************/
static int TmrCtrSetupIntrSystem(INTC *IntcInstancePtr,
			XTmrCtr *TmrCtrInstancePtr, u16 DeviceId, u16 IntrId)
{
	 int Status;

#ifdef XPAR_INTC_0_DEVICE_ID
	/*
	 * Initialize the interrupt controller driver so that
	 * it's ready to use, specify the device ID that is generated in
	 * xparameters.h
	 */
	Status = XIntc_Initialize(IntcInstancePtr, INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect a device driver handler that will be called when an interrupt
	 * for the device occurs, the device driver handler performs the
	 * specific interrupt processing for the device
	 */
	Status = XIntc_Connect(IntcInstancePtr, IntrId,
				(XInterruptHandler)XTmrCtr_InterruptHandler,
				(void *)TmrCtrInstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Start the interrupt controller such that interrupts are enabled for
	 * all devices that cause interrupts, specific real mode so that
	 * the timer counter can cause interrupts through the interrupt
	 * controller
	 */
	Status = XIntc_Start(IntcInstancePtr, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Enable the interrupt for the timer counter */
	XIntc_Enable(IntcInstancePtr, IntrId);
#else
	XScuGic_Config *IntcConfig;

	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use
	 */
	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(IntcInstancePtr, IntcConfig,
					IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XScuGic_SetPriorityTriggerType(IntcInstancePtr, IntrId,
					0xA0, 0x3);

	/*
	 * Connect the interrupt handler that will be called when an
	 * interrupt occurs for the device.
	 */
	Status = XScuGic_Connect(IntcInstancePtr, IntrId,
				 (Xil_ExceptionHandler)XTmrCtr_InterruptHandler,
				 TmrCtrInstancePtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/* Enable the interrupt for the Timer device */
	XScuGic_Enable(IntcInstancePtr, IntrId);
#endif /* XPAR_INTC_0_DEVICE_ID */

	/* Initialize the exception table */
	Xil_ExceptionInit();

	/* Register the interrupt controller handler with the exception table */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
					(Xil_ExceptionHandler)
					INTC_HANDLER,
					IntcInstancePtr);

	/* Enable non-critical exceptions */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}

/******************************************************************************/
/**
*
* This function disconnects the interrupts for the Timer.
*
* @param	IntcInstancePtr is a reference to the Interrupt Controller
*		driver Instance.
* @param	IntrId is XPAR_<INTC_instance>_<Timer_instance>_VEC_ID
*		value from xparameters.h.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void TmrCtrDisableIntr(INTC *IntcInstancePtr, u16 IntrId)
{
	/* Disconnect the interrupt for the timer counter */
#ifdef XPAR_INTC_0_DEVICE_ID
	XIntc_Disconnect(IntcInstancePtr, IntrId);
#else
	XScuGic_Disconnect(IntcInstancePtr, IntrId);
#endif
}
