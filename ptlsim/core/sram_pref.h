#include "Parameters.h"

//using namespace std;

class S_RAM_PREF{

  public :
    S_RAM_PREF(PREF_Package *& Package, PF_Buffer *&buf);
    void assign_latency();
    void update_latency();
    void run_every_cycle();
    void Send_to_Pref();

  private :
    PREF_Package *from_PREF;

    std::deque<PREF_Package> PREF_In_FIFO;
    std::deque<PREF_Package> PREF_Out_FIFO;
    PF_Buffer * Buffer;
    //double Buffer[LAPU_Size][LAPU_Size];
    //bool Buffer_Ready;
};

