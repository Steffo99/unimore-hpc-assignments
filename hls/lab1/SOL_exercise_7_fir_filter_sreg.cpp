#define SIZE 	128
#define N 		10

void fir(int * input, int * output) {

#pragma HLS INTERFACE m_axi port=input  offset=slave  bundle=input_mem
#pragma HLS INTERFACE m_axi port=output offset=slave bundle=output_mem

#pragma HLS INTERFACE s_axilite port=return bundle=params

	int coeff[N] 	 = {13, -2, 9, 11, 26, 18, 95, -43, 6, 74};
	int shift_reg[N] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	//TODO: insert array partition directive
	#pragma HLS ARRAY_PARTITION variable=shift_reg dim=0 complete

	loop_1: for (int n = 0; n < SIZE; n++) {
		//TODO: insert pipeline directive
		#pragma HLS PIPELINE

		int acc = 0;

		loop_2: for(int j = N-1; j > 0; j--) {
			//TODO: insert unroll directive
			#pragma HLS UNROLL
			shift_reg[j] = shift_reg[j-1];
		}

		shift_reg[0] = input[n];

		loop_3: for (int j= 0; j< N; j++ ) {
			//TODO: insert unroll directive
			#pragma HLS UNROLL
			acc += shift_reg[j]*coeff[j];
		}
		output[n] = acc;
	}
}
