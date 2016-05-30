#include "Parameters.h"
//#include <dram.h>
//#include <sram.h>
//#define DEBUG

class PF{

  public :

  PF(PREF_Package *& Package, PF_Buffer *&Buf, DRAM_Package *&req_dram,
      PF_DRAM_Buffer *&buf, LAP_PREF_Sync *& from_lap); //constructor

  void PF_SM_Issuer_DRAM();
  void PF_SM_SRAM();
  void Address_Gen_DRAM();
  void Address_Gen_SRAM();

  void Req_to_DRAM();   //put request to DRAM
  void Req_to_SRAM();   //put request to DRAM
  void Get_Cache_Line();
  void Write_to_SRAM();
  void Write_to_DRAM();
  void Update_SM();
  void Dump_PF_SMachine();
  void run_every_cycle();
  void Track_SRAM(int addr);


  private :
  
  int x_offset;
  int y_offset;
  int offset_C;
  int offset_B;


  int Chunk_B_Next;
  int Chunk_B_Curr;
  int Chunk_A_Next;
  int Chunk_A_Curr;
  //bool Req_to_DRAM;
  //bool Req_to_SRAM;
  bool WE_DRAM;
  bool WE_SRAM;
  bool DRAM_IN_DATA;
  bool DRAM_OUT_DATA;
  bool SRAM_IN_DATA;
  bool SRAM_OUT_DATA;
  bool ID_To_SRAM;

  typedef struct{
    int Addr;
    double Data[LAPU_Size];
    bool WE;
    int ID;
    bool Serviced;
  }DRAM_req_list;

  typedef struct{
    int Addr;
    double Data[LAPU_Size];
    bool WE;
  }SRAM_req_list;
  
  PREF_Package *sram_package;
  PF_Buffer *Buffer;
  
  DRAM_Package *dram_package ;
  PF_DRAM_Buffer *Buf_DRAM;
  
  bool Issue_Request_SRAM;
  bool Issue_Request_DRAM;
  bool Write_SRAM;
  bool Write_DRAM;
  int Address_SRAM;
  int Address_DRAM;

  enum Prefetch_SRAM {Send_C_SRAM, Send_B_SRAM, Send_A_SRAM, Read_C_SRAM,
                      Send_B_Init_SRAM, Idle} SRAM_Current, SRAM_Next;
  
  enum Prefetch_DRAM {Pref_C, Pref_B, Pref_A, Write_C, Pref_B_Init, Pref_A_Init,
                      Pref_Wait_for_Fetch_A_Again_Init, Wait, Pref_A_Init_Second} Pref_DRAM_Current, Pref_DRAM_Next;

  int Sent_C_Current;
  int Sent_C_Next;
  int Sent_C_Counter_Current;
  int Sent_C_Counter_Next;
  int Sent_B_Current;
  int Sent_B_Next;
  int Sent_B_Counter_Current;
  int Sent_B_Counter_Next;
  int Sent_A_Current;
  int Sent_A_Next;
  int Sent_A_Counter_Current;
  int Sent_A_Counter_Next;
  int Read_C_Current;
  int Read_C_Next;
  int Read_C_Counter_Current;
  int Read_C_Counter_Next;

  int SRAM_POS_Current;
  int SRAM_POS_Next;

  int IssueC_Current;
  int IssueC_Next;
  
  int IssueB_Current;
  int IssueB_Next;

  int IssueA_Current;
  int IssueA_Next;

  int BigC_Current;
  int BigC_Next;

  int BigB_Current;
  int BigB_Next;

  int BigA_Current;
  int BigA_Next;

  int WriteC_Current;
  int WriteC_Next;

  int done;
  int cout_ready;
  int count_cout;
  int cout_offset;
  int write_ready;

  int per_core_A_DRAM;
  int per_core_A_SRAM;
  int per_core_A_temp;
  int per_core_A;
  
  bool B_SRAM_Done;
  bool A_SRAM_Done;
  bool A_SRAM_Init;
  int A_SRAM_Unit;
  bool all_B_Ready;

  int wait_next_C;
  int fetched_Cin[NumofCore];
  int fetched_Cin_total;
  int fetched_B;
  int fetched_A[NumofCore];
  int fetched_A_total;

  int which_matrix;
  int last_A_Init;

  LAP_PREF_Sync *to_LS;

  int next_a;
  int A_Current;
  int A_Next;
  int counter_A;

  //SRAM * SRAM_IF;

};
