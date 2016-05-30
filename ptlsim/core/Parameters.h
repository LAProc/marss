/*
 * Parameters.h
 *
 *  Created on: Mar 5, 2010
 *      Author: ardavan
 */

#ifndef PARAMETERS_H_
#define PARAMETERS_H_
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
/*#include <math.h>
#include <time.h>*/
#include <vector>
#include <deque>

#define COL_MAJOR
#define SUBSTRACT

extern int LAST_STORE;
extern int ITER_COUNT;
extern bool NON_PART;
extern int MC ;
extern int NC ;
extern int LC ;
extern int KC ;
extern int K_V ;
extern int K_H ;
extern int LDA ;
extern int LDB ;
extern int LDC ;

extern int Panel_Size;
extern int Kernel_Size;
extern int howmanyA;
extern int HowManyPanel;

extern int Kernel_V;
extern int Kernel_H;
extern int Panel_V;
extern int Panel_H;

extern int Mem_Size;
extern int Mem_Size_A;
extern int Mem_Size_B1;
extern int Mem_Size_B2;
extern int NumofPartition;
extern int SRAM_Size;
extern int SRAM_OFFSET;

//#define ORIG
//#define NON_PART

#define FALSE 0
#define TRUE 1

#define N_MAX 128

#if 0
#define MC 160
#define NC 160
#define LC 32
#define KC 32
#define K_V 40
#define K_H KC
#define LDC MC
#endif

//#define PRINT_DEBUG
//#define HALFWORD

#define Print_Input_Output 0  // IF you want in and out set it to 1
#define Print_State_Machines 1
#define Print_BLAS 0

#define N_SIZE    512

#define NumofCore 1 

#if 0
#define Panel_Size   (MC/2)   // n in paper
#define Kernel_Size  32 // m_c or k_c in paper
#define howmanyA    Panel_Size/K_V      // how many As we are computing
#endif

#define NumofA      2       // to decide the size of local memory for A
#define LAPU_Size 4			// n_r  in paper
#define FMA_Latency  	7	 //cycles
#define Multiplication_Latency 7 		//cycles
#define Addition_Latency 7		//cycles
#define InvSqrt_Latency 15	//cycles
#define NumofMemB 2

#define MAX_REQUEST 1

//SRAM Parameters
#define Port_Width 256    //in bits
#define Line_Buffer_WH 4
#define BitsPerOneFetch 256
#define Request_Cycle_Time 1
#define Access_Time 2

#define Port_Bandwidth_Core 32 //in bytes

#define Port_Bandwidth 32 //in bytes


#define NumberofFetch (Line_Buffer_WH * BitsPerOneFetch / Port_Width )

#define LATENCY_RANGE (Access_Time + (Request_Cycle_Time * NumberofFetch) - 1)
#define SRAM_Latency 1 

//#define Mem_Size  (Kernel_Size*Kernel_Size)/(LAPU_Size*LAPU_Size)//SRAM of the PE size  for Cholesky
 //#define Mem_Size (((Kernel_Size*Kernel_Size)/(LAPU_Size*LAPU_Size)) + 2*Kernel_Size)
 
//#define Mem_Size (((Kernel_Size*Kernel_Size)/(LAPU_Size*LAPU_Size)) + 2*Kernel_Size)
// m_c*k_c + 2*k_c*n_r*n_r


//added by Mochamad
#if 0
#define Mem_Size_A ((K_V*K_H)/(LAPU_Size*LAPU_Size))
#define Mem_Size_B1 K_H //should add B2 also ?
#define Mem_Size_B2 K_H //should add B2 also ?
#endif

#define MAX(A,B) ((A) > (B) ? (A) : (B))


#define Scratch_Size 4 //Scratch Pad memory

//#define Memory_BW 8*( (2*LAPU_Size*LAPU_Size) /Kernel_Size)
#define Memory_BW 8*( (2*LAPU_Size*LAPU_Size) /4)

// Counters

//Power
#define FMA_Dynamic  1
#define FMA_Leakage  1
#define FAST_FORWARD  1
#define Cache_Line 64   //in byte
#define Element_Size 8  //in byte

#define InvSqrt_Dynamic 1
#define SRAM_Freq 2
#define DRAM_Freq 1;

#define DRAM_Latency 150;
enum Req_type {NONE, CIN, B, A, A_Pending, COUT};

#define Bus_Dynamic     1
#define Checkpoint 100000000

#include "data_structure.h"

enum LAPU_Function { LAPU_Cholesky, LAPU_Trsm, LAPU_Rank_Update};
enum ALU_op {ALU_Add,ALU_Mul, ALU_MAD, ALU_MAC};

//extern double SRAM[Panel_Size + Kernel_Size*4][Panel_Size];
//#define SRAM_Size (Panel_Size*Panel_Size) + (NumofCore*Kernel_Size*Kernel_Size) + (2*Kernel_Size*Panel_Size)

//#define SRAM_Size (Panel_Size*Panel_Size) + (2*Kernel_Size*Panel_Size) + (2*Kernel_Size*Panel_Size)

//For API Call
//#define SRAM_Size ((N_Size*N_Size)/4) + (2*KC*(MC/2)) + (KC*K_V)

extern double * SRAM;
//extern double SRAM[512];

#if 0

#ifdef NON_PART 
  #define NumofPartition 1
#else 
  #define NumofPartition 4
#endif

#endif 

#define DRAM_Size N_MAX*N_MAX*3
//#define HowManyPanel Panel_Size*2/Kernel_Size //assuming that the matrix is squarely divided


// for the API call


//assuming that it is squarely divided !!
//For the function Call, HowManyPanel should be only 1 !!

extern double DRAM[DRAM_Size];
// this means sizeof(C) + sizeof (B) + sizeof(A)

extern bool Pref_Ready_C[NumofCore];
extern bool Pref_Ready_A[NumofCore];
extern bool Pref_Ready_B;


//Kernel_Size*4 because we have A0, A1, B0, B1 in the SRAM

#endif /* PARAMETERS_H_ */
