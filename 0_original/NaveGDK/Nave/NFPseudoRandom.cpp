#include "NFPseudoRandom.h"
#include <time.h>

namespace Nave { namespace Private {

	NFMersenneTwister::NFMersenneTwister(BOOL useCurrentTime) 
	{
			if ( useCurrentTime) {
				time_t t; time(&t);
				SetSeed(unsigned long(t));
			} else {
				SetSeed(5489UL);  
			}
	}

	void NFMersenneTwister::GenerateState() { // generate new state vector
		for (int i = 0; i < (Constant1 - Constant2); ++i)
			state[i] = state[i + Constant2] ^ Twiddle(state[i], state[i + 1]);
		for (int i = Constant1 - Constant2; i < (Constant1 - 1); ++i)
			state[i] = state[i + Constant2 - Constant1] ^ Twiddle(state[i], state[i + 1]);
		state[Constant1 - 1] = state[Constant2 - 1] ^ Twiddle(state[Constant1 - 1], state[0]);
		position = 0; // reset position
	}

	void NFMersenneTwister::SetSeed(unsigned long s) {  // initialized by 32 bit seed

		for ( int i=0; i<Constant1; ++i) { state[i]= 0x0UL; }
		//state[Constant1] = {0x0UL};
		state[0] = s & 0xFFFFFFFFUL; // for > 32 bit machines
		for (int i = 1; i < Constant1; ++i) {
			state[i] = 1812433253UL * (state[i - 1] ^ (state[i - 1] >> 30)) + i;
			// see Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier
			// in the previous versions, MSBs of the seed affect only MSBs of the array state
			// 2002/01/09 modified by Makoto Matsumoto
			state[i] &= 0xFFFFFFFFUL; // for > 32 bit machines
		}
		position = Constant1; // force GenerateState() to be called for next random number
	}

	void NFMersenneTwister::SetSeed(const unsigned long* array, int size) { // initialized by array
		SetSeed(19650218UL);
		int i = 1, j = 0;
		for (int k = ((Constant1 > size) ? Constant1 : size); k; --k) {
			state[i] = (state[i] ^ ((state[i - 1] ^ (state[i - 1] >> 30)) * 1664525UL))
				+ array[j] + j; // non linear
			state[i] &= 0xFFFFFFFFUL; // for > 32 bit machines
			++j; j %= size;
			if ((++i) == Constant1) { state[0] = state[Constant1 - 1]; i = 1; }
		}
		for (int k = Constant1 - 1; k; --k) {
			state[i] = (state[i] ^ ((state[i - 1] ^ (state[i - 1] >> 30)) * 1566083941UL)) - i;
			state[i] &= 0xFFFFFFFFUL; // for > 32 bit machines
			if ((++i) == Constant1) { state[0] = state[Constant1 - 1]; i = 1; }
		}
		state[0] = 0x80000000UL; // MSB is 1; assuring non-zero initial array
		position = Constant1; // force GenerateState() to be called for next random number
	}

} } 