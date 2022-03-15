/***************************** Include Files *********************************/
#include "xparameters.h"
#include "xsysmon.h"
<<<<<<< HEAD
#include "xintc.h"
#include "xil_printf.h"
#include "phase_calc.h"
//#include "print_functions.h" // for some reason get duplicate error
=======
#include "xil_io.h"
>>>>>>> origin/working
#include "pwm_gen.h"
#include "xgpio.h"
#include "sleep.h"

/************************** Constant Definitions *****************************/
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are only defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define SYSMON_DEVICE_ID	XPAR_SYSMON_0_DEVICE_ID
#define GPIO_OUTPUT_BANK	2
#define GPIO_INPUT_BANK		1

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int XADCInit(XSysMon* SysMonInstPtr, u16 SysMonDeviceId);


/************************** Variable Definitions *****************************/
static XSysMon SysMonInst; 	  /* System Monitor driver instance */
static XGpio gpio;


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

double desired_angle=-35; // angle to steer the beam


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

	Status = XGpio_Initialize(&gpio, XPAR_AXI_GPIO_0_DEVICE_ID);

	if (Status != XST_SUCCESS) {
		xil_printf("Tmrctr PWM Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Hello World\n");

	XGpio_SetDataDirection(&gpio,GPIO_INPUT_BANK,0x1F);
	XGpio_SetDataDirection(&gpio,GPIO_OUTPUT_BANK,0x0);

	PWM_GEN_mWriteReg(XPAR_PWM_GEN_0_S_AXI_BASEADDR,PWM_GEN_S_AXI_SLV_REG0_OFFSET,0);
	PWM_GEN_mWriteReg(XPAR_PWM_GEN_0_S_AXI_BASEADDR,PWM_GEN_S_AXI_SLV_REG1_OFFSET,250);
	PWM_GEN_mWriteReg(XPAR_PWM_GEN_0_S_AXI_BASEADDR,PWM_GEN_S_AXI_SLV_REG2_OFFSET,500);
	PWM_GEN_mWriteReg(XPAR_PWM_GEN_0_S_AXI_BASEADDR,PWM_GEN_S_AXI_SLV_REG3_OFFSET,750);
	PWM_GEN_mWriteReg(XPAR_PWM_GEN_0_S_AXI_BASEADDR,PWM_GEN_S_AXI_SLV_REG4_OFFSET,1000);
	PWM_GEN_mWriteReg(XPAR_PWM_GEN_0_S_AXI_BASEADDR,PWM_GEN_S_AXI_SLV_REG5_OFFSET,1250);
	PWM_GEN_mWriteReg(XPAR_PWM_GEN_0_S_AXI_BASEADDR,PWM_GEN_S_AXI_SLV_REG6_OFFSET,1500);
	PWM_GEN_mWriteReg(XPAR_PWM_GEN_0_S_AXI_BASEADDR,PWM_GEN_S_AXI_SLV_REG7_OFFSET,1750);

	while(1){
		XGpio_DiscreteWrite(&gpio,GPIO_OUTPUT_BANK,0x1F);
		sleep(1);
		XGpio_DiscreteWrite(&gpio,GPIO_OUTPUT_BANK,0x0);
		sleep(1);
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
