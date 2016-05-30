#include "sram_pref.h"

S_RAM_PREF::S_RAM_PREF(PREF_Package *&PREF, PF_Buffer *& buf){

 Buffer = buf;
 from_PREF = PREF;

}

int COUNTER=0;

void S_RAM_PREF::assign_latency(){

  //SRAM_req_list temp;
  if(from_PREF->req){
    from_PREF->latency = SRAM_Latency;
    PREF_In_FIFO.push_back(*from_PREF);
    //getchar();
  }
}

#if 1
void dump_sram(){

  //for(int i=Panel_Size*Panel_Size; i<Panel_Size*Panel_Size + 2*Kernel_Size*Panel_Size + NumofCore*Kernel_Size*Kernel_Size; i++)
  /*for(int i=Panel_Size*Panel_Size; i<Panel_Size*Panel_Size + 2*Kernel_Size*Panel_Size; i++){
    std::cout<<"SRAM["<<i<<"] = "<< SRAM[i]<<std::endl;
    std::cout<<"SRAM from B is ready "<<std::endl;
  
  }*/
  //std::cout<<"SRAM from A is ready "<<std::endl;

  //for(int i=0; i<K_H*K_V; i++)
  //for(int i=0; i<Panel_Size*Panel_Size + Panel_Size*Kernel_H*2 + K_H*K_V; i++)
  //for(int i=0; i<Panel_Size*K_H; i++)
  //for(int i=0; i<Panel_Size*Panel_Size + SRAM_OFFSET*Panel_H + 2*K_H*Panel_Size + K_H*K_V; i++)
    //std::cout << "SRAM["<<i<<"] is " <<SRAM[SRAM_OFFSET*Panel_H + i +Panel_Size*Panel_Size + 2*K_H*Panel_Size]<<std::endl;
    //std::cout << "SRAM["<<i<<"] is " <<SRAM[i + Panel_Size*Panel_Size + K_H*Panel_Size]<<std::endl;
    //std::cout << "SRAM["<<(i+Panel_Size*Panel_Size) +SRAM_OFFSET*Panel_H<<"] is " <<SRAM[SRAM_OFFSET*Panel_H + i + Panel_Size*Panel_Size]<<std::endl;
    //std::cout << "SRAM["<<i<<"] is " <<SRAM[i]<<std::endl;
  for(int i=0; i<Panel_Size*Panel_Size; i++)
    printf("SRAM[%d] is %lf\n", i, SRAM[i]);
    //printf("SRAM[%d] is %lf\n", i+ Panel_Size*Panel_Size, SRAM[i+ Panel_Size*Panel_Size]);
    exit(0);
  //getchar();
}
#endif

void S_RAM_PREF::update_latency(){

  for(int i=0; i<PREF_In_FIFO.size(); i++) {

    if(PREF_In_FIFO[i].latency!=0){
      PREF_In_FIFO[i].latency--;
    }
  }
  
  for(int i=0; i<PREF_In_FIFO.size(); i++) {

    if (PREF_In_FIFO[i].latency==0){ 
      PREF_In_FIFO[i].Serviced=1;      
      int fetch_amount = Port_Bandwidth/Element_Size;
      if (PREF_In_FIFO[i].res==TRUE)
        fetch_amount = PREF_In_FIFO[i].res_amnt;
      
      for (int j=0; j<(fetch_amount); j++){
        int *addr = PREF_In_FIFO[i].addresses;
        if (PREF_In_FIFO[i].WE){
//          printf("address is %d\n", addr[j]);
          SRAM[addr[j]] = PREF_In_FIFO[i].data[j];
          //COUNTER++;

#if 0
          printf("SRAM_In_FIFO is %lf \n", PREF_In_FIFO[i].data[j]);
          printf("COUNTER is %d \n",COUNTER);

          printf("SRAM_In_FIFO is %lf\n",PREF_In_FIFO[i].data[j]);
          printf("Writing SRAM \n");

#endif          
          //if (COUNTER==(Panel_Size*Panel_Size + 2*K_H*Panel_Size + NumofCore*K_V*K_H -1))
          //if (addr[j]==(MC*NC/4 - 1)){
          //if (COUNTER==(Panel_Size*Panel_Size + (K_H*Panel_Size) - 1)){
          //if (addr[j]==(Panel_Size*Panel_Size + SRAM_OFFSET*Panel_H + (K_H*Panel_Size) - 1)){
          //if (COUNTER==((Panel_Size*Panel_Size) + 2*K_H*Panel_Size + 2*K_V*K_H)){
           if (COUNTER==(Panel_Size*Panel_Size - 1)){
            static int counter=0;
            if(ITER_COUNT==0){
              //if (counter==0) dump_sram();
            //std::cout << "fetch_amount " <<fetch_amount<<std::endl;
            //if(counter==2)exit(0);
              counter++;
            }
          }
          //getchar();
        //COUNTER = (COUNTER+1);
        }
          
        
        else{  
          //SRAM_In_FIFO[i].data.push_back(SRAM[addr[j]]);         
          //PREF_In_FIFO[i].data[j] = 100-j;         
          //PREF_In_FIFO[i].data[j] = SRAM[addr[j]];         
          PREF_In_FIFO[i].data[j] = SRAM[ (COUNTER/Panel_V) +  (COUNTER%Panel_V)*(Panel_H)];
          COUNTER = (COUNTER+1)%(Panel_Size*Panel_Size);
          //printf ("Data from SRAM is %lf \n", SRAM[ (COUNTER/Panel_V) +  (COUNTER%Panel_V)*(Panel_H)]);
//          printf("COUNTER write is %d \n", COUNTER);
          
          //printf("C[%d][%d] = %f \n", addr[j]/Panel_V, addr[j]%Panel_V, SRAM[ (addr[j]/Panel_V) +  (addr[j]%Panel_V)*(Panel_H)]);

#if 0
          if(addr[j]==(Panel_Size*Panel_Size)-1){
          printf("in sram pref \n");
            exit(0);
          }
#endif
          //PREF_In_FIFO[i].data.push_back(SRAM[addr[j]]);         
          //std::cout << "Reading from SRAM " <<std::endl;
          //getchar();
        }
      }

#if 1
      if (PREF_In_FIFO[i].WE==FALSE) 
        PREF_Out_FIFO.push_back(PREF_In_FIFO[i]);
#endif 

      PREF_In_FIFO.erase(PREF_In_FIFO.begin()+i);
      break;
        
      }  

  }

  /*std::deque<PREF_Package>::iterator it = PREF_In_FIFO.begin();
  while (it->Serviced==TRUE){
    if (it->WE==FALSE)
      PREF_Out_FIFO.push_back(*it);//I put the result to DRAM_Out_BUff only when the request is READ
    PREF_In_FIFO.pop_front();
    if (PREF_In_FIFO.size()==0)
      break;
    it = PREF_In_FIFO.begin();
  }*/
  

}

void S_RAM_PREF::Send_to_Pref(){

    //if data is ready, then copy to Prefetcher_Buffer
    if(Buffer->buf.size()<Cache_Line/Element_Size)
       Buffer->buf_ready = FALSE;
    
    
    if (PREF_Out_FIFO.size()!=0){

      if(PREF_Out_FIFO[0].Serviced){
          int fetch_amount = Port_Bandwidth/Element_Size;
          if (PREF_Out_FIFO[0].res==TRUE){
            fetch_amount = PREF_Out_FIFO[0].res_amnt;
            //printf("fetch amount in SRAM %d \n", fetch_amount);
              
            //std::cout << "fetch_amount " <<fetch_amount<<std::endl;
            //exit(0);
          }
          for(int j=0; j<fetch_amount; j++){
            Buffer->buf.push_back(PREF_Out_FIFO[0].data[j]);
          } 

          if (Buffer->buf.size()>=Cache_Line/Element_Size)
              Buffer->buf_ready = TRUE;
          else 
              Buffer->buf_ready = FALSE;
      //delete first element
          PREF_Out_FIFO.pop_front();
      }
    }
    /*LAP_Package *temp =  &PREF_In_FIFO[i];
      PREF_Out_FIFO.push_back(*temp);
      PREF_In_FIFO.erase(LAP_in_FIFO.begin();*/

}

void S_RAM_PREF::run_every_cycle(){

  /*
     //from LAPU
     if(from_PREF->req) assign_latency();
      

   */
  
  static unsigned int counter;


  assign_latency();
  update_latency();
  
#ifdef HALFWORD
  if ((counter%2)==0)
#endif
  {
    //update_latency();
    Send_to_Pref();
  }

  counter++;
  
  /*update_latency();
  if (from_PREF->req) assign_latency();*/
  //update_latency();
  
    /*if(Req_Package.WE){
      int address = Req_Package.ID;  
      for (int i=0; i<LAPU_Size; i++){
      Matrix_SRAM[address+i] = Req_Package.Address+i;
      }
    }
    
    else assign_latency();*/
}
