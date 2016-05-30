#include "Parameters.h"


class dram{

  public :
    dram(DRAM_Package *&req_dram, PF_DRAM_Buffer *&buf);
    void assign_latency();
    void update_latency();
    void Send_to_Prefetcher();
    void run_every_cycle();
      
  private :
 
  int dram_cycle;
  DRAM_Package *dram_package;
  PF_DRAM_Buffer *Buffer;
  std::deque<DRAM_Package> DRAM_In_FIFO;
  std::deque<DRAM_Package> DRAM_Out_FIFO;

};
