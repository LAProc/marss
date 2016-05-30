#include "Parameters.h"

//using namespace std;

class S_RAM{

  public :
    S_RAM(LAP_Package &LAP, LS_Buffer &buf, int ID);
    void assign_latency();
    void update_latency();
    void run_every_cycle();
    void Send_to_LAPU();

  private :
    LAP_Package *from_LAP;

    std::deque<LAP_Package> LAP_In_FIFO;
    std::deque<LAP_Package> LAP_Out_FIFO;
    //double Buffer[LAPU_Size][LAPU_Size];
    //bool Buffer_Ready;
    LS_Buffer *Buffer;
    int MyCoreID;
};

