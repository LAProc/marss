#ifndef DATA_STRUCTURE_H
#define DATA_STRUCTURE_H

//#include<vector>
//using namespace std;
//using std::vector;
//#include <deque>

typedef struct lap_to_IO{
  
  bool req;
  int req_addresses[LAPU_Size];
  double data[LAPU_Size];
  bool WE;
  //need to pass stall and Kc by pointer at constructor
  int type;
}lap_IO;

typedef struct lap_pref{

  bool Cin[NumofCore];
  bool Cout[NumofCore];
  bool C_All_Ready;
  bool B[NumofCore];
  int currentB;
  bool A[NumofCore];

}LAP_PREF_Sync;


typedef struct lap_package{//this is from I/O cont to SRAM
    bool req; //if there is a request
    //bool packed;  //if the request is packed
    int addresses[LAPU_Size];
    double data[LAPU_Size];
    bool WE;
    int Offset;
    int latency;
    bool Serviced;
    int type;
}LAP_Package; 


typedef struct lapu_sram_read_buf{
    bool buf_ready;
    int type;
    std::deque <double> data;
}LAP_Read_Buff;

typedef struct lapu_sram_write_buf{
    bool ready;
    std::deque <double > data;
    bool packed;
    int addresses[LAPU_Size*LAPU_Size];
}LAP_Write_Buff;


typedef struct pref_package{
    bool req; //if there is a request
    bool packed;  //if the request is packed
    int addresses[Port_Bandwidth/Element_Size];
    double data[Port_Bandwidth/Element_Size];
    bool WE;
    bool fetch_done;
    bool get_port;
    bool buffer_ready;
    int latency;
    bool Serviced;
    bool res;
    int res_amnt;
}PREF_Package;

typedef struct buffer{
  int type;
  std::deque <double> data;
  bool buf_ready;
}LS_Buffer;

typedef struct pf_buffer{
  std::deque <double> buf;
  bool buf_ready;
}PF_Buffer;

typedef struct pref_dram_package{
    bool req; //if there is a request
    bool packed;  //if the request is packed
    int addresses[Cache_Line/Element_Size];
    unsigned long long phys_addresses[Cache_Line/Element_Size];
    //W64 phys_addresses[Cache_Line/Element_Size];
    unsigned long long virt_addresses[Cache_Line/Element_Size];
    int sram_addr[Cache_Line/Element_Size];
    //deque <int> addresses;
    std::deque <double> data;
    //double data [Cache_Line/Element_Size];
    bool WE;
    bool fetch_done;
    bool last;
    int last_count;
    int last_addr;
    bool get_port;
    bool buffer_ready;
    int latency;
    bool Serviced;
    bool ready;
    bool Part_Line;
    int  Part_Frac;
}DRAM_Package;

typedef struct pf_sram{
  //double  buf[Cache_Line/Element_Size];
  double data;
  bool Serviced;
}pf_to_sram;

typedef struct pf_dram_buffer{
  //double  buf[Cache_Line/Element_Size];
  bool buf_ready;
  std::deque <bool> last;
  std::deque <double>  buf;
  std::deque <int>  addr;  //address for SRAM
  std::deque <double> data; // for Marss
  std::deque <int> last_issue_count;
  std::deque <int> last_issue_addr;
}PF_DRAM_Buffer;


#endif
