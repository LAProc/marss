
//Input the address of Matrix 
//Output the cycle needed to get that data,
//Imagine that the bufer is tied to memory by memory controller
//So, there should be some randomize thing

#include "RandomBehav.h"


Randomize::Randomize(int *&Current_PORT, int *&Next_PORT, int *&Current_Arbiter, int
                    *&Next_Arbiter, int MyCoreID){

  Curr_PORT = Current_PORT;
  Nxt_PORT = Next_PORT;
  Curr_Arbiter = Current_Arbiter;
  Nxt_Arbiter = Next_Arbiter;
  CoreID = MyCoreID;
  
  A_per_Core = (howmanyA/NumofCore);
  
  Residue = (howmanyA%NumofCore);
  
  for (int k=0; k<Residue; k++){
      if (MyCoreID==k) A_per_Core++;
  }

}

int Randomize::Assign_input_Matrix( double **& matrix_A, double **& matrix_B, double **& matrix_C){

	Matrix_A=matrix_A;
	Matrix_B=matrix_B;
	Matrix_C=matrix_C;

}

int Randomize::GetPort(){

  if (*Curr_PORT) return 1;
  else return 0;

}

int Randomize::UnablePort(){
  PORT = FALSE;
}



//template <size_t rows, size_t cols>
void Randomize::assign_latency(double (&Matrix)[LAPU_Size][LAPU_Size], 
                  int (&Latency)[LAPU_Size][LAPU_Size], int &Ready){

  for (int i =0; i<LAPU_Size; i++){
    
    for (int j=0; j<LAPU_Size; j++){
    
      //Latency[i][j]= rand()%LATENCY_RANGE + 2;
      Latency[i][j] = LATENCY_RANGE;
      
      /*std::cout << "Latency is " << "i=" <<i << ", j=" << j << ", " << Latency[i][j]<<std::endl; 
      //Matrix[i][j] = NOT_READY;
      std::cout << "NOT_READY is " << "i=" <<i << ", j=" << j << ", " << Matrix[i][j]<<std::endl; 
      //getchar();*/

    }
  }

  //make PORT is always ready

  //*Nxt_PORT = FALSE;
  *Nxt_PORT = TRUE;

}

void Randomize::update_latency(int Type, double (&Matrix)[LAPU_Size][LAPU_Size], 
                  int (&Latency)[LAPU_Size][LAPU_Size], int &Ready, int Mc, int Ma, int N, int Kc, int Mc_Comm_Cin, int Mc_Comm_B, int Mc_Fetch_A, int N_Fetch_A, int N_Comm_B, int Ma_Fetch_A, int Stall){

  int latency=0;
  int N_;

  if (!Ready){  //if it is still waiting for data
  
    
    if(Stall) {
      if(Mc==0 && Kc==0 && N==0 && Ma!=0){
        //Mc=Kernel_Size-LAPU_Size;
        N = Panel_Size-LAPU_Size;
        Ma = Ma-1;
        //Mc = Mc-4;

      }
      
      else if(Mc==0 && Kc==0 && N!=0){
       // Mc=Kernel_Size-LAPU_Size;
        N = N-LAPU_Size;
        //Mc = Mc -4;
      }

      //else if (Mc==0 && Kc==0 && N==0 & Ma==0) Mc =Mc;
      //else Mc = Mc-4;
    }

    for (int i =0; i<LAPU_Size; i++){  
      for (int j=0; j<LAPU_Size; j++){
      
        if (Latency[i][j]>0) Latency[i][j]--;

        if (Latency[i][j]==0) {
          //Ready = Ready | 0x1;
          //
          if(Type == CIN){

            if(N==Panel_Size-LAPU_Size && Mc_Comm_Cin==Kernel_Size-LAPU_Size){
              
            
              if(Ma!=A_per_Core-1){//if no the end of A

            
                Matrix[i][j] = Matrix_C[((Mc_Comm_Cin+LAPU_Size)%Kernel_Size)+i + (Ma+1)*NumofCore*Kernel_Size + CoreID*Kernel_Size][(N+ ((Mc_Comm_Cin+LAPU_Size)/Kernel_Size)*LAPU_Size)%Panel_Size+j];
            
               } 
            }
            else {
            /*Matrix[i][j] = Matrix_C[((Mc+LAPU_Size)%Kernel_Size)+i + Ma*Kernel_Size]
                                     [(N+ ((Mc+LAPU_Size)/Kernel_Size)*LAPU_Size)% Panel_Size+j]; //TODO;*/
            
            Matrix[i][j] = Matrix_C[((Mc_Comm_Cin+LAPU_Size)%Kernel_Size)+i + Ma*NumofCore*Kernel_Size + CoreID*Kernel_Size]
                                     [(N+ ((Mc_Comm_Cin+LAPU_Size)/Kernel_Size)*LAPU_Size)% Panel_Size+j];

            std::cout << "Whats up Bro " << std::endl;
            std::cout << "Mc is " << Mc <<std::endl;
            }
            //TODO : DO I need to pass Mc, MA, and N also ?
          //I guess for every request, we got constant Mc, Ma and N.. So need to keep these values locally
          std::cout << "Ready is " <<Ready <<std::endl;
          // We can provide ready bit per buffer also, so that we do not need to put Matrix for every cycle


          }
          
          /*for (int k=0; k<LAPU_Size; k++)
            for (int l=0; l<LAPU_Size; l++)
            {std::cout << "Buffer Cin is | " << Matrix[i][j];
             if(l==LAPU_Size) std::cout << std::endl;
            }*/

          if (Type==A){
          
            Matrix[i][j]=Matrix_A[((Ma_Fetch_A+1)%A_per_Core)*NumofCore*Kernel_Size + N_Fetch_A + i + CoreID*Kernel_Size][Mc_Fetch_A+j];          
          }

          if (Type==B){
          
            Matrix[i][j]=Matrix_B[Mc_Comm_B+i][(N+LAPU_Size)% Panel_Size+j]; 
            //getchar();
          
          }
        }

        else latency++;
      
      }

    }
   
  }

  if(latency==0) {
     Ready = 1;
     *Nxt_PORT = TRUE; // Need to modify to accomodate multicore
     *Nxt_Arbiter = (*Curr_Arbiter +1)%NumofCore;
     std::cout << "Port is " << *Nxt_PORT << std::endl;
     if(Type==B) std::cout<<"Mc_Comm_B is " << Mc_Comm_B << "N_Comm_B is "<< N_Comm_B<<std::endl;
     //getchar();
     /*if(Type==B){
       std::cout << "fetching Matrix B" << std::endl;
      for (int i=0; i<LAPU_Size; i++){
        for(int j=0; j<LAPU_Size; j++){
        
          std::cout << Matrix_B[Mc_Comm_B+i][(N_Comm_B+LAPU_Size)% Panel_Size+j] << " | ";
            
            //Matrix[i][j] << " | ";           
          }
          std::cout << std::endl;
        }
        getchar();
      }*/
    }
  
  //make PORT is always ready

  //*Nxt_PORT = FALSE;

}

void Randomize::update_latency_write(int Type, double (&Matrix)[LAPU_Size][LAPU_Size], 
                  int (&Latency)[LAPU_Size][LAPU_Size], int &Ready, int Mc, int Ma, int N, int Kc, int Mc_Comm_Cout, int Mc_Comm_B, int Mc_Fetch_A, int N_Fetch_A, int N_Comm_B, int Ma_Fetch_A, int Stall){

  int latency=0;
  //Ready = 0;
  //PORT =0;

  if (!Ready){  //if it is still waiting for data
    
    Mc_Comm_Cout = Mc_Comm_Cout - LAPU_Size;

    /*if(Stall) {
      if(Mc==0 && Kc==0 && N==0 && Ma!=0){
        Mc=Kernel_Size-LAPU_Size;
        N = Panel_Size-LAPU_Size;
        Ma = Ma-1;
        Mc = Mc-4;

      }
      
      else if(Mc==0 && Kc==0 && N!=0){
        Mc=Kernel_Size-LAPU_Size;
        N = N-LAPU_Size;
        Mc = Mc -4;
      }

      //else if (Mc==0 && Kc==0 && N==0 & Ma==0) Mc =Mc;
      else Mc = Mc-4;
    }*/

    for (int i =0; i<LAPU_Size; i++){  
      for (int j=0; j<LAPU_Size; j++){
      
        if (Latency[i][j]>0) Latency[i][j]--;

          if (Latency[i][j]==0) {

            if ((Mc_Comm_Cout==0) && (N!=0) && ((N%Panel_Size)!=0)){
              Matrix_C[((Kernel_Size-LAPU_Size)%Kernel_Size)+i +Ma*NumofCore*Kernel_Size + CoreID*Kernel_Size] 
              [(N-LAPU_Size)+j] =Matrix[i][j];
              
              if ((((Kernel_Size-LAPU_Size)%Kernel_Size)+i)==(N-LAPU_Size)+j){
              std::cout << "Mc 0 but N is not, Buffer_Cout[i][j] is " <<  Matrix[i][j] << std::endl;
              //getchar();
              }

             std::cout << "C Ready is " <<Ready <<std::endl;
              
            }
            else if ( Ma!=0 && Mc_Comm_Cout==0 && N==0){ // do I need to declare N_Comm_Cout also ?
              Matrix_C[((Kernel_Size-LAPU_Size)%Kernel_Size)+i + (Ma-1)*NumofCore*Kernel_Size + CoreID*Kernel_Size]
              [((Panel_Size-LAPU_Size)%Panel_Size)+j] =Matrix[i][j]; 
              //chuui

              std::cout << "C Ready is " <<Ready <<std::endl;
              
              if ((((Kernel_Size-LAPU_Size)%Kernel_Size)+i)==((Kernel_Size-LAPU_Size)%Kernel_Size)+j){
              std::cout << "last 4X4 Buffer_Cout[i][j] is j " << Matrix[i][j] << std::endl;
              //getchar();
              }
            
            }
            
            else if (Mc_Comm_Cout>0){
              Matrix_C[(Mc_Comm_Cout-LAPU_Size)+i + Ma*NumofCore*Kernel_Size + CoreID*Kernel_Size][N+j] = Matrix[i][j];
              
              if((Mc-LAPU_Size+i)==N+j){ 
              //std::cout << "Buffer_Cout[i][j] is j " << Matrix[i][j] << std::endl;
              //getchar();
              }

             //getchar();

            }

            /*else if (Mc_Comm_Cin-LAPU_Size==0 && (N%Kernel_Size)==0 && N!=0){
            //Matrix_C[Kernel_Size-LAPU_Size + i + Ma*Kernel_Size][N-LAPU_Size+j] = Matrix[i][j];
              
            if((Kernel_Size-LAPU_Size+i + Ma*Kernel_Size)==N+LAPU_Size+j){ 
              std::cout << "Buffer_Cout[i][j] is j " << Matrix[i][j] << std::endl;
              getchar();
            }

            std::cout << "Mc 0 but N is not, Buffer_Cout[i][j] is " <<  Matrix[i][j] << std::endl;
              //getchar();

            std::cout << "C Ready is " <<Ready <<std::endl;
            }*/


         }

        else latency++;
      }
    }
  }


  //else PORT = FALSE;

  if(!Ready) {
    if (latency==0){
      Ready = 1;
      *Nxt_PORT = TRUE;
      *Nxt_Arbiter = (*Curr_Arbiter +1)%NumofCore;
      std::cout << "Cout has finished, port is ready " <<std::endl;
      /*for (int i=0; i<LAPU_Size; i++)
        for(int j=0; j<LAPU_Size; j++){
          if (Mc_Comm_Cout==0)  
            std::cout << Matrix_C[((Kernel_Size-LAPU_Size)%Kernel_Size)+i +Ma*Kernel_Size]
              [(N-LAPU_Size)+j]<<std::endl;
          else if ( Ma!=0 && Mc_Comm_Cout==0 && N==0){ // do I need to declare N_Comm_Cout also ?
            std::cout<< Matrix_C[((Kernel_Size-LAPU_Size)%Kernel_Size)+i + (Ma-1)*Kernel_Size][((Panel_Size-LAPU_Size)%Panel_Size)+j] << std::endl;
            
          }
          else {std::cout<< Matrix_C[Mc_Comm_Cout-LAPU_Size+i + Ma*Kernel_Size][N+j]<<std::endl;
          std::cout << " x is " << Mc_Comm_Cout-LAPU_Size+i + Ma*Kernel_Size <<std::endl;
          std::cout << " y is " << N+j <<std::endl;
          //std::cout << "Mc is " << Mc<<std::endl;
          getchar();
          }
        }
    */  
    }
    
    //Make port always ready
    //else *Nxt_PORT = 0;

  }

}

/*void Randomize::IssueRead(){

    //put 4 request addresses to fifo at one cycle + access latency
    //this is fifo for LSU
    
    SRAM_FIFO.address= request_addr;
    SRAM_FIFO.latency = SRAM_Latency;

}

void Randomize::IssueWrite(){

    
  
}

void Randomize::Run_Cycle(){
  
    // 1. For everything in FIFO, decrement the latency
    // 2. If there is which is zero, then pop that address,
    // 3. If packed request, then deallocate all the queue, if not only one
    // 4. Uses that popped address as reference to SRAM address
    // 5. If not packed request, put address 1 by 1, else 256 bits.
    // 6. Put the data into Data buffer / Write back to SRAM
    // 7. If data buffer has already 4 row elements, then assert 
    //    buffer_ready signal to CORE
    //    Fetch signal for newIO.cpp
    //    Size of output buffer at least 5~8
  
    //we declare another FIFO to manage latency
    //Technically, cop

}
*/
