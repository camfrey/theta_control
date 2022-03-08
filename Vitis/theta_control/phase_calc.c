#include "math.h"


// Declarations

const int  v_sound = 343; //Speed of sound in room temperature air
const float beam_freq = 25E3; // Transducer Resonant Frequency

const int lambda = v_sound / beam_freq;


double mapToPWM(double num, double in_min,double in_max,double out_min,double out_max){
	/*
	 * Mapping function to convert calculated phase angle into PWM width
	 */
	return (num - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


void print_float(float Input)
{
    /*
     * cast input and remove floating part
     */
    long int fix_part = abs((long int) Input);
    /*
     * remove integer part, multiply by 1000 to adjust to 3 decimal points then cast to integer
     */
    long int frac_part = (long int) (Input*10000.0 - fix_part*10000);
    if(frac_part<0){
    	frac_part=abs(frac_part);
        xil_printf("-%d", fix_part);
        xil_printf(".%d\r", frac_part);
    }else{
    xil_printf("%d", fix_part);
    xil_printf(".%d\r", frac_part);
    }
}

void print_double(double Input)
{
    long int fix_part = (long int) Input;
    long int frac_part = (long int) (Input*1000.0 - fix_part*1000);
    long int dif = (long int) (1000.0-frac_part);
    xil_printf("%d,", fix_part);
    if(dif>900)
    {
        xil_printf("0");
    }
    xil_printf("%d\r", frac_part);
}


double getPhase(double pos_x, double angle ){
/*
 * Calculates phase for one specific position
 *
 * @param: pos_x is the x position for one element
 * @param: angle is the desired angle to calculate the phase
 *
 * return: phase which is the calculated
 */

	float radians=(angle*M_PI)/180; // converting angle to radians
	if(radians<0){ // handle negative angles
	radians+=2*M_PI;
	}
	double phase=(double)(2*M_PI*pos_x*sin(radians))/(double)(343/25E3);
//	print_float(phase);
//	xil_printf(" - This is the element phase calculation\n");
	return phase;
}

void calcPhase(double *pos_arr,double *phase_arr,int arr_size,double angle){
	/*
	 * Calculates the phase for a set of the transducers
	 *
	 *@param: pos_arr is an array of the x positions of the transducer element
	 *@param: angle is the desired angle to calculate the phase
	 *@param: arr_size refers to the size of both pos_arr & phase_arr (They are the same size)
	 *
	 *@return: Nothing
	 */
	int pos_size = arr_size; // size of position array
	xil_printf("pos array size %d\nAngle set: %d degrees\n",pos_size,(int)angle);  // debug

	for(int i=0; i < pos_size; i++){
		phase_arr[i]=getPhase(pos_arr[i],angle);
	}
}


