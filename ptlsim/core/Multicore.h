 /*
 * LAPU.h
 *
 *  Created on: Mar 4, 2010
 *      Author: ardavan
 */


/*#ifndef LAPU_H_
#define LAPU_H_*/

#include "Parameters.h"
#include "LAPU.h"
#include "prefetcher.h"
#include "sram.h"
#include "sram_pref.h"
#include "dram.h"
//#include "data_structure.h"

//namespace MC {
class Multicore
{
public:
	Multicore();
	virtual ~Multicore();
  int Execute();
  static int dump();

  int Assign_input_Matrix( double **& matrix_A, double **& matrix_B, double **& matrix_C);

private:

  int PORT;
  int *Current_Arbiter;
  int *Next_Arbiter;
  int *Current_PORT;
  int *Next_PORT;
  int total_cycles;
  LAPU ** CORES;
	
  double **Matrix_A;
	double **Matrix_B;
	double **Matrix_C;

  S_RAM **Sram;
  PF    *Pref;
  LS_Buffer *LAP_Buf;
  S_RAM_PREF *Sram_Pref;
  PF_Buffer *PF_Buf;
  dram *Dram;
  LAP_Package *lap_req; 
  PREF_Package *pref_req; 
  DRAM_Package *pref_dram_req;
  PF_DRAM_Buffer *PF_DRAM_Buf;
  LAP_PREF_Sync *lap_pref;  

};

//}
//#endif /* LAPU_H_ */
