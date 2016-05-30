 /*
 * LAPU.h
 *
 *  Created on: Mar 4, 2010
 *      Author: ardavan
 */


#ifndef LAPU_H_
#define LAPU_H_

#include "Parameters.h"
#include "Inv_Sqrt.h"
#include "PE.h"
#include "IO.h"
#include "newIO.h"
//#include "data_structure.h"
//#include "Multicore.h"

#define leha

class LAPU
{
public:
	LAPU(int MyCoreID, int *& Current_PORT, int *& Next_PORT, int *&Arbit_Current, int *&Arbit_Next, LAP_Package &Package, LS_Buffer &Buf, LAP_PREF_Sync *&from_pref );
	//LAPU(int MyCoreID, LAP_Package *&Package);
	virtual ~LAPU();

	int Rank_D_Update(int Global_index, int Trsm_index);
	int Trsm(int Global_index, int Trsm_index);
	int Cholesky(int Global_index);

	int Matmul_Kernel(int Global_index);

	int Cycle();
	int Return_Cycle_Count();
	int Return_LAPU_Power();
  void Address_Gen();
  int PEs_Execute();
  void Dump_SRAM(int type);
  void Reset();

/*	int Execute( LAPU_Function Function, int Global_index, int Trsm_index);
	int Cycle();
*/
  void GEMM_Test_Pref();
  void init_fetch_test();

  int Issuer_State_Machine();
  int print_matrix(int rows, int columns, double ** matrix_A);
	int Drive_Buses();

	int Initialize_Mem( double ** Input_matrix, int row_number, int column_number, int offset);

	int Flush_Mem(double **& Input_matrix, int row_number, int column_number, int offset); //TODO who makes decision about the offset/?

	int Initialize_Mem_New( double ** Input_matrix, int row_number, int column_number, int offset, char matr);

	int Flush_Mem_New(double **& Input_matrix, int row_number, int column_number, int offset, char matr); //TODO who makes decision about the offset/?
	
  int Assign_input_Matrix( double **& matrix_A, double **& matrix_B, double **& matrix_C);

	int Dump_PE_Registers(int row, int column);

	int Dump_All_PE_Registers();

	int Dump_PE_ALU(int row, int column, ALU_op operation_type);

	int Dump_All_PE_ALUs(ALU_op operation_type);


	int Dump_Row_Buses();
	int Dump_Column_Buses();

	int Dump_Trsm_SMachine();
	int Dump_Chol_SMachine();
	int Dump_Gemm_SMachine();
	int Dump_Matmul_SMachine();

	int Dump_Sqrt_Unit();

  void GEMM_Compute(int Global_index);
  void Matmul_Comm();

  int GetKernelStatus();
  int PrintStall();
  void init_fetch();

private:

	enum Chol_States {Chol_Initial, Chol_Feed_Sqrt, Chol_Inv_Sqrt, Chol_BC_InvSqrt,
					  Chol_Multiply, Chol_BC_Mul, Chol_Rank1_Update, Chol_End} Chol_Current_State,Chol_Next_State;


	enum Gemm_States{Gemm_Initial,Gemm_Pre_Fetch, Gemm_BC, Gemm_MAC_BC, Gemm_MAC, Gemm_End} Gemm_Current_State, Gemm_Next_State;

	enum Matmul_States { Matmul_Init, Matmul_FetchB, Matmul_FetchA, Matmul_BC0, Matmul_BC, Matmul_MAC_BC, Matmul_MAC_Flush, Matmul_Flush_Cout, Matmul_End,  Matmul_Idle} Matmul_Current_State, Matmul_Next_State;
	
  enum Init_States { Init_Wait_Ready,Init_Request_Cin,Init_Wait_Before_Fetch_Cin, Init_Comm_Fetch_Cin, Init_Request_B, Init_Wait_Before_Fetch_B, Init_Comm_Fetch_B, Init_Request_A, Init_Wait_Before_Fetch_A, Init_Comm_Fetch_A, Init_Idle, Init_Wait_To_Compute, fetch_B_test, fetch_A_test, Send_Cin_test, Send_Cout_test,Req_Cin_Intersection} init_fetch_current, init_fetch_next;

	enum Matmul_Comm_States {Comm_Init, Comm_Get_Port, Comm_Request_Cin, Comm_Fetch_Cin,
  Comm_Send_Cout, Comm_Fetch_B, Comm_Fetch_A,
  Comm_Request_Cout, Comm_Evaluate, Comm_Request_B, Comm_Request_A,
  Wait_Before_Fetch_Cin, Wait_Before_Fetch_B, Wait_Before_Fetch_A, Comm_Cout_Delay, Comm_Wait_to_Flush_C, Comm_Wait_for_Csync, Wait_for_Pref, Comm_Idle
  } 
  Matmul_Comm_Current, Matmul_Comm_Next;
	
  enum Matmul_Comp_Comm { Matmul_Executing, Matmul_Ending} Matmul_Comp_Comm_Current, Matmul_Comp_Comm_Next;
	
  enum Trsm_States{Trsm_Initial, Trsm_BC_InvSqrt, Trsm_Multiply, Trsm_BC_Mult, Trsm_Partial_Rank_1,Trsm_Trans, Trsm_End} Trsm_Current_State, Trsm_Next_State;


//	enum Routine_Type{Cholesky_R,Gemm_R, Trsm_R} Current_Routine;

	//TODO do I need different State enums for different types of operatons?
	int Size;
	int i,j;
	PE  ** PE_Array;

	double ** Matrix_A;
	double ** Matrix_B;
	double ** Matrix_C;

	int Counter_Curr;
	int Counter_Next;

	int Latency_Counter_Curr;
	int Latency_Counter_Next;
 // Added for Matmul

	int Mc_Counter_Curr;
	int Mc_Counter_Next;

	int Kc_Counter_Curr;
	int Kc_Counter_Next;

	int N_Counter_Curr;
	int N_Counter_Next;

  int Ma_Counter_Curr;
  int Ma_Counter_Next;

  int address_x;
  int address_y;

  bool Enough_A;
  bool Enough_B;
  int Fetch_Done;
  int IO_Ready;
  int next_request;

  int Last_Sending;
  bool done;

///
  int Req_Matrix;  // dont forget to put it in the class declaration
  int Buffer_Ready;
  bool IgetPort;
  int Mc_Comm_Cin_Current;
  int Mc_Comm_Cin_Next;
  int Mc_Comm_Cout_Current;
  int Mc_Comm_Cout_Next;
  int Mc_Comm_B;
  int Mc_Comm_A; 
  int Send_Done;
  int EnoughB ;
  int EnoughA ;

  int Ma_Comm_Cin;
  int Ma_Comm_Cout;

  int InitB_Next;
  int InitB_Current;
  
  int InitA_Next;
  int InitA_Current;

  int BigC_Current;
  int BigC_Next;
  
  int BigA_Current;
  int BigA_Next;

  int BigB_Current;
  int BigB_Next;
  
  int Mc_Fetch_A;
  int Ma_Fetch_A;
  int N_Fetch_A;
  bool FULL_A;
  bool FULL_B;

  int *PORT_Now;
  int *ARBIT_Now;
  int MyCoreID;

  int N_Comm_B;
  int N_Comm_A;
  int selector;

  int counter;
  int ff;   //for fast forward

  int fetchedA;
  int fetchedB;

  bool Stall;
  long long int stall_A;
  long long int stall_B;
  long long int stall_C;
  long long int stall_C_inter;
  
  int *req_addresses;
  bool Issue_Request_LS;
  bool pack_request;

  int row_offset;
  int req_counter;
  int intracore_offset;
  int intercore_offset;
  int base_addr;
  int y_offset;
  int x_offset;
  int afterC_offset;
  int afterB_offset;
  int next_line_offset;
  int N_Comm_Cin;
  int N_Comm_Bin;
  int N_Comm_Cout;
  int WE;
  int packed_req;
  int Chunk_B;
  int Chunk_A;
  int x_comm_cin;
  int x_comm_cout;
  int first_cin;
  int first_cout;
  
  int Comm_Go_C_Again;

  int A_per_Core;
  int Residue;
  
  int b_counter;
  int y_a_counter;
  int x_b_counter;

	bool State_Start;

	unsigned long long int Cycles_Passed;

	double * Row_Buses_Write;
	double * Row_Buses_Read;

	double * Column_Buses_Write;
	double * Column_Buses_Read;

	Inv_Sqrt * Sqrt_Unit;
	IO * Mem_IF;
  newIO * newMEM_IF;
  
  LAP_Package *req_package;
  LAP_PREF_Sync *from_Pref;

  int counter_A, counter_B, counter_Cin;
  int comp_count;
  bool inter_done;

  unsigned long long int Stall_Init;
  unsigned long long int Stall_Comm;
  unsigned long long int Cin_Intersection;
  bool DONE;

	int LAPU_Power_Consumed;
  bool NO_PART ;
  int Kernel_V;
  int Kernel_H;
  
  int FETCH_A_AMOUNT;

};

#endif /* LAPU_H_ */
