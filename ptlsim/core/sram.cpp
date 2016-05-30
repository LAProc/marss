#include "Parameters.h"
#include "sram.h"

S_RAM::S_RAM(LAP_Package &LAP, LS_Buffer &buf, int ID){

 from_LAP = &LAP;
 Buffer = &buf;
 MyCoreID = ID;

}

int counter=0;

void S_RAM::assign_latency(){

  //SRAM_req_list temp;

  if (from_LAP->req){
    from_LAP->latency = SRAM_Latency;
    from_LAP->Serviced = FALSE;
    from_LAP->Offset = 0;
    LAP_In_FIFO.push_back(*from_LAP);

    counter++;
    //std::cout << "SRAM fifo size is " << LAP_In_FIFO.size() << std::endl;
    //std::cout << "MySRAMCoreID is " << MyCoreID << std::endl;
    //getchar(); 
  }
}

void S_RAM::update_latency(){


  for(int i=0; i<LAP_In_FIFO.size(); i++) {
    
    if(LAP_In_FIFO[i].latency!=0){
      LAP_In_FIFO[i].latency--;
    }
  }
    
  for(int i=0; i<LAP_In_FIFO.size(); i++) {
    if (LAP_In_FIFO[i].latency==0){ 
      LAP_In_FIFO[i].Serviced=1;      
      
      int offset_count = LAP_In_FIFO[i].Offset ;
      
      for (int j=0; j<Port_Bandwidth_Core/Element_Size; j++){
        int *addr = LAP_In_FIFO[i].addresses;
        if (LAP_In_FIFO[i].WE){
          SRAM[addr[j+offset_count]] = LAP_In_FIFO[i].data[j+offset_count];
        }
        
        else {
          LAP_In_FIFO[i].data[j+offset_count] = SRAM[addr[j+offset_count]];     
          
          //std::cout << "Address is  " << addr[j]<<std::endl;
          //std::cout << "Data in SRAM is " << SRAM[addr[j]]<<std::endl;
          //getchar();
        }
      }
      
      offset_count = offset_count + Port_Bandwidth_Core/Element_Size;
      LAP_In_FIFO[i].Offset= offset_count;
      
      if (offset_count==LAPU_Size){

        if(LAP_In_FIFO[i].WE==FALSE)
            LAP_Out_FIFO.push_back(LAP_In_FIFO[i]);
        else 
          counter--;

        LAP_In_FIFO.erase(LAP_In_FIFO.begin()+i);
      }
      
      break; 
      //do i need this ?
      //LAP_Out_FIFO.push_back(LAP_In_FIFO[i]);

        
      }  

  }

  /*std::deque<LAP_Package>::iterator it = LAP_In_FIFO.begin();
  while (it->Serviced!=0){
    LAP_Out_FIFO.push_back(*it);
    LAP_In_FIFO.pop_front();
    it = LAP_In_FIFO.begin();
  }*/
  

}

void S_RAM::Send_to_LAPU(){

    if (Buffer->data.size()<LAPU_Size*LAPU_Size) 
      Buffer->buf_ready = FALSE;
   
    /*std::cout << " LAP_Out_FIFO.size is  "<< LAP_Out_FIFO.size()<<std::endl;

    std::cout << " The data package contain " << LAP_Out_FIFO[0].data[0]<<std::endl;
    std::cout << " The address package contain " << LAP_Out_FIFO[0].addresses[0]<<std::endl;
    std::cout << " The type contain " << LAP_Out_FIFO[0].type<<std::endl;*/

    if(LAP_Out_FIFO.size()>=LAPU_Size){
    //if data is ready, then copy to LAP_Buffer
      if(LAP_Out_FIFO[LAPU_Size-1].Serviced){
    
        for(int i=0; i<LAPU_Size; i++){
          for(int j=0; j<LAPU_Size; j++){
          //Buffer->buf[i][j] = LAP_Out_FIFO[i].data[j];
            Buffer->data.push_back(LAP_Out_FIFO[i].data[j]);
          }
        }
      
        Buffer->type = LAP_Out_FIFO[0].type;
        Buffer->buf_ready = TRUE;
        
        counter = counter - 4;
        
        /*std::cout<<std::endl;
        std::cout << "**************************"<<std::endl;
        std::cout << "Buffer from SRAM ready" << std::endl;
        std::cout << "MySRAMCoreID is " << MyCoreID << std::endl;
        std::cout << "Unserviced is " << counter << std::endl;
        std::cout << "**************************"<<std::endl;
        std::cout<<std::endl;*/
        //getchar();

        //delete first four elements
        for(int k=0; k<LAPU_Size; k++){
          LAP_Out_FIFO.pop_front();
        }
      }
    }

    /*LAP_Package *temp =  &LAP_In_FIFO[i];
      LAP_Out_FIFO.push_back(*temp);
      LAP_In_FIFO.erase(LAP_in_FIFO.begin();*/


}

void S_RAM::run_every_cycle(){

  /*
     //from LAPU
     if(from_LAP->req) assign_latency();


   */
  //Send_to_LAPU();
  //update_latency();

  assign_latency();
  update_latency();
  Send_to_LAPU();
  
  //std::cout<< "I am at sram " << std::endl;
  
    /*if(Req_Package.WE){
      int address = Req_Package.ID;  
      for (int i=0; i<LAPU_Size; i++){
      Matrix_SRAM[address+i] = Req_Package.Address+i;
      }
    }
    
    else assign_latency();*/
}
