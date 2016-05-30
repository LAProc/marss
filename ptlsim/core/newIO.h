/*
 * IO.h
 *
 *  Created on: May 21, 2010
 *      Author: ardavan
 */

//#ifndef IO_H_
//#define IO_H_
#include "Parameters.h"
#include "RandomBehav.h"
class newIO
{
public:
	//newIO();
  newIO(double *& Row_Buses_Write,double *& Row_Buses_Read,double *& Column_Buses_Write,double *& Column_Buses_Read, int *&Current_PORT, int *&Next_PORT, int *&Current_Arbiter, int *&Next_Arbiter, int MyCoreID, LS_Buffer &Buf, int &Kc_Counter, bool &stall);
	//newIO(double *& Row_Buses_Write,double *& Row_Buses_Read,double *& Column_Buses_Write,double *& Column_Buses_Read);

	int Assign_input_Matrix( double **& matrix_A, double **& matrix_B, double **& matrix_C);
  void newIO_Execute(int Req_Matrix, int Mc_Comm_Cin,int Mc_Comm_Cout, int Mc_Comm_B, int Mc_Fetch_a, int N_Fetch_A, int N_Comm_B, int Ma_Fetch_A, int stall, int N, int Mc, int Kc, int Ma);
  int CheckIO_Ready(int whatMatrix);
  int GetPort();
  int Check_Buffer_Ready(int whatMatrix);
  int isFetchDone(int whatMatrix);
  int NotifyStore();
  int UnablePort();
  void ServiceRequest(LAP_Package *&Package);
  void Reset();

	virtual ~newIO();

private:


	int data_transfer_total;
  

	double * Read_Col_Buses , * Read_Row_Buses;
	double * Write_Col_Buses, * Write_Row_Buses;


	double ** Matrix_A;
	double ** Matrix_B;
	double ** Matrix_C;

	double    Buffer_A[LAPU_Size][LAPU_Size];
	double    Buffer_B[LAPU_Size][LAPU_Size];
	double    Buffer_Cin[LAPU_Size][LAPU_Size];
	double    Buffer_Cout[LAPU_Size][LAPU_Size];
	
  int    Buffer_A_Latency[LAPU_Size][LAPU_Size];
	int    Buffer_B_Latency[LAPU_Size][LAPU_Size];
	int    Buffer_Cin_Latency[LAPU_Size][LAPU_Size];
	int    Buffer_Cout_Latency[LAPU_Size][LAPU_Size];

  int Buffer_A_Ready;
  int Buffer_B_Ready;
  int Buffer_Cin_Ready;
  int Buffer_Cout_Ready;
  int Buffer_Ready;
  int bus_Done;
  int Write_Now;
  int bus_Counter;
  int wr_bus_Counter;


  int request_count_Cin;
  int request_count_Cout;
  int request_count_B;
  int request_count_A;

	int Cin_Counter;
	int Cout_Counter;
	int Bin_Counter;
  int Ain_Counter;
	
  int Cin_Done;
	int Cout_Done;
	int Bin_Done;
  int Ain_Done;

  //added by Mochamad
	int Done;
  int last_x;
  int last_y;
  int FcA;
  
  int ReadNow;

  int *Kc;
  bool * Stall;

  Randomize * RAND_IF;
  LS_Buffer *from_SRAM;

};

//#endif /* IO_H_ */
