
#include "newIO.h"

//newIO::newIO(double *& Row_Buses_Write,double *& Row_Buses_Read,double *& Column_Buses_Write,double *& Column_Buses_Read){

newIO::newIO(double *& Row_Buses_Write,double *& Row_Buses_Read,double *& Column_Buses_Write,double *& Column_Buses_Read, int *&Current_Port, int *&Next_Port, 
    int *&Current_Arbiter, int *&Next_Arbiter, int MyCoreID, LS_Buffer &Buf, int &Kc_Counter, bool &stall_from_lap){

	Read_Col_Buses= Column_Buses_Read;
	Read_Row_Buses= Row_Buses_Read;
	Write_Col_Buses=Column_Buses_Write;
	Write_Row_Buses=Row_Buses_Write;
  
  from_SRAM = &Buf;
  Kc= &Kc_Counter;
  Stall = &stall_from_lap; 

	/*Read_Col_Buses= Column_Buses_Read;
	Read_Row_Buses= Row_Buses_Read;
	Write_Col_Buses=Column_Buses_Write;
	Write_Row_Buses=Row_Buses_Write;*/

	Cin_Counter=0;
	Cout_Counter=-2;
	Bin_Counter=0;
  Ain_Counter = 0;

  last_x =0;
  last_y =0;

  /*fetch_C = true;
  fetch_B = false;
  fetch_A = false;
  send_C = false;*/

  FcA = 0;
  
  RAND_IF = new Randomize(Current_Port, Next_Port, Current_Arbiter, Next_Arbiter, MyCoreID);

  Buffer_Cin_Ready=0;
  Buffer_B_Ready=0;
  Buffer_A_Ready=0;
  Buffer_Cout_Ready=1;  //for the first time store, assume that buffer is 0
}

#if 1
newIO::~newIO(){
}
#endif

void newIO::Reset(){

  Buffer_Cin_Ready=0;
  Buffer_B_Ready=0;
  Buffer_A_Ready=0;
  Buffer_Cout_Ready=1;  //for the first time store, assume that buffer is 0

	Cin_Counter=0;
	Cout_Counter=-2;
	Bin_Counter=0;
  Ain_Counter = 0;

  last_x =0;
  last_y =0;

  FcA = 0;
  
  bus_Done  = 0;
  Write_Now = 0;
  bus_Counter = 0;
  wr_bus_Counter = 0;

  Cin_Done  = 0;
	Cout_Done = 0;
	Bin_Done  = 0;
  Ain_Done  = 0;


}

int newIO::Assign_input_Matrix( double **& matrix_A, double **& matrix_B, double **& matrix_C){

	Matrix_A=matrix_A;
	Matrix_B=matrix_B;
	Matrix_C=matrix_C;
  
  RAND_IF -> Assign_input_Matrix(matrix_A, matrix_B, matrix_C);

}

//void newIO::newIO_Execute(int Global_index, int N, int Mc, int Kc, int Ma, int  Matmul_Current_State, int Latency_Counter_Curr,
//                     int Req_Matrix){

int newIO::Check_Buffer_Ready(int WhatMatrix){

  return from_SRAM->buf_ready;

  if (WhatMatrix==CIN) return Buffer_Cin_Ready;
  else if (WhatMatrix==B) return Buffer_B_Ready;
  else if (WhatMatrix==A) return Buffer_A_Ready;
  else if (WhatMatrix==COUT) return Buffer_Cout_Ready;
}

int newIO::GetPort(){
  return RAND_IF -> GetPort();
}

int newIO::UnablePort(){
  return RAND_IF -> UnablePort();
}

int newIO::NotifyStore(){

  ReadNow = 1;
}

int newIO::isFetchDone(int WhatMatrix){

  if (bus_Done) {
      bus_Done = FALSE;
      return TRUE;
    }
  else return FALSE;



  /*if (WhatMatrix==CIN) {
    if (Cin_Done) {
      Cin_Done = FALSE;
      return TRUE;
    }
    else return FALSE;
  }

  if (WhatMatrix==B) {
    if (Bin_Done) {
      Bin_Done = FALSE;
      return TRUE;
    }
    else return FALSE;
  }

  if (WhatMatrix==A) {
    if (Ain_Done) {
      Ain_Done = FALSE;
      return TRUE;
    }
    else return FALSE;
  }
  
  if (WhatMatrix==COUT) {
    if (Cout_Done) {
      Cout_Done = FALSE;
      return TRUE;
    }
    else return FALSE;
  }*/
}

/*void newIO::newIO_IssueRequest(int next_fetch){

  if (next_fetch == CIN){
    
    if(request_count_Cin<2){
      RAND_IF -> assign_latency(Buffer_Cin, Buffer_Cin_Latency, Buffer_Cin_Ready);
      request_count_Cin++;
    }
  }

  if (next_fetch == B){
    
    if(request_count_B<2){
      RAND_IF -> assign_latency(Buffer_B, Buffer_B_Latency, Buffer_B_Ready);
      request_count_B++;
    }
  }

  if (next_fetch == A){
    
    if(request_count_A<2){
      RAND_IF -> assign_latency(Buffer_A, Buffer_A_Latency, Buffer_A_Ready);
      request_count_B++;
    }
  }

  if (next_fetch == COUT){
    
    if(request_count_A<2){
      RAND_IF -> assign_latency(Buffer_Cout, Buffer_Cout_Latency, Buffer_Cout_Ready);
      request_count_Cout++;
    }
  }

}*/


void newIO::newIO_Execute(int Req_Matrix, int Mc_Comm_Cin, int Mc_Comm_Cout, int Mc_Comm_B, int Mc_Fetch_A, int N_Fetch_A, int N_Comm_B, int Ma_Fetch_A, int Stall, int N, int Mc, int Kc, int Ma){

  if (Req_Matrix == CIN){
    
    if(request_count_Cin<1){
      RAND_IF -> assign_latency(Buffer_Cin, Buffer_Cin_Latency, Buffer_Cin_Ready);
      request_count_Cin++;
    }
  }

  if (Req_Matrix == B){
    
    if(request_count_B<1){
      RAND_IF -> assign_latency(Buffer_B, Buffer_B_Latency, Buffer_B_Ready);
      request_count_B++;
    }
  }

  if (Req_Matrix == A){
    
    if(request_count_A<1){
      RAND_IF -> assign_latency(Buffer_A, Buffer_A_Latency, Buffer_A_Ready);
      request_count_A++;
    }
  }

  if (Req_Matrix == COUT){
    
    if(request_count_Cout<1){
      //RAND_IF -> assign_latency(Buffer_Cout, Buffer_Cout_Latency, Buffer_Cout_Ready);
      if(Buffer_Cout_Ready) request_count_Cout++;
    }
  }

  if (Buffer_Cin_Ready){

    //send_data to bus;
    std::cout << "test "<< std::endl;
  
    //next_request got from LAPU

    //if (Cin_Counter == 0)
    //RAND_IF->ReqPort(MyCoreID);
    //if granted 
    //IssueRequest(next_fetch)

    if (Cin_Counter< LAPU_Size){          
        
      std::cout <<"Cin on the bus are " ;
      
        for (int i=0;i<LAPU_Size; i++){                         
          Write_Col_Buses[i]=Buffer_Cin[Cin_Counter][i]; 
          //Col_Bus_Ready = CIN;
          //Write_Col_Buses[i]=LS_SRAM_queue[0].Data[i]
          //LS_SRAM_queue.pop_back();
          
          std::cout << "|" << Buffer_Cin[Cin_Counter][i];
        }
        std::cout << std::endl;

        //getchar(); //for debug

        Cin_Counter++;
    }
    
    /*if(Cin_Counter == LAPU_Size -1){
      Cin_Done = 1;
    }*/

    if (Cin_Counter == LAPU_Size) {
        Buffer_Cin_Ready =0;
        request_count_Cin--;
        Cin_Counter=0;
        Cin_Done =1 ;
    }

  }

  else if(Buffer_B_Ready){
    
    if (Bin_Counter< LAPU_Size){          
        
      for (int i=0;i<LAPU_Size; i++){                         
          Write_Col_Buses[i]=Buffer_B[Bin_Counter][i];
          //Col_Bus_Ready = B;
        }
        Bin_Counter++;
    }
     
    //getchar(); //for debug

    if (Bin_Counter == LAPU_Size) {
        Buffer_B_Ready =0;
        request_count_B--;
        Bin_Counter=0;
        Bin_Done =1;
    }

  }

  else if(Buffer_A_Ready){
    
    //bool stall = stall_from_lap;


    if (Stall){

      if (Ain_Counter< LAPU_Size){          
          
        for (int i=0;i<LAPU_Size; i++){                         
            Write_Col_Buses[i]=Buffer_A[Ain_Counter][i];
            //Col_Bus_Ready = B;
          }
          Ain_Counter++;
      }

   }

    else{
    
      if (Ain_Counter< LAPU_Size && ((Kc%LAPU_Size)!=2)){          
          
        for (int i=0;i<LAPU_Size; i++){                         
            Write_Col_Buses[i]=Buffer_A[Ain_Counter][i];
            //Col_Bus_Ready = B;
          }
          Ain_Counter++;
      }
    
    }
    
    //getchar(); //for debug

    if (Ain_Counter == LAPU_Size) {
        Buffer_A_Ready =0;
        request_count_A--;
        Ain_Counter=0;
        Ain_Done =1;
    }

  }

  else if (Buffer_Cout_Ready && ReadNow){

    //send_data to bus;
    std::cout << "test "<< std::endl;
  
    //next_request got from LAPU

    //if (Cin_Counter == 0)
    //RAND_IF->ReqPort(MyCoreID);
    //if granted 
    //IssueRequest(next_fetch)

    if (Cout_Counter< LAPU_Size){          
        
       if(Cout_Counter>=0){
          for (int i=0;i<LAPU_Size; i++){                         
            Buffer_Cout[Cout_Counter][i] = Read_Col_Buses[i];
            std::cout << "SAVING Cout  " << Read_Col_Buses[i] << std::endl;
        
          }  
         //getchar(); //for debug
       }
        Cout_Counter++;
    }
    
    if(Cout_Counter == LAPU_Size -1){
      Cout_Done = 1;      // I think this is correct
    }

    if (Cout_Counter == LAPU_Size) {
        Buffer_Cout_Ready =0;
        request_count_Cout--;
        Cout_Counter=-2;
        //Cout_Done =0 ;
        ReadNow = 0;
        RAND_IF -> assign_latency(Buffer_Cout, Buffer_Cout_Latency, Buffer_Cout_Ready);
    }

  }


/********************* TODO for  A and Cout**********************************
  else if(AReady){
  B
    send_data to bus;
    if counter==LAPU_Size AReady=0;
    //be careful of utlization of bus and port in Mem A
  }
***************************************************************************/

//  if (request_count_A>0 && request_count_A<=MAX_REQUEST) RAND_IF->update_latency(A);   //for all i and j, if it is ready then set Ready
//  if (request_count_B>0 && request_count_B<=MAX_REQUEST) RAND_IF->update_latency(B);
  if (request_count_Cin>0 && request_count_Cin<=MAX_REQUEST)
    RAND_IF->update_latency(CIN, Buffer_Cin, Buffer_Cin_Latency, Buffer_Cin_Ready, Mc, Ma, N, Kc, Mc_Comm_Cin, Mc_Comm_B, Mc_Fetch_A, N_Fetch_A, N_Comm_B, Ma_Fetch_A, Stall );  // Need to pass  all the function to random behavior 
  
  if (request_count_Cout==0 && request_count_Cout<=MAX_REQUEST)
    RAND_IF->update_latency_write(COUT, Buffer_Cout, Buffer_Cout_Latency, Buffer_Cout_Ready, Mc, Ma, N, Kc,  Mc_Comm_Cout, Mc_Comm_B, Mc_Fetch_A, N_Fetch_A, N_Comm_B, Ma_Fetch_A, Stall );

  if (request_count_A>0 && request_count_A<=MAX_REQUEST)
    RAND_IF->update_latency(A, Buffer_A, Buffer_A_Latency, Buffer_A_Ready, Mc, Ma, N, Kc,  Mc_Comm_Cin, Mc_Comm_B, Mc_Fetch_A, N_Fetch_A, N_Comm_B, Ma_Fetch_A, Stall );

  if (request_count_B>0 && request_count_B<=MAX_REQUEST)
    RAND_IF->update_latency(B, Buffer_B, Buffer_B_Latency, Buffer_B_Ready, Mc, Ma, N, Kc, Mc_Comm_Cin, Mc_Comm_B, Mc_Fetch_A, N_Fetch_A, N_Comm_B, Ma_Fetch_A, Stall);
  
    std::cout <<std::endl;
    std::cout << "BUffer Cin Ready is " << Buffer_Cin_Ready<<std::endl;
    std::cout << "BUffer B Ready is " << Buffer_B_Ready<<std::endl;
    std::cout << "BUffer A Ready is " << Buffer_A_Ready<<std::endl;
    std::cout << "BUffer Cout Ready is " << Buffer_Cout_Ready<<std::endl;
    std::cout << "Readnow is " << ReadNow <<std::endl;
    //getchar();
  
    //need to create buffer_cout function 
    //if (there_is_pending_request_for_Cout)check_latency(C);
}


void newIO::ServiceRequest(LAP_Package *&from_LAP){


  //it will do :
   // 1. Check request from LAP
    //2. Check buff_ready from SRAM
    

    LAP_Package * sram_package = from_LAP;

    if (from_LAP->req){
      
      //printf("inside req\n");
        
      sram_package->req = TRUE;
  
      
      if(from_LAP->WE){ //this is for write

        if(ReadNow){
        
          //package->issue_req=FALSE;
          //Need a function call to read from column_buses
          //Need a function call to send the package
          //to sram_package once ready
          
          if (wr_bus_Counter< LAPU_Size){          
          
            if(wr_bus_Counter>=0){
              for (int i=0;i<LAPU_Size; i++){                 
                sram_package->addresses[i] = from_LAP->addresses[i];
                sram_package->data[i]= Read_Col_Buses[i];
#ifdef PRINT_DEBUG
                std::cout << "SAVING Cout  " << Read_Col_Buses[i] << std::endl;
                std::cout << "Addresses  " << sram_package->addresses[i] << std::endl;
                std::cout << "wr_bus_Counter " << wr_bus_Counter << std::endl; 
#endif                //Col_Bus_Ready = B;
              }
              
            }
              
            wr_bus_Counter++;
          }
          if (wr_bus_Counter == LAPU_Size) {
            Buffer_Ready =0;
            wr_bus_Counter=0;
            ReadNow=0;
            //getchar();

          }

        }
      
      }

      else{   //for read, then put addresses in buffer
     
      sram_package->WE = FALSE;

      //std::cout << " I am at write to mem" <<std::endl;

      for (int i=0; i<LAPU_Size; i++){
        //may possibly be sram_package.

        sram_package->addresses[i] = from_LAP->addresses[i];
      }

      
      }


    }

    if(from_SRAM->buf_ready) Buffer_Ready=TRUE;

    if (Buffer_Ready) {
      
      //printf("inside buf\n");
   
      if (from_SRAM->type==A){//need to include type
        //std::cout << " Stall A in IO " << *Stall<<std::endl;
        ///std::cout << " Stall from LAP in IO " << stall_from_lap<<std::endl;
        //int stall = *Stall;

        if ((*Stall)){

          if (bus_Counter< LAPU_Size){          
              
            for (int i=0;i<LAPU_Size; i++){          
              Write_Col_Buses[i]= from_SRAM->data.at(i);
                //Col_Bus_Ready = B;
              }
              bus_Counter++;

              for (int j=0; j<LAPU_Size; j++)
                from_SRAM->data.pop_front();
          }
        }


        else{
    
          if (bus_Counter< LAPU_Size && (((*Kc)%LAPU_Size)!=2)){          
              
            for (int i=0;i<LAPU_Size; i++){                         
                Write_Col_Buses[i]= from_SRAM->data.at(i);
                //Col_Bus_Ready = B;
                //std::cout << "Sending A from IO " << Write_Col_Buses[i]<<std::endl;
               // getchar();
                }


            bus_Counter++;
            for (int j=0; j<LAPU_Size; j++)
              from_SRAM->data.pop_front();
          } 
        }

        if (bus_Counter == LAPU_Size) {
          Buffer_Ready =0;
          bus_Counter=0;
          from_SRAM->buf_ready=0;
          bus_Done =1;
        }
      }

      else if(from_SRAM->type!=COUT) {

          if (bus_Counter< LAPU_Size){          
        
          for (int i=0;i<LAPU_Size; i++){                         
            Write_Col_Buses[i]=from_SRAM->data.at(i);
            //Col_Bus_Ready = B;
            //std::cout << "Sending C from IO " << Write_Col_Buses[i]<<std::endl;
            //getchar();
          }
        
          bus_Counter++;

          for (int i=0; i<LAPU_Size; i++)
            from_SRAM->data.pop_front();
         } 
     
    //getchar(); //for debug

        if (bus_Counter == LAPU_Size) {
          Buffer_Ready =0;
          from_SRAM->buf_ready=0;
          bus_Counter=0;
          bus_Done =1;
        }
      }
  

    //set internal buf ready signal to one 

    //then drive the column bus , until fetch done
   

    }
}

