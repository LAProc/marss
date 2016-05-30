/*
 * FMA.cpp
 *
 *  Created on: Mar 4, 2010
 *      Author: ardavan
 */

#include "FMA.h"
#include <math.h>
//#include "Parameters.h"
//using namespace std;
FMA::FMA()
{
	// TODO Auto-generated constructor stub
	MAD_Latency=FMA_Latency-1;
	Mul_Latency= Multiplication_Latency-1;
	Add_Latency=Addition_Latency-1;

	MAD_Pipe_Regs_Curr= new double[MAD_Latency];
	MAD_Pipe_Regs_Next= new double[MAD_Latency];

	MAC_Pipe_Regs_Curr= new double[MAD_Latency];
	MAC_Pipe_Regs_Next= new double [MAD_Latency];


	Mul_Pipe_Regs_Curr= new double[Mul_Latency];
	Mul_Pipe_Regs_Next= new double[Mul_Latency];

	Add_Pipe_Regs_Curr= new double [Add_Latency];
	Add_Pipe_Regs_Next= new double [Add_Latency];



	Power_Consumed=0;
}

FMA::~FMA(){
	// TODO Auto-generated destructor stub
}


double FMA::Execute_MAD(double A, double B, double C){

	Power_Consumed++;

	MAD_Pipe_Regs_Next[0]= A*B+C ;
	int i;
	for (i=1; i< MAD_Latency; i++)
		MAD_Pipe_Regs_Next[i]=MAD_Pipe_Regs_Curr[i-1];

	return MAD_Pipe_Regs_Curr[MAD_Latency-1];
}

double FMA::Execute_Mul(double A, double B){

	Power_Consumed++;

	Mul_Pipe_Regs_Next[0]= A*B ;
	int i;
	for (i=1; i< Mul_Latency; i++)
		Mul_Pipe_Regs_Next[i]=Mul_Pipe_Regs_Curr[i-1];

	return Mul_Pipe_Regs_Curr[Mul_Latency-1];
}

double FMA::Execute_Add(double A, double B, int add_sub){

	Power_Consumed++;
	Add_Pipe_Regs_Next[0]= A + pow (-1, add_sub) *B ;
	int i;
	for (i=1; i< Add_Latency; i++)
		Add_Pipe_Regs_Next[i]=Add_Pipe_Regs_Curr[i-1];

	return Mul_Pipe_Regs_Curr[Add_Latency-1];
}




int FMA::Cycle(){

	int i;

	for (i=0; i < MAD_Latency; i++){
		MAD_Pipe_Regs_Curr[i]=MAD_Pipe_Regs_Next[i];
		MAC_Pipe_Regs_Curr[i]=MAC_Pipe_Regs_Next[i];
	}
	for (i=0; i < Mul_Latency; i++)
		Mul_Pipe_Regs_Curr[i]=Mul_Pipe_Regs_Next[i];

	for (i=0; i < Add_Latency; i++)
		Add_Pipe_Regs_Curr[i]=Add_Pipe_Regs_Next[i];

	Accumulator_Curr=Accumulator_Next;
}



double FMA::Execute_MAC(double A, double B){

	Power_Consumed++;

/*
	if ((A*B)!=0){
		Accumulator_Next= Accumulator_Curr+A+.01*B;
		MAC_Pipe_Regs_Next[0]=A*B+Accumulator_Curr;
	}
	else {
		Accumulator_Next= Accumulator_Curr+A*B;
		MAC_Pipe_Regs_Next[0]=A*B+Accumulator_Curr ;
	}
*/
#ifdef SUBSTRACT
  Accumulator_Next= Accumulator_Curr + (-1*A*B);
  MAC_Pipe_Regs_Next[0]= Accumulator_Curr +(-1*A*B);
#else
  Accumulator_Next= Accumulator_Curr+A*B;
  MAC_Pipe_Regs_Next[0]=A*B+Accumulator_Curr;
#endif

  int i, j;
	for (i=1; i< MAD_Latency; i++)
		MAC_Pipe_Regs_Next[i]=MAC_Pipe_Regs_Curr[i-1];

	return MAC_Pipe_Regs_Curr[MAD_Latency-1];



}

void FMA::Flush_Accumulator(){

	for (int i=0; i < MAD_Latency; i++){
		MAC_Pipe_Regs_Curr[i]=0;
    MAC_Pipe_Regs_Next[i]=0;
	}


}

void FMA::Load_Accumulator(double data){

	Accumulator_Next=data;


}

double FMA::Return_ACC(){
	return Accumulator_Curr;
}


int FMA::Return_FMA_Power_Consumed(){


	return Power_Consumed;

}

int FMA::Dump_Pipeline( ALU_op operation_type){

	int i;
	switch (operation_type){

		case ALU_Add:
			std::cout<<"ADD_Pipe"<<std::endl;
			for (i=0; i < Add_Latency; i++)
				std::cout<< "Pipe["<<i<<"]="<<Add_Pipe_Regs_Curr[i]<<std::endl;


		break;

		case ALU_Mul:

			std::cout<<"Mul_Pipe"<<std::endl;
			for (i=0; i < Mul_Latency; i++)
				std::cout<< "Pipe["<<i<<"]="<<Mul_Pipe_Regs_Curr[i]<<std::endl;

		break;


		case ALU_MAD:

			std::cout<<"MAD_Pipe"<<std::endl;
			for (i=0; i < MAD_Latency; i++)
				std::cout<< "Pipe["<<i<<"]="<<MAD_Pipe_Regs_Curr[i]<<std::endl;


		break;

		case ALU_MAC:

			std::cout<<"MAC_Pipe"<<std::endl;

			for (i=0; i < MAD_Latency; i++)
				std::cout<< "Pipe["<<i<<"]="<<MAC_Pipe_Regs_Curr[i]<<std::endl;


		break;

	}

}



