#include "dram.h"


dram::dram(DRAM_Package *&req_dram, PF_DRAM_Buffer *&buf){

  dram_package = req_dram;
  Buffer = buf;

}

void dump_dram(){

  //for(int i=Panel_Size*Panel_Size; i<Panel_Size*Panel_Size + 2*Kernel_Size*Panel_Size + NumofCore*Kernel_Size*Kernel_Size; i++)
  for(int i=0; i<Panel_Size*Panel_Size*NumofPartition; i++){
    //std::cout<<"DRAM["<<i<<"] = "<< DRAM[i]<<std::endl;
  
  }
  //std::cout<<"SRAM from A is ready "<<std::endl;

  //getchar();
}

void dram::assign_latency(){

  if (dram_package->req){
    //dram_package->latency = DRAM_Latency ;
    dram_package->latency = DRAM_Latency ;
    DRAM_In_FIFO.push_back(*dram_package);

    if (dram_package->WE){
      for (int i=0; i<Cache_Line/Element_Size; i++){ 
        dram_package->data.pop_front();
        //std::cout << "assign latency dram"<<std::endl;
        //getchar();
      }
    }
  }

  /*
  int size = DRAM_queue.size();

  memcpy(&temp, &DRAM_queue[size-1], sizeof(temp));
  temp.latency = RAND_LATENCY;

  size = DRAM_queue.size();
  memcpy(&DRAM_queue[size-1], &temp, sizeof(temp));
  */

}

void dram::update_latency(){

  //for every message in the FIFO
  for(int i=0; i<DRAM_In_FIFO.size(); i++) {
    
    if(DRAM_In_FIFO[i].latency!=0){
      DRAM_In_FIFO[i].latency--;
    }
  }
    //Only one message can get latency==0 at any given point 
    //of time
  
  for(int i=0; i<DRAM_In_FIFO.size();i++){

    if (DRAM_In_FIFO[i].latency==0){ 
      DRAM_In_FIFO[i].Serviced=1;      
      
      
      for (int j=0; j<Cache_Line/Element_Size; j++){
        int *addr = DRAM_In_FIFO[i].addresses;
        if (DRAM_In_FIFO[i].WE){
          //DRAM[addr[j]] = DRAM_In_FIFO[i].data.at(j);
          //std::cout <<"DRAM_In_FIFO is " << DRAM[addr[j]]<<std::endl;
          //std::cout << "Writing DRAM " <<std::endl;
          //getchar();
          
          if (addr[j]==((2*Panel_Size*Panel_Size) - Panel_Size)-1){        
            std::cout << "done storing 1st partition in DRAM"<<std::endl;
          }
          if (addr[j]==(2*Panel_Size*Panel_Size)-1){        
            std::cout << "done storing 2nd partition in DRAM"<<std::endl;
          }
          if (addr[j]==((Panel_Size*Panel_Size*4) -Panel_Size) -1){        
            std::cout << "done storing 3rd partition in DRAM"<<std::endl;
          }

          if (addr[j]==(Panel_Size*Panel_Size*NumofPartition)-1){
            std::cout << "done storing last partiton in DRAM"<<std::endl;
            dump_dram();
            exit(0);
          }
        }
        else{  
          //DRAM_In_FIFO[i].data.push_back(DRAM[addr[j]]);         
          //DRAM_In_FIFO[i].data.push_back(100-j);         
          //std::cout << "Reading from DRAM " <<std::endl;
          //std::cout << "DRAM[] is  " << DRAM[addr[j]]<<std::endl;
          //getchar();
            //if(DRAM[16]==100) getchar();
        }
      }
                
      if (DRAM_In_FIFO[i].WE==FALSE) 
        DRAM_Out_FIFO.push_back(DRAM_In_FIFO[i]); 
      
      DRAM_In_FIFO.erase(DRAM_In_FIFO.begin()+i);
      break;
    }  
  }


  //std::deque<DRAM_Package>::iterator it = DRAM_In_FIFO.begin();
  
  /*while (it->Serviced==TRUE){
    
    std::cout << " read or write ? " << it->WE << std::endl;
    getchar();
    if (it->WE==FALSE)
      DRAM_Out_FIFO.push_back(*it);//I put the result to DRAM_Out_BUff only when the request is READ
    
    DRAM_In_FIFO.pop_front();//here I deallocate Serviced request
    
    if (DRAM_In_FIFO.size()==0)
      break;
    it = DRAM_In_FIFO.begin();
  }*/

  /*std::cout << " Arrived safely ? " <<std::endl;
  getchar();*/
}

void dram::Send_to_Prefetcher(){

  int scale = SRAM_Freq / DRAM_Freq;
 
  if ((dram_cycle%scale)==0) 
      //assuming that SRAM runs faster than DRAM
  {   
    // this is PF_DRAM_Buffer
    if(Buffer->buf.size()<Port_Bandwidth/Element_Size)
       Buffer->buf_ready = FALSE;

    //getchar();
    //if data is ready, then copy to LAP_Buffer
      if(DRAM_Out_FIFO.size()!=0){
        if(DRAM_Out_FIFO[0].Serviced){
      
          for(int i=0; i<Cache_Line/Element_Size; i++){
            Buffer->buf.push_back (DRAM_Out_FIFO[0].data[i]);
            Buffer->addr.push_back (DRAM_Out_FIFO[0].sram_addr[i]);
            //std::cout << "Out DRAM Fifo is " << Buffer->buf[i]<<std::endl;
          }
        
          Buffer->buf_ready = 1;
        
          for(int j=0; j<Cache_Line/Element_Size; j++){
          DRAM_Out_FIFO[0].data.pop_front();
          }


        //delete first element
        if(DRAM_Out_FIFO[0].data.size()==0)
          DRAM_Out_FIFO.pop_front();
      }

    /*LAP_Package *temp =  &LAP_In_FIFO[i];
      DRAM_Out_FIFO.push_back(*temp);
      LAP_In_FIFO.erase(LAP_in_FIFO.begin();*/
  //

    }
  }
  dram_cycle++;
  dram_cycle= dram_cycle%(scale);

}

void dram::run_every_cycle(){
  
    //Send_to_Prefetcher();
    //update_latency();
    assign_latency();    
    update_latency();
    //std::cout << "Hello dram " <<std::endl;
    Send_to_Prefetcher();


    /*if (request)  assign_latency(&Req_Package);    
    update_latency();*/
}
