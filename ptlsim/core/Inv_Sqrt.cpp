/*
 * Inv_Sqrt.cpp
 *
 *  Created on: Mar 5, 2010
 *      Author: ardavan
 */

#include "Inv_Sqrt.h"

Inv_Sqrt::Inv_Sqrt(){}


Inv_Sqrt::Inv_Sqrt(double *& Row_Buses_Write,double *& Row_Buses_Read,double *& Column_Buses_Write,double *& Column_Buses_Read)
{
	Latency=InvSqrt_Latency;// TODO Auto-generated constructor stub
	//Pipe_Regs_Curr=(double *) malloc ( sizeof(double)* Latency) ;
	//Pipe_Regs_Next=(double *) malloc ( sizeof(double)* Latency) ;
	
  Pipe_Regs_Curr= new double[Latency] ;
	Pipe_Regs_Next= new double[Latency] ;

	Read_Col_Buses= Column_Buses_Read;
	Read_Row_Buses= Row_Buses_Read;
	Write_Col_Buses=Row_Buses_Write;
	Write_Row_Buses=Column_Buses_Write;

}

Inv_Sqrt::~Inv_Sqrt()
{
	// TODO Auto-generated destructor stub
}

double Inv_Sqrt::Inv_Sqrt_Execute(double input){

	//Pipe_Regs_Next[0]= pow(input,- 0.5);
	//masri power compiled error
  int i;
	for (i=1; i< Latency; i++)
		Pipe_Regs_Next[i]=Pipe_Regs_Curr[i-1];


	return Pipe_Regs_Curr[Latency-1];
}


int Inv_Sqrt::Cycle(){

	int i;
	for (i=0; i< Latency; i++)
		Pipe_Regs_Curr[i]=Pipe_Regs_Next[i];

}


int Inv_Sqrt::Execute (int Global_index, int Iter_Counter, int Latency_Counter,
		LAPU_Function routine,int LAPU_Current_State, int State_Start){

		switch (routine){

		case LAPU_Cholesky:

			switch (LAPU_Current_State){
				case 2: //  Chol_Inv_Sqrt

				//	if (Latency_Counter< (FMA_Latency-1))
						Inv_Sqrt_Execute( Read_Row_Buses[Iter_Counter]);
				/*	else{

						Write_Col_Buses[Iter_Counter]=Inv_Sqrt_Execute( Read_Row_Buses[Iter_Counter]);

						// std::cout<<"BUZZZZZZZZZZZZZZZZZZZZZZZZZZ"<<std::endl;
						// std::cout<<Inv_Sqrt_Execute( Read_Row_Buses[Iter_Counter])<<std::endl;

						 std::cout <<"Write_Col_Buses[Iter_Counter]"<<Write_Col_Buses[Iter_Counter]<<std::endl;
						 Write_Row_Buses[Iter_Counter]=Write_Col_Buses[Iter_Counter];
					}
				*/
				case 3: //Chol_BC_Invqrt:

					Write_Col_Buses[Iter_Counter]=Pipe_Regs_Curr[Latency-1];
					Write_Row_Buses[Iter_Counter]=Write_Col_Buses[Iter_Counter];

				break;
			}
		break;

		default: ;


		}


}


int Inv_Sqrt::Dump_Inv_Sqrt_Regs(){



	std::cout<<"Inv_Sqrt:....."<<std::endl;

//	std::cout<<"Write_My_Col_Reg_Curr="<<Write_My_Col_Reg_Curr<<std::endl;
//	std::cout<<"Write_My_Row_Reg_Curr="<<Write_My_Row_Reg_Curr<<std::endl;

	//What I see
//	std::cout<<"Read_My_Col_Bus="<<*Read_My_Col_Bus<<std::endl;
//	std::cout<<"Read_My_Row_Bus="<<*Read_My_Row_Bus<<std::endl;

	int i;
	for (i=0; i< Latency; i++)
			std::cout<<"Pipe_Regs_Curr["<<i<<"]="<<Pipe_Regs_Curr[i]<<std::endl;






}




