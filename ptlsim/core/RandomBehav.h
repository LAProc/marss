#include "Parameters.h"

class Randomize
{
  public :
  //Randomize();
  Randomize(int *&Current_PORT, int *&Next_PORT, int *&Current_Arbiter, int *&Next_Arbiter, int MyCoreID);

    int GetPort();

   //template <size_t rows, size_t cols>
  
    void assign_latency(double (&Matrix) [LAPU_Size][LAPU_Size], 
                  int (&Latency)[LAPU_Size][LAPU_Size], int &Ready);
  
    void update_latency(int Type, double (&Matrix)[LAPU_Size][LAPU_Size], 
                  int (&Latency)[LAPU_Size][LAPU_Size], int &Ready, int Mc, int Ma, int N, int Kc, int Mc_Comm_Cin, int Mc_Comm_B, int Mc_Fetch_A, int N_Fetch_A, int N_Comm_B, int Ma_Fetch_A, int Stall);
   
    void update_latency_write(int Type, double (&Matrix)[LAPU_Size][LAPU_Size], 
                  int (&Latency)[LAPU_Size][LAPU_Size], int &Ready, int Mc, int Ma, int N, int Kc, int Mc_Comm_Cout, int Mc_Comm_B, int Mc_Fetch_A, int N_Fetch_A, int N_Comm_B, int Ma_Fetch_A, int Stall);
    
    int Assign_input_Matrix( double **& matrix_A, double **& matrix_B, double **& matrix_C);
    int UnablePort();
  
  private :

	double ** Matrix_A;
	double ** Matrix_B;
	double ** Matrix_C;
  int PORT;
  
  int * Curr_PORT ;
  int * Nxt_PORT ;
  int * Curr_Arbiter;
  int * Nxt_Arbiter;
  int CoreID;
  int A_per_Core;
  int Residue;

  struct pack{
    int * address;
    int latency;
  };
 
  //vector<pack> SRAM_Fifo;


};
  
