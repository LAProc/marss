#include "prefetcher.h"

//declare queue
/*std::deque<DRAM_req_list*> DRAM_queue;
std::deque<SRAM_req_list*> SRAM_queue;

DRAM_req_list Req_Package_DRAM;
SRAM_req_list Req_Package_SRAM;*/
int iter=0;
bool init =TRUE;
bool init_sram=TRUE;


PF::PF(PREF_Package *&to_from_SRAM, PF_Buffer *&PF_Buf_From_SRAM , DRAM_Package *&req_dram, PF_DRAM_Buffer *&dram_buf, LAP_PREF_Sync *&from_LAPU){

  to_LS = from_LAPU;
  sram_package = to_from_SRAM;
  Buffer = PF_Buf_From_SRAM;
  dram_package = req_dram;
  Buf_DRAM = dram_buf;
  Issue_Request_DRAM = FALSE;
  
  IssueC_Current=0;
  IssueC_Next=0;
    
  /*for (int t=0; t<4*Cache_Line/Element_Size; t++){
      Buf_DRAM->buf.push_back(t);
  }*/
  Pref_DRAM_Current = Pref_C;
  BigA_Current=0;
  BigA_Next =0;

}

//void PF::PF_SM(){
void PF::PF_SM_Issuer_DRAM(){

//State Machine
//This state machine should get signal from SRAM controller/LAPU Unit saying that we need to start fetching new data or not

  Issue_Request_DRAM = FALSE;
  Write_DRAM = FALSE;

  bool amode;
  int currA;
  int numfetchA;
  //if (Buffer->buf_ready){

  //if interrupted from LAPU (store C), we will do 
  //1. save the current state
  //2. Change state to Write C
  //3. Write C to DRAM
  //4. Once finished move to Pref_C
  //5. Set Pref_DRAM_Next to last saved state, clear the interrupt signal
  //only issue when req_count < Mem_Req_Count;
  //if (request_count < MAX_REQUEST_COUNT) then 
  //execute below

  switch (Pref_DRAM_Current){
  
  case Pref_C : //should be the starting state

    Issue_Request_DRAM  = TRUE;

    IssueC_Next = 
    (IssueC_Current + (Cache_Line/Element_Size))%(Panel_Size*Panel_Size);// this should be improved if C is not square

    //Should tell per core if it is ready

    //std::cout <<"*************************"<<std::endl;
    //std::cout << " I am Pref C " << std::endl;

   //
   //
   //
   //
   //


   if(IssueC_Current == (Panel_Size*Panel_Size)- (Cache_Line/Element_Size))
    {
      BigC_Next = BigC_Current + 1; //this is kind of counter, right?
      

      Pref_DRAM_Next = Pref_B;
      
      if(init){
        Pref_DRAM_Next = Pref_B_Init;
        init=FALSE;
      }
      //std::cout <<"Pref_DRAM_Next is " << Pref_DRAM_Next << std::endl;
      //getchar();
      //I should request Issue Current to zero right ? Maybe we do not need to do that that
      //IssueC_Next = 0;
      //Tell every core that I am ready
    }
  
    break;

  case Pref_B_Init:

    Issue_Request_DRAM  = TRUE;
    IssueB_Next = (IssueB_Current + (Cache_Line/Element_Size))%(Kernel_Size*Panel_Size);// this should be improved if C is not square
   /*std::cout<< " I am at Pref_B_Init " <<std::endl;
   std::cout<< " IssueB Next is " << IssueB_Next<<std::endl;*/
   //getchar(); 
    
     if(IssueB_Current == (Kernel_Size*Panel_Size)- (Cache_Line/Element_Size))
    {
      BigB_Next = BigB_Current + 1;
      Pref_DRAM_Next = Pref_A_Init;
      B_SRAM_Done=FALSE;
      IssueB_Next=0;
#ifdef PRINT_DEBUG
      std::cout<< " Moving from Pref_B_Init " <<std::endl;
      std::cout<< " BigB_Next is " << BigB_Next<<std::endl;
#endif
      std::cout<< " Moving from Pref_B_Init " <<std::endl;
      std::cout<< " BigB_Next is " << BigB_Next<<std::endl;
      //getchar(); 
      //tell every core that you are ready
    }

  break;

  case Pref_A_Init_Second:

    /*std::cout << "In Pref A " << std::endl;
    std::cout << "IssueA_Current is " << IssueA_Current<<std::endl;
    std::cout << "Chunk_A is " << Chunk_A_Curr<<std::endl;
    std::cout << "BigA is  " << BigA_Current<<std::endl;
    getchar();*/
    
    /*if (howmanyA==NumofCore){
        Pref_DRAM_Next =Pref_B;
        BigA_Next = BigA_Current+1;
        Issue_Request_DRAM = FALSE;
    }*/

    /*else*/{
    
      Issue_Request_DRAM = TRUE; 
      
      numfetchA = (howmanyA%NumofCore==0)? NumofCore: (howmanyA/NumofCore==1)? howmanyA%NumofCore: NumofCore;

      IssueA_Next = (IssueA_Current + (Cache_Line/Element_Size))%(howmanyA*Kernel_Size*Kernel_Size);// this should be improved if C is not square

      #ifndef DEBUG

      if(to_LS->A[per_core_A%NumofCore]==FALSE);//this is the first time we issue
      

      else{ 
      
        IssueA_Next = IssueA_Current;
        Issue_Request_DRAM =FALSE;
      }
      
      #else
      #endif

      if(!amode){
      
        if(IssueA_Current == (NumofCore*Kernel_Size*Kernel_Size)- (Cache_Line/Element_Size))
        {
          Pref_DRAM_Next = Pref_B;
          A_SRAM_Init=TRUE;
          A_SRAM_Unit= numfetchA;
#ifdef PRINT_DEBUG
          std::cout << "In amode "<<std::endl;
          std::cout << "IssueA_Current is " << IssueA_Current<<std::endl;
          std::cout << "per_core_A is  "<< per_core_A << std::endl;
          std::cout << "Moving from A 2nd to B " << std::endl;
          std::cout<< " BigA_Next is " << BigA_Next<<std::endl;
#endif
          BigA_Next = BigA_Current + 1;
          std::cout << "Moving from A 2nd to B " << std::endl;
          std::cout<< " BigA_Next is " << BigA_Next<<std::endl;
          std::cout << "In amode "<<std::endl;
          std::cout << "IssueA_Current is " << IssueA_Current<<std::endl;

          //per_core_A = per_core_A + per_core_A_temp;

        //tell every core that you are ready
        }

      }

    
      else if(IssueA_Current == ((NumofCore + numfetchA)*Kernel_Size*Kernel_Size)- (Cache_Line/Element_Size))
      {
        Pref_DRAM_Next = Pref_B;
        A_SRAM_Init=TRUE;
        A_SRAM_Unit= numfetchA;
#ifdef PRINT_DEBUG
        std::cout << "IssueA_Current is " << IssueA_Current<<std::endl;
        std::cout << "per_core_A is  "<< per_core_A << std::endl;
        std::cout << "Moving from A 2nd to B " << std::endl;
        std::cout<< " BigA_Next is " << BigA_Next<<std::endl;
#endif
        //getchar();

        if(howmanyA==NumofCore + numfetchA)
          BigA_Next = BigA_Current + 1;

        //per_core_A = per_core_A + per_core_A_temp;

      //tell every core that you are ready
      }
  
    if(IssueA_Current== ((per_core_A+1)*Kernel_Size*Kernel_Size)-(Cache_Line/Element_Size)){
       
      per_core_A = (per_core_A+1);
      if(per_core_A==howmanyA)
        per_core_A=0;
    }

  }

  break;

  case Pref_A_Init:

   //std::cout<< " I am at Pref_A_Init " <<std::endl;
   //std::cout<< " IssueA Current is " << IssueA_Current<<std::endl;
   //getchar(); 
    
    /*std::cout << "In Pref A " << std::endl;
    std::cout << "IssueA_Current is " << IssueA_Current<<std::endl;
    std::cout << "Chunk_A is " << Chunk_A_Curr<<std::endl;
    std::cout << "BigA is  " << BigA_Current<<std::endl;
    getchar();*/
   

      Issue_Request_DRAM = TRUE;
  
      IssueA_Next = (IssueA_Current + (Cache_Line/Element_Size))%(howmanyA*Kernel_Size*Kernel_Size);// this should be improved if C is not square
      
        #ifndef DEBUG    

        if(to_LS->A[per_core_A%NumofCore]==FALSE);//this is the first time we issue
        else{ 
          IssueA_Next = IssueA_Current;
          Issue_Request_DRAM =FALSE;
        }

        #else
        #endif
  
      if(IssueA_Current== ((per_core_A+1)*Kernel_Size*Kernel_Size)-(Cache_Line/Element_Size)){
        per_core_A = (per_core_A+1);
        if(per_core_A==howmanyA)
          per_core_A=0;
      }


     // if(!last_A_Init){
        if(IssueA_Current == (NumofCore*Kernel_Size*Kernel_Size)- (Cache_Line/Element_Size))
        {
          Pref_DRAM_Next = Pref_Wait_for_Fetch_A_Again_Init;
          per_core_A_temp = per_core_A;
          A_SRAM_Done=FALSE;
          std::cout << "IssueA_Current is " << IssueA_Current<<std::endl;
          std::cout << "Moving from 1st A " << std::endl;
          std::cout<< " BigA_Next is " << BigA_Next<<std::endl;

          //getchar();
      //tell every core that you are ready
        }
      //}
      
      /*else {
      
        if(IssueA_Current == (numfetchA*Kernel_Size*Kernel_Size)- (Cache_Line/Element_Size))
        {
          Pref_DRAM_Next = Pref_B;
          per_core_A = per_core_A + per_core_A_temp;

      //tell every core that you are ready
        }
      
      }*/

  break;

  case Pref_Wait_for_Fetch_A_Again_Init:
  
  amode = (howmanyA>NumofCore)? TRUE :FALSE;

  if(amode){
    
    #ifndef DEBUG
    if (to_LS->A[0]==FALSE && A_SRAM_Done)
    #else
    #endif
    
    {  //that means already consumed in LS
      Pref_DRAM_Next = Pref_A_Init_Second;
      //per_core_A=0;
      last_A_Init=TRUE;
      A_SRAM_Done=FALSE;
      std::cout << "waiting at pref a init" << std::endl;
      //IssueA_Next=0;
    }
  }

  else {
    if (to_LS->A[0]==FALSE && A_SRAM_Done){  //that means already consumed in LS
      Pref_DRAM_Next = Pref_A_Init_Second;  //original masri 0605
      BigA_Next=1;
      
      /*//added below
      Pref_DRAM_Next = Pref_B;
      BigA_Next=1;
      //added up*/

      //per_core_A=0;
      last_A_Init=TRUE;
      A_SRAM_Done=FALSE;
      //IssueA_Next=0;
    }
  }


  break;

  case Pref_B :

    all_B_Ready=TRUE;
    Issue_Request_DRAM = TRUE;
    
    IssueB_Next = 
    (IssueB_Current + (Cache_Line/Element_Size))%(Kernel_Size*Panel_Size);// this should be improved if C is not square

    for (int i=0; i<NumofCore; i++){
    
      if(to_LS->B[i]){
        all_B_Ready=FALSE;
        break;
      }
    }

    //std::cout << "In pref B" <<std::endl;

    /*std::cout << "BigB is  " << BigB_Current<<std::endl;
    std::cout << "BigA is  " << BigA_Current<<std::endl;*/
    
    /*std::cout << "B_SRAM_Done is " << B_SRAM_Done<<std::endl;
    std::cout << "all_B_Ready is " << all_B_Ready<<std::endl;
    std::cout << "IssueB_Next is " << IssueB_Next<<std::endl;
    std::cout << "IssueB_Current is " << IssueB_Current<<std::endl;
    std::cout << "Substract is " << (Kernel_Size*Panel_Size)-(Cache_Line/Element_Size) <<std::endl;*/
    //getchar();

    if(IssueB_Current == (Kernel_Size*Panel_Size)-(Cache_Line/Element_Size)){
      BigB_Next = BigB_Current + 1;
      Pref_DRAM_Next = Pref_A;
      B_SRAM_Done=FALSE;
      
      bool is_same = (howmanyA==NumofCore)? TRUE:FALSE;
      if (is_same && BigB_Next==HowManyPanel && Chunk_B_Curr==NumofPartition-1){
      
        Pref_DRAM_Next= Write_C;

      }

      if(BigB_Next==HowManyPanel){
        BigB_Next = 0;
        Chunk_B_Next++;
      }
      //A_SRAM_Done=FALSE;
      //per_core_A=0;
#ifdef PRINT_DEBUG
      std::cout << "IssueB_Current is " << IssueB_Current<<std::endl;
      std::cout << "Moving from B to A" <<std::endl;
      std::cout<< " BigB_Next is " << BigB_Next<<std::endl;
      std::cout<< " Chunk_B next is " << Chunk_B_Next<<std::endl;
#endif
      std::cout << "Moving from B to A" <<std::endl;
      std::cout<< " BigB_Next is " << BigB_Next<<std::endl;
      std::cout<< " Chunk_B next is " << Chunk_B_Next<<std::endl;
      std::cout << "IssueB_Current is " << IssueB_Current<<std::endl;
      std::cout << "all_B_Ready is " << all_B_Ready<<std::endl;
      //getchar();
      //tell every core that you are ready
    }
      
    #ifndef DEBUG
    else{
      if(!all_B_Ready)
      {
        IssueB_Next=IssueB_Current;
        Issue_Request_DRAM = FALSE;
      }

      else if(all_B_Ready==TRUE && !B_SRAM_Done)
      {
        IssueB_Next=IssueB_Current;
        Issue_Request_DRAM = FALSE;
      }

    }
    #else
    #endif

    break;
    
  case Pref_A :

    Issue_Request_DRAM = TRUE;
    
    //std::cout << "In Pref A " << std::endl;
    
    /*if(Chunk_A_Curr==1){
    std::cout << "IssueA_Current is " << IssueA_Current<<std::endl;
    //std::cout << "Chunk_A is " << Chunk_A_Curr<<std::endl;
    //std::cout << "BigA is  " << BigA_Current<<std::endl;
    std::cout << "to_LS->A is " << to_LS->A[per_core_A%NumofCore]<<std::endl;
    std::cout << "per_core_A is " << per_core_A <<std::endl;
    std::cout << "A_SRAM_Done is " << A_SRAM_Done<<std::endl;
    }*/

    IssueA_Next = 
    (IssueA_Current + (Cache_Line/Element_Size))%(howmanyA*Kernel_Size*Kernel_Size);// this should be improved if C is not square
    //int currA = to_LS->currentpartA[per_core%NumofCore]
    //if(to_LS->A[per_core_A%NumofCore][currA] ==FALSE);
    //
    #ifndef DEBUG

    if(to_LS->A[per_core_A%NumofCore]==FALSE && A_SRAM_Done) ;
    else{
      IssueA_Next = IssueA_Current;
      Issue_Request_DRAM=FALSE;
      break;
    }
    
    /*if(to_LS->A[per_core_A%NumofCore]==FALSE)
    {
      IssueA_Next=IssueA_Current;
      Issue_Request_DRAM = FALSE;
      break;
    }*/
    #else
    #endif
    
    /*if(Chunk_A_Curr!=0 && BigA_Current==0){

    if(IssueA_Current== (NumofCore*Kernel_Size*Kernel_Size)-(Cache_Line/Element_Size))
          Pref_DRAM_Next = Write_C;
   }*/
  
   if(IssueA_Current == (howmanyA*Kernel_Size*Kernel_Size)- (Cache_Line/Element_Size)){ 
      BigA_Next = BigA_Current + 1;
      if(BigA_Current==HowManyPanel-1)
        Chunk_A_Next=Chunk_A_Curr + 1;

      if (BigA_Next == HowManyPanel)
        BigA_Next = 0;
   }
   
   /*if((IssueA_Current == (NumofCore*Kernel_Size*Kernel_Size)- (Cache_Line/Element_Size)) && 
       ((BigA_Current!=0 && Chunk_A_Curr==0) || (BigA_Current==0 && Chunk_A_Curr!=0)) ) 
    {
      
      //if (!(BigB_Current==1 && Chunk_B_Curr!=0))
        //Pref_DRAM_Next = Pref_B;
      //else 
        //Pref_DRAM_Next = Pref_A;

      
      if (BigB_Current == HowManyPanel){
      
        Chunk_B_Next= Chunk_B_Curr + 1;
        BigB_Next=0; 
      }

      std::cout << "BigA_Current is " << BigA_Current<<std::endl;
      std::cout << "Moving from A to B " << std::endl;
      //getchar();

      //tell every core that I am ready
      //For now, just make it simple, it will synchronize every Kernel*Panel
      //In the LAPU side, if A_Per_Core is more than 1, then need to wait when syncLAPU < syncPref
      //Yeah, this is the key !
      if(BigA_Current == 11){
        std::cout << "BigA is eleven" <<std::endl;
        //getchar();
      }
        

      if(BigA_Current==0 && Chunk_A_Curr!=0){
        //Pref_DRAM_Next = Write_C;
        // Assume that we go back to fetch_C again
        // In reality we fetch next B, waiting for interrupt of current C
        //Pref_DRAM_Next = Write_C;
        Pref_DRAM_Next = Write_C;
        //Chunk_A_Next= Chunk_A_Curr + 1;
        //BigA_Next=0;
        iter++;
        std::cout << "Moving to last B " << std::endl;
        //getchar();

      }
    }*/
   
    if(IssueA_Current==((per_core_A+1)*Kernel_Size*Kernel_Size)-(Cache_Line/Element_Size)){
         
        per_core_A = (per_core_A+1);
        
        int pos = (howmanyA==2*NumofCore)? FALSE: TRUE;
        bool is_same = (howmanyA==NumofCore)? TRUE:FALSE;

        
        if(is_same){
        
          if(per_core_A==howmanyA && BigA_Next==2 && Chunk_A_Curr!=0){
            Pref_DRAM_Next=Write_C;
            std::cout << "Moving from A to WriteC" <<std::endl;
          }
          else {
            if (per_core_A==howmanyA){
              Pref_DRAM_Next=Pref_B;
              std::cout << "Moving from A to B" <<std::endl;
              std::cout<< " BigA_Current is " << BigA_Current<<std::endl;
              std::cout<< " BigA_Next is " << BigA_Next<<std::endl;
              std::cout<< " Chunk_A_Curr is " << Chunk_A_Curr<<std::endl;
            }
            else 
              Pref_DRAM_Next=Pref_A;
          }
        }

        if(per_core_A==2*NumofCore && (BigA_Current!=0 || (BigA_Current==0 && Chunk_A_Curr!=0)) 
            && pos && !is_same){
          Pref_DRAM_Next=Pref_B;
          std::cout << "Moving from A to B" <<std::endl;
          std::cout<< " BigA_Current is " << BigA_Current<<std::endl;
          std::cout<< " BigA_Next is " << BigA_Next<<std::endl;
          std::cout<< " Chunk_A_Curr is " << Chunk_A_Curr<<std::endl;
          
          if(BigA_Current==0 && Chunk_A_Curr!=0)
            Pref_DRAM_Next=Write_C;
            //std::cout << "Moving from A to WriteC" <<std::endl;
          //if(per_core_A==howmanyA)
            //per_core_A=0;
        }
        
        if (per_core_A==howmanyA){
          per_core_A=0;
          
          if(!pos) {
            Pref_DRAM_Next=Pref_B;
            std::cout << "Moving from A to B" <<std::endl;
            std::cout<< " BigA_Current is " << BigA_Current<<std::endl;
            std::cout<< " BigA_Next is " << BigA_Next<<std::endl;
            std::cout<< " Chunk_A_Curr is " << Chunk_A_Curr<<std::endl;
          }

          if(!pos)
            if(BigA_Next==1 && Chunk_A_Curr!=0)
              Pref_DRAM_Next=Write_C;

          if(!is_same && BigA_Current==HowManyPanel-1 && Chunk_A_Curr==NumofPartition-1 && BigC_Current==NumofPartition)
            Pref_DRAM_Next=Write_C;             
        }

        
        if (counter_A==NumofCore-1)
          A_SRAM_Done=FALSE;

        /*std::cout << "counter_A is " << counter_A<<std::endl;
        std::cout << "A_SRAM_Done is " << A_SRAM_Done<<std::endl;*/

        counter_A= (counter_A+1)%NumofCore;
        
    }            
    
    /*if(IssueA_Current==(NumofCore*Kernel_Size*Kernel_Size)-(Cache_Line/Element_Size)){

      A_SRAM_Done=FALSE;
    }*/            


    break;
  
  case Write_C :

    //need to ensure that output buffer fro SRAM is ready here. i.e need to check buffer at SRAM
    
    //std::cout << "In Write C " << std::endl;
    
    if (Buffer->buf.size()>=Cache_Line/Element_Size){

    //std::cout << "In Write C " << std::endl;
    
    //if(Buffer->buf_ready){
      
      //std::cout << " I am at write_C " <<std::endl;
      //getchar();
    
      Issue_Request_DRAM = TRUE;
      Write_DRAM = TRUE;

      WriteC_Next = 
      (WriteC_Current + (Cache_Line/Element_Size))%(Panel_Size*Panel_Size);// this should be improved if C is not square

      if(WriteC_Current == (Panel_Size*Panel_Size)- (Cache_Line/Element_Size))
      {
        Pref_DRAM_Next = Pref_C;
        std::cout << "Moving to Pref_C from Write_C"<<std::endl;
        if (BigC_Current==NumofPartition)
          Pref_DRAM_Next = Wait;
      }
    }

    break;

    case Wait:

    Issue_Request_DRAM = FALSE;
    
    break;

  }
  //}

  //write or read
  //sram or dram;

  //if(request) Address_Gen();
 
}

void PF::Address_Gen_DRAM(){

  std::cout.precision(4);

  switch(Pref_DRAM_Current){
  
  case Pref_C : //assuming for square

  y_offset = (BigC_Current==1 || BigC_Current==3)? Panel_Size: 0;
  x_offset = (BigC_Current==2 || BigC_Current==3)? (Panel_Size*Panel_Size*2):0;

  if(Issue_Request_DRAM){
      Address_DRAM = (IssueC_Current/Panel_Size)*(Panel_Size+Panel_Size)
      + IssueC_Current%Panel_Size + y_offset + x_offset; //need to add offset here which I believe should be BigC_Current*Panel_Size
     
      /*std::cout << " I am at Address_Gen" <<std::endl;
      std::cout << " IssueC_Current is " << IssueC_Current<<std::endl;
      std::cout << " y_ofsett is " << y_offset<<std::endl;
      std::cout << " x_offset is " << x_offset<<std::endl;
      std::cout << " Panel_Size is " << Panel_Size<<std::endl;
      std::cout << " Address_DRAM is  " << Address_DRAM<<std::endl;
      std::cout << " Its value  is  " << DRAM[Address_DRAM]<<std::endl;*/
      
      //getchar();
      
      Address_SRAM = IssueC_Current;
    }
    //TODO : Confirm this addressing works

  break;

  case Pref_B_Init:
  
  y_offset = ((Chunk_B_Curr%2)==1)? Panel_Size: 0;
  x_offset = (BigB_Current*Kernel_Size)*Panel_Size*2;
  
  offset_C = Panel_Size*Panel_Size*NumofPartition;

  if(Issue_Request_DRAM){
    Address_DRAM = offset_C + (IssueB_Current/Panel_Size)*(Panel_Size+Panel_Size)+ IssueB_Current%Panel_Size + y_offset + x_offset;
    Address_SRAM = IssueB_Current + Panel_Size*Panel_Size +((BigB_Current%2)*(Panel_Size*Kernel_Size));  //Panel_Size*Panel_Size is offset of C 
  
  }

 /* std::cout << " IssueB_Current is " << IssueB_Current<<std::endl;
  std::cout << " y_ofsett is " << y_offset<<std::endl;
  std::cout << " x_offset is " << x_offset<<std::endl;
  std::cout << " Address_DRAM in PrefB init is  " << Address_DRAM<<std::endl;
  std::cout << " Its value in PrefB is init  " << DRAM[Address_DRAM]<<std::endl;
  std::cout << " Address_SRAM in PrefB is init  " << Address_SRAM<<std::endl;
  *///getchar();

  break;

  case Pref_A_Init:
  
    x_offset = ((Chunk_A_Curr)>=2)? Panel_Size*Panel_Size*2: 0;
  y_offset = BigA_Current*Kernel_Size;
  
  offset_B = Panel_Size*Panel_Size*NumofPartition*2;
 
  if(Issue_Request_DRAM){
    Address_DRAM = offset_B + (IssueA_Current/Kernel_Size)*(Panel_Size+Panel_Size) + IssueA_Current%Kernel_Size + x_offset + y_offset;
    Address_SRAM = IssueA_Current%(NumofCore*Kernel_Size*Kernel_Size) + (Panel_Size*Panel_Size + 2*Kernel_Size*Panel_Size) ;// Consider offset for SRAM also 
  }

  /*std::cout << " IssueA_Current in 1st is  " << IssueA_Current<<std::endl;
  std::cout << " Address_DRAM in PrefA init 1st is " << Address_DRAM<<std::endl;
  std::cout << " Its value in PrefA init is 1st  " << DRAM[Address_DRAM]<<std::endl;
  std::cout << " Address_SRAM in PrefA init is 1st " << Address_SRAM<<std::endl;
  *///getchar();

  break;
  
  case Pref_A_Init_Second:
  
  x_offset = ((Chunk_A_Curr)>=2)? Panel_Size*Panel_Size*2: 0;
  y_offset = BigA_Current*Kernel_Size;
  
  offset_B = Panel_Size*Panel_Size*NumofPartition*2;
 
  if(Issue_Request_DRAM){
    Address_DRAM = offset_B + (IssueA_Current/Kernel_Size)*(Panel_Size+Panel_Size) + IssueA_Current%Kernel_Size + x_offset + y_offset;
    Address_SRAM = IssueA_Current%(NumofCore*Kernel_Size*Kernel_Size) + (Panel_Size*Panel_Size + 2*Kernel_Size*Panel_Size) ;// Consider offset for SRAM also 
  }

  /*std::cout << " IssueA_Current is 2nd " << IssueA_Current<<std::endl;
  std::cout << " Address_DRAM in PrefA init is 2nd " << Address_DRAM<<std::endl;
  std::cout << " Its value in PrefA init is  2nd " << DRAM[Address_DRAM]<<std::endl;
  std::cout << " Address_SRAM in PrefA init is 2nd" << Address_SRAM<<std::endl;
  *///getchar();

  break;

  case Pref_B:
  
  y_offset = ((Chunk_B_Curr%2)==1)? Panel_Size: 0;
  x_offset = (BigB_Current*Kernel_Size)*Panel_Size*2;
  
  offset_C = Panel_Size*Panel_Size*NumofPartition;
  
  /*std::cout << " IssueB_Current is " << IssueB_Current<<std::endl;
  std::cout << " y_ofsett is " << y_offset<<std::endl;
  std::cout << " x_offset is " << x_offset<<std::endl;
  std::cout << " Address_DRAM in PrefB init is  " << Address_DRAM<<std::endl;
  std::cout << " Its value in PrefB is init  " << DRAM[Address_DRAM]<<std::endl;
  std::cout << " Address_SRAM in PrefB is init  " << Address_SRAM<<std::endl;*/

  if(Issue_Request_DRAM){
    Address_DRAM = offset_C + (IssueB_Current/Panel_Size)*(Panel_Size+Panel_Size)+ IssueB_Current%Panel_Size + y_offset + x_offset;
    Address_SRAM = IssueB_Current + Panel_Size*Panel_Size +((BigB_Current%2)*(Panel_Size*Kernel_Size));  //Panel_Size*Panel_Size is offset of C 
  
  }
  break;

  case Pref_A:
  
  x_offset = ((Chunk_A_Curr)>=2)? Panel_Size*Panel_Size*2: 0;
  y_offset = BigA_Current*Kernel_Size;
  
  offset_B = Panel_Size*Panel_Size*NumofPartition*2;
 
  if(Issue_Request_DRAM){
    Address_DRAM = offset_B + (IssueA_Current/Kernel_Size)*(Panel_Size+Panel_Size) + IssueA_Current%Kernel_Size + x_offset + y_offset;
    Address_SRAM = IssueA_Current%(NumofCore*Kernel_Size*Kernel_Size) + (Panel_Size*Panel_Size + 2*Kernel_Size*Panel_Size) ;// Consider offset for SRAM also 
  }

  
  /*std::cout << " y_offset is  is " << y_offset<<std::endl;
  std::cout << " Address_DRAM in PrefA is " << Address_DRAM<<std::endl;
  std::cout << " Its value in PrefA is  " << DRAM[Address_DRAM]<<std::endl;
  std::cout << " Address_SRAM in PrefA is " << Address_SRAM<<std::endl;*/
  //getchar();

  break;

  case Write_C:

   y_offset = ((BigC_Current-1)==1 || ((BigC_Current-1)==3))? Panel_Size: 0;
   x_offset = ((BigC_Current-1)==2 || ((BigC_Current-1)==3))? (Panel_Size*Panel_Size*2):0;

  if(Issue_Request_DRAM)
    Address_DRAM = (WriteC_Current/Panel_Size)*(Panel_Size+Panel_Size)
    + WriteC_Current%Panel_Size + y_offset + x_offset; //need to add offset here which I believe should be BigC_Current*Panel_Size
  break;

  }


}


void PF::Address_Gen_SRAM(){

  int offset_C;
  int offset_B;

  switch(SRAM_Current){
  
  case Send_C_SRAM:
  if (Issue_Request_SRAM){
    
    //Address_SRAM = SRAM_POS_Current;
    Address_SRAM = Sent_C_Current;
    // is this right ? confirm it !!
   //SRAM_POS_Next = SRAM_POS_Current + Port_Bandwidth/Element_Size;
    }

  break;


  case Send_B_SRAM:

  offset_C = Panel_Size*Panel_Size;
 
  int offset_Bin_SRAM;

  if(Sent_B_Current%2)  
    offset_Bin_SRAM = offset_C + Panel_Size*Kernel_Size;//
  else 
    offset_Bin_SRAM = offset_C ;

  if (Issue_Request_SRAM){
    //Address_SRAM = SRAM_POS_Current;
    Address_SRAM = Sent_B_Current + offset_Bin_SRAM;
    //SRAM_POS_Next = SRAM_POS_Current + Port_Bandwidth/Element_Size;
  }

  break;

  case Send_A_SRAM:
  
  int offset_Ain_SRAM;
  
  offset_C = Panel_Size*Panel_Size;

  if(Sent_A_Current%2)  
    offset_Ain_SRAM = offset_C + Panel_Size*Kernel_Size*3;//
  else 
    offset_Ain_SRAM = offset_C + Panel_Size*Kernel_Size*2;  

  if (Issue_Request_SRAM){
    Address_SRAM = Sent_A_Current + offset_Ain_SRAM;
    //SRAM_POS_Next = SRAM_POS_Current + Port_Bandwidth/Element_Size;
  }
  
  break;
  
  case Read_C_SRAM:

  if (Issue_Request_SRAM){
    
    Address_SRAM = Read_C_Current;// this should be the starting address of C
    //SRAM_POS_Next = SRAM_POS_Current + Port_Bandwidth/Element_Size;
  }
  
  break;

  }

}


/*void PF::Map_DRAM_SRAM(){

  if(Buf_DRAM->addr[0]<offset_C){
    Address_SRAM = Buf_DRAM->addr[0]%(Panel_Size*Panel_Size); 
  }
  
  else if(Buf_DRAM->addr[0]>=offset_C && Buf_DRAM->addr[0]<offset_B){
    Address_SRAM = (Buf_DRAM->addr[0]-offset_C)%(Kernel_Size*Panel_Size); 
  }
  
  // In case of A
  else {
    Address_SRAM = ((Buf_DRAM->addr[0]-offset_B)/Kernel_Size)%(Kernel_Size*Panel_Size); 
  }

}*/


void PF::PF_SM_SRAM(){

  Issue_Request_SRAM = FALSE;


  //need address_mapping DRAM_to_SRAM
  int tmp_addr = Buf_DRAM->addr[0];

  int offset_C = Panel_Size*Panel_Size;
  int offset_B = offset_C + (2*Panel_Size*Kernel_Size);

  if(Buf_DRAM->buf_ready){
    SRAM_Current = (tmp_addr< offset_C)? Send_C_SRAM: (tmp_addr<offset_B)? Send_B_SRAM : Send_A_SRAM;
  }
  else {
    if(wait_next_C)
      SRAM_Current=Idle; 
  }

#ifdef DEBUG
  if(SRAM_Current==Send_B_SRAM){
   std::cout << "Address to store to B " << tmp_addr<<std::endl; 
    //getchar();
  }
#endif
  //overriding if it wants to read from SRAM
  //Assuming that we will have signal from LAPU telling that C is ready in SRAM
  //static bool C_ready

  // Need to put else if here
  
  int i=0;
  int j=0;
  static int counter;


  /*for(i=0; i<NumofCore; i++){

      if(to_LS->Cout[i]){
        done=TRUE;
        counter =i;
        LS->Cout[i]=FALSE;
        break;
      }
  }*/


  for(i=0; i<NumofCore; i++){

      if(to_LS->Cout[i]){
        done=TRUE;
      }
      else {
        done = FALSE;
        break;
      }
  }

  //j would be the total number of howmanyA
  //once reach maximum, then write C
 

  if(BigC_Current!=NumofPartition){
    write_ready = done && (fetched_Cin_total>=1) &&(fetched_B==HowManyPanel+1) && (fetched_A_total==howmanyA*HowManyPanel + NumofCore*2) && (Pref_DRAM_Current==Write_C);
    /*std::cout << "Write Ready NOT at last C is " << write_ready<<std::endl;
    std::cout<<"fetched_B  is " << fetched_B<<std::endl;
    std::cout<<"fetched_A_total is " << fetched_A_total<<std::endl;*/
  }
  else{ 
    write_ready = done && (fetched_Cin_total>=1) &&(fetched_B==HowManyPanel) && (fetched_A_total==howmanyA*HowManyPanel) && (Pref_DRAM_Current==Write_C);
    /*std::cout << "Write Ready at last C is " << write_ready<<std::endl;
    std::cout<<"fetched_B  is " << fetched_B<<std::endl;
    std::cout<<"fetched_A_total is " << fetched_A_total<<std::endl;*/
  }

  if(write_ready) {
    SRAM_Current=Read_C_SRAM;
    //done=FALSE;
  }

  /*std::cout<<"BigA_Curr is  " << BigA_Current<<std::endl;
  std::cout<<"Chunk_A_Curr is " << Chunk_A_Curr<<std::endl;
  std::cout<<"BigB_Curr is  " << BigB_Current<<std::endl;
  std::cout<<"Chunk_B_Curr is " << Chunk_B_Curr<<std::endl;
  std::cout<<"write_ready is " << write_ready<<std::endl;
  std::cout<<"fetched_Cin_total is " << fetched_Cin_total<<std::endl;
  std::cout<<"fetched_B  is " << fetched_B<<std::endl;
  std::cout<<"fetched_A_total is " << fetched_A_total<<std::endl;
  std::cout<<"Pref_DRAM_Current " << Pref_DRAM_Current<<std::endl;*/

  int amount = (A_SRAM_Init)? A_SRAM_Unit: NumofCore;
  
  if (Buf_DRAM->buf_ready || write_ready){

  //std::cout << "SRAM_Current is " << SRAM_Current << std::endl;
  
  switch(SRAM_Current){

  case Send_C_SRAM :
  
    Issue_Request_SRAM  = TRUE;
    Write_SRAM = TRUE;
    
      //for Pref
       Track_SRAM(tmp_addr);
       /*for (int i=0; i<howmanyA; i++){
          if (fetched_Cin[i]==Kernel_Size*Panel_Size)
            {
            to_LS->Cin[i%NumofCore]=TRUE;
            std::cout << "ready Core ID " << i%NumofCore<<std::endl;
            getchar();
            fetched_Cin[i]=0;
            fetched_Cin_total++;
          }
        }*/
      // 
      // //if(Sent_C_Current== ((per_core_Cin+1)*Kernel_Size*Panel_Size)-(Port_Bandwidth/Element_Size)){
        //to_LS->C[per_core_Cin]=TRUE;
        //per_core_Cin = (per_core_Cin+1)%NumofCore;
        //locally, remind that you fetch Cin;
        //fetched_Cin=TRUE;
      //}
    
      Sent_C_Current = (Sent_C_Current + (Port_Bandwidth/Element_Size)) %(Panel_Size*Panel_Size);
    

     if(Sent_C_Current == Panel_Size*Panel_Size-(Port_Bandwidth/Element_Size))
    {
      to_LS->C_All_Ready=TRUE;
      for(int i=0; i<NumofCore; i++){
        to_LS->Cin[i] = TRUE;
      }
      fetched_Cin_total++;
      // I dont think we need to set SRAM_Next anymore
      //SRAM_Next = Send_B_SRAM;
      //Sent_C_Counter_Next = Sent_C_Counter_Current + 1;
      //should activate C_Ready here
    }

  break;

  case Send_B_Init_SRAM:   //Need to send 2 panels of B
 
    Issue_Request_SRAM  = TRUE;
    Write_SRAM = TRUE;

    //if(Sent_B_Current== (*Kernel_Size*Panel_Size)-(Port_Bandwidth/Element_Size)){
      //to_LS->B[0]=TRUE;
      
    //to_LS->whichB = 1;
    //Next State is State A
    //fetched_B++;
  //}

  Sent_B_Current = (Sent_B_Current + (Port_Bandwidth/Element_Size)) %(Kernel_Size*Panel_Size);
  
  break;


  case Send_B_SRAM:

    Issue_Request_SRAM  = TRUE;
    Write_SRAM = TRUE;
    /*std::cout << " in send_B_SRAM " << std::endl;
    std::cout << " Sent_B_Current is " << Sent_B_Current<< std::endl;*/
    

    if(Sent_B_Current== (Kernel_Size*Panel_Size)-(Port_Bandwidth/Element_Size)){
      for(int i =0; i<NumofCore; i++)
        to_LS->B[i]=TRUE;  
      to_LS->currentB = (to_LS->currentB +1)%NumofMemB;
      fetched_B++;
      /*std::cout << " fetched_B is  "<<fetched_B << std::endl;
      std::cout << " in send_B_SRAM " << std::endl;
      std::cout << "to_LS->B is " << to_LS->B << std::endl;*/
      last_A_Init=TRUE;
      B_SRAM_Done=TRUE;


    }
    Sent_B_Current = (Sent_B_Current + (Port_Bandwidth/Element_Size)) %(Kernel_Size*Panel_Size);
    
    /*if(Sent_B_Current == Panel_Size*Kernel_Size-(Port_Bandwidth/Element_Size)){
        Sent_B_Counter_Next = Sent_B_Counter_Current + 1;
        
        //I dont think we need to set SRAM_Next anymore, since we track above
        SRAM_Next = Send_A_SRAM;

        //if (Sent_B_Counter_Current==HowManyPanel)

        }*/

    break;

    case Send_A_SRAM:

      Issue_Request_SRAM = TRUE;
      Write_SRAM = TRUE;
      
      Track_SRAM(tmp_addr);
       for (int i=0; i<NumofCore; i++){
          if (fetched_A[i]==Kernel_Size*Kernel_Size){
            to_LS->A[i]=TRUE;
            std::cout << "Sent_A_Current " << Sent_A_Current << std::endl;
            //getchar();
            fetched_A[i]=0;
            fetched_A_total++;
            std::cout << "fetched_A_total " << fetched_A_total << std::endl;
          }
       }
    
      //std::cout << " amount is  "<< amount << std::endl;
    //  getchar(); 
    
       if(Sent_A_Current == amount*Kernel_Size*Kernel_Size-(Port_Bandwidth/Element_Size)){
          if(A_SRAM_Init) A_SRAM_Init=FALSE;
          A_SRAM_Done=TRUE; 
          /*static int a ;
          a++;

          std::cout << "I am here for " << a <<"times" <<std::endl;
          to_LS->A[0]=TRUE;
          to_LS->A[1]=TRUE;*/

      }
      
      Sent_A_Current = (Sent_A_Current + (Port_Bandwidth/Element_Size)) %(amount*Kernel_Size*Kernel_Size);

      break;

    case Read_C_SRAM:

      Issue_Request_SRAM = TRUE;
      Write_SRAM = FALSE;
      
      /*for(int i=0; i<NumofCore; i++){
      
          if(to_LS->Cout[i]){
          j=i;
          to_LS->Cout[i]=FALSE;
          break;
        }
      }*/
      //std::cout<<"At READ SRAM " <<std::endl;
      //std::cout<<"Read_C_Current is " << Read_C_Current<<std::endl;

      if(done==TRUE){

        for(int i=0; i<Port_Bandwidth/Element_Size; i++){
          sram_package->addresses[i] = Read_C_Current +i;
        }
      }

     /*if(Read_C_Current== (count_cout)*(Kernel_Size*Panel_Size) - (Port_Bandwidth/Element_Size)){
          done=FALSE;
          count_cout++;
      }*/

      if(Read_C_Current== (Panel_Size*Panel_Size)- (Port_Bandwidth/Element_Size)){
      
        write_ready=FALSE;
        fetched_Cin_total=0;
        fetched_A_total=2*NumofCore;
        fetched_B=1;
        wait_next_C=TRUE;
        done =FALSE;
        std::cout << " READ SRAM LAST" <<std::endl;
        std::cout << " fetched_B " << fetched_B<<std::endl;
        std::cout << " fetched_A_total " << fetched_A_total<<std::endl;

        for(i=0; i<NumofCore; i++)
          to_LS->Cout[i]=FALSE;
      }

      Read_C_Current = (Read_C_Current + (Port_Bandwidth/Element_Size)) %(Panel_Size*Panel_Size);
    
      break;

      case Idle:

      break;

    }
  }
}

void PF::Track_SRAM(int address){

  int offset_C = Panel_Size*Panel_Size;
  int offset_B = offset_C + (2*Panel_Size*Kernel_Size);
    
  which_matrix = (address< offset_C)? CIN: (address<offset_B)? B : A;

  switch(which_matrix){
  
    case CIN:

      for(int i=0; i<NumofCore; i++){
        if(address < (i+1)*Kernel_Size*Panel_Size){
         fetched_Cin[i] = fetched_Cin[i] + Port_Bandwidth/Element_Size;
         break;
        }
      }

    break;
    
    case A:

      for(int i=0; i<NumofCore; i++){
        if(address < offset_B +  ((i+1)%(NumofCore+1))*Kernel_Size*Kernel_Size){
          fetched_A[i] = fetched_A[i] + Port_Bandwidth/Element_Size;
          if(fetched_A[i]==Kernel_Size*Kernel_Size){
            //std::cout << "fetched_A[i] is " << fetched_A[i]<<std::endl;
            //getchar();
          }
          //std::cout << "i is "<<i<<std::endl;
          //std::cout << "Fetched A is " << fetched_A[i]<<std::endl;
          //getchar();
          break;
        }
      }
    break; 
  }

}

void PF::Req_to_DRAM(){
 
  dram_package->req=FALSE;

  if(Issue_Request_DRAM){
   
    dram_package->req=TRUE;
    for (int i=0; i<Cache_Line/Element_Size; i++){
      dram_package->addresses[i]= Address_DRAM + i;// in real world, I think we only consider cache aligned address
      dram_package->sram_addr[i]= Address_SRAM + i;
      //debug 
      //std::cout << " addresses are " << dram_package->addresses[i] <<std::endl;;
    }
    //std::cout << " dram_package->addresses[0]  is" << dram_package->addresses[0] <<std::endl;
    //std::cout << " dram_package->sram_addr[0]  is" << dram_package->sram_addr[0] <<std::endl;
    
    //Address_DRAM = Address_DRAM + Cache_Line/Element_Size;


    //getchar();

    //std::cout << " I put addresses in dram_package " <<std::endl;


    dram_package->Serviced= FALSE;
   
    //testing buffer from SRAM
    /*for (int t=0; t<Cache_Line/Element_Size; t++){
      Buffer->buf.push_back(t);
    }*/


    if (Write_DRAM){
      dram_package->WE=TRUE;
      for (int i=0; i<Cache_Line/Element_Size; i++){
        //Buffer->buf.push_back(i);
        dram_package->data.push_back(Buffer->buf.at(i));
      //////////////////////////////////////////////
      
        /*std::cout << " buffer are " << Buffer->buf.at(i)<<std::endl;
        std::cout << " data are " << dram_package->data.at(i)<<std::endl;
        std::cout << " dram_package size is " << dram_package->data.size()<<std::endl;*/
      //getchar();
      }

    
      //We clear something on the output buffer of SRAM
    
      for (int j=0; j<Cache_Line/Element_Size; j++)
        Buffer->buf.pop_front();

    }
    else dram_package->WE=FALSE;
    
    /*std::cout << " I clear data in SRAM buffer " <<std::endl;
    std::cout << "Buf_Size now is " << Buffer->buf.size()<<std::endl;*/
  }
}

void PF::Req_to_SRAM(){ // to write or to read

    sram_package->req=FALSE;
  if(Issue_Request_SRAM){
  //if(Buf_DRAM->buf_ready){
   
    sram_package->req=TRUE;
    
      //Address_SRAM = Address_SRAM + Port_Bandwidth/Element_Size;
      
    sram_package->Serviced= FALSE;

    if (Write_SRAM){
      
      sram_package->WE=TRUE;
    
      for (int i=0; i<Port_Bandwidth/Element_Size; i++)
        sram_package->addresses[i]= Buf_DRAM->addr[i]; 

      //if Buf_DRAM->buf_ready should be called in SM_SRAM

      for (int i=0; i<Port_Bandwidth/Element_Size; i++){
        
        //for testing
        //Buf_DRAM->buf.push_back(i);
        
        sram_package->data[i]= Buf_DRAM->buf[i];      
      }
      
      for (int i=0; i<Port_Bandwidth/Element_Size; i++){
        Buf_DRAM->buf.pop_front();
        Buf_DRAM->addr.pop_front();
      }

      if (Buf_DRAM->buf.size()==0){
        Buf_DRAM->buf_ready=FALSE;
        /*std::cout << "Buf false at SRAM " << Buf_DRAM->buf_ready<<std::endl;
        //getchar();*/
      }
    }

    else {

      sram_package->WE=FALSE;
      
      /*for (int i=0; i<Port_Bandwidth/Element_Size; i++)
        sram_package->addresses[i]= Read_C_Current+i; */
    
    }

  } 
  /*if(Issue_Request_SRAM)
    DRAM->Issue_Req(dram);*/

  /*Allocate_Result_in_Buffer();*/
  /*Req_Package_DRAM.Addr = Current_Addr_DRAM; 
  Package_DRAM.WE = Current_WE_DRAM;
  DRAM_queue.push_back(Req_Package_DRAM);    */
}

void PF::Get_Cache_Line(){

  /*if (Get_Cache_Line_Now){
    
    //scan entire queue, see for the matched data
    //if matches, push the address and result to results queue.
  
    for (int i=0; i<DRAM_queue.size(); i++){
    
      //When there is finished request
      if (DRAM_queue[i].Serviced){
      
        Req_Package_SRAM.Addr = DRAM_queue[i].Addr;
        
        for (int j=0; j<LAPU_Size; j++){
          Req_Package_SRAM.Data[j]= DRAM_queue[i].Data[j];
        }
        
        Req_Package_SRAM.WE = 1;
        // deallocate that queue
        DRAM_queue.erase(DRAM_queue.begin() + i);
        break; //only one package can transfer to SRAM every cycle
      }
        
    }

  }*/


}

void PF::Write_to_SRAM(){

  /*if(Write_to_SRAM_Now){      // meaning that 256 consecutive bits ready to be written to SRAM


     pop result queue and send to SRAM the result
  
  }*/

}

void PF::Write_to_DRAM(){

  /*if(Issue_Request && WE){
 
    DRAM->Write(Data);
  
  }*/

}

void PF::Update_SM(){

  IssueC_Current = IssueC_Next;
  IssueB_Current = IssueB_Next;
  IssueA_Current = IssueA_Next;
  
  WriteC_Current = WriteC_Next;

	BigC_Current = BigC_Next;
	BigB_Current = BigB_Next;
	BigA_Current = BigA_Next;

  A_Current= A_Next;

  Pref_DRAM_Current = Pref_DRAM_Next;
  Chunk_B_Curr= Chunk_B_Next;
  Chunk_A_Curr= Chunk_A_Next;

  //Read_C_Current = Read_C_Next;

  /*Sent_A_Current= Sent_A_Next;
  Sent_B_Current= Sent_B_Next;
  Sent_C_Current= Sent_C_Current;*/
}


void PF::Dump_PF_SMachine(){

  std::cout<<std::endl;
  std::cout<<std::endl;
  std::cout<<"----------------State Machine Pref-----------------"<<std::endl;

  /*std::cout<<"Pref_DRAM_Current : ";
  
  switch (Pref_DRAM_Current){
		case Pref_C:
			std::cout<<"Pref_C"<<std::endl;
		break;
		
    case Pref_B:
			std::cout<<"Pref_B"<<std::endl;
		break;
		
    case Pref_A:
			std::cout<<"Pref_A"<<std::endl;
		break;

		case Write_C:
			std::cout<<"Write_C"<<std::endl;
		break;
  }

  std::cout<<"Pref_DRAM_Next : ";
  
  switch (Pref_DRAM_Next){


		case Pref_C:
			std::cout<<"Pref_C"<<std::endl;
		break;
		
    case Pref_B:
			std::cout<<"Pref_B"<<std::endl;
		break;
		
    case Pref_A:
			std::cout<<"Pref_A"<<std::endl;
		break;

		case Write_C:
			std::cout<<"Write_C"<<std::endl;
		break;
  }*/

  /*
  std::cout<<"Pref_SRAM_Current : ";
  
  switch (SRAM_Current){
		case Send_C_SRAM:
			std::cout<<"Send_C_SRAM"<<std::endl;
		break;
		
    case Send_B_SRAM:
			std::cout<<"Send_B_SRAM"<<std::endl;
		break;
		
    case Send_A_SRAM:
			std::cout<<"Send_A_SRAM"<<std::endl;
		break;

		case Read_C_SRAM:
			std::cout<<"Read_C_SRAM"<<std::endl;
		break;
  
  }

  std::cout<<"Pref_SRAM_Next : ";
  
  switch (SRAM_Next){
		case Send_C_SRAM:
			std::cout<<"Send_C_SRAM"<<std::endl;
		break;
		
    case Send_B_SRAM:
			std::cout<<"Send_B_SRAM"<<std::endl;
		break;
		
    case Send_A_SRAM:
			std::cout<<"Send_A_SRAM"<<std::endl;
		break;

		case Read_C_SRAM:
			std::cout<<"Read_C_SRAM"<<std::endl;
		break;
  } */

  //dump Address_Gen and Issuer Counter
  
  /*std::cout << "Issue_Request_DRAM : " << Issue_Request_DRAM<<std::endl; 
  std::cout << "Write_DRAM : " << Issue_Request_DRAM<<std::endl; 

  std::cout << "IssueA_Current : " << IssueA_Current<<std::endl;
  std::cout << "IssueA_Next : "    << IssueA_Next<<std::endl;
  
  std::cout << "IssueB_Current : " << IssueB_Current<<std::endl;
  std::cout << "IssueB_Next : "    << IssueB_Next<<std::endl;
  
  std::cout << "IssueC_Current : " << IssueC_Current<<std::endl;
  std::cout << "IssueC_Next : "    << IssueC_Next<<std::endl;

  std::cout << "WriteC_Current : " << WriteC_Current<<std::endl;
  std::cout << "WriteC_Next : "    << WriteC_Next<<std::endl;

  std::cout << "BigA_Current : " << BigA_Current<<std::endl;
  std::cout << "BigB_Current : " << BigB_Current<<std::endl;*/

  /*
  std::cout << "Issue_Request_SRAM : " << Issue_Request_SRAM<<std::endl;
  std::cout << "Write_SRAM : " << Issue_Request_SRAM<<std::endl; 
  
  std::cout << "Sent_C_Current : " << Sent_C_Current<<std::endl;
  std::cout << "Sent_C_Next : "    << Sent_C_Next<<std::endl;
  
  std::cout << "Sent_C_Counter_Current : " << Sent_C_Counter_Current<<std::endl;
  std::cout << "Sent_C_Counter_Next : "    << Sent_C_Counter_Next<<std::endl;
  
  std::cout << "Sent_B_Current : " << Sent_B_Current<<std::endl;
  std::cout << "Sent_B_Next : "    << Sent_B_Next<<std::endl;
  
  std::cout << "Sent_B_Counter_Current : " << Sent_B_Counter_Current<<std::endl;
  std::cout << "Sent_B_Counter_Next : "    << Sent_B_Counter_Next<<std::endl;
  
  std::cout << "Sent_C_Current : " << Sent_C_Current<<std::endl;
  std::cout << "Sent_C_Next : "    << Sent_C_Next<<std::endl;
  
  std::cout << "Sent_C_Counter_Current : " << Sent_C_Counter_Current<<std::endl;
  std::cout << "Sent_C_Counter_Next : "    << Sent_C_Counter_Next<<std::endl;

  std::cout << "Read_C_Current : " << Read_C_Current<<std::endl;
  std::cout << "Read_C_Next : "    << Read_C_Next<<std::endl;

  std::cout << "Read_C_Counter_Current : " << Read_C_Counter_Current<<std::endl;
  std::cout << "Read_C_Counter_Next : "    << Read_C_Counter_Next<<std::endl;

  std::cout << "-----Package to DRAM------" <<std::endl;
  std::cout << " Req " << dram_package->req <<std::endl;
  std::cout << " WE " << dram_package->WE <<std::endl;*/
    
  /*for (int i=0; i<Cache_Line/Element_Size; i++)
      std::cout << "Addresses are "<<  dram_package->addresses[i]<<std::endl;;

  for (int i=0; i<dram_package->data.size(); i++){
    std::cout << "Data in package is " << i << " " << dram_package->data[i]<<std::endl;
  }*/

  std::cout.precision(4);

  /*std::cout << "Address_DRAM is " << Address_DRAM <<std::endl;
  std::cout << "Address_SRAM is " << Address_SRAM <<std::endl;
  std::cout << "DRAM["<< Address_DRAM<<"] = " << DRAM[Address_DRAM] <<std::endl;*/


  /*
  std::cout << "-----Package to SRAM------" <<std::endl;
  std::cout << " Req " << sram_package->req <<std::endl;
  std::cout << " WE " << sram_package->WE <<std::endl;
    
  if(TRUE){
    for (int i=0; i<Port_Bandwidth/Element_Size; i++)
      std::cout << "Addresses are "<<  sram_package->addresses[i]<<std::endl;

    for (int i=0; i<Port_Bandwidth/Element_Size; i++){
      std::cout << "Data in package is " << i << " " << sram_package->data[i]<<std::endl;
    } 
  }*/

}


void PF::run_every_cycle(){

    //PF_SM();  
    
    static unsigned int counter=0;
    //Issue_Request_DRAM = TRUE;
    
    /*if(counter>0) 
      Issue_Request_DRAM = FALSE;*/


    //Issue_Request_SRAM = TRUE;
    // buffer from SRAM
    
    //std::cout << "buffer ready from SRAM " << Buffer->buf_ready << std::endl;
  
    //if data from SRAM is ready
    
    //masri
    /*if(Buffer->buf_ready){
      
      for(int i=0; i<Cache_Line/Element_Size; i++)
        Buffer->buf.pop_front();
    }*/

    //Buffer->buf_ready=TRUE;
   
    //buffer from DRAM
    //Buf_DRAM->buf_ready=TRUE;
    /*if(counter%2==1) 
      Write_DRAM = FALSE;
    else */
    
    //Write_DRAM = TRUE;
    
    //Write_SRAM = TRUE;
    //Write_DRAM = TRUE;
    
    
    #ifdef DEBUG
    std::cout<<std::endl;
    std::cout<<std::endl;
    std::cout<<"******************************* "<<std::endl;
    std::cout<<"Prefetcher Now  "<<std::endl;
    #endif

    PF_SM_Issuer_DRAM();
    Address_Gen_DRAM();
    Req_to_DRAM();  //put request first then evaluate state machine
    //if(iter>=1) getchar();
  

#ifdef HALFWORD
    Issue_Request_SRAM = FALSE;
    if((counter%2)==0)
#endif     
    {
    PF_SM_SRAM();
    Req_to_SRAM();
    }
    
    Update_SM();
    counter = (counter+1)%2;

    #ifdef DEBUG
    std::cout<<"Pref_DRAM_Next is "<< Pref_DRAM_Next<<std::endl;
    //Dump_PF_SMachine();
    std::cout<<"******************************* "<<std::endl;
    //std::cout<<std::endl;
    #endif

    //this two function is basically for generating address
    /*PF_SM_Issuer_DRAM();
    Address_Gen_DRAM();*/
    
    // To SRAM
    
    //masri
    //PF_SM_SRAM();
    /*Req_to_SRAM();
    //Dump_PF_SMachine();
    //Address_Gen_SRAM();*/
   
    /*//Update_SM();
    counter++;*/

    //Get_Cache_Line();
    //Write_to_SRAM();
    //Write_to_DRAM();
    
    //DRAM->Run_Every_Cycle(Req_to_DRAM, Req_Package_DRAM);
    //SRAM->Run_Every_Cycle(Req_to_SRAM, Req_Package_SRAM);

}
