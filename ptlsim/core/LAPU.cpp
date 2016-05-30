/*
 * LAPU.cpp
 *
 *  Created on: Mar 4, 2010
 *      Author: ardavan
 */

#include "LAPU.h"
//#include "Multicore.h"
//#include "data_structure.h"


//void Clear_SRAM_C();



LAPU::LAPU(int ID, int *&Current_PORT, int *&Next_PORT, int *&Arbit_Current, int *&Arbit_Next,  LAP_Package &Package, LS_Buffer &Buf, LAP_PREF_Sync *& lap_pref )
//LAPU::LAPU(int ID, , LAP_Package *&Package)
{
	// TODO Auto-generated constructor stub

  FETCH_A_AMOUNT = ((K_V*K_H)/(LAPU_Size*LAPU_Size));

  Kernel_V = K_V;
  Kernel_H = KC;
  
  NO_PART = NON_PART;

  Comm_Go_C_Again = TRUE;
  from_Pref = lap_pref;
  req_package = &Package;
  MyCoreID = ID;

  PORT_Now = Current_PORT;
  ARBIT_Now = Arbit_Current;

  Matmul_Comp_Comm_Current = Matmul_Executing;
  Matmul_Comp_Comm_Next = Matmul_Executing;

  Matmul_Current_State= Matmul_Idle;
  Matmul_Next_State = Matmul_Idle;

	Cycles_Passed=0;
	Counter_Curr=0;
	Counter_Next=1;

	Mc_Counter_Curr=0;
	Mc_Counter_Next=0;

	Kc_Counter_Curr=0;
	Kc_Counter_Next=0;

	N_Counter_Curr=0;
	N_Counter_Next=0;

  Mc_Comm_Cin_Current=0;

  Last_Sending = -2; //considering bus delay
  done = false;

  first_cin=TRUE;
  first_cout= TRUE;
  //Last_Sending = 0;

  Enough_A = TRUE;
  Enough_B = TRUE;
  
	State_Start= FALSE;

	Size=LAPU_Size;

  Fetch_Done = NONE;
    
  Req_Matrix = NONE;  // dont forget to put it in the class declaration
  Fetch_Done = NONE;
  Buffer_Ready = NONE;
  IgetPort = FALSE;
  Mc_Comm_Cin_Next = 0;
  Mc_Comm_B = 0;
  Mc_Comm_A = 0;
  Send_Done = FALSE;
  EnoughB =TRUE;
  EnoughA =TRUE;
  Stall = FALSE;

  Row_Buses_Read= new double [LAPU_Size];
	Column_Buses_Read= new double [LAPU_Size];
	Row_Buses_Write= new double [LAPU_Size];
	Column_Buses_Write= new double [LAPU_Size];

  Stall=TRUE;
	
  

  PE_Array =new PE *[LAPU_Size];
	int i,j;
	for ( i=0; i<Size; i++){
		PE_Array[i]= new PE [LAPU_Size];
		for (j=0; j<Size; j++)
			PE_Array[i][j]= *new PE(i,j, &Row_Buses_Write[i],&Row_Buses_Read[i],
										&Column_Buses_Write[j],&Column_Buses_Read[j], ID);
	}
  //req_addresses = (int *) malloc(sizeof(int) * LAPU_SIZE);
#if 0
  Row_Buses_Read= (double *) malloc (sizeof (double) * LAPU_Size);
	Column_Buses_Read= (double *) malloc (sizeof (double) * LAPU_Size);
	Row_Buses_Write= (double *) malloc (sizeof (double) * LAPU_Size);
	Column_Buses_Write= (double *) malloc (sizeof (double) * LAPU_Size);

	PE_Array =(PE **)  malloc ( sizeof (PE *) * (LAPU_Size));
	int i,j;
	for ( i=0; i<Size; i++){
		PE_Array[i]=(PE *)  malloc ( sizeof (PE ) * (LAPU_Size));
		for (j=0; j<Size; j++)
			PE_Array[i][j]= *new PE(i,j, &Row_Buses_Write[i],&Row_Buses_Read[i],
										&Column_Buses_Write[j],&Column_Buses_Read[j]);
	}
#endif
	 Sqrt_Unit= new Inv_Sqrt(Row_Buses_Write, Row_Buses_Read, Column_Buses_Write, Column_Buses_Read);
		std::cout<< "in lapu const"<<std::endl;

  //Construct IO
	Mem_IF= new IO(Row_Buses_Write, Row_Buses_Read, Column_Buses_Write, Column_Buses_Read);

 /* newMEM_IF= new newIO(Row_Buses_Write, Row_Buses_Read, Column_Buses_Write, Column_Buses_Read);*/
  
  newMEM_IF= new newIO(Row_Buses_Write, Row_Buses_Read, Column_Buses_Write, Column_Buses_Read, Current_PORT, Next_PORT, Arbit_Current, Arbit_Next, MyCoreID, Buf, Kc_Counter_Curr, Stall);

	LAPU_Power_Consumed=0;
}



LAPU::~LAPU()
{
	// TODO Auto-generated destructor stub
}

void LAPU::Reset(){
  
  FETCH_A_AMOUNT = ((K_V*K_H)/(LAPU_Size*LAPU_Size));
  NO_PART = NON_PART;

  Matmul_Comp_Comm_Current = Matmul_Executing;
  Matmul_Comp_Comm_Next = Matmul_Executing;

  Matmul_Current_State= Matmul_Idle;
  Matmul_Next_State = Matmul_Idle;

	Cycles_Passed=0;
	
  Counter_Curr=0;
	Counter_Next=1;

	Latency_Counter_Curr=0;
	Latency_Counter_Next=0;
	
  Mc_Counter_Curr=0;
	Mc_Counter_Next=0;

	Kc_Counter_Curr=0;
	Kc_Counter_Next=0;

	N_Counter_Curr=0;
	N_Counter_Next=0;

  Ma_Counter_Curr = 0;
  Ma_Counter_Next = 0;

  address_x = 0;
  address_y = 0;
  Fetch_Done = NONE;
  IO_Ready = FALSE;

  Last_Sending = -2; //considering bus delay
  done = false;

  
  first_cin=TRUE;
  first_cout= TRUE;
  //Last_Sending = 0;
  Req_Matrix = NONE;  // dont forget to put it in the class declaration
  Buffer_Ready = NONE;
  IgetPort = FALSE;

  Mc_Comm_Cin_Current=0;
  Mc_Comm_Cin_Next=0;
  
  Mc_Comm_Cout_Current=0;
  Mc_Comm_Cout_Next=0;
 
  Mc_Comm_B = 0;
  Mc_Comm_A = 0;
  Send_Done = FALSE;
  EnoughB =TRUE;
  EnoughA =TRUE;

  Ma_Comm_Cin = 0;
  Ma_Comm_Cout = 0;

	
  InitB_Next=0;
  InitB_Current=0;
  
  InitA_Next=0;
  InitA_Current=0;
  
  BigC_Current=0;
  BigC_Next=0;
  
  BigA_Current=0;
  BigA_Next=0;

  BigB_Current=0;
  BigB_Next=0;
 
  Mc_Fetch_A  = 0;
  Ma_Fetch_A  = 0;
  N_Fetch_A   = 0;
  FULL_A      = FALSE;
  FULL_B      = FALSE;

  
  N_Comm_B    = 0;
  N_Comm_A    = 0;
  selector    = 0;
  
  counter = 0;
  ff      = 0;   //for fast forward

  fetchedA = FALSE;
  fetchedB = FALSE;

  State_Start= FALSE;
  stall_A = FALSE;
  stall_B = FALSE;
  stall_C = FALSE;
  stall_C_inter = FALSE;

  Issue_Request_LS = FALSE;

  N_Comm_Cin = 0;
  N_Comm_Bin = 0;
  N_Comm_Cout= 0;
  WE = FALSE;
  packed_req = 0;
  Chunk_B = 0;
  Chunk_A = 0;
  x_comm_cin = 0;
  x_comm_cout = 0;
  Comm_Go_C_Again = TRUE;

  b_counter   = 0;
  y_a_counter = 0;
  x_b_counter = 0;

  counter_A = 0;
  counter_B = 0;
  counter_Cin = 0;
  comp_count = 0;
  inter_done = 0;

  Stall_Init = 0;
  Stall_Comm = 0;
  Cin_Intersection = 0;
  DONE = 0;

  Fetch_Done = NONE;
    
  Req_Matrix = NONE;  // dont forget to put it in the class declaration
  Buffer_Ready = NONE;
  IgetPort = FALSE;

  Stall=TRUE;
	
  Kernel_V = K_V;
  Kernel_H = KC;
  NO_PART = NON_PART;

#if 1  
  for (i=0;i<Size;i++)
		for (j=0;j<Size;j++){
			PE_Array[i][j].Reset();
		}
#endif
	Matmul_Comm_Next = Comm_Init;
  init_fetch_next=Init_Wait_Ready;

}

int LAPU::Return_Cycle_Count(){
	return Cycles_Passed;
}
int LAPU::Initialize_Mem( double ** Input_matrix, int row_number, int column_number, int offset){

	for (i=0;i<Size;i++)
		for (j=0;j<Size;j++){
			PE_Array[i][j].Intialize_Local_Mem(Input_matrix,row_number,column_number, offset);

		}


}

int LAPU::Initialize_Mem_New( double ** Input_matrix, int row_number, int column_number, int offset, char matr){

	for (i=0;i<Size;i++)
		for (j=0;j<Size;j++){
			PE_Array[i][j].Initialize_Local_Mem_New(Input_matrix,row_number,column_number, offset, matr);

		}
}

int LAPU::Flush_Mem( double **& Input_matrix, int row_number, int column_number,int offset){

	for (i=0;i<Size;i++)
		for (j=0;j<Size;j++){
			PE_Array[i][j].Flush_Local_Mem(Input_matrix,row_number,column_number, offset);

		}
}

int LAPU::Flush_Mem_New( double **& Input_matrix, int row_number, int column_number,int offset, char matr){

	for (i=0;i<Size;i++)
		for (j=0;j<Size;j++){
			PE_Array[i][j].Flush_Local_Mem_New(Input_matrix,row_number,column_number, offset, matr);

		}
}


int LAPU::Assign_input_Matrix( double **& matrix_A, double **& matrix_B, double **& matrix_C){

	Matrix_A=matrix_A;
	Matrix_B=matrix_B;
	Matrix_C=matrix_C;

	Mem_IF->Assign_input_Matrix(matrix_A, matrix_B, matrix_C);
  newMEM_IF->Assign_input_Matrix(matrix_A, matrix_B, matrix_C);
}

int LAPU::PrintStall(){


  std::cout << "Stall A is " << stall_A <<std::endl;
  std::cout << "Stall B is " << stall_B <<std::endl;
  std::cout << "Stall C is " << stall_C <<std::endl;

}



int LAPU::Return_LAPU_Power(){


	for (i=0;i<Size;i++)
		for (j=0;j<Size;j++){
			LAPU_Power_Consumed+=PE_Array[i][j].Return_PE_Power();
		}

	return LAPU_Power_Consumed;

}


int LAPU::Dump_Row_Buses(){

	std::cout<<"Row_Bus_Write  And ROw Bus Read :"<<std::endl;

	for (i=0; i <LAPU_Size;i++){
		std::cout<<"-"<<i<<":"<< Row_Buses_Write[i]<<"  ";
		std::cout<<""<< Row_Buses_Read[i]<<std::endl;
	}
	std::cout<<"****"<<std::endl;

}


int LAPU::Dump_Column_Buses(){

	std::cout<< "Column Bus Write"<<std::endl;
	std::cout<< "Column Bus Read"<<std::endl;

	for (i=0; i <LAPU_Size;i++)
		std::cout<<i<<"| : ";
	std::cout<<std::endl;

	for (i=0; i <LAPU_Size;i++)
			std::cout<<Column_Buses_Write[i]<<" , ";
	std::cout<<std::endl;
	for (i=0; i <LAPU_Size;i++)
		std::cout<<Column_Buses_Read[i]<<" , ";

	std::cout<<std::endl;


	std::cout<<"****"<<std::endl;


}


int LAPU::Dump_Trsm_SMachine(){


	std::cout<<"Iteration :"<< Counter_Curr<<std::endl;
	std::cout<<" \t State:"<<std::endl;
	switch (Trsm_Current_State){

		case Trsm_Initial:

			std::cout<<"Trsm_Initial"<<std::endl;
		break;


		case Trsm_BC_InvSqrt:

			std::cout<<"Trsm_BC_InvSqrt"<<std::endl;
		break;

		case Trsm_Multiply:
			std::cout<<"Trsm_Multiply"<<std::endl;
		break;

		case Trsm_BC_Mult:
			std::cout<<"Trsm_BC_Mult"<<std::endl;
		break;

		case Trsm_Partial_Rank_1:
			std::cout<<"Trsm_Partial_Rank_1"<<std::endl;
		break;

		case Trsm_End:
			std::cout<<"Trsm_End"<<std::endl;
		break;

	}
	std::cout<<"Latency_Count ="<<Latency_Counter_Curr<<std::endl;

	switch (Trsm_Next_State){

		case Trsm_Initial:

			std::cout<<"Trsm_Initial"<<std::endl;
		break;


		case Trsm_BC_InvSqrt:

			std::cout<<"Trsm_BC_InvSqrt"<<std::endl;
		break;

		case Trsm_Multiply:
			std::cout<<"Trsm_Multiply"<<std::endl;
		break;

		case Trsm_BC_Mult:
			std::cout<<"Trsm_BC_Mult"<<std::endl;
		break;

		case Trsm_Partial_Rank_1:
			std::cout<<"Trsm_Partial_Rank_1"<<std::endl;
		break;

		case Trsm_End:
			std::cout<<"Trsm_End"<<std::endl;
		break;

	}


	std::cout<<std::endl;


}



int LAPU::Dump_Chol_SMachine(){
	std::cout<<"Iteration :"<< Counter_Curr<<std::endl;
	std::cout<<"State:"<<std::endl;
	switch (Chol_Current_State){

		case Chol_Initial:
			std::cout<<"Chol_Initial"<<std::endl;
		break;


		case Chol_Feed_Sqrt:
			std::cout<<" Chol_Feed_Sqrt"<<std::endl;

		break;

		case Chol_Inv_Sqrt:
			std::cout<<" Chol_Inv_Sqrt"<<std::endl;

		break;

		case Chol_BC_InvSqrt:
			std::cout<<" Chol_BC_InvSqrt"<<std::endl;

		break;

		case Chol_Multiply:
			std::cout<<" Chol_Multiply"<<std::endl;

		break;

		case Chol_BC_Mul:
			std::cout<<" Chol_BC_Mul"<<std::endl;

		break;

		case Chol_Rank1_Update:
			std::cout<<" Chol_Rank1_Update"<<std::endl;

		break;

		case Chol_End:
			std::cout<<" Chol_End"<<std::endl;

		break;


	}
	std::cout<<"Latency_Count ="<<Latency_Counter_Curr<<std::endl;

	/*
	switch (Chol_Next_State){

		case Chol_Initial:
			std::cout<<" Chol_Initial"<<std::endl;
		break;


		case Chol_Feed_Sqrt:
			std::cout<<"Nextt_State=Chol_Feed_Sqrt"<<std::endl;

		break;

		case Chol_Inv_Sqrt:
			std::cout<<"Nextt_State=Chol_Inv_Sqrt"<<std::endl;

		break;

		case Chol_BC_InvSqrt:
			std::cout<<"Nextt_State=Chol_BC_InvSqrt"<<std::endl;

		break;

		case Chol_Multiply:
			std::cout<<" Chol_Multiply"<<std::endl;

		break;

		case Chol_BC_Mul:
			std::cout<<" Chol_BC_Mul"<<std::endl;

		break;

		case Chol_Rank1_Update:
			std::cout<<"Chol_Rank1_Update"<<std::endl;

		break;

		case Chol_End:
			std::cout<<" Chol_End"<<std::endl;

		break;



	}

*/


	std::cout<<std::endl;

}


int LAPU::Dump_Gemm_SMachine(){

	std::cout<<"Iteration :"<< Counter_Curr<<std::endl;
	std::cout<<"State:"<<std::endl;
	switch (Gemm_Current_State){

		case Gemm_Initial:
			std::cout<<" Gemm_Initial"<<std::endl;
		break;


		case Gemm_Pre_Fetch:
			std::cout<<" Gemm_Pre_Fetch"<<std::endl;

		break;

		case Gemm_BC:
			std::cout<<" Gemm_BC"<<std::endl;

		break;

		case Gemm_MAC_BC:
			std::cout<<" Gemm_MAC_BC"<<std::endl;

		break;

		case Gemm_MAC:
			std::cout<<" Gemm_MAC"<<std::endl;

		break;

		case Gemm_End:
			std::cout<<" Gemm_End"<<std::endl;

		break;


	}

	std::cout<<"Latency_Count ="<<Latency_Counter_Curr<<std::endl;
	switch (Gemm_Next_State){

		case Gemm_Initial:
			std::cout<<" Gemm_Initial"<<std::endl;
		break;


		case Gemm_Pre_Fetch:
			std::cout<<" Gemm_Pre_Fetch"<<std::endl;

		break;

		case Gemm_BC:
			std::cout<<" Gemm_BC"<<std::endl;

		break;

		case Gemm_MAC_BC:
			std::cout<<" Gemm_MAC_BC"<<std::endl;

		break;

		case Gemm_MAC:
			std::cout<<" Gemm_MAC"<<std::endl;

		break;

		case Gemm_End:
			std::cout<<" Gemm_End"<<std::endl;

		break;

	}




	//std::cout<<std::endl;

}


int LAPU::Dump_Matmul_SMachine(){


#if 1
  printf("==============================\n");
  printf("Cycle %lu\n", Cycles_Passed);
  printf("==============================\n");
  printf("MyCoreID is %d\n", MyCoreID);
  printf("Ma : %d\n", Ma_Counter_Curr);
  printf("BigA : %d\n", BigA_Current);
  printf("BigC : %d\n", BigC_Current);
  printf("Cin Ready : %d\n", from_Pref->Cin[MyCoreID]);
  printf("B Ready %d\n", from_Pref->B[MyCoreID]);
  printf("A Ready %d\n", from_Pref->A[MyCoreID]);

  printf("Ma_Comm_Cin : %d\n", Ma_Comm_Cin);
  printf("N_Comm_Cin : %d\n", N_Comm_Cin);
  printf("x_comm_cin : %d\n", x_comm_cin);

  printf("Ma_Comm_Cout : %d\n", Ma_Comm_Cout);
  printf("N_Comm_Cout : %d\n", N_Comm_Cout);
  printf("x_comm_cout : %d\n", x_comm_cout);

  printf("N : %d\n", N_Counter_Curr);
  printf("Mc : %d\n", Mc_Counter_Curr);
  printf("Kc : %d\n", Kc_Counter_Curr);

  printf("Mc_Comm_Cin : %d\n", Mc_Comm_Cin_Current);
  printf("Mc_Comm_Cout : %d\n", Mc_Comm_Cout_Current);
  
  printf("Mc_Comm_B : %d\n", Mc_Comm_B);
  printf("N_Comm_Bin : %d\n", N_Comm_Bin);

  printf("Mc_Fetch_A : %d\n", Mc_Fetch_A);
  printf("N_Fetch_A : %d\n", N_Fetch_A);
  printf("Ma_Fetch_A : %d\n", Ma_Fetch_A);

  printf("Mc_Comm_A : %d\n", Mc_Comm_A);
  printf("N_Comm_A : %d\n", N_Comm_A);

  printf("EnoughA : %d\n", EnoughA);
  printf("FULL_A : %d\n", FULL_A);
  printf("EnoughB : %d\n", EnoughB);
  printf("FULL_B : %d\n", FULL_B);

  printf("\n");
  printf("*********STALL***********\n");
  printf("stall_A : %lu\n", stall_A);
  printf("stall_B : %lu\n", stall_B);
  printf("stall_C : %lu\n", stall_C);
  printf("stall_C_inter : %lu\n", stall_C_inter);
  printf("stall_Init : %lu\n", Stall_Init);

#ifdef HALFWORD
  int last_cout = (((Panel_Size*Panel_Size)/(Port_Bandwidth/Element_Size))*2) + DRAM_Latency; 
#else
  int last_cout = ((Panel_Size*Panel_Size)/(Port_Bandwidth/Element_Size)) + DRAM_Latency; 
#endif

  //printf("stall_last_cout %d\n", last_cout);
  
  int total_stalls = stall_A + stall_B + stall_C + stall_C_inter + Stall_Init;
  
  printf("total stalls : %lu\n", total_stalls);
  printf("Cycle : %lu\n", Cycles_Passed);
  
  double util = (double)(Cycles_Passed-total_stalls)/(double)(Cycles_Passed);

  printf("Util : %lf\n", util);
  //printf("Computation Cycles : %d\n", Cycles_Passed - total_stalls);

  printf("*********STALL***********\n");
  printf("\n");
#endif

#if 0
	std::cout<<"==============================";
	std::cout<<"Cycle "<<Cycles_Passed<<std::endl; 
	std::cout<<"==============================";
  std::cout<<"MyCoreID is " << MyCoreID << std::endl;
	std::cout<<"Ma :"<< Ma_Counter_Curr<<std::endl;
	std::cout<<"BigA :"<< BigA_Current<<std::endl;
	std::cout<<"BigC :"<< BigC_Current<<std::endl;
	std::cout<<"Cin Ready :"<< from_Pref->Cin[MyCoreID]<<std::endl;
	std::cout<<"B Ready :"<< from_Pref->B[MyCoreID]<<std::endl;
	std::cout<<"A Ready :"<< from_Pref->A[MyCoreID]<<std::endl;
	std::cout<<"N :"<< N_Counter_Curr<<std::endl;
	std::cout<<"Mc"<<Mc_Counter_Curr<<std::endl;
	std::cout<<"Kc"<<Kc_Counter_Curr<<std::endl;

#if 1  
	std::cout<<"Ma_Comm_Cin :"<< Ma_Comm_Cin<<std::endl;
	std::cout<<"N_Comm_Cin :"<< N_Comm_Cin<<std::endl;
	std::cout<<"x_comm_cin :"<< x_comm_cin<<std::endl;
	std::cout<<"Ma_Comm_Cout :"<< Ma_Comm_Cout<<std::endl;
	std::cout<<"N_Comm_Cout :"<< N_Comm_Cout<<std::endl;
	std::cout<<"x_comm_cout :"<< x_comm_cout<<std::endl;

	std::cout<<"Mc_Comm_Cin "<<Mc_Comm_Cin_Current<<std::endl;
	std::cout<<"Mc_Comm_Cout "<<Mc_Comm_Cout_Current<<std::endl;
	
  std::cout<<"Mc_Comm_B "<<Mc_Comm_B<<std::endl;
	std::cout<<"N_Comm_Bin "<<N_Comm_Bin<<std::endl;
	
  std::cout<<"Mc_Fetch_A "<<Mc_Fetch_A<<std::endl;
	std::cout<<"N_Fetch_A "<<N_Fetch_A<<std::endl;
  std::cout<<"Ma_Fetch_A "<<Ma_Fetch_A<<std::endl;
  
  std::cout<<"Mc_Comm_A "<<Mc_Comm_A<<std::endl;
	std::cout<<"N_Comm_A "<<N_Comm_A<<std::endl;

  std::cout<<"EnoughA "<<EnoughA<<std::endl;
	std::cout<<"FULL_A "<<FULL_A<<std::endl;
	std::cout<<"EnoughB "<<EnoughB<<std::endl;
	std::cout<<"FULL_B "<<FULL_B<<std::endl;
	//std::cout<<"init_fetch_current " << init_fetch_current<<std::endl;
  
  std::cout<<std::endl;  
  std::cout<< "*******STALL********"<<std::endl;
  std::cout<<"stall_A "<<stall_A<<std::endl;
	std::cout<<"stall_B "<<stall_B<<std::endl; 	
	std::cout<<"stall_C "<<stall_C<<std::endl; 	
	std::cout<<"stall_C_inter "<<stall_C_inter<<std::endl; 	
	std::cout<<"stall_Init "<<Stall_Init<<std::endl;
#ifdef HALFWORD
  int last_cout = (((Panel_Size*Panel_Size)/(Port_Bandwidth/Element_Size))*2) + DRAM_Latency; 
#else
  int last_cout = ((Panel_Size*Panel_Size)/(Port_Bandwidth/Element_Size)) + DRAM_Latency; 
#endif

  //std::cout<<"stall_last_cout "<<last_cout<<std::endl;
  
  int total_stalls = stall_A + stall_B + stall_C + stall_C_inter + Stall_Init;
	
  std::cout<<"total stalls "<<total_stalls<<std::endl;
	std::cout<<"Cycle "<<Cycles_Passed<<std::endl;
	std::cout<<"Computation Cycles "<<Cycles_Passed - total_stalls <<std::endl;

  /*double util = double ((Cycles_Passed - total_stalls) / Cycles_Passed);
	
  std::cout<<"Utilization "<< util<<std::endl;*/

#endif
  
  std::cout<< "*******STALL********"<<std::endl;
  std::cout<<std::endl;  


  switch (Matmul_Current_State){

		case Matmul_Init:
			std::cout<<" Matmul_Init"<<std::endl;
		break;

		case Matmul_Flush_Cout:
			std::cout<<" Matmul_Flush_Cout"<<std::endl;
		break;

		case Matmul_FetchB:
			std::cout<<" Matmul_FetchB"<<std::endl;

		break;

		case Matmul_FetchA:
			std::cout<<" Matmul_FetchA"<<std::endl;

		break;

		case Matmul_BC0:
			std::cout<<" Matmul_BC0"<<std::endl;

		break;

		case Matmul_BC:
			std::cout<<" Matmul_BC"<<std::endl;

		break;

		case Matmul_MAC_BC:
			std::cout<<" Matmul_MAC_BC"<<std::endl;

		break;

		case Matmul_MAC_Flush:
			std::cout<<" Matmul_MAC_FLush"<<std::endl;

		break;

		case Matmul_End:
			std::cout<<" Matmul_End"<<std::endl;

		break;


	}

	//std::cout<<"Latency_Count ="<<Latency_Counter_Curr<<std::endl;


	switch (Matmul_Next_State){

		case Matmul_Init:
			std::cout<<" Matmul_Init"<<std::endl;
		break;


		case Matmul_FetchB:
			std::cout<<" Matmul_FetchB"<<std::endl;

		break;

		case Matmul_FetchA:
			std::cout<<" Matmul_FetchA"<<std::endl;

		break;

		case Matmul_BC0:
			std::cout<<" Matmul_BC0"<<std::endl;

		break;

		case Matmul_BC:
			std::cout<<" Matmul_BC"<<std::endl;

		break;

		case Matmul_MAC_BC:
			std::cout<<" Matmul_MAC_BC"<<std::endl;

		break;
		case Matmul_MAC_Flush:
			std::cout<<" Matmul_MAC_FLush"<<std::endl;

		break;

		case Matmul_End:
			std::cout<<" Matmul_End"<<std::endl;

		break;

	}

	switch (Matmul_Comm_Current){

		case Comm_Init:
			std::cout<<" Comm_Init"<<std::endl;
		break;


		case Comm_Get_Port:
			std::cout<<" Comm_Get_Port"<<std::endl;

		break;

		case Comm_Request_Cin:
			std::cout<<" Comm_Request_Cin"<<std::endl;

		break;

		case Comm_Fetch_Cin:
			std::cout<<" Comm_Fetch_Cin"<<std::endl;

		break;

		case Comm_Send_Cout:
			std::cout<<" Comm_Send_Cout"<<std::endl;
    break;

		case Comm_Cout_Delay:
			std::cout<<" Comm_Cout_Delay"<<std::endl;
		
    break;

		case Comm_Fetch_B:
			std::cout<<" Comm_Fetch_B"<<std::endl;

		break;
		case Comm_Fetch_A:
			std::cout<<" Comm_Fetch_A"<<std::endl;

		break;

		case Comm_Request_Cout:
			std::cout<<" Comm_Request_Cout"<<std::endl;
		break;

    case Comm_Evaluate:
			std::cout<<" Comm_Evaluate"<<std::endl;
		break;

    case Comm_Request_B:
			std::cout<<" Comm_Request_B"<<std::endl;
		break;
    
    case Comm_Request_A:
			std::cout<<" Comm_Request_A"<<std::endl;
		break;
    
    case Wait_Before_Fetch_Cin:
			std::cout<<" Wait_Before_Fetch_Cin"<<std::endl;
		break;
    
    case Wait_Before_Fetch_B:
			std::cout<<" Wait_Before_Fetch_B"<<std::endl;
		break;
    
    case Wait_Before_Fetch_A:
			std::cout<<" Wait_Before_Fetch_A"<<std::endl;
		break;

	}


	std::cout<<std::endl;

	switch (Matmul_Comm_Next){

		case Comm_Init:
			std::cout<<" Comm_Init"<<std::endl;
		break;


		case Comm_Get_Port:
			std::cout<<" Comm_Get_Port"<<std::endl;

		break;

		case Comm_Request_Cin:
			std::cout<<" Comm_Request_Cin"<<std::endl;

		break;

		case Comm_Fetch_Cin:
			std::cout<<" Comm_Fetch_Cin"<<std::endl;

		break;

		case Comm_Cout_Delay:
			std::cout<<" Comm_Cout_Delay"<<std::endl;
	  break;

    case Comm_Send_Cout:
			std::cout<<" Comm_Send_Cout"<<std::endl;

		break;

		case Comm_Fetch_B:
			std::cout<<" Comm_Fetch_B"<<std::endl;

		break;
		case Comm_Fetch_A:
			std::cout<<" Comm_Fetch_A"<<std::endl;

		break;

		case Comm_Request_Cout:
			std::cout<<" Comm_Request_Cout"<<std::endl;
		break;

    case Comm_Evaluate:
			std::cout<<" Comm_Evaluate"<<std::endl;
		break;

    case Comm_Request_B:
			std::cout<<" Comm_Request_B"<<std::endl;
		break;
    
    case Comm_Request_A:
			std::cout<<" Comm_Request_A"<<std::endl;
		break;

	}
#endif

  std::cout<<std::endl;
}




int LAPU::Dump_PE_Registers(int row, int column){


 PE_Array[row][column].Dump_Regs();

}


int LAPU::Dump_All_PE_Registers(){


	for (i=0;i<LAPU_Size;i++)
		for (j=0;j<LAPU_Size;j++)
			 PE_Array[i][j].Dump_Regs();

}



int LAPU::Dump_PE_ALU(int row, int column, ALU_op operation_type){

	 PE_Array[row][column].Dump_ALU_Pipeline( operation_type);
}

int LAPU::Dump_All_PE_ALUs(ALU_op operation_type){

	for (i=0;i<LAPU_Size;i++)
		for (j=0;j<LAPU_Size;j++)
			 PE_Array[i][j].Dump_ALU_Pipeline( operation_type);
}

int LAPU::Dump_Sqrt_Unit(){


	Sqrt_Unit->Dump_Inv_Sqrt_Regs();

}

int LAPU::print_matrix(int rows, int columns, double ** matrix_A){

	int i,j;
  std::cout.precision(2);

	for (i=0;i<rows;i++){
		for (j=0;j<columns;j++){
			std::cout<< std::fixed<< std::left  <<matrix_A[i][j]<<" , ";
      if ((i+1)%(rows/howmanyA)==0 && j==columns-1) {
        std::cout<<std::endl;
        std::cout << "********************end of a matrix********************" << std::endl;
      }
		}
		std::cout<<std::endl;

    if (i%4==3) std::cout << std::endl;
	}


}


int LAPU::Cycle(){



//	int i,j;
	for (i=0;i<Size;i++)
		for (j=0;j<Size;j++){
			PE_Array[i][j].Cycle();
		}
	Sqrt_Unit->Cycle();

	Counter_Curr=Counter_Next;
	Latency_Counter_Curr=Latency_Counter_Next;

	Mc_Counter_Curr=Mc_Counter_Next;

	Kc_Counter_Curr=Kc_Counter_Next;

	N_Counter_Curr=N_Counter_Next;

  Ma_Counter_Curr = Ma_Counter_Next;

	Chol_Current_State=Chol_Next_State;
	Trsm_Current_State=Trsm_Next_State;
	Gemm_Current_State=Gemm_Next_State;
	Matmul_Current_State=Matmul_Next_State;

  Matmul_Comm_Current = Matmul_Comm_Next;
  Matmul_Comp_Comm_Current = Matmul_Comp_Comm_Next;

  init_fetch_current = init_fetch_next;
  InitA_Current = InitA_Next;
  InitB_Current = InitB_Next;
 
  Mc_Comm_Cin_Current = Mc_Comm_Cin_Next;
  Mc_Comm_Cout_Current = Mc_Comm_Cout_Next;

  BigC_Current = BigC_Next;
  
  BigA_Current = BigA_Next;

  BigB_Current = BigB_Next;

	Cycles_Passed++;
}

int LAPU::Drive_Buses(){

	for (i=0; i<LAPU_Size ; i++){
		Row_Buses_Read[i]=Row_Buses_Write[i];
		Column_Buses_Read[i]=Column_Buses_Write[i];

	}

	return 0;
}

int LAPU::Cholesky(int Global_index){



	while (1){


		switch (Chol_Current_State) {// TODO comment

			case Chol_Initial: // Read from Local Memory put it on the bus
				Chol_Next_State= Chol_Feed_Sqrt;
				Latency_Counter_Next=0;
				Latency_Counter_Curr=0;
				Counter_Curr=0;
				Counter_Next=0;
				State_Start=FALSE;


			break;

			case Chol_Feed_Sqrt: // transfer the data to Sqrt Unit  //TODO now I assume it is one clock latency
				Chol_Next_State=Chol_Inv_Sqrt;




				Latency_Counter_Next=0;
				Latency_Counter_Curr=0;
				State_Start=FALSE;

			break;

			case Chol_Inv_Sqrt: // start Sqrt

				if (Latency_Counter_Curr== 0){

					State_Start=TRUE;
					// Give the Data to the Sqrt unit
					//	PE_Array[Counter_Curr][Counter_Curr].Gen_Address(Global_index, Counter_Curr);
						//more to do maybe;

					}
				// I have to drive bus before calling this
					//InvSqrt.Inv_Sqrt_Execute(.5);
					//TODO here do I invoke Inv Sqrt all the time ? (I do but I do not invoke it other times) (Sqrt himself knows that he is sleep :D)
				Latency_Counter_Next=Latency_Counter_Curr+1;

				if (Latency_Counter_Curr < (InvSqrt_Latency -1)) {
					State_Start=FALSE;
					Chol_Next_State= Chol_Inv_Sqrt;
				}
				else {
					State_Start=FALSE;

					Chol_Next_State= Chol_BC_InvSqrt;
					Latency_Counter_Next=0;
				}

			break;

			case Chol_BC_InvSqrt: //TODO:for now BC delay is one Cycle

				//std::cout<<Chol_BC_InvSqrt<<std::endl;
				// Some signals should be sent to PE[Counter_Curr][Counter_Curr]
				// Still depends on the broad_cast delay if we loop back to this state or not

				//Send Counter_Curr and Chol_BC to all PEs

				Chol_Next_State= Chol_Multiply;
				Latency_Counter_Next=0;

			break;



			case Chol_Multiply:

				// Serious question: do I call Mul each cycle or
				//just once in the beginning and let the functions return after   delay cycle?
				// this decision should be uniform for Rank-K updates too

				//: the answer to previous question . I do not call PE.MUl I just have an Execute function for PEs.
				// All PEs are informed what State LAPU is in and how much latency has passed and all valuable info (See Execute arguments)
				// In each clock Execute is called at the end of this state machine code below here. PEs produce their signals and decide if they are dead or alive
				//based on the arguments that they get.
				// LAPU state machine plus its latency counter  has cycle accurate info that passes to PEs and they work with this info and produce needed control signals



				if (Latency_Counter_Curr==0){

					State_Start=TRUE;
					/*for (i=0;i<Size;i++)
						for (j=0;j<Size;j++){
						//	PE_Array[i][j].Multiply();
						}
					*/

				}
				Latency_Counter_Next=Latency_Counter_Curr+1;

				if (Latency_Counter_Curr < (Multiplication_Latency-1)){
					State_Start=FALSE;
					Chol_Next_State=Chol_Multiply;

				}
				else{

					if (Counter_Curr< (LAPU_Size-1)){  //TODO: Maybe (LAPU_Size -1)
						State_Start=FALSE;
						Chol_Next_State= Chol_BC_Mul;
//						Counter_Next=Counter_Curr+1;
						Latency_Counter_Next=0;
					}
					else Chol_Next_State= Chol_End; // start is already False

				}

			break;


			case Chol_BC_Mul:

				//Again Some of the PEs are dead and some others are alive here
				//Do I selectively call their functions or just set signals
				//and call all PEs uniformly out of the state machine ?
				// Answer: I set signals (I do not even set any signal I give general info to them (See PE.Execute Arguments)
								//and call all PEs uniformly out of the state machine ?

				Chol_Next_State= Chol_Rank1_Update;
				Latency_Counter_Next=0;

			break;

			case Chol_Rank1_Update:

				//Same scenario here;
				if (Latency_Counter_Curr==0){

					State_Start=TRUE; //maybe I even can get rid of this since I have the latency counter passed to PEs
									 // for readability I will keep it though

					/*for (i=0;i<Size;i++)
							for (j=0;j<Size;j++){
								//PE_Array[i][j].Rank_Update(); // start or continue ?
								//: I just say when is the start :D and I call everybody all the times
							}
						*/
				}

				Latency_Counter_Next=Latency_Counter_Curr +1;

				if (Latency_Counter_Curr< (FMA_Latency-1)){
						State_Start=FALSE;
						Chol_Next_State=Chol_Rank1_Update;
				}
				else { // in this case we are certainly in the first 3 iterations so we directly  go to Feed_Sqrt

							State_Start=FALSE;
							Chol_Next_State= Chol_Feed_Sqrt;
							Counter_Next=Counter_Curr+1;
							Latency_Counter_Next=0;


				}

			break;

			case Chol_End:
				Counter_Next=0;
				Counter_Curr=0;
			//	return Cycles_Passed;


		}  //switch-case


		// DO I Call Functions in the state machine or I just set signals and then Here I pass them to Live PEs?
		//: Answer: Everybody is alive here below
		if (Print_State_Machines==1){
			std::cout<<"==============================";
			std::cout<<"Cycle"<<Cycles_Passed<<std::endl;

			Dump_Chol_SMachine();
		}

		for (i=0;i<Size;i++)
			for (j=0;j<Size;j++){
				PE_Array[i][j].Execute(Global_index, 0, Counter_Curr, Latency_Counter_Curr,
											LAPU_Cholesky, Chol_Current_State, State_Start );  // We pass this Routine an the current state of this routine to PE
			}

		Sqrt_Unit->Execute(Global_index, Counter_Curr, Latency_Counter_Curr,
				LAPU_Cholesky, Chol_Current_State, State_Start );


		 if (Chol_Current_State==Chol_End){
			 Chol_Current_State=Chol_Initial;
			 Chol_Next_State=Chol_Initial;
			 return Cycles_Passed;
		 }

		//Dump_Row_Buses();
		//Dump_Column_Buses();

	//	if ( (Chol_Current_State==Chol_Inv_Sqrt) || (Chol_Current_State==Chol_BC_InvSqrt))
	//		Sqrt_Unit->Dump_Inv_Sqrt_Regs();
		Drive_Buses(); // Can it be a part of Cycle function? I remember I just seperated it for readability
		// It does not matter which one comes first Drive_Bus or Cycle;

		Cycle();


			// Drive Bus

	}






}




int LAPU::Trsm(int Global_index, int Trsm_index){

	while (1){

		switch (Trsm_Current_State){


			case Trsm_Initial:

				Trsm_Next_State=Trsm_Multiply;
				Latency_Counter_Next=0;
				Latency_Counter_Curr=0;
				State_Start=FALSE;
				Counter_Curr=0;
				Counter_Next=0;

			break;


			case Trsm_Multiply:

				if (Latency_Counter_Curr== 0){

						State_Start=TRUE;

				}

				Latency_Counter_Next=Latency_Counter_Curr+1;

				if (Latency_Counter_Curr < (Multiplication_Latency-1)) {
					State_Start=FALSE;
					Trsm_Next_State= Trsm_Multiply;

				}
				else {

					State_Start=FALSE;
					Trsm_Next_State= Trsm_BC_Mult;
					Latency_Counter_Next=0;
/*
					if (Counter_Curr< (LAPU_Size-1)){  //TODO: Maybe (LAPU_Size -1)
						State_Start=FALSE;
						Trsm_Next_State= Trsm_BC_Mult;
						Latency_Counter_Next=0;
					}
					else Trsm_Next_State= Trsm_End; // start is already False
*/
				}

			break;


			case Trsm_BC_Mult:

				if (Counter_Curr< (LAPU_Size-1))
					Trsm_Next_State= Trsm_Partial_Rank_1;
				else
					Trsm_Next_State=Trsm_Trans;

				Latency_Counter_Next=0;



			break;


			case Trsm_Partial_Rank_1:

				if (Latency_Counter_Curr==0){

					State_Start=TRUE; //maybe I even can get rid of this since I have the latency counter passed to PEs
									 // for readability I will keep it though

					/*for (i=0;i<Size;i++)
							for (j=0;j<Size;j++){
								//PE_Array[i][j].Rank_Update(); // start or continue ?
								//: I just say when is the start :D and I call everybody all the times
							}
						*/
				}

				Latency_Counter_Next=Latency_Counter_Curr +1;

				if (Latency_Counter_Curr< (FMA_Latency-1)){
						State_Start=FALSE;
						Trsm_Next_State=Trsm_Partial_Rank_1;
				}
				else { // in this case we are certainly in the first 3 iterations so we directly  go to Feed_Sqrt

							State_Start=FALSE;
							Trsm_Next_State= Trsm_Multiply;
							Counter_Next=Counter_Curr+1;
							Latency_Counter_Next=0;


				}




			break;


			case Trsm_Trans:

				Latency_Counter_Next=Latency_Counter_Curr +1;

				if (Latency_Counter_Curr==0)
					Trsm_Next_State=Trsm_Trans;
				else{
					Trsm_Next_State=Trsm_End;
					Latency_Counter_Next=0;
				}

			break;


			case Trsm_End:
				Counter_Next=0;
				Counter_Curr=0;

				//return Cycles_Passed;

			break;



		}

		if (Print_State_Machines==1){
			std::cout<<"==============================";
			std::cout<<"Cycle"<<Cycles_Passed<<std::endl;
		//	char test2;
		//	std::cout<<"Press Enter"<<std::endl;
		//	cin>>test2;
			Dump_Trsm_SMachine();
		}

		for (i=0;i<Size;i++)
			for (j=0;j<Size;j++){
				PE_Array[i][j].Execute(Global_index, Trsm_index, Counter_Curr, Latency_Counter_Curr,
											LAPU_Trsm, Trsm_Current_State, State_Start );  // We pass this Routine an the current state of this routine to PE
			}

		Sqrt_Unit->Execute(Global_index, Counter_Curr, Latency_Counter_Curr,
				LAPU_Trsm, Trsm_Current_State, State_Start );
	//	Dump_Row_Buses();
	//	Dump_Column_Buses();

		if (Trsm_Current_State==Trsm_End){
				 Trsm_Current_State=Trsm_Initial;
				 Trsm_Next_State=Trsm_Initial;
				 return Cycles_Passed;
			 }

		Drive_Buses(); // Can it be a part of Cycle function? I remember I just seperated it for readability
		// It does not matter which one comes first Drive_Bus or Cycle;

		Cycle();



	}


}




int LAPU::Rank_D_Update(int Global_index, int Trsm_index){


	while (1){

		switch (Gemm_Current_State){

			case Gemm_Initial:



				Gemm_Next_State=Gemm_Pre_Fetch;
				Latency_Counter_Next=0;
				Latency_Counter_Curr=0;
				State_Start=TRUE;


			break;

			case Gemm_Pre_Fetch:

				Latency_Counter_Next=Latency_Counter_Curr +1;
				if (Latency_Counter_Curr==0){
					Gemm_Next_State=Gemm_Pre_Fetch;
					State_Start=TRUE;
				}
				else{
					Gemm_Next_State=Gemm_BC;
					State_Start=FALSE;
					Latency_Counter_Next=0;
				}



			break;

			case Gemm_BC:


				Gemm_Next_State= Gemm_MAC_BC;
				Latency_Counter_Next=0;
				Counter_Next=Counter_Curr+1;

			break;

			case Gemm_MAC_BC:




				if (Latency_Counter_Curr==0){

					State_Start=TRUE;

				}

				//Latency_Counter_Next=Latency_Counter_Curr +1;



				//if (Latency_Counter_Curr< (FMA_Latency-1)){
				//		State_Start=FALSE;
				//		Gemm_Next_State=Gemm_MAC_BC;
				//}
				//else {

				//	Latency_Counter_Next=0;
					Counter_Next=Counter_Curr+1;
					if (Counter_Curr <  ( ( (Global_index)* LAPU_Size ) -1) ){
						Gemm_Next_State=Gemm_MAC_BC;

					}
					else{
						Gemm_Next_State=Gemm_MAC;
						Counter_Next=0;

					}

			//	}


			break;

			case Gemm_MAC:

				Latency_Counter_Next=Latency_Counter_Curr+1;

				if (Latency_Counter_Curr< (FMA_Latency-1))
					Gemm_Next_State=Gemm_MAC;
				else{
					Gemm_Next_State=Gemm_End;
					Latency_Counter_Next=0;
				}
			break;


			case Gemm_End:

				Counter_Next=0;
				Counter_Curr=0;


			break;


		}

		if (Print_State_Machines==1){
			std::cout<<"==============================";
			std::cout<<"Cycle"<<Cycles_Passed<<std::endl;
		//	char test2;
		//	std::cout<<"Press Enter"<<std::endl;
		//	cin>>test2;
			Dump_Gemm_SMachine();
		}
		for (i=0;i<Size;i++)
			for (j=0;j<Size;j++){
				PE_Array[i][j].Execute(Global_index, Trsm_index, Counter_Curr, Latency_Counter_Curr,
						LAPU_Rank_Update, Gemm_Current_State, State_Start );  // We pass this Routine an the current state of this routine to PE
			}

		Sqrt_Unit->Execute(Global_index, Counter_Curr, Latency_Counter_Curr,
				LAPU_Rank_Update, Gemm_Current_State, State_Start ); // TODO fix the input;

		 if (Gemm_Current_State==Gemm_End){
			 Gemm_Current_State=Gemm_Initial;
			 Gemm_Next_State=Gemm_Initial;
			 return Cycles_Passed;
		 }
	//	Dump_Row_Buses();
	//	Dump_Column_Buses();

		Drive_Buses(); // Can it be a part of Cycle function? I remember I just seperated it for readability
		// It does not matter which one comes first Drive_Bus or Cycle;

		Cycle();


	}


}



/*

int LAPU::Matmul_Rank_D(int Global_index){


	while (1){

		switch(MatMul_Current_State){

//		case







		}








	}

}


*/


void LAPU::Matmul_Comm(){

  if ((N_Counter_Curr != Panel_Size-LAPU_Size) && (Mc_Counter_Curr == Kernel_V-LAPU_Size) && Kc_Counter_Curr==Kernel_H-3 && Mc_Comm_B<Kernel_H){
    if(FULL_B)
      EnoughB = TRUE;
    else 
      EnoughB = FALSE;
  }
  /*if (Kc_Counter_Curr==Kernel_Size-3 && Mc_Comm_B==0  && EnoughB==1)
      EnoughB = TRUE;*/
  
  if ((N_Counter_Curr == Panel_Size-LAPU_Size) && (Mc_Counter_Curr==Kernel_V-LAPU_Size )&& Kc_Counter_Curr==Kernel_H-LAPU_Size-3 && Mc_Comm_B<Kernel_H) //always calculate this
  {
    if(FULL_B)
      EnoughB = TRUE;
    else 
      EnoughB = FALSE;
  }

  if ((N_Counter_Curr == Panel_Size-LAPU_Size) && (Mc_Counter_Curr==Kernel_V-LAPU_Size )&& Kc_Counter_Curr==Kernel_H-LAPU_Size-2 && Mc_Comm_A<FETCH_A_AMOUNT/*Mc_Comm_A<Mem_Size_A*/ && howmanyA>0) //always calculate this
  {
    if (FULL_A)
      EnoughA = TRUE;
    else 
      EnoughA = FALSE;
  }

  if (Mc_Counter_Curr == 0 && Mc_Comm_Cin_Current == Kernel_V){
    Mc_Comm_Cin_Next = 0; //bug
    Mc_Comm_Cin_Current=0;
  }

  if (Mc_Counter_Curr == 0 && Mc_Comm_Cout_Current == Kernel_V){
    Mc_Comm_Cout_Next= 0;
    Mc_Comm_Cout_Current=0;
  }
  if(Kc_Counter_Curr==0){ 
    if (fetchedB) fetchedB = 0;
    if (fetchedA) fetchedA = 0;
  }

  if(Kc_Counter_Curr==1 && Mc_Counter_Curr==0 && (N_Counter_Curr!=0 
        ||(!N_Counter_Curr && (Ma_Counter_Curr!=0 || Ma_Counter_Curr==0 && BigA_Current!=0)))){ 
    FULL_B = FALSE;
  }
  
  if(Kc_Counter_Curr==1 && Mc_Counter_Curr==0 && N_Counter_Curr==0 && (Ma_Counter_Curr!=0 || Ma_Counter_Curr==0 && BigA_Current!=0) ){ 
    FULL_A = FALSE;
  }

  //when the next chunk of B is not ready
  if ((Ma_Counter_Curr==A_per_Core-1 && N_Counter_Curr == Panel_Size-LAPU_Size) && (Mc_Counter_Curr==0)&& Kc_Counter_Curr==0 && (BigC_Current!=NumofPartition-1 || (BigC_Current==NumofPartition-1 && BigA_Current!=HowManyPanel-1 )) ){ //always calculate this
    if (from_Pref->B[MyCoreID]==FALSE && !NO_PART){
      Stall = TRUE;
      Stall_Comm =TRUE;
      Matmul_Comm_Current = Wait_for_Pref;
      Matmul_Comm_Next = Wait_for_Pref;
#ifdef PRINT_DEBUG
      std::cout << " B_Stall :is not ready"<<std::endl;
#endif
      stall_B++;
    }
    else {
      Stall = FALSE;
      Stall_Comm = FALSE;
      from_Pref->B[MyCoreID] = FALSE;
      Matmul_Comm_Next = Comm_Evaluate;
    }
  }
  
  if (((Ma_Counter_Curr!=0 || (Ma_Counter_Curr==0 && BigA_Current!=0)) && N_Counter_Curr == 0) && (Mc_Counter_Curr==0)&& Kc_Counter_Curr==0 && !inter_done && !(Ma_Counter_Curr==A_per_Core-1 && BigA_Current==HowManyPanel-1 && BigC_Current==NumofPartition-1) && BigC_Current!=NumofPartition){ //always calculate this

    /*std::cout << " from_Pref->A in Comm is "<<from_Pref->A[MyCoreID] <<std::endl;
    std::cout << " BigA_Current is "<<BigA_Current <<std::endl;*/
    
    if (from_Pref->A[MyCoreID]==FALSE){
      Stall = TRUE;
      Stall_Comm =TRUE;
#ifdef PRINT_DEBUG
      std::cout << " A_Stall :is not ready"<<std::endl;
#endif
      Matmul_Comm_Current = Wait_for_Pref;
      Matmul_Comm_Next = Wait_for_Pref;
      stall_A++;
    }
    else {
      Stall = FALSE;
      Stall_Comm = FALSE;
      //from_Pref->A[MyCoreID] = FALSE;
      if(Ma_Counter_Curr==0 && BigC_Current!=0 && BigA_Current==HowManyPanel)
        Matmul_Comm_Next = Comm_Wait_to_Flush_C;
      else
        Matmul_Comm_Next = Comm_Evaluate;
    }
  }
  
  /*if (Ma_Counter_Curr!=0 && Kc_Counter_Curr==0 && Mc_Counter_Curr==0
      && N_Counter_Curr==0)
      FULL_A = FALSE;*/
 
  switch (Matmul_Comm_Current){

    /****************** Initial State***************/
    case Comm_Init :
    
    if(Matmul_Current_State==Matmul_BC)
      Matmul_Comm_Next = Comm_Get_Port;
    else Matmul_Comm_Next = Matmul_Comm_Current;

    break;
    /**********************************************/

    /****************** GetPort State **************/
    case Comm_Get_Port :

    //IgetPort = (*ARBIT_Now==MyCoreID && newMEM_IF->GetPort());
    
    //We assume SRAM has many ports here
    IgetPort = newMEM_IF->GetPort();


    if(IgetPort)  // check memory port usage
      Matmul_Comm_Next = Comm_Request_Cin;

    else Matmul_Comm_Next = Matmul_Comm_Current;
    break;
    /***********************************************/

    /****************** Request_Cin State **************/
    case Comm_Request_Cin :
    
    Req_Matrix = CIN;
    Issue_Request_LS = TRUE;
    
    //activate issue request
  // if (req_counter < LAPU_Size) issue_reqeust = TRUE;
   

    if (req_counter==LAPU_Size-1)
      Matmul_Comm_Next = Wait_Before_Fetch_Cin;
      
    /*if (Mc_Counter_Curr==0 && N_Counter_Curr==0 && Ma_Counter_Curr==0){
      if(Mc_Comm_Cin_Current==0){
        first_cin=TRUE;
        first_cout= TRUE;
      }
    }*/
    

    //for the last iteration
    if(Ma_Counter_Curr==A_per_Core-1 && BigA_Current==HowManyPanel-1 && Mc_Comm_Cin_Current == Kernel_V-LAPU_Size && N_Counter_Curr==Panel_Size-LAPU_Size && Mc_Counter_Curr==Kernel_V-LAPU_Size){
    
      Req_Matrix = NONE;
      Issue_Request_LS = FALSE;
      Matmul_Comm_Next = Comm_Request_Cout;
      Mc_Comm_Cin_Next = (Mc_Comm_Cin_Current+LAPU_Size);
      
      //bug
      x_comm_cin =0;
      N_Comm_Cin =0;
      Ma_Comm_Cin=0;
    
    }
  
   /* Buffer_Ready = newMEM_IF-> Check_Buffer_Ready(Req_Matrix);

    if(Buffer_Ready) { // check if buffer is ready
      Matmul_Comm_Next = Comm_Fetch_Cin;
    }
    else Matmul_Comm_Next = Matmul_Comm_Current;*/
    
    break;
    /***********************************************/

    /******************* Wait for Cin to be ready ************/

    case Wait_Before_Fetch_Cin :

    Req_Matrix = NONE;
    Issue_Request_LS = FALSE;
    Buffer_Ready = newMEM_IF-> Check_Buffer_Ready(Req_Matrix);

    if(Buffer_Ready) { // check if buffer is ready
      Matmul_Comm_Next = Comm_Fetch_Cin;
    }
    else Matmul_Comm_Next = Matmul_Comm_Current;

    break;

    /****************** Fetch_Cin State **************/
    case Comm_Fetch_Cin :

    Fetch_Done = newMEM_IF->isFetchDone(CIN);
    //IgetPort = (*ARBIT_Now==MyCoreID && newMEM_IF->GetPort());
    //We assume SRAM has many ports here
    
    IgetPort = newMEM_IF->GetPort();

    //this is for PE execute
    if(Cin_Intersection)
      init_fetch_current= Req_Cin_Intersection;

    if(Fetch_Done){  // check if fetch is done
      
      Matmul_Comm_Next = Comm_Request_Cout;
      
      Mc_Comm_Cin_Next = (Mc_Comm_Cin_Current+LAPU_Size);
      
      if(Mc_Counter_Curr ==0 && Kc_Counter_Curr>=0 && N_Counter_Curr==0 && Ma_Counter_Curr==0&&
          BigC_Current!=0 && Cin_Intersection){
        Matmul_Next_State=Matmul_MAC_BC;
        Matmul_Comm_Next = Comm_Request_Cin;
        Comm_Go_C_Again = TRUE;
        Mc_Comm_Cin_Next = Mc_Comm_Cin_Current;
        Cin_Intersection = FALSE;
        init_fetch_next=Init_Idle;
        first_cin = 1;
        Stall = FALSE;
      }
      
      else if (first_cin){
        Matmul_Comm_Next = Comm_Evaluate;
        //Mc_Comm_Cout = Mc_Comm_Cout +4;
        Mc_Comm_Cout_Next = (Mc_Comm_Cout_Current+ LAPU_Size);
        first_cin=0;
        first_cout=0;
      }
      //x_comm_cin = (x_comm_cin+LAPU_Size);
      
    }
    break;
    /***********************************************/
    
    /****************** Request_Cout State **************/
    case Comm_Request_Cout :

    Issue_Request_LS=FALSE;
    Req_Matrix = NONE;

		/*if(Matmul_Current_State== Matmul_Flush_Cout)
			std::cout<<" Matmul_Flush_Cout"<<std::endl;
    
		std::cout<<" Matmul_Current_State is "<< Matmul_Current_State <<std::endl;*/


      if (((Kc_Counter_Curr > FMA_Latency) && (Mc_Counter_Curr == Mc_Comm_Cout_Current /*|| 
              (Mc_Counter_Curr< Mc_Comm_Cout_Current && )*/) ) 
          || ((Mc_Counter_Curr>Mc_Comm_Cout_Current)) || Matmul_Current_State==Matmul_Flush_Cout/*|| ((Mc_Counter_Curr<Mc_Comm_Cout_Current &&
              (N_Counter_Curr >= N_Comm_B || (N_Comm_B==0 && Ma_Counter_Curr!=0) )       )))*/)
      {
          newMEM_IF ->NotifyStore();//Set the counter to -1 on the I/O side
          Matmul_Comm_Next = Comm_Send_Cout;
#ifdef PRINT_DEBUG
          if (Matmul_Current_State==Matmul_Flush_Cout)
            std::cout << " I entered flush cout" <<std::endl;
#endif
      }

    break;
    /***********************************************/
    
    case Comm_Wait_to_Flush_C:
  

    break;


    /****************** Send_Cout State **************/
    case Comm_Send_Cout :
 
      Req_Matrix = COUT;

      Issue_Request_LS = TRUE;

      inter_done = FALSE;

      if (req_counter==LAPU_Size-1) {
            Mc_Comm_Cout_Next = (Mc_Comm_Cout_Current + LAPU_Size);
           
            if(Matmul_Current_State==Matmul_Flush_Cout)
              Mc_Comm_Cout_Next = Mc_Comm_Cout_Current;

            #ifdef PRINT_DEBUG
            std::cout << "Mc_Comm_Cout_Next is " << Mc_Comm_Cout_Next <<std::endl;
            //getchar();
            #endif
            
            Matmul_Comm_Next = Comm_Cout_Delay;
      }
    

    break;
    /***********************************************/

    case Comm_Cout_Delay :

      Req_Matrix=COUT;
      Issue_Request_LS=TRUE;

      //std::cout << " req_matrix is " << std::endl;
      //getchar();

      Matmul_Comm_Next=Comm_Evaluate;
      
      if(Mc_Counter_Curr ==0 && Kc_Counter_Curr>=0 && N_Counter_Curr==0 && Ma_Counter_Curr==0&&
          BigC_Current!=0 && BigA_Current==HowManyPanel){

        from_Pref->Cout[MyCoreID]=TRUE;
        Matmul_Comm_Next = Comm_Wait_for_Csync;
        inter_done = TRUE;
        BigA_Next=0;
        if(BigC_Current==1){ 
          //Dump_SRAM(0);
          //getchar();
        }
      }

#ifdef PRINT_DEBUG
      if(/*BigA_Current!=0 &&*/ Mc_Counter_Curr==0 /*&& Ma_Counter_Curr==1*/ && N_Counter_Curr==0){
          Dump_SRAM(0);
          //if (Ma_Counter_Curr==0 && BigA_Current!=0)
            //Clear_SRAM_C();

      }
#endif

    break;

    /*************************************************************/
    case Comm_Wait_for_Csync:

      Req_Matrix=NONE;
      Issue_Request_LS=NONE;
      stall_C_inter++;

#if 0           
      if (BigC_Current==NumofPartition && MyCoreID==NumofCore-1){  
          Dump_Matmul_SMachine();
          //Dump_SRAM(0);
          Matmul_Comm_Next = Comm_Idle;
          std::cout << "I am the last core done ! " << std::endl;
          std::cout << "Cycles is " << Cycles_Passed<<std::endl;
          //exit(0); 
      }
#endif

#if 0
      if (BigC_Current==NumofPartition && ITER_COUNT==1){ 
          if(MyCoreID==NumofCore-1)
            //Dump_Matmul_SMachine();
            //Dump_SRAM(0);
            //exit(0);
      }  
#endif

      if (BigC_Current==NumofPartition){ 
        
        if(LAST_STORE==1){

          std::cout << "I am the last core done ! " << std::endl;
          std::cout << "Cycles is " << Cycles_Passed<<std::endl;
          Dump_Matmul_SMachine();
          Matmul_Comm_Next = Comm_Idle;
          DONE = TRUE;
          //exit(0); 

          LAST_STORE=0;
        
        }
          //ITER_COUNT++;
      }

      if (from_Pref->Cin[MyCoreID]) {
          Matmul_Comm_Next=Comm_Request_Cin;
          Cin_Intersection = TRUE;
          from_Pref->Cin[MyCoreID] = FALSE;
          //Dump_SRAM(0);
          
          //getchar();//masri
        }
    break;
  
    case Comm_Idle :
    
    //std::cout << "I am at Comm_Idle ! " << std::endl;
    break;

    /************************Evaluate Stage************************/
    case Comm_Evaluate :

    //Send_Done = newMEM_IF->isCoutWritten(COUT);
      
      //IgetPort = (*ARBIT_Now==MyCoreID && newMEM_IF->GetPort());
      //We assume SRAM has many ports here
      IgetPort = newMEM_IF->GetPort();

      //std::cout << "Did I get port on evaluate " << IgetPort << std::endl;


      if (IgetPort){      //Check whether we could occupy the next port
        //Here we need to make smart decisions
        if (EnoughB && EnoughA &&     // this means we dont have time 
            (Mc_Counter_Curr>=Mc_Comm_Cin_Current))
          {
               //Req_Matrix = CIN;
               Matmul_Comm_Next= Comm_Request_Cin;
          }

        else if (EnoughB && EnoughA && 
                  (Mc_Counter_Curr<Mc_Comm_Cin_Current && 
                   Kc_Counter_Curr >Kernel_H-5 )){
    
                //Req_Matrix = NONE;
                Matmul_Comm_Next = Comm_Evaluate;
        }
        
        else if (((EnoughB && !FULL_B) && EnoughA &&    //added by mochamad
                Kc_Counter_Curr < Kernel_H- 10)){

                if (fetchedB) {
                  if(FULL_A) { 
                    
                   // Req_Matrix = NONE;
                    Matmul_Comm_Next = Comm_Evaluate;
                  
                  }

                  else {
                      
                    if (!fetchedA){
                     // Req_Matrix=A;
                      Matmul_Comm_Next = Comm_Request_A;
                      //fetchedA=TRUE;
                    }

                    else {        
                     //Req_Matrix = NONE;
                     Matmul_Comm_Next = Comm_Evaluate;
            
                    }
                  }

                }
                
                else{
                  //Req_Matrix = B;
                  Matmul_Comm_Next = Comm_Request_B; 

                  if (Mc_Comm_B==Kernel_H-LAPU_Size)
                    Req_Matrix = NONE;
                  //fetchedB = TRUE;
                }
        }

        else if (!EnoughB){
                //Req_Matrix = B;
                Matmul_Comm_Next = Comm_Request_B; 
                
        }

        else if (!EnoughA || (EnoughA && Kc_Counter_Curr<Kernel_H- 10)){
             
              if(FULL_A) {
                //Req_Matrix = NONE;
                Matmul_Comm_Next = Comm_Evaluate;
              }

              else{
                //Req_Matrix = A;
                
                Matmul_Comm_Next = Comm_Request_A;
                //fetchedA=TRUE;
              }       
        }
      }

      else {
        //Req_Matrix = NONE;
        Matmul_Comm_Next = Comm_Evaluate;
      }
      /*if(Fetch_Done){  // check if fetch is done
        Matmul_Comm_Next = Comm_Evaluate;
      }*/

    break;
    /*******************************************************************/
    
    /****************************Comm_Request_B*************************/
    case Comm_Request_B:

    fetchedB = TRUE;
    
    Issue_Request_LS = TRUE;
    Req_Matrix = B;
        
    if (req_counter==LAPU_Size-1)
        Matmul_Comm_Next = Wait_Before_Fetch_B;
    
    /*Buffer_Ready = newMEM_IF->Check_Buffer_Ready(Req_Matrix);

    if(Buffer_Ready)  // check if buffer is ready
      Matmul_Comm_Next = Comm_Fetch_B;

    else Matmul_Comm_Next = Matmul_Comm_Current;*/
    break;
    /******************* Wait for Bin to be ready ************/

    case Wait_Before_Fetch_B :

    Issue_Request_LS = FALSE;
    Req_Matrix = FALSE;

    Buffer_Ready = newMEM_IF-> Check_Buffer_Ready(Req_Matrix);

    //std::cout << "Buffer_Ready in wait b is " << Buffer_Ready;

    if(Buffer_Ready) { // check if buffer is ready
      Matmul_Comm_Next = Comm_Fetch_B;
    }
    else Matmul_Comm_Next = Matmul_Comm_Current;

    break;
    
    /***************************Fetch B*********************************/
    case Comm_Fetch_B:
    
    Fetch_Done = newMEM_IF->isFetchDone(B);
    IgetPort = newMEM_IF->GetPort();

    //std::cout << "Fetch Done is " << Fetch_Done << std::endl;

      if (IgetPort && !counter){      //Check whether we could occupy the next port
        //Here we need to make smart decisions
        counter++;
        if (EnoughB && EnoughA &&     // this means we dont have time 
            (Mc_Counter_Curr>=Mc_Comm_Cin_Current))
          {
                //next_state= Comm_Fetch_Cin;
                //Req_Matrix = CIN;
                selector = 1;
          }

        else if (EnoughB && EnoughA && 
                  (Mc_Counter_Curr<Mc_Comm_Cin_Current && 
                   Kc_Counter_Curr >Kernel_H-5 )){
    
                //Req_Matrix = NONE;
                //next_state = Comm_Evaluate;
                selector = 2;
        }

        else if ((EnoughB && EnoughA &&
                Kc_Counter_Curr < Kernel_H- 10)){

                if (fetchedB) {
                  if(FULL_A) { 
                    
                    //Req_Matrix = NONE;
                    selector = 2;
                  
                  }

                  else {
                  //  Req_Matrix=A;
                    //fetchedA=TRUE;
                    selector = 4;
                  }

                }
                
                else {
                  //Req_Matrix = B;

                  if (Mc_Comm_B==Kernel_H-LAPU_Size)
                    //Req_Matrix = NONE;
                  
                  //fetchedB = TRUE;
                  //next_state = Comm_Request_B; 
                  selector = 3;
                }
        }

        else if (!EnoughB){
                //Req_Matrix = B;

                /*if (Mc_Comm_B==Kernel_Size-LAPU_Size)
                  Req_Matrix = NONE;*/

                //next_state = Comm_Request_B; 
                selector = 3;
        }

        else if (!EnoughA || (EnoughA && Kc_Counter_Curr<Kernel_H-10)){
             
              if(FULL_A) {
               // Req_Matrix = NONE;
                //Matmul_Comm_Next = Comm_Evaluate;
                selector = 2;
              }

              else{
                //Req_Matrix = A;
                
                //Matmul_Comm_Next = Comm_Request_A;
                selector = 4;
                //fetchedA=TRUE;
              }
                
        }
      }

      if (Fetch_Done){
        Matmul_Comm_Next = (selector==1)?Comm_Request_Cin:(selector==2)?Comm_Evaluate: (selector==3)? Comm_Request_B: (selector==4)? Comm_Request_A: Comm_Evaluate ;
        Mc_Comm_B = Mc_Comm_B + LAPU_Size;
        selector = 0;

        if (Mc_Comm_B==Kernel_H){ 
          EnoughB = TRUE;
          FULL_B=TRUE;
        
        
          //std::cout<< "FULL_B is " << FULL_B << std::endl;
        
          Matmul_Comm_Next = Comm_Evaluate;

          Mc_Comm_B = 0;
          //N_Comm_B = (N_Comm_B + LAPU_Size)%Panel_Size;
          N_Comm_B = (N_Comm_B + LAPU_Size);
          if(N_Comm_B==Panel_Size)
            N_Comm_B=0;
          
          N_Comm_Bin = (N_Comm_Bin + LAPU_Size);

          if(N_Comm_Bin==Panel_Size){
            N_Comm_Bin=0;
            if(Ma_Counter_Curr==A_per_Core-1) 
              BigB_Next= BigB_Current+1;
              //Chunk_B++; 
            
            //from_Pref->B[MyCoreID]=FALSE;
            //from_Pref->currentB= (from_Pref->currentB +1) %2;
          }
        }
        counter=0;
      }


    break;
    /*******************************************************************/
    
    /****************************Comm_Request_A*************************/
    case Comm_Request_A:
    
    fetchedA = TRUE;
    
    Req_Matrix = A ;
    Issue_Request_LS = TRUE;

    if (req_counter==LAPU_Size-1)
      Matmul_Comm_Next = Wait_Before_Fetch_A;
    
    break;
    /*******************************************************************/

    /******************* Wait for Ain to be ready ************/

    case Wait_Before_Fetch_A :

    Issue_Request_LS = FALSE;
    Req_Matrix = NONE;

    Buffer_Ready = newMEM_IF-> Check_Buffer_Ready(Req_Matrix);

    if(Buffer_Ready) { // check if buffer is ready
      Matmul_Comm_Next = Comm_Fetch_A;
    }
    else Matmul_Comm_Next = Matmul_Comm_Current;

    break;

    
    /***************************Fetch A*********************************/
    case Comm_Fetch_A:
    
    Fetch_Done = newMEM_IF->isFetchDone(A);
    //IgetPort = (MyCoreID==*ARBIT_Now && newMEM_IF->GetPort());
    
    //We assume SRAM has many ports here
    IgetPort = newMEM_IF->GetPort();

    //std::cout << "Reach A " <<std::endl;
    //std::cout << "Fetch_Done A is" << Fetch_Done<<std::endl;
    //getchar();


      if (IgetPort && !counter){      //Check whether we could occupy the next port
        //Here we need to make smart decisions
        counter++;
        if (EnoughB && EnoughA &&     // this means we dont have time 
            (Mc_Counter_Curr>=Mc_Comm_Cin_Current))
          {
                //next_state= Comm_Fetch_Cin;
                //Req_Matrix = CIN;
                selector = 1;
          }

        else if (EnoughB && EnoughA && 
                  (Mc_Counter_Curr<Mc_Comm_Cin_Current && 
                   Kc_Counter_Curr >Kernel_H-5 )){
    
                //Req_Matrix = NONE;
                //next_state = Comm_Evaluate;
                selector = 2;
        }

        else if (!EnoughA){
          //Req_Matrix=A;
          selector = 4;

        }

        else {
        
          //Req_Matrix = NONE;
          selector = 2;
        }
     }

      if (Fetch_Done){
        Matmul_Comm_Next = (selector==1)?Comm_Request_Cin:(selector==2)?Comm_Evaluate: (selector==3)? Comm_Request_B: (selector==4)? Comm_Request_A: Comm_Evaluate ;
        //Matmul_Comm_Next = next_state;
        Mc_Comm_A++;  // The counter for the local memoryA
        Mc_Fetch_A = Mc_Fetch_A+ LAPU_Size; //the counter for MatrixA to be passed to randombehav

        if (Mc_Fetch_A == Kernel_H){
          Mc_Fetch_A = 0;
          N_Fetch_A = N_Fetch_A + LAPU_Size;

          if (N_Fetch_A == Kernel_V){//should be kernel_size
          
            N_Fetch_A = 0;
            FULL_A=TRUE;
            Enough_A = TRUE;

            from_Pref->A[MyCoreID]=FALSE;
            
            if (Matmul_Comm_Next==Comm_Request_A)
              Matmul_Comm_Next = Comm_Evaluate;
            //EnoughA = TRUE;
          
          }
        }
        
        if (Mc_Comm_A== FETCH_A_AMOUNT  /*Mem_Size_A*/) {
          EnoughA = TRUE;
          FULL_A = TRUE;
            
          if (Matmul_Comm_Next==Comm_Request_A)
              Matmul_Comm_Next = Comm_Evaluate;

          Mc_Comm_A = 0;
          N_Comm_A = N_Comm_A++;
           
          if(N_Comm_A == Kernel_V){ 
            N_Comm_A = 0;
            //N_Comm_A what for ?
          
          }
  
          
        }

        counter=0;
      }
     

    break;
    /*******************************************************************/


  }

}

int LAPU::Matmul_Kernel(int Global_index){

	/*while (1)*/{

			switch (Matmul_Current_State){
       /* case Matmul_Stall:

            //if
            //if (Mc_Counter_Current>Mc_Cin_Comm_Current) then stall;
            //if (Mc_Counter_Current>=Mc_Bin_Comm_Current) then stall;
            //dont forget that Mc_Bin_Comm_Current initially would be Mc_Counter_Current + Memsizeofb/2
            //Ain apply the same also

            //if (Ma_Counter_Current>Mc_Cin_Comm_Current) then Matmul_Next_State = Matmul_Stall;
            
            //else Matmul_Next_State = Matmul_MAC_BC;
            //remember only stall computation

            
        break;*/

				case Matmul_Init:

					Kc_Counter_Next=(Kc_Counter_Curr+1);
					if (Kc_Counter_Curr==(LAPU_Size-1)){
						Matmul_Next_State=Matmul_FetchB;
						Kc_Counter_Next=0;
						Mc_Counter_Next=0;

					}

					//wait for PEs to get the data
					Latency_Counter_Next=0;
					Latency_Counter_Curr=0;
          
          A_per_Core = (howmanyA/NumofCore);
          Residue = (howmanyA%NumofCore);
          int k;  
          for (k=0; k<Residue; k++){
            if (MyCoreID==k) A_per_Core++; 
          }



				break;


				case Matmul_FetchB:

					Kc_Counter_Next=(Kc_Counter_Curr+1);

					if (Kc_Counter_Curr==(Kernel_H-1)){ //bus latency is 1
						Matmul_Next_State=Matmul_FetchA;
						Kc_Counter_Next=0;
						Mc_Counter_Next=0;

					}

				break;


				case Matmul_FetchA:
          
          //Mochamad --> This was updated by Ardavan
  

					/*
					Kc_Counter_Next=(Kc_Counter_Curr+1) % Kernel_Size;
					Matmul_Next_State=Matmul_FetchA;

					if (Kc_Counter_Curr== (Kernel_Size -1)){

						Mc_Counter_Next=(Mc_Counter_Curr+LAPU_Size)% Kernel_Size;
						//Mc_Counter_Next=(Mc_Counter_Curr+1)% Kernel_Size;

						if (Mc_Counter_Curr==(Kernel_Size-LAPU_Size)){
						//if (Mc_Counter_Curr==(Kernel_Size-1)){
							Matmul_Next_State=Matmul_BC0;
							//MAtmul_Next_State=Matmul_End;
							Kc_Counter_Next=0;
							Mc_Counter_Next=0;

						}
					}
					 */


					Kc_Counter_Next=(Kc_Counter_Curr+LAPU_Size) % Kernel_H;
					Matmul_Next_State=Matmul_FetchA;

					if (Kc_Counter_Curr== (Kernel_H -LAPU_Size)){

						Mc_Counter_Next=(Mc_Counter_Curr+1)% Kernel_V;
						//Mc_Counter_Next=(Mc_Counter_Curr+1)% Kernel_Size;

						if (Mc_Counter_Curr==(Kernel_V-1)){
						//if (Mc_Counter_Curr==(Kernel_Size-1)){
							Matmul_Next_State=Matmul_BC0;
							//Matmul_Next_State=Matmul_End;
							Kc_Counter_Next=0;
							Mc_Counter_Next=0;

						}
					}


				break;

				case Matmul_BC0: // Write_Row_Reg<-SRAM[A(0,0)] in the previous state
					// just BC the 0th
					// and read the 1st from the SRAM
					//Nothing is on the buS
					//Matmul_Next_State=Matmul_End;
          Stall = FALSE;
					Matmul_Next_State=Matmul_BC;

				break;


				case Matmul_BC:  // it loops equal to bus delay for future
          
					Matmul_Next_State= Matmul_MAC_BC;
					Kc_Counter_Next=Kc_Counter_Curr+1;

				break;
        

				case Matmul_MAC_BC:

						Matmul_Next_State=Matmul_MAC_BC;

            /*std::cout << "Kc and Mc in FSM are" << Kc_Counter_Curr 
                 << " " << Mc_Counter_Curr << std::endl;
            getchar();*/

            //B
            //if {

            //if (Kc_Counter_Current>Kc_Comm_Current) then stall;
            //if (Mc_Counter_Current>Mc_Comm_Current) then stall;
            //if (Ma_Counter_Current>Ma_Comm_Current) then Matmul_Next_State = Matmul_Stall;
            //Stall_at_MAC_BC = 1;

            //}
            
            //else if{}

            if (Mc_Counter_Curr > Mc_Comm_Cin_Current || !Comm_Go_C_Again /*(Ma_Counter_Curr > Ma_Comm_Cin) ||*/ 
                /*((Ma_Counter_Curr > Ma_Comm_Cout)) ||*/ 
                /*||(Mc_Counter_Curr==0 && Kc_Counter_Curr==0 && N_Counter_Curr==0 && BigA_Current!=0
                  && (FULL_B && from_Pref->A[MyCoreID]==FALSE))*/ ||
                (Mc_Counter_Curr > Mc_Comm_Cout_Current && Kc_Counter_Curr==FMA_Latency-2) || Stall_Comm){
              
              Stall = TRUE;
              
              #ifdef PRINT_DEBUG
              if((ff%FAST_FORWARD)==0)
                std::cout << " MACBC_Stall :is not ready"<<std::endl; 
              #endif
              if(Matmul_Comm_Current == Comm_Request_A || Matmul_Comm_Current== Comm_Fetch_A)
              {stall_A++;
                //getchar();
              }
              else if(Matmul_Comm_Current == Comm_Request_B || Matmul_Comm_Current== Comm_Fetch_B)
                stall_B++;
              else if (Stall_Comm);
              else
                stall_C++;
                
            }

            else if (!EnoughA) { 
              Stall = TRUE;
              stall_A++;
              //getchar();
            }
            else if (!EnoughB)  {
              Stall = TRUE;
              stall_B++;
            }

            else{
             
            Stall = FALSE; 
            
            #ifdef PRINT_DEBUG
              if((ff%FAST_FORWARD)==0)
                std::cout << "I am not Stalling " << std::endl;
            #endif
						Kc_Counter_Next=(Kc_Counter_Curr+1) % Kernel_H;
						if (Kc_Counter_Curr== (Kernel_H -1)){
							Mc_Counter_Next=(Mc_Counter_Curr+LAPU_Size)% Kernel_V;
							if (Mc_Counter_Curr==(Kernel_V-LAPU_Size)){
								N_Counter_Next=(N_Counter_Curr+LAPU_Size)% Panel_Size;
								if (N_Counter_Curr== (Panel_Size -LAPU_Size)){
                  //if (howmanyA) Ma_Counter_Next = 0;
                   Ma_Counter_Next = (Ma_Counter_Curr + 1)%A_per_Core;
                  if (Ma_Counter_Curr == A_per_Core - 1){
                      BigA_Next = BigA_Current + 1;

								  	  if(BigA_Current == HowManyPanel-1){
                        Matmul_Next_State = Matmul_Flush_Cout;
                        Matmul_Comm_Next = Comm_Wait_to_Flush_C;
                        BigC_Next = BigC_Current + 1;
                        Comm_Go_C_Again= FALSE;
                      }
                    }//Matmul_Next_State=Matmul_MAC_Flush;
                  }
                }

							}

						}

            

            #ifdef PRINT_DEBUG
              if(ff%FAST_FORWARD)
            std::cout << "Stall is " << Stall << std::endl;
            #endif
            /*std::cout << "EnoughA is " << EnoughA << std::endl;
            std::cout << "EnoughB is " << EnoughB << std::endl;
	          std::cout<<"FULL_A "<<FULL_A<<std::endl;
	          std::cout<<"FULL_B "<<FULL_B<<std::endl;
	          std::cout<<"fetchedB "<<fetchedB<<std::endl;
	          std::cout<<"fetchedA "<<fetchedA<<std::endl;*/

				break;

				case Matmul_MAC_Flush:
        
          //updated by Mochamad 
          
          //Kc_Counter_Next=(Kc_Counter_Curr+1) % Kernel_Size;
          

					Latency_Counter_Next=Latency_Counter_Curr+1;

					if (Latency_Counter_Curr< (FMA_Latency-1))
						Matmul_Next_State=Matmul_MAC_Flush;
					else{
						Matmul_Next_State=Matmul_End;
						Latency_Counter_Next=0;
					}
				break;

        case Matmul_Flush_Cout :

					//Latency_Counter_Next=Latency_Counter_Curr+1;
          // do not load anything to the ACC
          // Wait until flush_Cout

          /*if(Mc_Counter_Curr==0 && Kc_Counter_Curr==0 &&
                (from_Pref->A[MyCoreID]==FALSE)){
            Stall= TRUE;
          
          }*/

          /*else*/
          {
            if(!Stall){          
					    Latency_Counter_Next=Latency_Counter_Curr+1;
						  
              if( Latency_Counter_Curr<=FMA_Latency-1)
                Kc_Counter_Next=Kc_Counter_Curr+1 ;
              else 
                Stall=TRUE;
#ifdef PRINT_DEBUG
              std::cout<<"not stalling at flushcout"<<std::endl;
					    std::cout<<"Latency Counter is " << Latency_Counter_Next<<std::endl;
              std::cout<<"Kc_Next is "<< Kc_Counter_Next <<std::endl;
#endif
            }
            
          }

          if (Latency_Counter_Curr==FMA_Latency-1){
            
             
            Matmul_Comm_Next =  Comm_Request_Cout;
#ifdef PRINT_DEBUG
            std::cout<<"move from flushcout to comm_cout"<<std::endl;
#endif
            //then 
            /*if (BigC_Current==NumofPartition){
              Matmul_Next_State= Matmul_End;
            }*/
          }

          if(Comm_Go_C_Again){

            Kc_Counter_Next=0;
            Kc_Counter_Curr=0;
            Mc_Counter_Curr=0;
            Mc_Counter_Next=0;
            N_Counter_Curr=0;
            N_Counter_Next=0;
            Ma_Counter_Curr=0;
            Ma_Counter_Next=0;
            Latency_Counter_Next=0;
            
            Matmul_Next_State = Matmul_MAC_BC;
            //Matmul_Next_State = Matmul_BC;
          
          }

        break;


				case Matmul_End:

          //getchar();
          
          if (Last_Sending==LAPU_Size) done =true;
          
          Last_Sending++;

					Kc_Counter_Next=0;
					Kc_Counter_Curr=0;
					Mc_Counter_Curr=0;
					Mc_Counter_Next=0;
					N_Counter_Curr=0;
					N_Counter_Next=0;

				break;


			}

	  }

}
void LAPU::Address_Gen(){

  req_package->req=FALSE;

  if (Issue_Request_LS){ // generate when we want to request data from mem

    req_package->req=TRUE;
    req_package->type=NONE;

    packed_req = TRUE;
    row_offset = req_counter * Panel_Size;
    //intracore_offset = Ma_Counter_Curr*NumofCore*Kernel_Size*Kernel_Size;
    intercore_offset = MyCoreID*Kernel_V*Kernel_H;
    
    req_package->WE = FALSE;

    int Ma_Cout_Count;
    int Ma_Cin_Count;



    if(init_fetch_current==Init_Idle){

      switch (Req_Matrix){

      /*********************** Case Cin ***************************/

      case CIN :
       
        //should I put Mc_Comm_Current and Next?
        base_addr = x_comm_cin*Panel_Size;//every time we fetch, increment Mc_Comm_Cin by 4!! termasuk pada saat initialization

        //y_offset = (x_comm_cin==Kernel_Size-LAPU_Size)? N_Comm_Cin-LAPU_Size : N_Comm_Cin;
        y_offset =  N_Comm_Cin;

        // which column are we now 
        //Ma_Cin_Count = (N_Comm_Cout==Panel_Size-LAPU_Size)? Ma_Comm_Cin-1: Ma_Comm_Cin;
        Ma_Cin_Count = Ma_Comm_Cin;

        intracore_offset = Ma_Cin_Count*NumofCore*Kernel_V*Panel_Size;
        intercore_offset = MyCoreID*Kernel_V*Panel_Size;

        //so for address generator, only once it fires address
        
        if (req_counter<LAPU_Size){// request counter each for 4 elements

          for (int i=0; i<LAPU_Size; i++){
            req_package->addresses[i] = (base_addr + row_offset+ intracore_offset + 
                               intercore_offset + y_offset)+ i; 
           // std::cout << " Generate address Cin is "<< req_package->addresses[i]<<std::endl;
          }
          req_package->type = CIN;

          req_counter++;

        }

#if 0
        std::cout << " Gen address CIN is " << req_package->addresses[0] << std::endl;
        
        std::cout << " Its value is " << SRAM[req_package->addresses[0]] << std::endl;
#endif     

        #ifdef PRINT_DEBUG
        std::cout << " intracore_offset is " << intracore_offset << std::endl;
        std::cout << " row_offset is " << row_offset<<std::endl;
        std::cout << " y_offset is " << y_offset<<std::endl;
        std::cout << " x_comm_cin is " << x_comm_cin<<std::endl;
        std::cout << " req_counter at cin before if is  "<< req_counter <<std::endl;
        #endif

        if (req_counter == LAPU_Size){ 
        
          req_counter = FALSE;
          
          Ma_Comm_Cin =     (x_comm_cin==Kernel_V-LAPU_Size && 
                            N_Comm_Cin==Panel_Size-LAPU_Size)?
                            Ma_Comm_Cin+1: Ma_Comm_Cin;
          
          if(Ma_Comm_Cin==A_per_Core)
            Ma_Comm_Cin=0;
          
          if (x_comm_cin==Kernel_V-LAPU_Size)
            N_Comm_Cin = (N_Comm_Cin+LAPU_Size)%Panel_Size;
          
          x_comm_cin = (x_comm_cin + LAPU_Size)%Kernel_V;

            /*if(N_Comm_Cin==Panel_Size-LAPU_Size){
            
              Ma_Comm_Cin++;
            }*/

        }

        
      break;
      
      /*********************** Case COUT ***************************/

      case COUT :
        
        base_addr = x_comm_cout*Panel_Size;//every time we fetch, increment Mc_Comm_Cin by 4!! termasuk pada saat initialization
        //y_offset = (x_comm_cout==Kernel_Size-LAPU_Size)? N_Comm_Cout-LAPU_Size : N_Comm_Cout;
        y_offset = N_Comm_Cout;
          
          //(Ma_Counter_Curr!=0 && N_Comm_Cout==0 && x_comm_cout==0)? Panel_Size-LAPU_Size
            //        :(N_Comm_Cout!=0 && x_comm_cout==0)? N_Comm_Cout-LAPU_Size : N_Comm_Cout;
        
        req_package->WE = TRUE;
        row_offset = (req_counter-1) * Panel_Size;

        //Ma_Cout_Count = (N_Comm_Cout==Panel_Size-LAPU_Size)? Ma_Comm_Cout-1: Ma_Comm_Cout;
        Ma_Cout_Count = Ma_Comm_Cout;

        intracore_offset = Ma_Cout_Count*NumofCore*Kernel_V*Panel_Size;
        //intracore_offset = 0*NumofCore*Kernel_Size*Panel_Size;
        intercore_offset = MyCoreID*Kernel_V*Panel_Size;

        
        if (req_counter<LAPU_Size + 1){// request counter for 4 elements

          if (req_counter>=1){

            for (int i=0; i<LAPU_Size; i++){
            req_package->addresses[i] = (base_addr + row_offset+ intracore_offset + 
                               intercore_offset + y_offset)+ i; 
            #ifdef PRINT_DEBUG
            std::cout << " intercore_offset is " << intercore_offset << std::endl;
            std::cout << " base_addr is " << base_addr<<std::endl;
            std::cout << " row_offset is " << row_offset<<std::endl;
            std::cout << " y_offset is " << y_offset<<std::endl;
            std::cout << " Generate address C is "<< req_package->addresses[i]<<std::endl;
            std::cout << " req_counter at cout before if is  "<< req_counter <<std::endl;
            #endif

            }
#if 0
            std::cout << " Gen address COUT is " << req_package->addresses[0] << std::endl;
        
            std::cout << " Its value is " << Column_Buses_Read[0] << std::endl;
#endif

          }
          
          else{ 
            req_package->req=FALSE;
            req_package->WE=TRUE;
          }
          
          req_counter++;
          req_package->type= COUT;

        }
          
        if (req_counter == LAPU_Size+1){

          req_counter = FALSE;
          Ma_Comm_Cout =     (x_comm_cout==Kernel_V-LAPU_Size && 
                            N_Comm_Cout==Panel_Size-LAPU_Size)?
                            Ma_Comm_Cout+1: Ma_Comm_Cout;

          if(Ma_Comm_Cout==A_per_Core)
            Ma_Comm_Cout=0;
          
          if (x_comm_cout==Kernel_V-LAPU_Size)
            N_Comm_Cout = (N_Comm_Cout+LAPU_Size)%Panel_Size;
          
          x_comm_cout = (x_comm_cout+LAPU_Size)%Kernel_V;
          //std::cout << " I am cout done " <<std::endl;
          //getchar();

            /*if(N_Comm_Cout==Panel_Size-LAPU_Size){
            
              Ma_Comm_Cout++;
            }*/

        }
      break;
      
      /************************* Case B *************************/

      case B :

        //Need to provide counter of how many chunk and 
        //how many partition of C now

        afterC_offset = SRAM_OFFSET*Panel_Size + Panel_Size*Panel_Size + (BigB_Current%2)*Panel_Size*Kernel_H; // state in the parameters offset of C. Ini configurabel + (Chunk_B%2)*Panel_Size*Kernel_Size
        base_addr = (Mc_Comm_B+req_counter)*Panel_Size + afterC_offset;//

        //std::cout<<"Mc_Comm_B is "<< Mc_Comm_B << std::endl;
        //std::cout<<"req_counter "<< req_counter << std::endl;
        //getchar();


        y_offset = N_Comm_Bin; // vary from 0 to Panel_Size

        if (req_counter<LAPU_Size){// request counter for 4 elements

          for (int i=0; i<LAPU_Size; i++){
            req_package->addresses[i] = (base_addr +  y_offset)+ i; 
          }

          req_counter++;
          req_package->type = B;
        }
          
        if (req_counter == LAPU_Size){ 
          req_counter = FALSE; 

          /*if (Mc_Comm_B==Kernel_Size-LAPU_Size){
             N_Comm_Bin = N_Comm_Bin+LAPU_Size;
              if(N_Comm_Bin==Panel_Size){
                N_Comm_Bin = 0;
                Chunk_B++; 
              }

          }*/
        }
      break;

      
      case A :

      /************************* Case A *************************/
        afterB_offset = SRAM_OFFSET*Panel_Size + Panel_Size*Panel_Size + 2*Kernel_H*Panel_Size;// TODO do I need to do (Chunk_A%2)*Kernel_Size*Panel_Size ?
        base_addr = afterB_offset /*+ ((Ma_Counter_Curr+1)%A_per_Core)*NumofCore*Kernel_Size*Kernel_Size*/;
        row_offset = (req_counter+N_Fetch_A )*Kernel_H;
        //next_line_offset = N_Fetch_A*Kernel_Size*LAPU_Size;
          
        y_offset = Mc_Fetch_A;


        if (req_counter<LAPU_Size){// request counter for 4 elements

          for (int i=0; i<LAPU_Size; i++){
            req_package->addresses[i] = (base_addr + row_offset + intercore_offset + y_offset)+ i;
           
            #ifdef PRINT_DEBUG
            std::cout << " intercore_offset is " << intercore_offset << std::endl;
            std::cout << " Ma_Counter_Curr " << Ma_Counter_Curr<<std::endl;
            std::cout << " after_B_offset " << afterB_offset<<std::endl;
            std::cout << " base_addr is " << base_addr<<std::endl;
            std::cout << " row_offset is " << row_offset<<std::endl;
            std::cout << " y_offset is " << y_offset<<std::endl;
            std::cout << " Generate address A is "<< req_package->addresses[i]<<std::endl;
            std::cout << " SRAM [Addr] is "<<SRAM[req_package->addresses[i]]<<std::endl;
            //getchar();
            #endif
          }

          req_package->type = A;
          req_counter++;
        }
          
        if (req_counter == LAPU_Size) 
          req_counter = FALSE; 

      break;

    
      }   
    }

    else { //if in init state
    
      //TODO

      switch(Req_Matrix){
      
        case CIN:
        // do CIN initial fetch
        
        base_addr = Mc_Comm_Cin_Current*Panel_Size;
        //y_offset = N_Comm_Cin;  // which column are we now
        intracore_offset = Ma_Counter_Curr*NumofCore*Kernel_V*Panel_Size;
        intercore_offset = MyCoreID*Kernel_V*Panel_Size;

        //so for address generator, only once it fires address
        
        if (req_counter<LAPU_Size){// request counter each for 4 elements

          for (int i=0; i<LAPU_Size; i++){
            req_package->addresses[i] = (base_addr + row_offset+ intracore_offset + 
                               intercore_offset)+ i;

          //std::cout<< "req_pakcage->addreses in CIN init " << req_package->addresses[i]<<std::endl;
          }
          //std::cout<< "intercore_offset in Cin is " << intercore_offset<<std::endl;
          //getchar();
          
          req_package->type = CIN;

          req_counter++;

        }
          
        if (req_counter == LAPU_Size) 
          req_counter=FALSE;
        break;


        case B:


        //do B initial Fetch
        afterC_offset = Panel_Size*Panel_Size +SRAM_OFFSET*Panel_Size; // state in the parameters offset of C. Ini configurabel + (Chunk_B%2)*Panel_Size*Kernel_Size
        base_addr = b_counter*Panel_Size + afterC_offset;
        
        if (req_counter<LAPU_Size){// request counter each for 4 elements

          for (int i=0; i<LAPU_Size; i++){
            req_package->addresses[i] = base_addr + i; 
          }
#if 0            
          std::cout << " Generate address B is "<< req_package->addresses[0]<<std::endl;
          std::cout << " SRAM [Addr] is "<<SRAM[req_package->addresses[0]]<<std::endl;
#endif

          req_package->type = B;

          req_counter++;

        }
          
        b_counter++;
        
        if (req_counter == LAPU_Size) 
          req_counter=FALSE;


        break;

        case A:

        //do A initial fetch
       

        afterB_offset = afterC_offset + 2*Kernel_H*Panel_Size;//Ini juga configurable (Chunk_A%2)*Kernel_Size*Panel_Size
        base_addr = afterB_offset + MyCoreID*Kernel_H*Kernel_V;
        //base_addr = afterB_offset + ((Ma_Counter_Curr+1)%A_per_Core)*NumofCore*Kernel_Size*Kernel_Size;
        row_offset = (x_b_counter+req_counter)*Kernel_H;
        //next_line_offset = N_Fetch_A*Kernel_Size*LAPU_Size;
          


        if (req_counter<LAPU_Size){// request counter for 4 elements

          for (int i=0; i<LAPU_Size; i++){
            req_package->addresses[i] = (base_addr + row_offset + 
                y_a_counter)+ i; 
#if 0          
            std::cout << " Generate address A is "<< req_package->addresses[0]<<std::endl;
            std::cout << " SRAM [Addr] is "<<SRAM[req_package->addresses[0]]<<std::endl;
#endif          
          }
          
          //std::cout<< "base_addr in Ain init is " << base_addr<<std::endl;
          //std::cout<< "row_offset in Ain init is " << row_offset<<std::endl;
          //getchar();

          req_package->type = A;
          req_counter++;
        }
         
        if (req_counter == LAPU_Size){
          req_counter = FALSE; 
          
          if (y_a_counter == Kernel_H - LAPU_Size)
            x_b_counter = (x_b_counter+LAPU_Size)%Kernel_V;

          y_a_counter = (y_a_counter + LAPU_Size)%Kernel_H;
        }
        break;
          
      }
    
    
    }


  }


}

int LAPU::PEs_Execute(){

      int Global_index = 0;


      /**************************** Printing the State Machine **********************************/




      /***************************** PE Execution ***********************************************/
      

			for (i=0;i<Size;i++)
				for (j=0;j<Size;j++){
					if (Stall);
          else {
            PE_Array[i][j].Execute_Matmul (Global_index, N_Counter_Curr, Mc_Counter_Curr, Kc_Counter_Curr, Ma_Counter_Curr, Matmul_Current_State, Latency_Counter_Curr, BigA_Current, BigC_Current);
          }
          ////////////////////////////////////// Fetching from column bus //////////////////////////////

          //if(Matmul_Current_State==Matmul_MAC_BC) 
            PE_Array[i][j].Fetch(Buffer_Ready, Matmul_Comm_Current, Stall, Kc_Counter_Curr, init_fetch_current);
            

          // We pass this Routine an the current state of this routine to PE
          // : Moch --> Separate Communication and Computation in Execute_Matmul
          // PE.Array[i][j].Fetch (the required argumentsf from fetch logic, and send back the status of stall to fetch 
          // state machine, you need to think _current and _next also);
          // This PE.Array[i][j].Fetch will communicate with memory controller to judge to get data or not. 
          // Memory controller would arbitrate whether to serve core 0 or core 1
          // May put request in the buffer, but not guaranteed service soon
				}

        /*std::cout << "init_fetch_current is " << init_fetch_current<<std::endl;
        std::cout << "req_matrix is " << Req_Matrix<<std::endl;
        std::cout << "Matmul_Comm_Current is " << Matmul_Comm_Current <<std::endl;
        std::cout << "req_counter is " << req_counter <<std::endl;*/

        //getchar();
          
  
      //Generate address when there is a request;
      //if(Issue_Request_LS) Address_Gen();
      //Then send the request to SRAM via newIO_Execute, but only pass request list of request addresses and pack_request


			/*if (Matmul_Current_State!=Matmul_MAC_BC){ 
        Mem_IF ->PassMyCoreID(MyCoreID);
        Mem_IF->IO_Execute_Matmul (Global_index, N_Counter_Curr, Mc_Counter_Curr, Kc_Counter_Curr, Ma_Counter_Curr, Matmul_Current_State, Latency_Counter_Curr);
      }*/

      //Need to call address generator here
      //Only need to pass X, Y and req_matrix
      //

      //Add SRAM interface here
      newMEM_IF->ServiceRequest(req_package);


      /*if (Matmul_Current_State==Matmul_MAC_BC) newMEM_IF->newIO_Execute(Req_Matrix, Mc_Comm_Cin, Mc_Comm_Cout, Mc_Comm_B, Mc_Fetch_A, N_Fetch_A, N_Comm_B,
                                                                          Ma_Fetch_A, Stall,
                                                                          
                                                                          N_Counter_Curr, Mc_Counter_Curr, Kc_Counter_Curr, Ma_Counter_Curr);*/
        // I need Kc_Counter for A
       

			 if (((Matmul_Current_State==Matmul_End) && done) /*|| Ma_Counter_Curr*/ ){
				 Matmul_Current_State=Matmul_Init;
				 Matmul_Next_State=Matmul_Init;
				 return Cycles_Passed;
			 }
			//Dump_Row_Buses();
			
      #ifdef PRINT_DEBUG
			Dump_Row_Buses();
      Dump_Column_Buses();
      //if(MyCoreID==0) Dump_Column_Buses();
      #endif

#if 0
      if(ITER_COUNT==0)
        Dump_Column_Buses();
#endif

			Drive_Buses(); // Can it be a part of Cycle function? I remember I just separated it for readability
			// It does not matter which one comes first Drive_Bus or Cycle;
			/*if (Kc_Counter_Curr==0 && Matmul_Current_State==Matmul_MAC_BC) {
				getchar();
			}*/
      //if(Matmul_Current_State==Matmul_MAC_BC) getchar();
    

}

//main function of LAPU

int LAPU::GetKernelStatus(){
  if (Matmul_Comp_Comm_Current==Matmul_Ending)
    return TRUE;
  else
    return FALSE;
}

int LAPU::Issuer_State_Machine(){


  switch(Matmul_Comm_Current){

    case Comm_Request_Cin:
      if (Req_Matrix ==CIN){
        
        if (req_counter < LAPU_Size) Issue_Request_LS = TRUE;
        if(req_counter==LAPU_Size) {
          req_counter = 0;
          Issue_Request_LS = FALSE;
        }
      }
    break;

    case Wait_Before_Fetch_Cin:
      
    Req_Matrix = NONE;


    break;

    case Comm_Send_Cout:

      //if (Req_Matrix ==COUT){
        
        //if (req_counter < LAPU_Size) Issue_Request_LS = TRUE;
        /*if(req_counter==LAPU_Size) {
          req_counter = 0;
          WE = FALSE;
          Issue_Request_LS = FALSE;
        }*/
     // }

    break;

    case Comm_Cout_Delay:

    break;

    case Comm_Evaluate:

    Req_Matrix=NONE;
    Issue_Request_LS = FALSE;
    req_counter=FALSE;
    req_package->req=FALSE;

    break;
 

    case Comm_Request_B:

     if(Req_Matrix==B){
      if (req_counter < LAPU_Size) Issue_Request_LS = TRUE;
      if (req_counter==LAPU_Size) {
        req_counter = 0;
        Issue_Request_LS = FALSE;
      }
     }
    break;

    
    case Wait_Before_Fetch_B :
    Req_Matrix = NONE;
    break;

    
    case Comm_Fetch_B:
    
      if (Req_Matrix!=NONE){
      
        if (req_counter < LAPU_Size) Issue_Request_LS = TRUE;
        if (req_counter==LAPU_Size) {
          req_counter = 0;
          Issue_Request_LS = FALSE;
        }
      } 

    break;

    case Comm_Request_A:
     
     if(Req_Matrix==A){
      if (req_counter < LAPU_Size) Issue_Request_LS = TRUE;
      if (req_counter==LAPU_Size) {
        req_counter = 0;
        Issue_Request_LS = FALSE;
      }
     }
    break;

    case  Wait_Before_Fetch_A :
    
    Req_Matrix =NONE;

    break;

    case Comm_Fetch_A :

      if (Req_Matrix!=NONE){
      
        if (req_counter < LAPU_Size) Issue_Request_LS = TRUE;
        if (req_counter==LAPU_Size) {
          req_counter = 0;
          Issue_Request_LS = FALSE;
        }
      } 
    break;

  }
  Address_Gen();

}

void LAPU::init_fetch(){
  
  A_per_Core = (howmanyA/NumofCore);
  Residue = (howmanyA%NumofCore);
  int k;  
  for (k=0; k<Residue; k++){
    if (MyCoreID==k) A_per_Core++; 
  }

  bool skip =false;

  switch(init_fetch_current){

      case Init_Wait_Ready:

        //wait for sync signal from SRAM for C
        /*if(Synch[MyCoreID].sram_ready)*/{
          //std::cout<< " I am at init_ready"<<std::endl;
          Stall=TRUE;

          /*for Pref
           int B_Ready=TRUE;

          for (int i=0; i<NumofMemB; i++){
            if(from_Pref->B[MyCoreID][i]){
              B_Ready=FALSE;
              break;
            }
          }*/

          Stall_Init++;

          if (from_Pref->B[MyCoreID] && from_Pref->Cin[MyCoreID] &&
              from_Pref->A[MyCoreID])
            init_fetch_next = Init_Request_Cin;
        }
#if 0
        if(MyCoreID==NumofCore-1){
          std::cout << " B ready is " << from_Pref->B[MyCoreID]<<std::endl;
          std::cout << " C ready is " << from_Pref->Cin[MyCoreID]<<std::endl;
          std::cout << " A ready is " << from_Pref->A[MyCoreID]<<std::endl;
        }
#endif
      break;

      case Req_Cin_Intersection :

      break;

      case Init_Request_Cin:

#if 0
        std::cout << " Finally I am at init request! " << std::endl;
#endif
        //getchar();
        Stall_Init++;

        Issue_Request_LS = TRUE;
        Req_Matrix = CIN;
      
        if (req_counter==LAPU_Size-1){         
          init_fetch_next = Init_Wait_Before_Fetch_Cin;
        }
      break;

      
      case Init_Wait_Before_Fetch_Cin:
#ifdef PRINT_DEBUG
        std::cout << " Finally I am at wait before Cin " << std::endl;
#endif
        //getchar();
        Stall_Init++;
        
        Issue_Request_LS = FALSE;
        Req_Matrix = NONE;

        Buffer_Ready = newMEM_IF-> Check_Buffer_Ready(Req_Matrix);

        if(Buffer_Ready) { // check if buffer is ready
          init_fetch_next = Init_Comm_Fetch_Cin;
        }
        
      break;

      case Init_Comm_Fetch_Cin:
#ifdef PRINT_DEBUG
        std::cout << " Finally I am at Comm Cin " << std::endl;
#endif
        //getchar();
        Stall_Init++;

        Matmul_Comm_Current = Comm_Fetch_Cin;

      Fetch_Done = newMEM_IF->isFetchDone(CIN);

      if (Fetch_Done){
        init_fetch_next = Init_Request_B;
        Matmul_Comm_Next = Comm_Init;
        x_comm_cin = x_comm_cin + LAPU_Size;
      }

      break;

      case Init_Request_B:
       
        //check for ready signal for B
        //if (Sync_B is okay)
#if 0
        std::cout << " Finally I am at Req B " << std::endl;
        //getchar();
#endif        
        Stall_Init++;
        Issue_Request_LS = TRUE;
        Req_Matrix = B;
        
        if (req_counter==LAPU_Size-1)
          init_fetch_next = Init_Wait_Before_Fetch_B;

        InitB_Next = (InitB_Current+1);

				/*if (InitB_Current==(Kernel_Size-1)){ //bus latency is 1
						
            init_fetch_next=Init_Wait_Before_Fetch_B;
            InitB_Next = 0;
          }*/

      break;

      case Init_Wait_Before_Fetch_B:

        Stall_Init++;

        Issue_Request_LS = FALSE;
        Req_Matrix = FALSE;

        Buffer_Ready = newMEM_IF-> Check_Buffer_Ready(Req_Matrix);
        
        if(Buffer_Ready) { // check if buffer is ready
          init_fetch_next = Init_Comm_Fetch_B;
        }

      break;

      case Init_Comm_Fetch_B:
#ifdef PRINT_DEBUG      
        std::cout << " Finally I am at Fetch B " << std::endl;
        //getchar();
#endif
        Stall_Init++;

        Fetch_Done = newMEM_IF->isFetchDone(B);

        Matmul_Comm_Current = Comm_Fetch_B;
        
        if (Fetch_Done){
          if (InitB_Current==Kernel_H){
            init_fetch_next = Init_Request_A;
            InitB_Next=0;
            Matmul_Comm_Next = Comm_Init;
            N_Comm_Bin = N_Comm_Bin + LAPU_Size;
          }
          else{ 
            init_fetch_next = Init_Request_B;
          }
        }
      
      break;

      case Init_Request_A:

        //check ready signal for A from prefetcher
      
        Stall_Init++;

        Issue_Request_LS = TRUE;
        Req_Matrix = A;
        
        if (req_counter==LAPU_Size-1)
          init_fetch_next = Init_Wait_Before_Fetch_A;

        InitA_Next = (InitA_Current+1);

				/*if (InitA_Current==(Kernel_Size*Kernel_Size/LAPU_Size)){ //bus latency is 1						
            init_fetch_next=Init_Wait_Before_Fetch_A;
            InitA_Next = 0;
        }*/
      break;

      case Init_Wait_Before_Fetch_A:

#if 0
        std::cout << " Finally I am at Req A " << std::endl;
        //getchar();
#endif        
        Stall_Init++;

        Issue_Request_LS = FALSE;
        Req_Matrix = NONE;

        Buffer_Ready = newMEM_IF-> Check_Buffer_Ready(Req_Matrix);
        
        if(Buffer_Ready) { // check if buffer is ready
          init_fetch_next = Init_Comm_Fetch_A;
        }

      break;

      case Init_Comm_Fetch_A:

        Stall_Init++;

        Fetch_Done = newMEM_IF->isFetchDone(A);

        Matmul_Comm_Current = Comm_Fetch_A;
#if 0 
        std::cout << "InitA_Current is " << InitA_Current << std::endl;
#endif

        if (Fetch_Done){
          if (InitA_Current==Kernel_H*Kernel_V/LAPU_Size){
            InitA_Next = 0;    
            init_fetch_next = Init_Wait_To_Compute;   
             
            from_Pref->A[MyCoreID]=FALSE;
            from_Pref->Cin[MyCoreID]=FALSE;
            printf("Fetching A is done ! \n");
            //exit(0);
            //from_Pref->B[MyCoreID] = FALSE;
            //masri
            //Uncomment for Pref
            
            //May need to uncomment this for Pref
            
            /*init_fetch_next=Init_Idle;
            Matmul_Comm_Next= Comm_Init;
            Matmul_Next_State= Matmul_BC0;*/

            //tell the prefetcher that I already fetch a Mc*Kc of A
            //clear the ready signal now, and fetch the new A 
            
          }
          else 
            init_fetch_next = Init_Request_A;
        }

      break;

      case Init_Wait_To_Compute:

      Stall_Init++;

      //For Pref
        #if 0
        std::cout << " Finally I am at wait to compute " << std::endl;
        std::cout << "from_Pref->A " << from_Pref->A[MyCoreID] <<std::endl;
        std::cout << "from_Pref->A after " << from_Pref->A[MyCoreID] <<std::endl;
        

        std::cout << "before enter if wait to compute" << std::endl;
        #endif

        if (A_per_Core>=1){

          //when we do not use partition method
          if((NO_PART==TRUE) && (A_per_Core==1))
          {            
            from_Pref->B[MyCoreID] = FALSE;
            init_fetch_next = Init_Idle;
            Matmul_Comm_Next= Comm_Init;
            Matmul_Next_State= Matmul_BC0;
            counter_B =1;
            counter_A =1;
            std::cout <<"Finish init " << std::endl;
            skip=TRUE;
          }

          else if (from_Pref->A[MyCoreID]==TRUE){
            //from_Pref->A[MyCoreID] = FALSE;
            //#ifdef PRINT_DEBUG
            from_Pref->B[MyCoreID] = FALSE;
            printf("Core : Moving from wait compute to Init_Idle : Start Comp \n");
#if 0            
            Dump_SRAM(0);
            Dump_SRAM(1);
            Dump_SRAM(2);
            exit(0);
#endif

            //#endif
            init_fetch_next = Init_Idle;
            Matmul_Comm_Next= Comm_Init;
            Matmul_Next_State= Matmul_BC0;
            counter_B =1;
            counter_A =2;
            std::cout <<"Finish init " << std::endl;
            skip=TRUE;
          }
        }

        else {
            
            init_fetch_next = Init_Idle;
            Matmul_Comm_Next= Comm_Init;
            Matmul_Next_State= Matmul_BC0;
        }
      break;

      case Init_Idle:

      break;

  }

  if(!skip) Address_Gen();
  else 
    skip=false;

}


void LAPU::init_fetch_test(){
  
  A_per_Core = (howmanyA/NumofCore);
  Residue = (howmanyA%NumofCore);
  int k;  
  for (k=0; k<Residue; k++){
    if (MyCoreID==k) A_per_Core++; 
  }

  switch(init_fetch_current){

      case Init_Wait_Ready:

        //wait for sync signal from SRAM for C
        /*if(Synch[MyCoreID].sram_ready)*/{
          std::cout<< " I am at init_ready"<<std::endl;
          Stall=TRUE;

          /*for Pref
           int B_Ready=TRUE;

          for (int i=0; i<NumofMemB; i++){
            if(from_Pref->B[MyCoreID][i]){
              B_Ready=FALSE;
              break;
            }
          }*/
        std::cout << "from_Pref->A " << from_Pref->A[MyCoreID] <<std::endl;
        std::cout << "from_Pref->B " << from_Pref->B[MyCoreID] <<std::endl;
        std::cout << "from_Pref->C " << from_Pref->Cin[MyCoreID] <<std::endl;

          if (from_Pref->B[MyCoreID] && from_Pref->Cin[MyCoreID] &&
              from_Pref->A[MyCoreID]){
            init_fetch_next = Init_Request_Cin;

          }
        }

        /*if(MyCoreID=1){
          std::cout << " B ready is " << from_Pref->B[MyCoreID]<<std::endl;
          std::cout << " C ready is " << from_Pref->Cin[MyCoreID]<<std::endl;
          std::cout << " A ready is " << from_Pref->A[MyCoreID]<<std::endl;
        }*/

      break;

      case Init_Request_Cin:
        
        std::cout << " Finally I am at init request! " << std::endl;
        //getchar();

        init_fetch_next = Init_Wait_Before_Fetch_Cin;
      
      break;

      
      case Init_Wait_Before_Fetch_Cin:
 
        std::cout << " Finally I am at wait before Cin " << std::endl;
        //getchar();

        init_fetch_next = Init_Comm_Fetch_Cin;
        
      break;

      case Init_Comm_Fetch_Cin:
      
        std::cout << " Finally I am at Comm Cin " << std::endl;
        //getchar();
      
        init_fetch_next = Init_Request_B;
 
      break;

      case Init_Request_B:
       
        //check for ready signal for B
        //if (Sync_B is okay)

        std::cout << " Finally I am at Req B " << std::endl;
        //getchar();
        
        init_fetch_next = Init_Wait_Before_Fetch_B;

				/*if (InitB_Current==(Kernel_Size-1)){ //bus latency is 1
						
            init_fetch_next=Init_Wait_Before_Fetch_B;
            InitB_Next = 0;
          }*/

      break;

      case Init_Wait_Before_Fetch_B:

          init_fetch_next = Init_Comm_Fetch_B;

      break;

      case Init_Comm_Fetch_B:
      
        std::cout << " Finally I am at Fetch B " << std::endl;
        //getchar();
        
        init_fetch_next = Init_Request_A;
      
      break;

      case Init_Request_A:

        //check ready signal for A from prefetcher
        
        init_fetch_next = Init_Wait_Before_Fetch_A;

        InitA_Next = (InitA_Current+1);

				/*if (InitA_Current==(Kernel_Size*Kernel_Size/LAPU_Size)){ //bus latency is 1						
            init_fetch_next=Init_Wait_Before_Fetch_A;
            InitA_Next = 0;
        }*/
      break;

      case Init_Wait_Before_Fetch_A:

        init_fetch_next = Init_Comm_Fetch_A;

      break;

      case Init_Comm_Fetch_A:

            InitA_Next = 0;    
             
          if (from_Pref->B[MyCoreID] && from_Pref->Cin[MyCoreID] &&
              from_Pref->A[MyCoreID]){
            init_fetch_next = Init_Request_Cin;
            from_Pref->A[MyCoreID]=FALSE;
            from_Pref->Cin[MyCoreID]=FALSE;
            from_Pref->B[MyCoreID] = FALSE;
            std::cout << " init final fetch A " << std::endl;
            //getchar();
            init_fetch_next = Init_Wait_To_Compute;   

          }
            
            //Uncomment for Pref
            
            //May need to uncomment this for Pref
            
            /*init_fetch_next=Init_Idle;
            Matmul_Comm_Next= Comm_Init;
            Matmul_Next_State= Matmul_BC0;*/

            //tell the prefetcher that I already fetch a Mc*Kc of A
            //clear the ready signal now, and fetch the new A 
            
      break;

      case Init_Wait_To_Compute:
            
      //For Pref
      
        std::cout << " Finally I am at wait to compute " << std::endl;
        std::cout << "from_Pref->A " << from_Pref->A[MyCoreID] <<std::endl;
        std::cout << "from_Pref->A after " << from_Pref->A[MyCoreID] <<std::endl;
        

        std::cout << "before enter if wait to compute" << std::endl;

        if (A_per_Core>1){
          std::cout << "upper " << std::endl;
          if (from_Pref->A[MyCoreID]==TRUE){
            from_Pref->A[MyCoreID] = FALSE;
            init_fetch_next = fetch_B_test;
            std::cout << "Core : Moving from wait compute to Fetch B "<<std::endl;
            std::cout << "MyCoreID is "<<MyCoreID<<std::endl;
            getchar();
            counter_B =1;
            counter_A =2;
          }
        }

        else {    
          std::cout << "lower " << std::endl;
          init_fetch_next = fetch_B_test;
        }
      break;

      case fetch_B_test:
      
      std::cout <<"I am fetching B "<<std::endl;
      std::cout <<"Counter B is  "<< counter_B <<std::endl;
      std::cout <<"from pref->B is "<< from_Pref->B[MyCoreID] << std::endl;
      std::cout <<"MyCoreID is  "<< MyCoreID<<std::endl;

      if (comp_count==10);
      else comp_count++;
      
      if (from_Pref->B[MyCoreID]==TRUE &&comp_count==10){
          init_fetch_next = fetch_A_test;
          counter_B = (counter_B+1);
          from_Pref->B[MyCoreID]= FALSE;
          std::cout <<"Move to Fetch_A now "<<std::endl;
          std::cout <<"MyCoreID is  "<< MyCoreID<<std::endl;
          std::cout <<"Counter_B is  " << counter_B <<std::endl;
          std::cout <<"Counter_A is  " << counter_A <<std::endl;
          getchar();
          comp_count=0;
      }

      break;

      case fetch_A_test:
      
      std::cout <<"I am fetching A "<<std::endl;
      std::cout <<"Counter_A is  "<< counter_A <<std::endl;
      std::cout <<"Counter_B is  " << counter_B <<std::endl;
      std::cout <<"from pref->A is "<< from_Pref->A[MyCoreID] << std::endl;
      std::cout <<"MyCoreID is  "<< MyCoreID<<std::endl;
     
      if (comp_count==10);
      else comp_count++;
      
      if (from_Pref->A[MyCoreID]==TRUE && comp_count ==10){
       
        from_Pref->A[MyCoreID]= FALSE;
        comp_count =0;

        std::cout <<"from pref->A is "<< from_Pref->A[MyCoreID] << std::endl;
        std::cout <<"Counter_A is  "<< counter_A <<std::endl;
        std::cout <<"Counter_B is  " << counter_B <<std::endl;
        std::cout <<"MyCoreID is  "<< MyCoreID<<std::endl;
        getchar();
        
        if(A_per_Core>1){
          init_fetch_next = fetch_A_test;
          
          //if (counter_A==A_per_Core-1)
          if (counter_A==A_per_Core)
            init_fetch_next = fetch_B_test;
          
          //if(counter_B==HowManyPanel+1 && counter_A==howmanyA-1){
          if(counter_B==HowManyPanel+1 && counter_A==A_per_Core){
            init_fetch_next = Send_Cout_test;
            from_Pref->Cout[MyCoreID]=TRUE;
            counter_B=0;
            std::cout <<"Counter_A is  " << counter_A <<std::endl;
            std::cout <<"Counter_B is  " << counter_B <<std::endl;
            std::cout <<"Move to Send_Cout now "<<std::endl;
            getchar();
          }
          counter_A = (counter_A+1)%(A_per_Core+1);
          if(counter_A==0) counter_A = 1;
          //counter_A = (counter_A+1)%((A_per_Core*HowManyPanel)+1);
        }
        
        else {
          init_fetch_next= fetch_B_test;
          std::cout <<"Move to Fetch_B now "<<std::endl;

        }

      }

      break;

      case Send_Cout_test:
      
      if (comp_count==10);
      else comp_count++;

      if (from_Pref->Cout[MyCoreID]==FALSE && comp_count==10){
        
        std::cout <<"Move to Send Cin now "<<std::endl;
        std::cout <<"MyCoreID is  "<< MyCoreID<<std::endl;
        getchar();
        
        from_Pref->Cout[MyCoreID]=FALSE;
        init_fetch_next = Send_Cin_test;
        comp_count=0;
      }
      break;

      case Send_Cin_test:
     
      if (comp_count==10);
      else comp_count++;
      
      std::cout <<"I am fetching Cin "<<std::endl;
      
      if (from_Pref->Cin[MyCoreID]==TRUE && comp_count==10){
        init_fetch_next = Send_Cin_test;
       
        std::cout <<"Move to Send Cin now "<<std::endl;
        std::cout <<"MyCoreID is  "<< MyCoreID<<std::endl;
        std::cout <<"Counter_ is Cin  " << counter_Cin <<std::endl;
        getchar();
        
        if(counter_Cin==NumofPartition){
          std::cout <<" I am done " << std::endl;
          getchar();
        }
      std::cout <<"Move to Fetch_B now "<<std::endl;

        counter_Cin++;
      }

      break;

      case Init_Idle:

      break;

  }

  //Address_Gen();

}

void LAPU::GEMM_Compute(int Global_index){

  /*while(1)*/{
 
    switch (Matmul_Comp_Comm_Current){
    
      case Matmul_Executing :
        
        /*SRAM[0][0]= 1;
        std::cout << "SRAM [0][0] is " << SRAM[0][0] << std::endl;
        std::cout << "SRAM [0][1] is " << SRAM[0][1] << std::endl;*/
        
        if(Matmul_Current_State==Matmul_Idle)
          init_fetch();

        /*std::cout << "Matmul_Current_State is " << Matmul_Current_State<<std::endl;
        getchar();*/
          

      if (Print_State_Machines==1){
				
        #ifdef PRINT_DEBUG
        if((ff%FAST_FORWARD)==0)
        {
          std::cout<<"==============================";
				  std::cout<<"Cycle"<<Cycles_Passed<<std::endl;
          Dump_Matmul_SMachine();
        }
        #endif  
			}
      /////////////////////////////////////////////////////////////////////////////////////////////

        if(Matmul_Current_State==Matmul_MAC_BC || Matmul_Current_State == Matmul_BC || Matmul_Current_State==Matmul_BC0 || Matmul_Current_State ==Matmul_Flush_Cout){
          Matmul_Comm();
          Issuer_State_Machine();
          Matmul_Kernel(Global_index);
        }

        if(Matmul_Current_State==Matmul_MAC_BC) 
          if(ff%FAST_FORWARD==0)
          //getchar();
        ff++;
        
        
        PEs_Execute();

        //when it is done
        //if ((Matmul_Current_State==Matmul_Init) && done) Matmul_Comp_Comm_Next = Matmul_Ending;
        if (DONE) {
          Matmul_Comp_Comm_Next = Matmul_Ending;
          printf("move to matmul ending \n");
          printf("Matmul_Comp_Comm_Next is %d \n", Matmul_Comp_Comm_Next);
        }
   
			  Cycle();
        //for debug
        /*if(Ma_Counter_Curr==0 && Kc_Counter_Curr==0 && N_Counter_Curr==0 && BigA_Current == A_per_Core &&
            BigC_Current!=0)
          Dump_SRAM(0);*/

      if ((Cycles_Passed%Checkpoint)==0 && MyCoreID==0){
         
          if (Cycles_Passed>=10000) Dump_Matmul_SMachine();
        //Multicore::dump();
        //test
         if (Cycles_Passed==80000) {
           //Dump_SRAM(0);
           //exit(0); 
         }
      }
      break;
      
      case Matmul_Ending :

          printf("at matmul ending \n");
        //return 0;

      break;

    }
  
  
  }

}

void LAPU::GEMM_Test_Pref(){

  /*while(1)*/{
 
    switch (Matmul_Comp_Comm_Current){
    
      case Matmul_Executing :
        
        /*SRAM[0][0]= 1;
        std::cout << "SRAM [0][0] is " << SRAM[0][0] << std::endl;
        std::cout << "SRAM [0][1] is " << SRAM[0][1] << std::endl;*/
        int test = TRUE;

        if(Matmul_Current_State==Matmul_Idle)
          init_fetch_test();

        Cycle();
    }
  }
}

#if 0
void Clear_SRAM_C(){

  std::cout << "Clearing SRAM " << std::endl;
  for(int i=0; i<Panel_Size*Panel_Size; i++){
    SRAM[i]=i%(Panel_Size*Kernel_Size);
  }

}
#endif

void LAPU::Dump_SRAM(int type){

  int start, end;

  std::cout << "Dumping SRAM from PE " << std::endl;


  if(type==0){      
    start = 0;
    end = Panel_Size * Panel_Size;
  }

  else if(type==1){ 
    start = Panel_Size*Panel_Size;
    end = Panel_Size * Panel_Size + 2*Kernel_H*Panel_Size;
  }
  
  else{ 
    start = Panel_Size*Panel_Size + 2*Kernel_H*Panel_Size;
    end = Panel_Size * Panel_Size + 2*Kernel_H*Panel_Size + NumofCore*Kernel_V*Kernel_H;
  }


  for (int i=start; i<end; i++)
    printf("SRAMCORE[%d] = %lf\n", i, SRAM[i]);

#if 0  
  for (int i=start; i<end; i++){
    printf("C[%d][%d] = %lf \n", (i/Panel_V), i%Panel_V, SRAM[((i/Panel_V) + (i%Panel_V)*Panel_H)] );

  }
#endif

#if 0
  for(int i=start; i<end; i++)
    //std::cout << "SRAM["<<i<<"]"<< " = "<< SRAM[i/Panel_V + (i%Panel_V)*Panel_H]<<std::endl;
    std::cout << "C["<<(i/Panel_V) <<"]"<<"["<<(i%Panel_V)<< "]"<< " = "<< SRAM[i/Panel_V + (i%Panel_V)*Panel_H]<<std::endl;
#endif

}
