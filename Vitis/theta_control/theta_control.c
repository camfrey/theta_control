/***************************** Include Files *********************************/
#include "xparameters.h"
#include "xsysmon.h"
#include "xintc.h"
#include "xil_printf.h"
#include "phase_calc.h"
#include "xuartlite.h"
#include "xil_io.h"
#include "pwm_gen.h"
#include "xgpio.h"
#include "sleep.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"

/************************** Constant Definitions *****************************/
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are only defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define SYSMON_DEVICE_ID	XPAR_SYSMON_0_DEVICE_ID
#define GPIO_OUTPUT_BANK	2
#define GPIO_INPUT_BANK		1
#define COMMAND_BUFFER_SIZE 10
#define ENTER_ASCII_CODE 13
#define BACKSPACE_ASCII_CODE 8
#define ASCII_OFFSET 48

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int XADCInit(XSysMon* SysMonInstPtr, u16 SysMonDeviceId);
void printPrompt();
void bufferRefresh(char * RecvBuffer);
void getUserInput();
void updatePhase(double *phase_result, int *pwm_result, int pos_x_size,
		double *pos_x, double *desired_angle);
void displayInfo(double *phase_result, int *pwm_result, int pos_x_size,
		double *desired_angle);
void commandParser(double *phase_result, int *pwm_result, int pos_x_size,
		double *pos_x, double *desired_angle);
double stringToDouble(char * s);
void programPWM(int reg0, int reg1, int reg2, int reg3, int reg4, int reg5,
		int reg6, int reg7);
/************************** Variable Definitions *****************************/
static XSysMon SysMonInst; /* System Monitor driver instance */
static XGpio gpio;
XUartLite UartLite;
char RecvBuffer[COMMAND_BUFFER_SIZE];/* Buffer for Receiving Data on command line */
unsigned int ReceivedCount = 0; // recieve counter for the command line
const double phase_Min = -18.36089378; // Min phase angle of the left most element at (45 degrees)
const double phase_Max = 18.36089378; // Max phase angle of the right most element at (45 degrees)
const double pwm_Min = 0;
const double pwm_Max = pow(2, 11);
double phase_offset[] = { -2.111848395, 1.064650844, 1.361356817, -1.8675023,
		-0.523598776, -1.745329252, -1.396263402, -1.466076572 };
enum Mode {
	Display, New_Angle, Beam_Mode
};
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
//General Formatting
#define GEN_FORMAT_RESET                "0"
#define GEN_FORMAT_BRIGHT               "1"
#define GEN_FORMAT_DIM                  "2"
#define GEN_FORMAT_UNDERSCORE           "3"
#define GEN_FORMAT_BLINK                "4"
#define GEN_FORMAT_REVERSE              "5"
#define GEN_FORMAT_HIDDEN               "6"

//Foreground Colors
#define FOREGROUND_COL_BLACK            "30"
#define FOREGROUND_COL_RED              "31"
#define FOREGROUND_COL_GREEN            "32"
#define FOREGROUND_COL_YELLOW           "33"
#define FOREGROUND_COL_BLUE             "34"
#define FOREGROUND_COL_MAGENTA          "35"
#define FOREGROUND_COL_CYAN             "36"
#define FOREGROUND_COL_WHITE            "37"

//Background Colors
#define BACKGROUND_COL_BLACK            "40"
#define BACKGROUND_COL_RED              "41"
#define BACKGROUND_COL_GREEN            "42"
#define BACKGROUND_COL_YELLOW           "43"
#define BACKGROUND_COL_BLUE             "44"
#define BACKGROUND_COL_MAGENTA          "45"
#define BACKGROUND_COL_CYAN             "46"
#define BACKGROUND_COL_WHITE            "47"

#define SHELL_COLOR_ESCAPE_SEQ(X) "\x1b["X"m"
#define SHELL_FORMAT_RESET  "\x1b[0m"

int main(void) {

	int Status;
	double pos_x[] = { -0.0567, -0.0405, -0.0243, -0.0081, 0.0081, 0.0243,
			0.0405, 0.0567 }; // position of each array column (in mm)

	int pos_x_size = (sizeof pos_x / sizeof pos_x[0]); // size of position array

	double phase_result[pos_x_size]; // Array of the phase calculation for each element
	int pwm_result[pos_x_size]; // Array of the element values converted from phase to PWM
	double desired_angle = 0; // angle to steer the beam
	memset(phase_result, 0.0, sizeof phase_result); // initializing all elements in phase_result to 0
	Status = XADCInit(&SysMonInst, SYSMON_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Tmrctr PWM Example Failed\r\n");
		return XST_FAILURE;
	}

	Status = XUartLite_Initialize(&UartLite, XPAR_AXI_UARTLITE_0_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Uartlite polled Example Failed\r\n");
		return XST_FAILURE;
	}
	Status = XUartLite_SelfTest(&UartLite);
	if (Status != XST_SUCCESS) {
		xil_printf("Uartlite self test Failed\r\n");
		return XST_FAILURE;
	}

	Status = XGpio_Initialize(&gpio, XPAR_AXI_GPIO_0_DEVICE_ID);

	if (Status != XST_SUCCESS) {
		xil_printf("Tmrctr PWM Example Failed\r\n");
		return XST_FAILURE;
	}

	XGpio_SetDataDirection(&gpio, GPIO_INPUT_BANK, 0x1F);
	XGpio_SetDataDirection(&gpio, GPIO_OUTPUT_BANK, 0x0);

	printWelcome();
	updatePhase(phase_result, pwm_result, pos_x_size, pos_x, &desired_angle); // initially calculate the phase with angle 0

	while (1) {

		printPrompt(phase_result, pwm_result, pos_x_size, pos_x,
				&desired_angle);
//		XGpio_DiscreteWrite(&gpio, GPIO_OUTPUT_BANK, 0x1F);
//		sleep(1);
//		XGpio_DiscreteWrite(&gpio, GPIO_OUTPUT_BANK, 0x0);
//		sleep(1);

	}
	return XST_SUCCESS;
}

void printWelcome() {
/*
 * Terminal Splash Screen function
 */
	xil_printf(
			SHELL_COLOR_ESCAPE_SEQ(GEN_FORMAT_DIM";"FOREGROUND_COL_WHITE)"                                @@@@@@@@@@@@@@@@@@                              \r\n");
	xil_printf(
			"                         @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@                        \r\n");
	xil_printf(
			"                     @@@@@@@@@@                   @@@@@@@@@@                    \r\n");
	xil_printf(
			"                  @@@@@@@@@                           @@@@@@@@@                 \r\n");
	xil_printf(
			"                @@@@@@@@@                               @@@@@@@@@*              \r\n");
	xil_printf(
			"             @@@@@@@@@@                                   @@@@@@@@@@            \r\n");
	xil_printf(
			"           .@@@@@@@@@@                                     @@@@@@@@@@           \r\n");
	xil_printf(
			"          @@@@@@@@@@@                                       @@@@@@@@@@@         \r\n");
	xil_printf(
			"         @@@@@@@@@@@                                         @@@@@@@@@@@        \r\n");
	xil_printf(
			"        @@@@@@@@@@@                                          #@@@@@@@@@@@       \r\n");
	xil_printf(
			"       @@@@@@@@@@@@         "SHELL_COLOR_ESCAPE_SEQ(GEN_FORMAT_DIM";"FOREGROUND_COL_RED)"@@                     @@"SHELL_COLOR_ESCAPE_SEQ(GEN_FORMAT_DIM";"FOREGROUND_COL_WHITE)"         @@@@@@@@@@@&      \r\n");
	xil_printf(
			"       @@@@@@@@@@@          "SHELL_COLOR_ESCAPE_SEQ(GEN_FORMAT_DIM";"FOREGROUND_COL_RED)"@@@                   @@@"SHELL_COLOR_ESCAPE_SEQ(GEN_FORMAT_DIM";"FOREGROUND_COL_WHITE)"         @@@@@@@@@@@@      \r\n");
	xil_printf(
			"      &@@@@@@@@@@@          "SHELL_COLOR_ESCAPE_SEQ(GEN_FORMAT_DIM";"FOREGROUND_COL_RED)"@@@@@               @@@@@"SHELL_COLOR_ESCAPE_SEQ(GEN_FORMAT_DIM";"FOREGROUND_COL_WHITE)"          @@@@@@@@@@@.     \r\n");
	xil_printf(
			"      @@@@@@@@@@@@          "SHELL_COLOR_ESCAPE_SEQ(GEN_FORMAT_DIM";"FOREGROUND_COL_RED)"@@@@@@@@@@@@@@@@@@@@@@@@@"SHELL_COLOR_ESCAPE_SEQ(GEN_FORMAT_DIM";"FOREGROUND_COL_WHITE)"          @@@@@@@@@@@@     \r\n");
	xil_printf(
			"      @@@@@@@@@@@@          "SHELL_COLOR_ESCAPE_SEQ(GEN_FORMAT_DIM";"FOREGROUND_COL_RED)"@@@@@@@@@@@@@@@@@@@@@@@@@"SHELL_COLOR_ESCAPE_SEQ(GEN_FORMAT_DIM";"FOREGROUND_COL_WHITE)"          @@@@@@@@@@@@     \r\n");
	xil_printf(
			"      @@@@@@@@@@@@          "SHELL_COLOR_ESCAPE_SEQ(GEN_FORMAT_DIM";"FOREGROUND_COL_RED)"@@@@@@@@@@@@@@@@@@@@@@@@@"SHELL_COLOR_ESCAPE_SEQ(GEN_FORMAT_DIM";"FOREGROUND_COL_WHITE)"          @@@@@@@@@@@@     \r\n");
	xil_printf(
			"      @@@@@@@@@@@@          "SHELL_COLOR_ESCAPE_SEQ(GEN_FORMAT_DIM";"FOREGROUND_COL_RED)"@@@@@@@@@@@@@@@@@@@@@@@@@"SHELL_COLOR_ESCAPE_SEQ(GEN_FORMAT_DIM";"FOREGROUND_COL_WHITE)"          @@@@@@@@@@@*     \r\n");
	xil_printf(
			"       @@@@@@@@@@@          "SHELL_COLOR_ESCAPE_SEQ(GEN_FORMAT_DIM";"FOREGROUND_COL_RED)"@@@                   @@@"SHELL_COLOR_ESCAPE_SEQ(GEN_FORMAT_DIM";"FOREGROUND_COL_WHITE)"          @@@@@@@@@@@      \r\n");
	xil_printf(
			"       @@@@@@@@@@@@         "SHELL_COLOR_ESCAPE_SEQ(GEN_FORMAT_DIM";"FOREGROUND_COL_RED)"@@                     @@"SHELL_COLOR_ESCAPE_SEQ(GEN_FORMAT_DIM";"FOREGROUND_COL_WHITE)"         &@@@@@@@@@@@      \r\n");
	xil_printf(
			"        @@@@@@@@@@@         "SHELL_COLOR_ESCAPE_SEQ(GEN_FORMAT_DIM";"FOREGROUND_COL_RED)"@@                     @@"SHELL_COLOR_ESCAPE_SEQ(GEN_FORMAT_DIM";"FOREGROUND_COL_WHITE)"         @@@@@@@@@@@       \r\n");
	xil_printf(
			"         @@@@@@@@@@@                                         &@@@@@@@@@@        \r\n");
	xil_printf(
			"          @@@@@@@@@@&                                        @@@@@@@@@@         \r\n");
	xil_printf(
			"           @@@@@@@@@@*                                      @@@@@@@@@@          \r\n");
	xil_printf(
			"            #@@@@@@@@@@                                    @@@@@@@@@            \r\n");
	xil_printf(
			"              @@@@@@@@@@                                 @@@@@@@@@              \r\n");
	xil_printf(
			"                @@@@@@@@@@                             @@@@@@@@@                \r\n");
	xil_printf(
			"                   @@@@@@@@@@                       @@@@@@@@@                   \r\n");
	xil_printf(
			"                      @@@@@@@@@@@              *@@@@@@@@@@                      \r\n");
	xil_printf(
			"                           @@@@@@@@@@@@@@@@@@@@@@@@@@                           \r\n");
	xil_printf(
			"                                                                                                        \r\n\r\n\r\n");
	xil_printf(SHELL_COLOR_ESCAPE_SEQ(GEN_FORMAT_DIM";"FOREGROUND_COL_WHITE)
			"8888888 8888888888 8 8888        8 8 8888888888 8888888 8888888888"SHELL_COLOR_ESCAPE_SEQ(GEN_FORMAT_DIM";"FOREGROUND_COL_RED)"   .8."SHELL_COLOR_ESCAPE_SEQ(GEN_FORMAT_DIM";"FOREGROUND_COL_WHITE)"          \r\n");
	xil_printf(
			"      8 8888       8 8888        8 8 8888             8 8888"SHELL_COLOR_ESCAPE_SEQ(GEN_FORMAT_DIM";"FOREGROUND_COL_RED)"        .888."SHELL_COLOR_ESCAPE_SEQ(GEN_FORMAT_DIM";"FOREGROUND_COL_WHITE)"         \r\n");
	xil_printf(
			"      8 8888       8 8888        8 8 8888             8 8888"SHELL_COLOR_ESCAPE_SEQ(GEN_FORMAT_DIM";"FOREGROUND_COL_RED)"       :88888."SHELL_COLOR_ESCAPE_SEQ(GEN_FORMAT_DIM";"FOREGROUND_COL_WHITE)"        \r\n");
	xil_printf(
			"      8 8888       8 8888        8 8 8888             8 8888      ."SHELL_COLOR_ESCAPE_SEQ(GEN_FORMAT_DIM";"FOREGROUND_COL_RED)" `88888."SHELL_COLOR_ESCAPE_SEQ(GEN_FORMAT_DIM";"FOREGROUND_COL_WHITE)"       \r\n");
	xil_printf(
			"      8 8888       8 8888        8 8 888888888888     8 8888     .8."SHELL_COLOR_ESCAPE_SEQ(GEN_FORMAT_DIM";"FOREGROUND_COL_RED)" `88888."SHELL_COLOR_ESCAPE_SEQ(GEN_FORMAT_DIM";"FOREGROUND_COL_WHITE)"      \r\n");
	xil_printf(
			"      8 8888       8 8888        8 8 8888             8 8888    .8`8."SHELL_COLOR_ESCAPE_SEQ(GEN_FORMAT_DIM";"FOREGROUND_COL_RED)" `88888."SHELL_COLOR_ESCAPE_SEQ(GEN_FORMAT_DIM";"FOREGROUND_COL_WHITE)"     \r\n");
	xil_printf(
			"      8 8888       8 8888888888888 8 8888             8 8888   .8' `8."SHELL_COLOR_ESCAPE_SEQ(GEN_FORMAT_DIM";"FOREGROUND_COL_RED)" `88888."SHELL_COLOR_ESCAPE_SEQ(GEN_FORMAT_DIM";"FOREGROUND_COL_WHITE)"    \r\n");
	xil_printf(
			"      8 8888       8 8888        8 8 8888             8 8888  .8'   `8."SHELL_COLOR_ESCAPE_SEQ(GEN_FORMAT_DIM";"FOREGROUND_COL_RED)" `88888."SHELL_COLOR_ESCAPE_SEQ(GEN_FORMAT_DIM";"FOREGROUND_COL_WHITE)"   \r\n");
	xil_printf(
			"      8 8888       8 8888        8 8 8888             8 8888 .888888888."SHELL_COLOR_ESCAPE_SEQ(GEN_FORMAT_DIM";"FOREGROUND_COL_RED)" `88888."SHELL_COLOR_ESCAPE_SEQ(GEN_FORMAT_DIM";"FOREGROUND_COL_WHITE)"  \r\n");
	xil_printf(
			"      8 8888       8 8888        8 8 888888888888     8 8888.8'       `8."SHELL_COLOR_ESCAPE_SEQ(GEN_FORMAT_DIM";"FOREGROUND_COL_RED)" `88888."SHELL_COLOR_ESCAPE_SEQ(GEN_FORMAT_DIM";"FOREGROUND_COL_WHITE)" \r\n");
	xil_printf("\r\n");
	xil_printf(
			"=====================================================================================\r\n");
	xil_printf(
			"=====================================================================================\r\n\r\n");

}

void programPWM(int reg0, int reg1, int reg2, int reg3, int reg4, int reg5,
		int reg6, int reg7) {
	PWM_GEN_mWriteReg(XPAR_PWM_GEN_0_S_AXI_BASEADDR,
			PWM_GEN_S_AXI_SLV_REG0_OFFSET, reg0);
	PWM_GEN_mWriteReg(XPAR_PWM_GEN_0_S_AXI_BASEADDR,
			PWM_GEN_S_AXI_SLV_REG1_OFFSET, reg1);
	PWM_GEN_mWriteReg(XPAR_PWM_GEN_0_S_AXI_BASEADDR,
			PWM_GEN_S_AXI_SLV_REG2_OFFSET, reg2);
	PWM_GEN_mWriteReg(XPAR_PWM_GEN_0_S_AXI_BASEADDR,
			PWM_GEN_S_AXI_SLV_REG3_OFFSET, reg3);
	PWM_GEN_mWriteReg(XPAR_PWM_GEN_0_S_AXI_BASEADDR,
			PWM_GEN_S_AXI_SLV_REG4_OFFSET, reg4);
	PWM_GEN_mWriteReg(XPAR_PWM_GEN_0_S_AXI_BASEADDR,
			PWM_GEN_S_AXI_SLV_REG5_OFFSET, reg5);
	PWM_GEN_mWriteReg(XPAR_PWM_GEN_0_S_AXI_BASEADDR,
			PWM_GEN_S_AXI_SLV_REG6_OFFSET, reg6);
	PWM_GEN_mWriteReg(XPAR_PWM_GEN_0_S_AXI_BASEADDR,
			PWM_GEN_S_AXI_SLV_REG7_OFFSET, reg7);
}

void printPrompt(double *phase_result, int *pwm_result, int pos_x_size,
		double *pos_x, double *desired_angle) {
	/*
	 * Command line function that displays the interface and handles
	 * logic for user input
	 *
	 */
	xil_printf(SHELL_COLOR_ESCAPE_SEQ(GEN_FORMAT_DIM";"FOREGROUND_COL_YELLOW)"Current Modes: Display=0 New_Angle=1 Beam_Mode=2\r\n");
	xil_printf(SHELL_FORMAT_RESET"(WELAC) ==>");
	getUserInput();
	commandParser(phase_result, pwm_result, pos_x_size, pos_x, desired_angle);
	xil_printf(
			"__________________________________________________________________________\r\n");
}

void getUserInput() {
	/*
	 * Function to get user input and store data in the RecvBuffer Pointer
	 *
	 * Note: can check input with "xil_printf("Recieved: %s\r\n",RecvBuffer);"
	 */
	ReceivedCount = 0;
	bufferRefresh(RecvBuffer); // refresh the buffer before receiving data
	while (1) {
		ReceivedCount += XUartLite_Recv(&UartLite, RecvBuffer + ReceivedCount,
		COMMAND_BUFFER_SIZE - ReceivedCount);
		//xil_printf("%c",RecvBuffer[ReceivedCount-1]);

		if (ReceivedCount != 0) {
			if (RecvBuffer[ReceivedCount - 1] == BACKSPACE_ASCII_CODE) {

				RecvBuffer[ReceivedCount - 1] = '\0'; // erases the backspace

				if (ReceivedCount > 1) {
					RecvBuffer[ReceivedCount - 2] = '\0'; // erases previous character if exists
					ReceivedCount -= 1;
				}
				ReceivedCount -= 1;
			}
		}

		if (ReceivedCount == COMMAND_BUFFER_SIZE
				|| RecvBuffer[ReceivedCount - 1] == ENTER_ASCII_CODE) {
			xil_printf("\r\n"); // formatting
			//		xil_printf("RC: %d\r\n",ReceivedCount);
			break;
		}
	}
}

void commandParser(double *phase_result, int *pwm_result, int pos_x_size,
		double *pos_x, double *desired_angle) {
	/*
	 * Parses User input and calls the respective functions
	 *
	*/
	if (*RecvBuffer == (char) (Display + ASCII_OFFSET)) {
		xil_printf("Displaying Values: \r\n");
		displayInfo(phase_result, pwm_result, pos_x_size, desired_angle);
	} else if (*RecvBuffer == (char) (New_Angle + ASCII_OFFSET)) {
		xil_printf("Enter a new angle: ");
		getUserInput();
		*desired_angle = stringToDouble(RecvBuffer);
		updatePhase(phase_result, pwm_result, pos_x_size, pos_x, desired_angle);
		displayInfo(phase_result, pwm_result, pos_x_size, desired_angle);
	} else if (*RecvBuffer == (char) (Beam_Mode + ASCII_OFFSET)) {
		xil_printf("Beam_Mode Selected\r\n");
		beamMode(phase_result, pwm_result, pos_x_size, pos_x, desired_angle,5);
	}
}

void beamMode(double *phase_result, int *pwm_result, int pos_x_size,
		double *pos_x, double *desired_angle,int angle_interval) {
	/*
	 * Function that automatically updates and applies a gradual phase shift
	 */
	bufferRefresh(RecvBuffer); // refresh the buffer before receiving data
	xil_printf(SHELL_COLOR_ESCAPE_SEQ(GEN_FORMAT_DIM";"FOREGROUND_COL_CYAN)"Press Enter to Exit\r\n\r\n"SHELL_COLOR_ESCAPE_SEQ(GEN_FORMAT_DIM";"FOREGROUND_COL_WHITE));
	*desired_angle=0;
	while (1) {
			*desired_angle+=angle_interval;
			updatePhase(phase_result, pwm_result, pos_x_size, pos_x, desired_angle);
			xil_printf("Current Angle: ");
			print_double(*desired_angle);
			xil_printf("\r\n");
			ReceivedCount += XUartLite_Recv(&UartLite, RecvBuffer + ReceivedCount,
			COMMAND_BUFFER_SIZE - ReceivedCount);
			if (RecvBuffer[ReceivedCount - 1] == ENTER_ASCII_CODE) {
				xil_printf("\r\n"); // formatting
				break;
			}
			sleep(1);
		}

}

void displayInfo(double *phase_result, int *pwm_result, int pos_x_size,
		double *desired_angle) {
	xil_printf("Current Angle: ");
	print_double(*desired_angle);
	xil_printf("\r\n");
	for (int i = 0; i < pos_x_size; i++) { // debug to make sure phase calculations are being calculated and assigned correctly
		xil_printf("Phase element %d: ", i); //,(int)(phase_result[i]*100000));
		print_float(phase_result[i]);
		xil_printf("\r\n");
	}

	for (int i = 0; i < pos_x_size; i++) { // debug to make sure PWM calculations are being calculated and assigned correctly
		xil_printf("PWM element %d: %d\r\n", i, pwm_result[i]);
	}
}

void updatePhase(double *phase_result, int *pwm_result, int pos_x_size,
		double *pos_x, double *desired_angle) {
	calcPhase(pos_x, phase_result, pos_x_size, desired_angle, phase_offset); // calculate the phases for each array element

	for (int i = 0; i < pos_x_size; i++) { // Map phase caluculations to PWM range (0-2^11)
		pwm_result[i] = mapToPWM(phase_result[i]);
	}

	programPWM(pwm_result[0], pwm_result[1], pwm_result[2], pwm_result[3],
			pwm_result[4], pwm_result[5], pwm_result[6], pwm_result[7]);
}

void bufferRefresh(char * RecvBuffer) {
	/*
	 * Refreshes the Buffer before new data is written to it
	 *
	 * @param: Recieve buffer address
	 *
	 */
	for (int i = 0; i < COMMAND_BUFFER_SIZE; i++) {
		RecvBuffer[i] = '\0';
	}
}

double stringToDouble(char * s) {
	double result = 0;
	double fractal = 0;
	int decimal_index = strlen(s) - 1;
	int foundDecimal = 0;
	for (int i = 0; i < strlen(s); i++) {
		if (s[i] == '.') {
			//xil_printf("decimal found\r\n"); //debug
			decimal_index = i;
			foundDecimal = 1;
		}
	}
	result += atoi(s); // add the fixed numbers
	if (foundDecimal) {
		for (int i = decimal_index + 1, j = 1; i < strlen(s) - 1; i++) { // add up fractal parts
			fractal += (s[i] - 48) * pow(10, -(j++));
//			xil_printf("Fractal: %d: %c\r\n",fractal,s[i]); //debug
		}

	}
	result += fractal;
	return result;
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
int XADCInit(XSysMon* SysMonInstPtr, u16 SysMonDeviceId) {
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
	Status = XSysMon_SetSingleChParams(SysMonInstPtr, XSM_CH_AUX_MIN + 4,
	FALSE, FALSE, FALSE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XSysMon_SetAlarmEnables(SysMonInstPtr, 0x0);

	return XST_SUCCESS;
}
