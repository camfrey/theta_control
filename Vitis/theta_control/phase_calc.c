#include "math.h"

// Declarations

const int  v_sound = 343; //Speed of sound in room temperature air
const int beam_freq = 25E3; // Transducer Resonant Frequency

const int lambda = v_sound / beam_freq;

float getPhase(float pos_x, float angle ){
/*
 * Calculates phase for one specific position
 *
 * @param: pos_x is the x position for one element
 * @param: angle is the desired angle to calculate the phase
 *
 * return: phase which is the calculated
 */
	float phase=(2*M_PI*pos_x*sin(angle))/lambda;

	return phase;
}

float * calcPhase(float pos_arr[], float angle){
	/*
	 * Calculates the phase for a set of the transducers
	 *
	 *@param: pos_arr is an array of the x positions of the transducer element
	 *@param: angle is the desired angle to calculate the phase
	 *
	 *@return: phase_arr which is an array of phases for each x position
	 */
	int pos_size = sizeof *pos_arr; // size of position array
	printf("array size %d",pos_size);  // debug
	float phase_arr[pos_size];
	for(int i=0; i < pos_size; i++){
		phase_arr[i]=getPhase(pos_arr[i],angle);
	}
	return phase_arr;
}



