/*
 * LAPU.cpp
 *
 *  Created on: Mar 4, 2010
 *      Author: ardavan
 */

#include "Multicore.h"
//#include "data_structure.h"

//using namespace MC;

Multicore::Multicore()
{


  //CORES = (LAPU *) malloc (sizeof(LAPU) * (NumofCore));
  
  CORES = new LAPU *[NumofCore];
  Sram =  new S_RAM *[NumofCore];

  Pref = (PF *) malloc (sizeof(PF));
  //Sram = (S_RAM *) malloc (sizeof(S_RAM) *(NumofCore)); 
  
  Sram_Pref = (S_RAM_PREF *) malloc (sizeof(S_RAM_PREF));

  Current_PORT = (int *) malloc(sizeof(int));
  Next_PORT = (int *) malloc(sizeof(int));

  Current_Arbiter = (int *) malloc(sizeof(int));
  Next_Arbiter = (int *) malloc(sizeof(int));
  
  *Current_PORT =1;
  *Next_PORT = 1;


  *Current_Arbiter = 0;  //This means Core 0 has the port now.
  *Next_Arbiter = 0;
	

  // LAP_Buf = (LS_Buffer *) malloc(sizeof(LS_Buffer)*(NumofCore));

  lap_pref = new LAP_PREF_Sync;

  LAP_Buf = new LS_Buffer[NumofCore];
  
  //PF_Buf = (PF_Buffer *) malloc(sizeof(PF_Buffer));

  //struct rec *ptr_one;
 // lap_req =(LAP_Package *) malloc (sizeof(LAP_Package)*NumofCore);
  lap_req = new LAP_Package[NumofCore];
  

  //pref_req =(PREF_Package *) malloc (sizeof(PREF_Package));
  
  //pref_req =(PREF_Package *) malloc (sizeof(PREF_Package));
  PF_Buf = new PF_Buffer;
  pref_req = new PREF_Package; 
  
  pref_dram_req = new DRAM_Package;

  //debugging 
  /*std::cout <<"pref_dram_req->WE " << pref_dram_req->WE << std::endl;

  pref_dram_req->data.push_back(100);

  std::cout <<"pref_dram_req->data(0) " << pref_dram_req->data.at(0) << std::endl;
  std::cout <<"pref_dram_req->data.size() " << pref_dram_req->data.size() << std::endl;
  getchar();*/


  //PF_DRAM_Buf =  (PF_DRAM_Buffer *) malloc(sizeof(PF_DRAM_Buffer));
  PF_DRAM_Buf =  new PF_DRAM_Buffer;


  for (int ID=0; ID<NumofCore; ID++){
    CORES[ID] = new LAPU(ID, Current_PORT, Next_PORT, Current_Arbiter, Next_Arbiter, lap_req[ID], LAP_Buf[ID], lap_pref);
    Sram[ID] =  new S_RAM(lap_req[ID],LAP_Buf[ID], ID);

    lap_pref->B[ID]=FALSE;
    lap_pref->Cin[ID]=FALSE; 
    lap_pref->Cout[ID]=FALSE;
    lap_pref->A[ID]=FALSE;
    lap_pref->currentB=FALSE;
    //CORES[ID] = *new LAPU(ID, Current_PORT, Next_PORT, Current_Arbiter, Next_Arbiter, lap_req[ID]);
    //CORES[ID] = *new LAPU(ID, lap_req[ID]);
  }
  
  Sram_Pref = new S_RAM_PREF(pref_req, PF_Buf);
  Pref = new PF(pref_req, PF_Buf ,pref_dram_req, PF_DRAM_Buf, lap_pref);
  Dram = new dram(pref_dram_req, PF_DRAM_Buf);
}


Multicore::~Multicore()
{
	// TODO Auto-generated destructor stub
}

int Multicore::Assign_input_Matrix( double **& matrix_A, double **& matrix_B, double **& matrix_C){

	Matrix_A=matrix_A;
	Matrix_B=matrix_B;
	Matrix_C=matrix_C;

  for (int i = 0; i<NumofCore; i++)
	  (CORES[i])->Assign_input_Matrix(matrix_A, matrix_B, matrix_C);
}

int Multicore::dump(){

  std::cout << "calling multiple core dump"<<std::endl;
  getchar();

}

int Multicore::Execute(){

  int finish=-1;

  while (1){
  
    for (int i=0; i<NumofCore; i++){
      
      if (CORES[i]->GetKernelStatus()==FALSE) 
          CORES[i]->GEMM_Compute(0);
          //CORES[i]-> GEMM_Test_Pref();

      else 
        finish = i; //notice which core has finished
   }
  
    Pref->run_every_cycle();
    Dram->run_every_cycle();

    for (int j=0; j<NumofCore; j++){
      Sram[j]->run_every_cycle();
    } //Something did not match in Sram module

    Sram_Pref->run_every_cycle();


    //
   /* PF_IF->run_every_cycle();

    for (int i=0; i<NumofCore; i++){
      cout_status[i].Cout_Ready=CORES[i].Cout_Ready; 
    }
    
    SRAM_Controller(cout_status);

    //These two from LAPU should be stated here
    
    //for every LAPU
    drive_bus();
    cycle();*/

    
    //Update Arbitration
    *Next_Arbiter = (finish==0)?(*Next_Arbiter=1): (finish==1)? 
      (*Next_Arbiter=0): *Next_Arbiter;
    
    *Current_Arbiter = *Next_Arbiter;
    *Current_PORT = *Next_PORT;

    /*std::cout<<"Next_Arbiter is " << *Next_Arbiter << std::endl;
    getchar();*/

 


    int DONE = -NumofCore;

    
    for(int j=0; j<NumofCore; j++){
      if (CORES[j]->GetKernelStatus()) DONE++;
    }

    if (DONE==0){

      int k = 0;
      
      for(k=0; k<NumofCore; k++){

        total_cycles = MAX(total_cycles, CORES[k]->Return_Cycle_Count());
        CORES[k]->PrintStall();
      }
      
      break;

    }



  }

  return total_cycles;

}

