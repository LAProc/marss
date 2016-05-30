#include <globals.h>
#include <atomcore.h>
#include <machine.h>
#include <accelerator.h>
#include <interconnect.h>
#include <basecore.h>
#include <cpuController.h>
//#include <cacheTypes.h>

//TODO
/*bool init =TRUE;
bool init_sram=TRUE;*/

#include <hw/irq.h>
#include <hw/isa.h>

#define ACCEL_FREQ 1400
#define CPU_FREQ   2000

double * SRAM;
//double SRAM[SRAM_Size];
//double DRAM[1];
//double testbuf[SRAM_Size];
//W64 addrbuf[SRAM_Size];
//W64 v_addrbuf[SRAM_Size];
unsigned long long TOT_LAP_CYCLES=0;
unsigned long long PER_ITER_END_CYCLES= 0;
unsigned long long PER_ITER_BEGIN_CYCLES= 0;

int MOVE_TO_LAST = 0;
int LAST_STORE =0;
int ITER_COUNT = 0;
bool first_wake= false;
int first_attempt = TRUE;
int SRAM_OFFSET = 0;
int ctr;
int LAST_COUNTER = 0;

bool NON_PART;
int MC ;
int NC ;
int LC ;
int KC ;
int K_V ;
int K_H ;
int LDA ;
int LDB ;
int LDC ;

int Panel_Size;
int Kernel_Size;
int howmanyA;
int HowManyPanel;

int Mem_Size;
int Mem_Size_A;
int Mem_Size_B1;
int Mem_Size_B2;
int NumofPartition;
int SRAM_Size;

int Kernel_V;
int Kernel_H;
int Panel_V;
int Panel_H;

int idxbuf=0;
int addridx=0;
int v_addridx=0;
int count_access=0;
int send_amount=0;

using namespace Core;
using namespace Memory;

uint64_t lap_mmio_reg;
uint64_t lap_buf_addr;
#define LAP_MMIO_ADDR 0x200000000
#define BYTE 8

uint64_t cal_cycle_count;
uint64_t max_cal_cycle;

bool init_pref=true;
//bool init_sram=TRUE;
bool last_pref = FALSE;
int last_req_addr=0;
int  last_req_count= 0;
bool is_residue = false;
unsigned int counter_half=0;

int read_second=0;
int read_third =0;
enum AccelState {
    Accel_Idle,
    Accel_Load_header,
    Accel_Load_content_row_major,
    Accel_Load_content_column_major,
    Accel_Load_content_block_major,
    Accel_Cal,
    Accel_Store,
    Accel_Prefetching,
    Accel_Construct,
    Accel_Intermediate,

    MAX_Accel_State
};

#if 0
typedef struct {
    //int cblas_order;
    //int transa;
    //int transb;
    int m;
    int n;
    //W64 c;
    //W64 base;
    //int k;
} matrix_header_t;
#endif 

#if 1
typedef struct {
    int cblas_order;
    int transa;
    int transb;
    int m;
    int n;
    int k;
    int lda;
    int ldb;
    int ldc;
    //W64 alpha;
    //W64 betha;
} matrix_header_t;
#endif

typedef struct{
    W64 A;
    W64 B;
    W64 C;
    W64 base;
} matrix_address_t;

typedef struct{
    W64 alpha;
    W64 betha;
} matrix_alpha_betha_t;

typedef struct {
    double *A;
    double *B;
    double *C;
} matrix_data_t;

// Temporary variable to be used in testing memory Hierarchy.
AccelState temp_state = Accel_Idle;
W64 temp_virt_addr;
W64 temp_phys_addr;
W64 temp_virt_addr_A;
W64 temp_phys_addr_A;
W64 temp_virt_addr_B;
W64 temp_phys_addr_B;
W64 temp_virt_addr_C;
W64 temp_phys_addr_C;
W64 temp_uuid, temp_rip;
W64 Phys_Addr, Virt_Addr;

matrix_header_t matrix_header;
matrix_data_t matrix_data;
matrix_address_t      matrix_address;
matrix_alpha_betha_t  matrix_alpha_betha;

double *matrix_data_buf;
size_t matrix_data_buf_size;

double store_data[Cache_Line/Element_Size];
double * data_pointer;

int str_counter=0;
std::map<W64, bool> cache_ready_map;

ofstream addrtrace;

#define ADDR_TRACE "/home/local/bumblebee/masri/trace.txt"

Accelerator::Accelerator(BaseMachine& machine, const char* name)
    : Statable(name, &machine)
      , name(name), machine(machine)
{
    // Per cycle event
	stringbuf sg_name;
	sg_name << name << "-run-cycle";
	run_cycle.set_name(sg_name.buf);
	run_cycle.connect(signal_mem_ptr(*this, &Accelerator::runcycle));
	marss_register_per_cycle_event(&run_cycle);

  //TODO:
  //change runcycle with accelerator state machine
  //move all load related function to dram function
  //recompile 
  //test whether it works
  
    addrtrace.open(ADDR_TRACE);
    addrtrace << "/***************** Load Virtual Address Log **************/" << endl;
    addrtrace.close();

    // Interconnect for the accelarator and CPU controller
    stringbuf sig_name;
    sig_name << "Core" << 1 << "-Th" << 0 << "-dcache-wakeup";
    dcache_signal.set_name(sig_name.buf);
    dcache_signal.connect(signal_mem_ptr(*this, &Accelerator::load_cb));
    

  Current_PORT = new int;
  Next_PORT = new int;

  Current_Arbiter = new int;
  Next_Arbiter = new int;
  
  *Current_PORT =1;
  *Next_PORT = 1;


}

/*int Accelerator::Execute(){

}*/

void Accelerator::update_memory_hierarchy_ptr()
{
    memoryHierarchy = machine.memoryHierarchyPtr;
    memoryHierarchy->add_request_pool();

    //machine.setup_interconnects();
    //memoryHierarchy->setup_full_flags();
}

void Accelerator::init()
{
    // TODO: Specify id, instead of a magic number here.
    id = 1;
    cache_ready = false;

    ctx = &machine.contextof(0);

    // XXX Change this during experiment
    max_cal_cycle = 1000000L;

#if 0
    // Initialize ISA and IRQ information
    lap_isa_info.qdev.name = "lap",
    //lap_isa_info.qdev.no_user = 1,
    lap_isa_info.init = lap_init_fn;
    isa_qdev_register(&lap_isa_info);
    this->lap_irq = irq_lap;
#endif


    printf("Initiating Accelerator!\n");

}

// Check the MMIO register for request
bool Accelerator::do_idle(void *nothing)
{
    //W64 reg = 0;
    //reg = ctx->loadphys(LAP_MMIO_ADDR, false, 3); // sizeshift=3 for 64bit-data
    //printf("LAP MMIO register value = %ld", reg);

    //if (reg == 1) {
    if (lap_mmio_reg != 0) {
        cache_ready = true;
        temp_virt_addr = lap_buf_addr;
        // TODO Use actual physical address
        temp_phys_addr = lap_buf_addr;
        temp_rip = 0;
        temp_uuid = 0;
        printf("Inside do_idle: LAP mmio reg: %ld, Virtual Address: %p\n",
                lap_mmio_reg, temp_virt_addr);
        return true;
    }

    //printf("Inside do_idle: Virtual Address: %ld\n", temp_virt_addr);
    return false;
}


// Load the matrix header
bool Accelerator::do_load_header(void *nothing)
{

#if 1
  int rc;
  int offset;
  static int counter = 0;

    printf("before load, size=%lu\n", sizeof(matrix_header));

  #if 0
    if(read_third){
      rc = load_buf(temp_virt_addr + 9*sizeof(int) + 2*sizeof(double) , temp_phys_addr +9*sizeof(int) 
           + 2*sizeof(double),
            &matrix_address, sizeof(matrix_address), temp_rip, temp_uuid, true);
      if (rc != ACCESS_OK) {
        return false;
      }

    }
  #endif

#if 1
    if(read_second){
      
      rc = load_buf(temp_virt_addr + 9*sizeof(int), temp_phys_addr +9*sizeof(int),
            &matrix_alpha_betha, sizeof(matrix_alpha_betha), temp_rip, temp_uuid, true);
      if (rc != ACCESS_OK) {
        return false;
      }
    }


    if(!read_second){

      rc = load_buf(temp_virt_addr, temp_phys_addr, &matrix_header, sizeof(matrix_header), temp_rip, temp_uuid, true);
      if (rc != ACCESS_OK) {
        return false;
      }
      else {
        read_second = TRUE;
        return false;
      }
    }
#endif

#if 0
    rc = load_buf(temp_virt_addr, temp_phys_addr, &matrix_header, sizeof(matrix_header), temp_rip, temp_uuid, true);
      if (rc != ACCESS_OK) {
        return false;
    }
#endif

    printf("after load\n");
    
    //exit(0);
        
    printf("Inside do_header: Virtual Address: %p\n, Phys Address : %p\n", temp_virt_addr, temp_phys_addr);
    //exit(0);
    
    //Here we load all the meta data needed for calculation
    printf ("after load\n");
    printf ("cblas_order %d \n",matrix_header.cblas_order);
    printf ("trans_a %d \n",matrix_header.transa);
    printf ("trans_b %d \n",matrix_header.transb);
    printf ("m %d \n",matrix_header.m);
    printf ("n %d \n",matrix_header.n);
    printf ("k %d \n",matrix_header.k);
    printf ("lda %d \n",matrix_header.lda);
    printf ("ldb %d \n",matrix_header.ldb);
    printf ("ldc %d \n",matrix_header.ldc);
 
    printf ("alpha %f \n",W64toDouble(matrix_alpha_betha.alpha));
    printf ("betha %f \n",W64toDouble(matrix_alpha_betha.betha));
    
    size_t size = matrix_header.m * matrix_header.n;
    matrix_data_buf_size = sizeof(double) * size * 3;
    matrix_data_buf = (double *) malloc(matrix_data_buf_size);
    matrix_data.A = matrix_data_buf;
    matrix_data.B = matrix_data.A + size*sizeof(double);
    matrix_data.C = matrix_data.B + size*sizeof(double);

    size_t sizeA =matrix_header.k * matrix_header.ldc;
    size_t kc =matrix_header.k;


//for DGEMM

    temp_virt_addr_A = temp_virt_addr + 9*sizeof(int) + 2*sizeof(double);
    temp_phys_addr_A = temp_phys_addr + 9*sizeof(int) + 2*sizeof(double);

    temp_virt_addr_B = temp_virt_addr_A + size*sizeof(double);
    temp_phys_addr_B = temp_phys_addr_A + size*sizeof(double);

    temp_virt_addr_C = temp_virt_addr_B + size*sizeof(double);
    temp_phys_addr_C = temp_phys_addr_B + size*sizeof(double);

//activate for LU
#if 0 

    temp_virt_addr_A = temp_virt_addr + 9*sizeof(int) + 2*sizeof(double);
    temp_phys_addr_A = temp_phys_addr + 9*sizeof(int) + 2*sizeof(double);
    
    temp_virt_addr_C = temp_virt_addr_A + sizeA*sizeof(double);
    temp_phys_addr_C = temp_phys_addr_A + sizeA*sizeof(double);

    temp_virt_addr_B = temp_virt_addr_C - kc*sizeof(double);
    temp_phys_addr_B = temp_phys_addr_C - kc*sizeof(double);


#endif



    printf("Inside do_header: Virtual Address: %p\n, Phys Address : %p\n", temp_virt_addr, temp_phys_addr);
    printf("Inside do_header: Virtual Address: %#x\n, Phys Address : %#x\n", temp_virt_addr, temp_phys_addr);
    printf("m=%d, n=%d\n", matrix_header.m, matrix_header.n);
    

    Check_Data(temp_virt_addr_A);
    Check_Data(temp_virt_addr_B);
    Check_Data(temp_virt_addr_C);

    //exit(0);

    
    //exit(0);

    read_second = false;
    return true;

#endif 

}

int Accelerator::do_modify_kv(int threshold){
  
  int divisor = (Panel_Size/threshold);

  if(((Panel_Size%threshold)/divisor)<LAPU_Size){

      if(ctr==0)
        K_V = threshold + LAPU_Size;
      
      if((Panel_Size%threshold)==0)
        K_V = threshold;

      printf("modified KV is %d\n", K_V);
      printf("Panel is %d\n", Panel_Size);
      printf("Threshold %d\n", threshold);
      
      ctr++;
      if ((Panel_Size%threshold)==0){
        ctr=0;
        return 0;
      }
#if 0
      else if(threshold<2*KC){
        ctr++;
        do_modify_kv(threshold + LAPU_Size);
      }
#endif
  }

  if(threshold>((2*KC)-LAPU_Size))
        ctr=0;
  else {
    //ctr++;
    do_modify_kv(threshold + LAPU_Size);
  }
}

void Accelerator::do_construct(){


    //initial set-up before computing

    MC    = matrix_header.m;
    NC    = matrix_header.n;
    LC    = matrix_header.k;
    KC    = matrix_header.k;
    LDC   = matrix_header.ldc;
    LDB   = matrix_header.ldb;
    LDA   = matrix_header.lda;


    int threshold = 36;

    if((KC/threshold)>=2){
      LC = KC;
      //KC = find_kc(KC, threshold);
    }
    else {
      //nothing
    }

    K_H   = KC;
    K_V   = KC;

    if(((MC/2)/K_V) >=2){
      Panel_Size  = MC/2;
      Kernel_Size = KC;
      NON_PART = FALSE;
      NumofPartition= 4;
      
      //MOCH : For LU = LC/KC
      //HowManyPanel = LC/KC;
      //-MOCH. lets modify this to real matrix-matrix multiplication
      HowManyPanel = 2*Panel_Size/KC;
    }
    else {
      NON_PART = TRUE;
      NumofPartition= 1; 
      Panel_Size  = MC;
      Kernel_Size = KC;
      howmanyA    = Panel_Size/K_V;
      HowManyPanel = LC/KC;
    }
    
   
    //case for non-partitioning computation

    if(!NON_PART){
        
        threshold = KC;       
        if((Panel_Size%KC)!=0){        
          do_modify_kv(KC);
          howmanyA    = ((Panel_Size%K_V)==0)? Panel_Size/K_V :(Panel_Size+K_V)/K_V;        
          SRAM_OFFSET = ((Panel_Size%K_V)==0)? 0 : (((Panel_Size+K_V)/K_V) * K_V)- Panel_Size;
        }
        else{
            howmanyA= Panel_Size/K_V;
            SRAM_OFFSET = 0;
        }
    }


    Panel_V = Panel_Size;
    Panel_H = Panel_Size;
    Kernel_V = K_V;
    Kernel_H = KC;
  
    printf("MC is %d \n", MC);
    printf("NC is %d \n", NC);
    printf("KC is %d \n", KC);
    printf("LDC is %d \n", LDC);
    printf("LDB is %d \n", LDB);
    printf("LDA is %d \n", LDA);
    printf("K_H is %d \n", K_H);
    printf("K_V is %d \n", K_V);
    printf("Panel_Size is %d \n", Panel_Size);
    printf("Kernel_Size is %d \n", Kernel_Size);
    printf("howmanyA is %d \n", howmanyA);
    printf("HowManyPanel is %d \n", HowManyPanel);
    printf("NON_PART is %d \n", NON_PART);
    printf("NumofPartition is %d \n", NumofPartition);
    printf("SRAM_OFFSET is %d \n", SRAM_OFFSET);

    if (first_attempt){
      Mem_Size = (((Kernel_Size*Kernel_Size)/(LAPU_Size*LAPU_Size)) 
                  + 2*Kernel_Size);

      Mem_Size_A = ((2*K_V*K_H)/(LAPU_Size*LAPU_Size));
      Mem_Size_B1 = K_H;
      Mem_Size_B2 = K_H;
      
      SRAM_Size = (Panel_V*Panel_H) + SRAM_OFFSET*Panel_H + (2*K_H*Panel_H) + 
                  (K_H*K_V*NumofCore); 

      //SRAM = new double[SRAM_Size]; 
      //Need to create interface for allocating SRAM
      SRAM = new double[SRAM_Size];  
    
      printf("SRAM_Size is %d \n", SRAM_Size);
      
      first_attempt=false;
      //exit(0);
      
      //create a buffer to DRAM
      Buf_DRAM =  new PF_DRAM_Buffer;  
      Buf_DRAM->buf_ready=FALSE;
      
      //create cores and SRAM interface for each core
      CORES = new LAPU *[NumofCore]; 
      Sram =  new S_RAM *[NumofCore];

      //create interface between LAPU and prefetcher
      lap_pref = new LAP_PREF_Sync;
      to_LS = lap_pref;

      //create interface between LS Unit with LAPU
      LAP_Buf = new LS_Buffer[NumofCore]; 
      lap_req = new LAP_Package[NumofCore];
     
      //initialize signals
      for(int i =0; i<NumofCore; i++){
          LAP_Buf[i].buf_ready=FALSE;
          lap_req[i].req= FALSE;
          lap_req[i].Serviced= FALSE;
      }

      //create buffer for prefetcher
      PF_Buf = new PF_Buffer;
      pref_req = new PREF_Package; 
      
      Buf_from_SRAM = PF_Buf; 
      sram_package = pref_req;

      dram_package = new DRAM_Package;
      PF_DRAM_Buf =  new PF_DRAM_Buffer;

      //Init 
      PF_Buf->buf_ready=FALSE;
      pref_req->req = FALSE;
      pref_req->buffer_ready = FALSE;
      pref_req->Serviced = FALSE;
      pref_req->res=FALSE;
      pref_req->res_amnt=0;
      dram_package->req = FALSE;
      dram_package->buffer_ready = FALSE;
      dram_package->Serviced = FALSE;
      dram_package->last=FALSE;
      dram_package->last_count=0;
      dram_package->last_addr=0;
      PF_DRAM_Buf->buf_ready=FALSE;

      /*sram_package->req=FALSE;
      sram_package->res=FALSE;
      sram_package->res_amnt=0;
      sram_package->Serviced=0;*/


      //basically, connect all the signals into PEs
      for (int ID=0; ID<NumofCore; ID++){
        CORES[ID] = new LAPU(ID, Current_PORT, Next_PORT, Current_Arbiter, Next_Arbiter, lap_req[ID], LAP_Buf[ID], lap_pref);
        Sram[ID] =  new S_RAM(lap_req[ID],LAP_Buf[ID], ID);

        lap_pref->B[ID]=FALSE;
        lap_pref->Cin[ID]=FALSE; 
        lap_pref->Cout[ID]=FALSE;
        lap_pref->A[ID]=FALSE;
        lap_pref->currentB=FALSE;
        lap_pref->C_All_Ready=FALSE;
      }
     
      //Create interface between SRAM and Prefetcher
      Sram_Pref = new S_RAM_PREF(pref_req, PF_Buf);
           
  }

  else {  // This is for reset for the next computation    

    //do_intermediate();
    SRAM_Size = (Panel_V*Panel_H) + SRAM_OFFSET*Panel_H + (2*K_H*Panel_H) + 
                (K_H*K_V*NumofCore); 
    
    //SRAM = new double[SRAM_Size]; 
    SRAM = new double[SRAM_Size];  
   
    //Reseting the Accelerator
    for(int i =0; i<NumofCore; i++){
      CORES[i]->Reset();
    }

    //Prefetcher reset

      Kernel_V = K_V;
      Kernel_H = KC;
      
      Issue_Request_DRAM = FALSE;
      
      IssueC_Current=0;
      IssueC_Next=0;
        
      /*for (int t=0; t<4*Cache_Line/Element_Size; t++){
          Buf_DRAM->buf.push_back(t);
      }*/
      Pref_DRAM_Current = Pref_C;
      BigA_Current=0;
      BigA_Next =0;

      A_SRAM_Done = TRUE;
      Chunk_B_Next  = 0 ;
      Chunk_B_Curr  = 0 ;
      Chunk_A_Next  = 0 ;
      Chunk_A_Curr  = 0 ;

      Sent_C_Current          = 0;
      Sent_C_Next             = 0;
      Sent_C_Counter_Current  = 0;
      Sent_C_Counter_Next     = 0;
      Sent_B_Current          = 0;
      Sent_B_Next             = 0;
      Sent_B_Counter_Current  = 0;
      Sent_B_Counter_Next     = 0;
      Sent_A_Current          = 0;
      Sent_A_Next             = 0;
      Sent_A_Counter_Current  = 0;
      Sent_A_Counter_Next     = 0;
      Read_C_Current          = 0;
      Read_C_Next             = 0;
      Read_C_Counter_Current  = 0;
      Read_C_Counter_Next     = 0;

      IssueC_Current  = 0;
      IssueC_Next     = 0;
      IssueB_Current  = 0;
      IssueB_Next     = 0;

      IssueA_Current  = 0;
      IssueA_Next     = 0;

      BigC_Current    = 0;
      BigC_Next       = 0;

      BigB_Current    = 0;
      BigB_Next       = 0;

      BigA_Current    = 0;
      BigA_Next       = 0;

      WriteC_Current  = 0;
      WriteC_Next     = 0;
     
      done = FALSE;
      per_core_A = 0;
      
      B_SRAM_Done = FALSE;
      A_SRAM_Done = TRUE;
      
      wait_next_C = false;

      for(int i = 0; i<NumofCore; i++){
        fetched_Cin[i] = FALSE;
        fetched_A[i] = FALSE;
      }

      for(int i=0; i<NumofCore; i++)
        to_LS->Cout[i]=FALSE;
  
      to_LS->C_All_Ready=FALSE;

      counter_A = 0;

      fetched_B = 0;
      fetched_Cin_total = 0;
      fetched_A_total = 0;
      
      Pref_DRAM_Next = Pref_C;
      Pref_DRAM_Current = Pref_C;
      init_pref =TRUE;
      //init_sram=TRUE;
      last_pref= FALSE;
      last_req_addr= 0;
      last_req_count= 0;
      is_residue =false;
      counter_half=0;


    for (int ID=0; ID<NumofCore; ID++){
      lap_pref->B[ID]=FALSE;
      lap_pref->Cin[ID]=FALSE; 
      lap_pref->Cout[ID]=FALSE;
      lap_pref->A[ID]=FALSE;
      lap_pref->currentB=FALSE;
      lap_pref->C_All_Ready=FALSE;
    }

    //Reseting ends here
   printf("Reseting ends\n");
   printf("Buf_SRAM_Size is %d\n", Buf_from_SRAM->buf.size());
   printf("dram_package data size %d\n", dram_package->data.size());
   printf("Reseting ends\n");
   //exit (0);



  }


}

void Accelerator::do_intermediate(){


    MC    = matrix_header.m;
    NC    = matrix_header.n;
    LC    = matrix_header.k;
    KC    = matrix_header.k;
    LDC   = matrix_header.ldc;
    LDB   = matrix_header.ldb;
    LDA   = matrix_header.lda;


    int threshold = 36;

    if((KC/threshold)>=2){
      LC = KC;
      //KC = find_kc(KC, threshold);
    }
    else {
      //nothing
    }

    K_H   = KC;
    K_V   = KC;

    if(((MC/2)/K_V) >=2){
      Panel_Size  = MC/2;
      Kernel_Size = KC;
      NON_PART = FALSE;
      NumofPartition= 4;
      HowManyPanel = 2*Panel_Size/KC;
    }
    else {
      NON_PART = TRUE;
      NumofPartition= 1; 
      Panel_Size  = MC;
      Kernel_Size = KC;
      howmanyA    = Panel_Size/K_V;
      HowManyPanel = LC/KC;
    }
    
    if(!NON_PART){
      //do_modify_kv(threshold);
      howmanyA    = Panel_Size/K_V;
    }


    Panel_V = Panel_Size;
    Panel_H = Panel_Size;
    Kernel_V = K_V;
    Kernel_H = KC;

    Chunk_B_Next  = 0 ;
    Chunk_B_Curr  = 0 ;
    Chunk_A_Next  = 0 ;
    Chunk_A_Curr  = 0 ;

    Sent_C_Current          = 0;
    Sent_C_Next             = 0;
    Sent_C_Counter_Current  = 0;
    Sent_C_Counter_Next     = 0;
    Sent_B_Current          = 0;
    Sent_B_Next             = 0;
    Sent_B_Counter_Current  = 0;
    Sent_B_Counter_Next     = 0;
    Sent_A_Current          = 0;
    Sent_A_Next             = 0;
    Sent_A_Counter_Current  = 0;
    Sent_A_Counter_Next     = 0;
    Read_C_Current          = 0;
    Read_C_Next             = 0;
    Read_C_Counter_Current  = 0;
    Read_C_Counter_Next     = 0;
  
    IssueC_Current  = 0;
    IssueC_Next     = 0;
    IssueB_Current  = 0;
    IssueB_Next     = 0;

    IssueA_Current  = 0;
    IssueA_Next     = 0;

    BigC_Current    = 0;
    BigC_Next       = 0;

    BigB_Current    = 0;
    BigB_Next       = 0;

    BigA_Current    = 0;
    BigA_Next       = 0;

    WriteC_Current  = 0;
    WriteC_Next     = 0;
   
    done = FALSE;
    per_core_A = 0;
    
    B_SRAM_Done = FALSE;
    A_SRAM_Done = TRUE;
    
    wait_next_C = false;

    for(int i = 0; i<NumofCore; i++){
      fetched_Cin[i] = FALSE;
      fetched_A[i] = FALSE;
    }

    counter_A = 0;

    fetched_B = 0;
    fetched_Cin_total = 0;
    fetched_A_total = 0;
    
    Pref_DRAM_Next = Pref_C;
    Pref_DRAM_Current = Pref_C;

    Buf_DRAM->buf_ready=FALSE;
    PF_Buf->buf_ready=FALSE;
    pref_req->req = FALSE;
    pref_req->buffer_ready = FALSE;
    pref_req->Serviced = FALSE;
    pref_req->res=FALSE;
    pref_req->res_amnt=0;
    dram_package->req = FALSE;
    dram_package->buffer_ready = FALSE;
    dram_package->Serviced = FALSE;
    dram_package->last=FALSE;
    dram_package->last_count=0;
    dram_package->last_addr=0;
    PF_DRAM_Buf->buf_ready=FALSE;
    
    for (int ID=0; ID<NumofCore; ID++){
      lap_pref->B[ID]=FALSE;
      lap_pref->Cin[ID]=FALSE; 
      lap_pref->Cout[ID]=FALSE;
      lap_pref->A[ID]=FALSE;
      lap_pref->currentB=FALSE;
      lap_pref->C_All_Ready=FALSE;
    }

}


// Load the matrix content
bool Accelerator::do_load_content(void *nothing)
{
    int rc;

    rc = load_buf(temp_virt_addr+2*sizeof(int), temp_phys_addr+2*sizeof(int),
            matrix_data_buf, matrix_data_buf_size, temp_rip, temp_uuid, true);
    if (rc != ACCESS_OK) {
        return false;
    }

    return true;
}

// The Maximum number of elements in the experiment, used as array boundary.
#define MAX_SIZE_IN_TEST 100000000
// The row size for block-major access
#define BLOCK_ROW_SIZE 32
// The column size for block-major access
#define BLOCK_COLUMN_SIZE 32
// The row count for block-major access
#define BLOCK_ROW_COUNT 16
// The column count for block-major access
#define BLOCK_COLUMN_COUNT 16


// The maximum number of active requests in memory system.
#define MAX_REQUEST_COUNT 30


// The number of bytes in each memory request
#define MEM_REQ_SIZE 64

// The number of elements in each memory request
#define MEM_REQ_COUNT 16

// The number of words in each memory request
#define MEM_REQ_WORDS 8

// The number of rows that have been processed (used in row-major only).
int row_count;
// The number of columns that have been processed (used in column-major only).
int column_count;
// The number of blocks that have been processed (used in block-major only).
int block_count;

// The array indicates which cache line has been issued a request.
bool requested[MAX_SIZE_IN_TEST];

// wait_count indicates how many requests are still being processed by memory
// system.
int wait_count = 0;

// request_count indicates how many active requests are in the memory system.
int request_count = 0;

// Block-major access. Please read row-major comments first
bool Accelerator::do_load_content_block_major(void *nothing)
{
    int rc;

    W64 cur_virt_addr, cur_phys_addr;
    W64 base_virt_addr, base_phys_addr;
    W64 offset;
    W64 data;

    //base_virt_addr = temp_virt_addr + 2 * sizeof(int);
    //base_phys_addr = temp_phys_addr + 2 * sizeof(int);
    base_virt_addr = temp_virt_addr;
    base_phys_addr = temp_phys_addr;

    //printf("Inside Load Content Block Major.\n");
    // Block-based seek
    for (int i = 0; i < BLOCK_ROW_SIZE * BLOCK_COLUMN_SIZE / MEM_REQ_COUNT; ++i) {
        // Calculate row and column number based on block_count and block_index
        // (i)
        W64 row_offset = (block_count / BLOCK_COLUMN_COUNT) * BLOCK_ROW_SIZE + i / (BLOCK_COLUMN_SIZE/MEM_REQ_COUNT);
        W64 column_offset = (block_count % BLOCK_COLUMN_COUNT) * BLOCK_COLUMN_SIZE + i % (BLOCK_COLUMN_SIZE/MEM_REQ_COUNT) * MEM_REQ_COUNT;

        // Calculate offset using row and column number
        offset = (row_offset*matrix_header.n+column_offset) * sizeof(int);
        cur_virt_addr = base_virt_addr + offset;
        cur_phys_addr = base_phys_addr + offset;

        if unlikely (!requested[i] && request_count < MAX_REQUEST_COUNT) {
            rc = this->load(cur_virt_addr, cur_phys_addr, matrix_data_buf+offset, temp_rip,
                    temp_uuid, false, 6);
            printf("FIRST ATTEMP to load [%lld, %lld], %p, result = %d, simcycle = %ld\n", row_offset, column_offset, cur_phys_addr, rc, sim_cycle);
            requested[i] = true;
            if (rc == ACCESS_OK) {
                ++wait_count;
            } else {
                cache_ready_map[cur_phys_addr] = false;
                ++request_count;
            }
        } else if unlikely (cache_ready_map[cur_phys_addr]) {
            rc = this->load(cur_virt_addr, cur_phys_addr, matrix_data_buf+offset, temp_rip,
                    temp_uuid, true, 6);
            printf("SECOND ATTEMP to load [%lld, %lld], %p, result = %d, simcycle = %ld\n", row_offset, column_offset, cur_phys_addr, rc, sim_cycle);
            ++wait_count;
            cache_ready_map[cur_phys_addr] = false;
            --request_count;
        }
    }

    // When all rows have finished, proceed to the next column
    if unlikely (wait_count >= BLOCK_ROW_SIZE * BLOCK_COLUMN_SIZE/MEM_REQ_COUNT) {
        printf("Block_count = %d, Wait_count = %d\n", block_count, wait_count);

        ++block_count;
        wait_count = 0;

        for (int i = 0; i < MAX_SIZE_IN_TEST; ++i) {
            requested[i] = false;
        }
    }

    // When all blocks have finished, return true indicates this stage has
    // finished.
    if (block_count >= BLOCK_ROW_COUNT * BLOCK_COLUMN_COUNT) {
        return true;
    } else {
        return false;
    }
}

// Column-major access. Please read row-major comments first.
bool Accelerator::do_load_content_column_major(void *nothing)
{
    int rc;
    //bool all_ready = false;

    W64 cur_virt_addr, cur_phys_addr;
    W64 base_virt_addr, base_phys_addr;
    W64 offset;
    W64 data;

    //base_virt_addr = temp_virt_addr + 2 * sizeof(int);
    //base_phys_addr = temp_phys_addr + 2 * sizeof(int);
    base_virt_addr = temp_virt_addr;
    base_phys_addr = temp_phys_addr;

    // Column-major seek

    // i indicates the current row number
    for (int i = 0; i < matrix_header.m; ++i) {
        // Calculate offset based on row number (i) and column number
        offset = (i*matrix_header.n+column_count) * sizeof(int);
        cur_virt_addr = base_virt_addr + offset;
        cur_phys_addr = base_phys_addr + offset;

        if unlikely (!requested[i] && request_count < MAX_REQUEST_COUNT) {
            rc = this->load(cur_virt_addr, cur_phys_addr, matrix_data_buf+offset, temp_rip,
                    temp_uuid, false, 6);
            //printf("FIRST ATTEMP to load %p, result = %d, simcycle = %ld\n", cur_phys_addr, rc, sim_cycle);
            requested[i] = true;
            if (rc == ACCESS_OK) {
                ++wait_count;
            }
            cache_ready_map[cur_phys_addr] = false;
            ++request_count;
        } else if unlikely (cache_ready_map[cur_phys_addr]) {
            rc = this->load(cur_virt_addr, cur_phys_addr, matrix_data_buf+offset, temp_rip,
                    temp_uuid, true, 6);
            //printf("SECOND ATTEMP to load %p, result = %d, simcycle = %ld\n", cur_phys_addr, rc, sim_cycle);
            ++wait_count;
            cache_ready_map[cur_phys_addr] = false;
            --request_count;
        }
    }

    //printf("wait_count = %d, column_count = %d\n", wait_count, column_count);
    // When all rows have finished, proceed to the next column
    if unlikely (wait_count == matrix_header.m) {
        printf("Column_count = %d, Wait_count = %d\n", column_count, wait_count);

        // Each time, column count increases by the number of elements per cache
        // line.
        column_count += MEM_REQ_COUNT;
        wait_count = 0;

        for (int i = 0; i < matrix_header.m; ++i) {
            requested[i] = false;
        }
    }

    // When all columns have finished, return true indicates this stage has
    // finished.
    if (column_count >= matrix_header.n) {
        return true;
    } else {
        return false;
    }
}

//Pref_State_Machine()//put Issue_Request_DRAM here
//Address_Gen()
//Pref_load() //Issue Request
//Check_Ready()
//  if(Ready_Queue.size()>0){
//    Pref_load(Ready_Queue[0]);    //we insert data to Buf_Pref in load_virt function
//    Ready_Queue.pop_front();
//  }
//
//  for(int i=0; i<Cache_Line/Element_Size; i++)
//      Buf_DRAM.data.push_back(matrix_data_buf[i]);
//      Buf_DRAM.
//
//  }

//I guess I am gonna make cache_ready_map as a deque, consists of phys_addr

#if 0

void Accelerator::PF_SM_Issuer_DRAM(){

  Issue_Request_DRAM = FALSE;
  Write_DRAM = FALSE;
  
  
  last_pref=FALSE;
  last_req_addr = 0;
  last_req_count=0;

  bool amode;
  int currA;
  int numfetchA;

  //if interrupted from LAPU (store C), we will do 
  //1. save the current state
  //2. Change state to Write C
  //3. Write C to DRAM
  //4. Once finished move to Pref_C
  //5. Set Pref_DRAM_Next to last saved state, clear the interrupt signal
  //only issue when req_count < Mem_Req_Count;
  if (request_count < MAX_REQUEST_COUNT)
  {
  //execute below

  switch (Pref_DRAM_Current){
  
  case Pref_C : //should be the starting state

    Issue_Request_DRAM  = TRUE;
    //Write_DRAM = TRUE;

    IssueC_Next = 
    (IssueC_Current + (Cache_Line/Element_Size))%(Panel_Size*Panel_Size);// this should be improved if C is not square

    //Should tell per core if it is ready

    //cout <<"*************************"<<endl;
    //cout << " I am Pref C " << endl;

   //
   //
   //
   //
   //


   if(IssueC_Current == (Panel_Size*Panel_Size)- (Cache_Line/Element_Size))
    {
      BigC_Next = BigC_Current + 1; //this is kind of counter, right?
      
      last_pref=TRUE;
      last_req_count= IssueC_Current + Cache_Line/Element_Size;

      Pref_DRAM_Next = Pref_B;
      
      if(init_pref){
        Pref_DRAM_Next = Pref_B_Init;
        init_pref=false;
        //printf("firstC takes \n");
        //exit(0);
        //Pref_DRAM_Next = Wait;
      }
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
      //Pref_DRAM_Next = Wait;
      B_SRAM_Done=FALSE;
      IssueB_Next=0;
#ifdef PRINT_DEBUG
      std::cout<< " Moving from Pref_B_Init " <<std::endl;
      std::cout<< " BigB_Next is " << BigB_Next<<std::endl;
#endif
      std::cout<< " Moving from Pref_B_Init " <<std::endl;
      std::cout<< " BigB_Next is " << BigB_Next<<std::endl;
      last_pref=TRUE;
      last_req_count= IssueB_Current + Cache_Line/Element_Size;
      //getchar(); 
      //tell every core that you are ready
    }

  break;



  case Pref_B :

    all_B_Ready=TRUE;
    Issue_Request_DRAM = TRUE;
    
    IssueB_Next = 
    (IssueB_Current + (Cache_Line/Element_Size))%(Kernel_Size*Panel_Size);// this should be improved if C is not square

    //Added later when we incorporate CPU as well
    for (int i=0; i<NumofCore; i++){
    
      if(to_LS->B[i]){
        all_B_Ready=FALSE;
        break;
      }
    }

    //cout << "In pref B" <<endl;

    /*cout << "BigB is  " << BigB_Current<<endl;
    cout << "BigA is  " << BigA_Current<<endl;*/
    
    /*cout << "B_SRAM_Done is " << B_SRAM_Done<<endl;
    cout << "all_B_Ready is " << all_B_Ready<<endl;
    cout << "IssueB_Next is " << IssueB_Next<<endl;
    cout << "IssueB_Current is " << IssueB_Current<<endl;
    cout << "Substract is " << (Kernel_Size*Panel_Size)-(Cache_Line/Element_Size) <<endl;*/
    //getchar();

    if(IssueB_Current == (Kernel_Size*Panel_Size)-(Cache_Line/Element_Size)){
      BigB_Next = BigB_Current + 1;
      Pref_DRAM_Next = Pref_A;
      B_SRAM_Done=FALSE;
      
      last_pref=TRUE;
      last_req_count= IssueB_Current + Cache_Line/Element_Size;
      
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
      cout << "IssueB_Current is " << IssueB_Current<<endl;
      cout << "Moving from B to A" <<endl;
      cout<< " BigB_Next is " << BigB_Next<<endl;
      cout<< " Chunk_B next is " << Chunk_B_Next<<endl;
#endif
      cout << "Moving from B to A" <<endl;
      cout<< " BigB_Next is " << BigB_Next<<endl;
      cout<< " Chunk_B next is " << Chunk_B_Next<<endl;
      cout << "IssueB_Current is " << IssueB_Current<<endl;
      cout << "all_B_Ready is " << all_B_Ready<<endl;
      //getchar();
      //tell every core that you are ready
    }
      
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
      

        if(to_LS->A[per_core_A%NumofCore]==FALSE);//this is the first time we issue
        else{ 
          IssueA_Next = IssueA_Current;
          Issue_Request_DRAM =FALSE;
        }

      
        amode = (howmanyA>NumofCore)? TRUE :FALSE;
  
      if(IssueA_Current== ((per_core_A+1)*Kernel_Size*Kernel_Size)-(Cache_Line/Element_Size)){
        per_core_A = (per_core_A+1);
        if(per_core_A==howmanyA)
          per_core_A=0;
      }


     // if(!last_A_Init){
        if(IssueA_Current == (NumofCore*Kernel_Size*Kernel_Size)- (Cache_Line/Element_Size))
        {
          Pref_DRAM_Next = Pref_Wait_for_Fetch_A_Again_Init;
          //Pref_DRAM_Next = Wait;
          per_core_A_temp = per_core_A;
          A_SRAM_Done=FALSE;
          std::cout << "IssueA_Current is " << IssueA_Current<<std::endl;
          std::cout << "Moving from 1st A " << std::endl;
          std::cout<< " BigA_Next is " << BigA_Next<<std::endl;
          
          last_pref=TRUE;
          last_req_count= IssueA_Current + Cache_Line/Element_Size;
          if(!amode)
            BigA_Next = BigA_Current+1;

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
    
    if (to_LS->A[0]==FALSE && A_SRAM_Done)
    
    {  //that means already consumed in LS
      Pref_DRAM_Next = Pref_A_Init_Second;
      //Pref_DRAM_Next = Wait;
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

      amode = (howmanyA>NumofCore)? TRUE :FALSE;
      
      IssueA_Next = (IssueA_Current + (Cache_Line/Element_Size))%(howmanyA*Kernel_Size*Kernel_Size);// this should be improved if C is not square
      amode = (howmanyA>NumofCore)? TRUE :FALSE;


      if(to_LS->A[per_core_A%NumofCore]==FALSE);//this is the first time we issue
      

      else{ 
      
        IssueA_Next = IssueA_Current;
        Issue_Request_DRAM =FALSE;
      }
      

      if(!amode){
      
        if(IssueA_Current == (NumofCore*Kernel_Size*Kernel_Size)- (Cache_Line/Element_Size))
        {
          Pref_DRAM_Next = Pref_B;
          //Pref_DRAM_Next = Wait;
          A_SRAM_Init=TRUE;
          A_SRAM_Unit= numfetchA;
#ifdef PRINT_DEBUG
          std::cout << "In amode "<<std::endl;
          std::cout << "IssueA_Current is " << IssueA_Current<<std::endl;
          std::cout << "per_core_A is  "<< per_core_A << std::endl;
          std::cout << "Moving from A 2nd to B " << std::endl;
          std::cout<< " BigA_Next is " << BigA_Next<<std::endl;
#endif
          BigA_Current = 1;          
          BigA_Next = BigA_Current + 1;
          if(BigA_Current==HowManyPanel-1)
            Chunk_A_Next=Chunk_A_Curr + 1;
          if (BigA_Next == HowManyPanel)
           BigA_Next = 0;
          
          std::cout << "Moving from A 2nd to B " << std::endl;
          std::cout<< " BigA_Next is " << BigA_Next<<std::endl;
          std::cout << "In amode "<<std::endl;
          std::cout << "IssueA_Current is " << IssueA_Current<<std::endl;
          
          last_pref=TRUE;
          last_req_count= IssueA_Current + Cache_Line/Element_Size;

          //per_core_A = per_core_A + per_core_A_temp;

        //tell every core that you are ready
        }

      }

    
      else if(IssueA_Current == ((NumofCore + numfetchA)*Kernel_Size*Kernel_Size)- (Cache_Line/Element_Size))
      {
        Pref_DRAM_Next = Pref_B;
        //Pref_DRAM_Next = Pref_B_Init;
        //Pref_DRAM_Next = Wait;
        A_SRAM_Init=TRUE;
        A_SRAM_Unit= numfetchA;
#ifdef PRINT_DEBUG
        std::cout << "IssueA_Current is " << IssueA_Current<<std::endl;
        std::cout << "per_core_A is  "<< per_core_A << std::endl;
        std::cout << "Moving from A 2nd to B " << std::endl;
        std::cout<< " BigA_Next is " << BigA_Next<<std::endl;
#endif
        //getchar();
        last_pref=TRUE;
        last_req_count= IssueA_Current + Cache_Line/Element_Size - NumofCore*Kernel_Size*Kernel_Size;

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
  
  case Pref_A :

    Issue_Request_DRAM = TRUE;
    
    //cout << "In Pref A " << endl;
    
    /*if(Chunk_A_Curr==1){
    cout << "IssueA_Current is " << IssueA_Current<<endl;
    //cout << "Chunk_A is " << Chunk_A_Curr<<endl;
    //cout << "BigA is  " << BigA_Current<<endl;
    cout << "to_LS->A is " << to_LS->A[per_core_A%NumofCore]<<endl;
    cout << "per_core_A is " << per_core_A <<endl;
    cout << "A_SRAM_Done is " << A_SRAM_Done<<endl;
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

      cout << "BigA_Current is " << BigA_Current<<endl;
      cout << "Moving from A to B " << endl;
      //getchar();

      //tell every core that I am ready
      //For now, just make it simple, it will synchronize every Kernel*Panel
      //In the LAPU side, if A_Per_Core is more than 1, then need to wait when syncLAPU < syncPref
      //Yeah, this is the key !
      if(BigA_Current == 11){
        cout << "BigA is eleven" <<endl;
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
        cout << "Moving to last B " << endl;
        //getchar();

      }
    }*/
   
    if(IssueA_Current==((per_core_A+1)*Kernel_Size*Kernel_Size)-(Cache_Line/Element_Size)){
         
        per_core_A = (per_core_A+1);
        
        int pos = (howmanyA==2*NumofCore)? FALSE: TRUE;
        bool is_same = (howmanyA==NumofCore)? TRUE:FALSE;

        
        if(is_same){
        
          if(per_core_A==howmanyA && BigA_Current==1 && Chunk_A_Curr!=0){
            Pref_DRAM_Next=Write_C;
            cout << "Moving from A to WriteC" <<endl;
            last_pref=TRUE;
            last_req_count= IssueA_Current + Cache_Line/Element_Size;
          }
          else if(per_core_A==howmanyA && BigA_Current==1 && Chunk_A_Curr!=0 && BigC_Current==NumofPartition){
          
          }
          
          else {
            if (per_core_A==howmanyA){
              Pref_DRAM_Next=Pref_B;
              last_pref=TRUE;
              last_req_count= IssueA_Current + Cache_Line/Element_Size;
              cout << "Moving from A to B" <<endl;
              cout<< " BigA_Current is " << BigA_Current<<endl;
              cout<< " BigA_Next is " << BigA_Next<<endl;
              cout<< " Chunk_A_Curr is " << Chunk_A_Curr<<endl;
            }
            else 
              Pref_DRAM_Next=Pref_A;
          }
        }

        if(per_core_A==2*NumofCore && (BigA_Current!=0 || (BigA_Current==0 && Chunk_A_Curr!=0)) 
            && pos && !is_same){
          Pref_DRAM_Next=Pref_B;
          cout << "Moving from A to B" <<endl;
          cout<< " BigA_Current is " << BigA_Current<<endl;
          cout<< " BigA_Next is " << BigA_Next<<endl;
          cout<< " Chunk_A_Curr is " << Chunk_A_Curr<<endl;
          
          if(BigA_Current==0 && Chunk_A_Curr!=0)
            Pref_DRAM_Next=Write_C;
          
          if(BigA_Current==HowManyPanel-1 && Chunk_A_Curr!=0 && BigC_Current==NumofPartition){
            Pref_DRAM_Next=Pref_A;
            printf("Got you ! \n");
          }
            //cout << "Moving from A to WriteC" <<endl;
          //if(per_core_A==howmanyA)
            //per_core_A=0;
        }
        
        if (per_core_A==howmanyA){
          per_core_A=0;
          
          if(!pos) {
            Pref_DRAM_Next=Pref_B;
            cout << "Moving from A to B" <<endl;
            cout<< " BigA_Current is " << BigA_Current<<endl;
            cout<< " BigA_Next is " << BigA_Next<<endl;
            cout<< " Chunk_A_Curr is " << Chunk_A_Curr<<endl;
          }

          if(!pos)
            if(BigA_Next==1 && Chunk_A_Curr!=0)
              Pref_DRAM_Next=Write_C;

          if(!is_same && BigA_Current==HowManyPanel-1 && Chunk_A_Curr==NumofPartition-1 && BigC_Current==NumofPartition)
            Pref_DRAM_Next=Write_C;             
        }

        
        if (counter_A==NumofCore-1)
          A_SRAM_Done=FALSE;

        /*cout << "counter_A is " << counter_A<<endl;
        cout << "A_SRAM_Done is " << A_SRAM_Done<<endl;*/

        counter_A= (counter_A+1)%NumofCore;
        
    }            
    
    /*if(IssueA_Current==(NumofCore*Kernel_Size*Kernel_Size)-(Cache_Line/Element_Size)){

      A_SRAM_Done=FALSE;
    }*/            


    break;

  case Write_C :

    //need to ensure that output buffer fro SRAM is ready here. i.e need to check buffer at SRAM
    
    //std::cout << "In Write C " << std::endl;
    //printf(" buf sram size %d\n", Buf_from_SRAM->buf.size());

    if (Buf_from_SRAM->buf.size()>=Cache_Line/Element_Size){

    //std::cout << "In Write C " << std::endl;
    
      
      //std::cout << " I am at write_C " <<std::endl;
      //getchar();
    
      Issue_Request_DRAM = TRUE;
      Write_DRAM = TRUE;

      WriteC_Next = 
      (WriteC_Current + (Cache_Line/Element_Size))%(Panel_Size*Panel_Size);// this should be improved if C is not square

      if(WriteC_Current == (Panel_Size*Panel_Size)- (Cache_Line/Element_Size))
      {
        //test
        Pref_DRAM_Next = Pref_C;
        std::cout << "Moving to Pref_C from Write_C"<<std::endl;
        if (BigC_Current==NumofPartition)
          Pref_DRAM_Next = Wait;
      }
    }
    break;
  }
  }
}
#endif

void Accelerator::PF_SM_Issuer_DRAM(){

  Issue_Request_DRAM = FALSE;
  Write_DRAM = FALSE;
  Partial_Line = FALSE;
  Partial_Frac = 0;
  
  
  last_pref=FALSE;
  last_req_addr = 0;
  last_req_count=0;

  bool amode;
  int currA;
  int numfetchA;

  //if interrupted from LAPU (store C), we will do 
  //1. save the current state
  //2. Change state to Write C
  //3. Write C to DRAM
  //4. Once finished move to Pref_C
  //5. Set Pref_DRAM_Next to last saved state, clear the interrupt signal
  //only issue when req_count < Mem_Req_Count;
  if (request_count < MAX_REQUEST_COUNT)
  {
  //execute below


  if(NON_PART){
  
    switch (Pref_DRAM_Current){

    case Pref_C : //should be the starting state
    
      Issue_Request_DRAM  = TRUE;

      IssueC_Next = 
      (IssueC_Current + (Cache_Line/Element_Size))%(Panel_Size*Panel_Size);// this should be improved if C is not square

      //Fetch the last cache line

     if(IssueC_Current == (Panel_Size*Panel_Size)- (Cache_Line/Element_Size))
      {
        BigC_Next = BigC_Current + 1; //this is kind of counter, right?
        
        last_pref=TRUE;
        last_req_count= IssueC_Current + Cache_Line/Element_Size;

        Pref_DRAM_Next = Pref_B;
      }

    
    break ;

    case Pref_B :
    
      Issue_Request_DRAM  = TRUE;
      IssueB_Next = (IssueB_Current + (Cache_Line/Element_Size))%(Panel_Size*KC);// this should be improved if C is not square
      
        //the last cache line of B
       if(IssueB_Current == (Panel_Size*KC)- (Cache_Line/Element_Size))
      {
        BigB_Next = BigB_Current + 1;
        Pref_DRAM_Next = Pref_A;
        B_SRAM_Done=FALSE;
        IssueB_Next=0;
        
        if(BigB_Next==HowManyPanel){
          BigB_Next = 0;
          Chunk_B_Next++;
        }

        std::cout<< " Moving from Pref_B_Init " <<std::endl;
        std::cout<< " BigB_Next is " << BigB_Next<<std::endl;
        last_pref=TRUE;
        last_req_count= IssueB_Current + Cache_Line/Element_Size;
        //Pref_DRAM_Next =Wait;
        //getchar(); 
        //tell every core that you are ready
      }

    break;

    case Pref_A :

      //Here, it depends whether you are only fetching one block or many blocks of A
      // N/Kc >= 4
      // In this case, howmanyPart is only 1 
      // That means block of A could be 1~3

      // Also howmanyA should be built configurable
      // HowMany Panel also

      Issue_Request_DRAM = TRUE;
  
      IssueA_Next = (IssueA_Current + (Cache_Line/Element_Size))%(howmanyA*KC*KC);// this should be improved if C is not square

      //if not done loading SRAM, then stopped issuing request 

      if(to_LS->A[per_core_A%NumofCore]==FALSE && A_SRAM_Done) ;
      else{
        IssueA_Next = IssueA_Current;
        Issue_Request_DRAM=FALSE;
        break;
      }
     
      //amode tells whether total A per core larger than NumofCore or not

      amode = (howmanyA>NumofCore)? TRUE :FALSE;
     
      if(IssueA_Current == ((per_core_A+1)*NumofCore*KC*KC)- (Cache_Line/Element_Size))
      {
        Pref_DRAM_Next = Pref_A;
        per_core_A_temp = per_core_A;
        A_SRAM_Done=FALSE;
        std::cout << "IssueA_Current is " << IssueA_Current<<std::endl;
        std::cout << "Moving from 1st A " << std::endl;
        std::cout<< " BigA_Next is " << BigA_Next<<std::endl;
        last_pref=TRUE;
        last_req_count= IssueA_Current + Cache_Line/Element_Size;
        
        //Pref_DRAM_Next = Wait;
      }

      if(IssueA_Current== ((per_core_A+1)*KC*KC)-(Cache_Line/Element_Size)){
        per_core_A = (per_core_A+1);
        if(per_core_A==howmanyA){
          per_core_A=0;
          Pref_DRAM_Next=Write_C;
        
        }
      }
    
    break;
  
    case Pref_Wait_for_Fetch_A_Again_Init:
   

    //Here we wait until core has fetched the initial A

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
        //Pref_DRAM_Next = Wait;
        
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
    
    case Write_C :

    static int comt=0;
      //need to ensure that output buffer fro SRAM is ready here. i.e need to check buffer at SRAM
#if 0    
    /*if(comt==3)*/{
      std::cout << "In Write C " << std::endl;
      std::cout << " Write C curr " << WriteC_Current<<std::endl;
      std::cout << " buf size curr " << Buffer->buf.size()<<std::endl;
      std::cout << " buf dram size " << Buf_DRAM->buf.size()<<std::endl;
      std::cout << "BigC_Current " << BigC_Current<<std::endl;
      //getchar();
    }
#endif

      //exit(0);
        
      //std::cout << " I am at write_C " <<std::endl;

      if (Buf_from_SRAM->buf.size()>=Cache_Line/Element_Size){
        
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
          //if (comt==3)exit(0);
          //comt++;
          if (comt==4) 
            exit(0);
          if (BigC_Current==NumofPartition)
            Pref_DRAM_Next = Wait;
        }
      }

      break;

    }
  
  }

  else {

    //This is for the case when we do partitioning computation


  switch (Pref_DRAM_Current){
  
  case Pref_C : //should be the starting state

    Issue_Request_DRAM  = TRUE;
    //Write_DRAM = TRUE;

    IssueC_Next = 
    (IssueC_Current + (Cache_Line/Element_Size))%(Panel_V*Panel_H);// this should be improved if C is not square

    //Patched for cache line which is not divisible by 8

#ifdef PATCHED    
    if ((Panel_H- (IssueC_Current%Panel_H))<(Cache_Line/Element_Size)){
      IssueC_Next = (IssueC_Current + (Panel_H- (IssueC_Current%Panel_H)))%(Panel_V*Panel_H);
      Partial_Line = TRUE;
      Partial_Frac = Panel_H- IssueC_Current%Panel_H;
      printf("Issue Current at non-divisiblae is %d\n", IssueC_Current);
      printf("Partial Frac is %d\n", Partial_Frac);
      
    }
#endif
    //the last line of C

   if((IssueC_Current == (Panel_V*Panel_H)- (Cache_Line/Element_Size)) ||
       (IssueC_Current == (Panel_V*Panel_H)- Partial_Frac))
    {
      BigC_Next = BigC_Current + 1; //this is kind of counter, right?
      
      last_pref=TRUE;
      last_req_count= IssueC_Current + Cache_Line/Element_Size;

      Pref_DRAM_Next = Pref_B;
      
      if(init_pref){
        Pref_DRAM_Next = Pref_B_Init;
        init_pref=false;
        printf("firstC takes \n");
        //exit(0);
        //Pref_DRAM_Next = Wait;
      }
    
      if(BigC_Current==NumofPartition-1){
      
        if(fetched_B == HowManyPanel){
        
          if(fetched_A_total==howmanyA*HowManyPanel){
            Pref_DRAM_Next = Write_C;
          }

          else 
            Pref_DRAM_Next = Pref_A;
        } 
      }
   
    }

    break;

  case Pref_B_Init:

    Issue_Request_DRAM  = TRUE;
    IssueB_Next = (IssueB_Current + (Cache_Line/Element_Size))%(Panel_H*Kernel_H);// this should be improved if C is not square
   /*std::cout<< " I am at Pref_B_Init " <<std::endl;
   std::cout<< " IssueB Next is " << IssueB_Next<<std::endl;*/
   //getchar();
#ifdef PATCHED
    if ((Kernel_H- (IssueB_Current%Kernel_H))<(Cache_Line/Element_Size)){
      IssueB_Next = (IssueB_Current + (Kernel_H- (IssueB_Current%Kernel_H)))%(Panel_H*Kernel_H);
      Partial_Line = TRUE;
      Partial_Frac = Kernel_H- IssueB_Current%Kernel_H;
      printf("Issue Current at non-divisiblae is %d\n", IssueB_Current);
      printf("Partial Frac is %d\n", Partial_Frac); 
    }
#endif
    //the last line of B
     if((IssueB_Current == (Panel_H*Kernel_H)- (Cache_Line/Element_Size)) || 
        (IssueB_Current == (Panel_H*Kernel_H)- Partial_Frac))
    {
      BigB_Next = BigB_Current + 1;
      Pref_DRAM_Next = Pref_A_Init;
      //Pref_DRAM_Next = Wait;
      B_SRAM_Done=FALSE;
      IssueB_Next=0;

      if(BigB_Next==HowManyPanel){
        BigB_Next = 0;
        Chunk_B_Next++;
      } 
      
      std::cout<< " Moving from Pref_B_Init " <<std::endl;
      std::cout<< " BigB_Next is " << BigB_Next<<std::endl;
      last_pref=TRUE;
      last_req_count= IssueB_Current + Cache_Line/Element_Size;
      //getchar(); 
      //tell every core that you are ready
    }

  break;



  case Pref_B :

    all_B_Ready=TRUE;
    Issue_Request_DRAM = TRUE;
        
    IssueB_Next = 
    (IssueB_Current + (Cache_Line/Element_Size))%(Kernel_H*Panel_H);// this should be improved if C is not square
#ifdef PATCHED
    if ((Kernel_H- (IssueB_Current%Kernel_H))<(Cache_Line/Element_Size)){
      IssueB_Next = (IssueB_Current + (Kernel_H- (IssueB_Current%Kernel_H)))%(Panel_H*Kernel_H);
      Partial_Line = TRUE;
      Partial_Frac = Kernel_H- IssueB_Current%Kernel_H;
      printf("Issue Current at non-divisiblae is %d\n", IssueB_Current);
      printf("Partial Frac is %d\n", Partial_Frac);
      
    }
#endif
    //Added later when we incorporate CPU as well
    for (int i=0; i<NumofCore; i++){
    
      if(to_LS->B[i]){
        all_B_Ready=FALSE;
        break;
      }
    }

    //cout << "In pref B" <<endl;

    /*cout << "BigB is  " << BigB_Current<<endl;
    cout << "BigA is  " << BigA_Current<<endl;*/
    
    /*cout << "B_SRAM_Done is " << B_SRAM_Done<<endl;
    cout << "all_B_Ready is " << all_B_Ready<<endl;
    cout << "IssueB_Next is " << IssueB_Next<<endl;
    cout << "IssueB_Current is " << IssueB_Current<<endl;
    cout << "Substract is " << (Kernel_Size*Panel_Size)-(Cache_Line/Element_Size) <<endl;*/
    //getchar();

     if((IssueB_Current == (Panel_H*Kernel_H)- (Cache_Line/Element_Size)) || 
        (IssueB_Current == (Panel_H*Kernel_H)- Partial_Frac))
     {
      BigB_Next = BigB_Current + 1;
      Pref_DRAM_Next = Pref_A;
      B_SRAM_Done=FALSE;
      //Pref_DRAM_Next = Wait;

      //printf("Done fetching second B \n");
      //exit(0);
      
      last_pref=TRUE;
      last_req_count= IssueB_Current + Cache_Line/Element_Size;
      
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
      cout << "IssueB_Current is " << IssueB_Current<<endl;
      cout << "Moving from B to A" <<endl;
      cout<< " BigB_Next is " << BigB_Next<<endl;
      cout<< " Chunk_B next is " << Chunk_B_Next<<endl;
#endif
      cout << "Moving from B to A" <<endl;
      cout<< " BigB_Next is " << BigB_Next<<endl;
      cout<< " Chunk_B next is " << Chunk_B_Next<<endl;
      cout << "IssueB_Current is " << IssueB_Current<<endl;
      cout << "all_B_Ready is " << all_B_Ready<<endl;
      //getchar();
      //tell every core that you are ready
    }
      
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
  
      //printf("in prefA init\n");  

      Issue_Request_DRAM = TRUE;
  
      IssueA_Next = (IssueA_Current + (Cache_Line/Element_Size))%(howmanyA*Kernel_V*Kernel_H);// this should be improved if C is not square
 
#ifdef PATCHED
      if ((Kernel_V- (IssueA_Current%Kernel_V))<(Cache_Line/Element_Size)){
        IssueA_Next = (IssueA_Current + (Kernel_V- (IssueA_Current%Kernel_V)))%(howmanyA*Kernel_V*Kernel_H);
        Partial_Line = TRUE;
        Partial_Frac = Kernel_V- IssueA_Current%Kernel_V;
        printf("IssueA Current at non-divisiblae is %d\n", IssueA_Current);
        
        if(Partial_Frac>(Cache_Line/Element_Size))
          printf("Partial Frac A is %d\n", Partial_Frac);
      }
#endif
        if(to_LS->A[per_core_A%NumofCore]==FALSE);//this is the first time we issue
        else{ 
          IssueA_Next = IssueA_Current;
          Issue_Request_DRAM =FALSE;
        }

      
        amode = (howmanyA>NumofCore)? TRUE :FALSE;
  
      if((IssueA_Current== ((per_core_A+1)*Kernel_V*Kernel_H)-(Cache_Line/Element_Size)) ||
         (IssueA_Current== ((per_core_A+1)*Kernel_V*Kernel_H) - Partial_Frac) ) 
      {
        per_core_A = (per_core_A+1);
        if(per_core_A==howmanyA)
          per_core_A=0;
      }


     // if(!last_A_Init){
        if((IssueA_Current == (NumofCore*Kernel_V*Kernel_H)- (Cache_Line/Element_Size)) ||
            (IssueA_Current == (NumofCore*Kernel_V*Kernel_H) - Partial_Frac))
        {
          Pref_DRAM_Next = Pref_Wait_for_Fetch_A_Again_Init;
          //Pref_DRAM_Next = Wait;
          per_core_A_temp = per_core_A;
          A_SRAM_Done=FALSE;
          std::cout << "IssueA_Current is " << IssueA_Current<<std::endl;
          std::cout << "Moving from 1st A " << std::endl;
          std::cout<< " BigA_Next is " << BigA_Next<<std::endl;
          //exit(0);
          last_pref=TRUE;
          last_req_count= IssueA_Current + Cache_Line/Element_Size;
          if(!amode){
            BigA_Next = BigA_Current+1;
            if(BigA_Next == HowManyPanel){
              Chunk_A_Next= Chunk_A_Curr + 1;
              BigA_Next=0;
            
            }
          }

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
    
    if (to_LS->A[0]==FALSE && A_SRAM_Done)
    
    {  //that means already consumed in LS
      Pref_DRAM_Next = Pref_A_Init_Second;
      //Pref_DRAM_Next = Wait;
      //per_core_A=0;
      last_A_Init=TRUE;
      A_SRAM_Done=FALSE;
      printf("moving from waiting\n");
      //exit(0);
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


  
  case Pref_A_Init_Second:


    //second time we send the initial A to PEs

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

      IssueA_Next = (IssueA_Current + (Cache_Line/Element_Size))%(howmanyA*Kernel_V*Kernel_H);// this should be improved if C is not square
      
#ifdef PATCHED
      if ((Kernel_V- (IssueA_Current%Kernel_V))<(Cache_Line/Element_Size)){
        IssueA_Next = (IssueA_Current + (Kernel_V- (IssueA_Current%Kernel_V)))%(howmanyA*Kernel_V*Kernel_H);
        Partial_Line = TRUE;
        Partial_Frac = Kernel_V- IssueA_Current%Kernel_V;
        printf("IssueA Current at non-divisiblae is %d\n", IssueA_Current);
        
        if(Partial_Frac>(Cache_Line/Element_Size))
          printf("Partial Frac A is %d\n", Partial_Frac);
      }
#endif

      amode = (howmanyA>NumofCore)? TRUE :FALSE;


      if(to_LS->A[per_core_A%NumofCore]==FALSE);//this is the first time we issue
      
      else{ 
      
        IssueA_Next = IssueA_Current;
        Issue_Request_DRAM =FALSE;
      }
      

      if(!amode){
      
        if((IssueA_Current == (NumofCore*Kernel_V*Kernel_H)- (Cache_Line/Element_Size))||
            (IssueA_Current == (NumofCore*Kernel_V*Kernel_H) - Partial_Frac))

        {
          Pref_DRAM_Next = Pref_B;
          //Pref_DRAM_Next = Wait;
          A_SRAM_Init=TRUE;
          A_SRAM_Unit= numfetchA;
#ifdef PRINT_DEBUG
          std::cout << "In amode "<<std::endl;
          std::cout << "IssueA_Current is " << IssueA_Current<<std::endl;
          std::cout << "per_core_A is  "<< per_core_A << std::endl;
          std::cout << "Moving from A 2nd to B " << std::endl;
          std::cout<< " BigA_Next is " << BigA_Next<<std::endl;
#endif
          BigA_Current = 1;          
          BigA_Next = BigA_Current + 1;
          
          if(BigA_Current==HowManyPanel){
            Chunk_A_Next=Chunk_A_Curr + 1; 
            BigA_Next = 0;
          }
        
          std::cout << "Moving from A 2nd to B " << std::endl;
          std::cout<< " BigA_Next is " << BigA_Next<<std::endl;
          std::cout << "In amode "<<std::endl;
          std::cout << "IssueA_Current is " << IssueA_Current<<std::endl;
          
          last_pref=TRUE;
          last_req_count= IssueA_Current + Cache_Line/Element_Size;

          //per_core_A = per_core_A + per_core_A_temp;

        //tell every core that you are ready
        }

      }

    
      else if((IssueA_Current == ((NumofCore + numfetchA)*Kernel_V*Kernel_H)- (Cache_Line/Element_Size)) ||
              (IssueA_Current == ((NumofCore + numfetchA)*Kernel_V*Kernel_H) - Partial_Frac))
      {
        Pref_DRAM_Next = Pref_B;
        //Pref_DRAM_Next = Pref_B_Init;
        //Pref_DRAM_Next = Wait;
        A_SRAM_Init=TRUE;
        A_SRAM_Unit= numfetchA;
        printf("Done with A second \n");
        //exit(0);

#ifdef PRINT_DEBUG
        std::cout << "IssueA_Current is " << IssueA_Current<<std::endl;
        std::cout << "per_core_A is  "<< per_core_A << std::endl;
        std::cout << "Moving from A 2nd to B " << std::endl;
        std::cout<< " BigA_Next is " << BigA_Next<<std::endl;
#endif
        //getchar();
        last_pref=TRUE;
        last_req_count= IssueA_Current + Cache_Line/Element_Size - NumofCore*Kernel_V*Kernel_H;

        if(howmanyA==NumofCore + numfetchA){
          BigA_Next = BigA_Current + 1;
          
          if(BigA_Current==HowManyPanel-1){
            Chunk_A_Next=Chunk_A_Curr + 1;
            BigA_Next = 0;
          }
        }
        //per_core_A = per_core_A + per_core_A_temp;

      //tell every core that you are ready
      }
  
    if((IssueA_Current== ((per_core_A+1)*Kernel_V*Kernel_H)-(Cache_Line/Element_Size)) ||
         (IssueA_Current== ((per_core_A+1)*Kernel_V*Kernel_H) - Partial_Frac) ) 
    {
       
      per_core_A = (per_core_A+1);
      if(per_core_A==howmanyA)
        per_core_A=0;
    }

  }

  break;
  
  case Pref_A :

    Issue_Request_DRAM = TRUE;
    
    //cout << "In Pref A " << endl;
    
    /*if(Chunk_A_Curr==1){
    cout << "IssueA_Current is " << IssueA_Current<<endl;
    //cout << "Chunk_A is " << Chunk_A_Curr<<endl;
    //cout << "BigA is  " << BigA_Current<<endl;
    cout << "to_LS->A is " << to_LS->A[per_core_A%NumofCore]<<endl;
    cout << "per_core_A is " << per_core_A <<endl;
    cout << "A_SRAM_Done is " << A_SRAM_Done<<endl;
    }*/

    IssueA_Next = 
    (IssueA_Current + (Cache_Line/Element_Size))%(howmanyA*Kernel_V*Kernel_H);// this should be improved if C is not square
    //int currA = to_LS->currentpartA[per_core%NumofCore]
    //if(to_LS->A[per_core_A%NumofCore][currA] ==FALSE);
    //

#ifdef PATCHED
    if ((Kernel_V- (IssueA_Current%Kernel_V))<(Cache_Line/Element_Size)){
      IssueA_Next = (IssueA_Current + (Kernel_V- (IssueA_Current%Kernel_V)))%(howmanyA*Kernel_V*Kernel_H);
      Partial_Line = TRUE;
      Partial_Frac = Kernel_V- IssueA_Current%Kernel_V;
      printf("IssueA Current at non-divisiblae is %d\n", IssueA_Current);
      
      if(Partial_Frac>(Cache_Line/Element_Size))
        printf("Partial Frac A is %d\n", Partial_Frac);
    }
#endif

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

  if((IssueA_Current == (howmanyA*Kernel_V*Kernel_H)- (Cache_Line/Element_Size)) ||
      (IssueA_Current == (howmanyA*Kernel_V*Kernel_H) - Partial_Frac))
   { 
      BigA_Next = BigA_Current + 1;
      if(BigA_Current==HowManyPanel-1)
        Chunk_A_Next=Chunk_A_Curr + 1;

      if (BigA_Next == HowManyPanel)
        BigA_Next = 0;
   }
   
    if ((IssueA_Current==((per_core_A+1)*Kernel_V*Kernel_H)-(Cache_Line/Element_Size)) ||
        (IssueA_Current==((per_core_A+1)*Kernel_V*Kernel_H)-(Partial_Frac))){
         
        per_core_A = (per_core_A+1);
        
        int pos = (howmanyA==2*NumofCore)? FALSE: TRUE;
        bool is_same = (howmanyA==NumofCore)? TRUE:FALSE;

        
        if(is_same){
        
          
          if(per_core_A==howmanyA && (BigA_Current==1 || (BigA_Current==0 && HowManyPanel==1))
                && Chunk_A_Curr!=0 /*&& BigC_Current!=NumofPartition*/){
            Pref_DRAM_Next=Write_C;
            cout << "Moving from A to WriteC" <<endl;
            last_pref=TRUE;
            last_req_count= IssueA_Current + Cache_Line/Element_Size;
          }
        
            else if(per_core_A==howmanyA && (BigA_Current==1 || (BigA_Current==0 && HowManyPanel==1))
                && Chunk_A_Curr!=0 && BigC_Current==NumofPartition){
          }
          
          else {
            if (per_core_A==howmanyA){
              Pref_DRAM_Next=Pref_B;
              last_pref=TRUE;
              last_req_count= IssueA_Current + Cache_Line/Element_Size;
              cout << "Moving from A to B" <<endl;
              cout<< " BigA_Current is " << BigA_Current<<endl;
              cout<< " BigA_Next is " << BigA_Next<<endl;
              cout<< " Chunk_A_Curr is " << Chunk_A_Curr<<endl;
            }
            else 
              Pref_DRAM_Next=Pref_A;
          }
        }

        if(per_core_A==2*NumofCore && (BigA_Current!=0 || (BigA_Current==0 && Chunk_A_Curr!=0)) 
            && pos && !is_same){
          Pref_DRAM_Next=Pref_B;
          cout << "Moving from A to B" <<endl;
          cout<< " BigA_Current is " << BigA_Current<<endl;
          cout<< " BigA_Next is " << BigA_Next<<endl;
          cout<< " Chunk_A_Curr is " << Chunk_A_Curr<<endl;
          
          if(BigA_Current==0 && Chunk_A_Curr!=0)
            Pref_DRAM_Next=Write_C;
          
          if(BigA_Current==HowManyPanel-1 && Chunk_A_Curr!=0 && BigC_Current==NumofPartition){
            Pref_DRAM_Next=Pref_A;
            printf("Got you ! \n");
          }
            //cout << "Moving from A to WriteC" <<endl;
          //if(per_core_A==howmanyA)
            //per_core_A=0;
        }
        
        if (per_core_A==howmanyA){
          per_core_A=0;
          
          if(!pos) {
            Pref_DRAM_Next=Pref_B;
            cout << "Moving from A to B" <<endl;
            cout<< " BigA_Current is " << BigA_Current<<endl;
            cout<< " BigA_Next is " << BigA_Next<<endl;
            cout<< " Chunk_A_Curr is " << Chunk_A_Curr<<endl;
          }

          if(!pos)
            if(BigA_Current==0 && Chunk_A_Curr!=0)
              Pref_DRAM_Next=Write_C;

          if(!is_same && BigA_Current==HowManyPanel-1 /*&& Chunk_A_Curr==NumofPartition-1*/ && BigC_Current==NumofPartition)
            Pref_DRAM_Next=Write_C;             
        }

        
        if (counter_A==NumofCore-1)
          A_SRAM_Done=FALSE;

        /*cout << "counter_A is " << counter_A<<endl;
        cout << "A_SRAM_Done is " << A_SRAM_Done<<endl;*/

        counter_A= (counter_A+1)%NumofCore;
        
    }            
    
    /*if(IssueA_Current==(NumofCore*Kernel_Size*Kernel_Size)-(Cache_Line/Element_Size)){

      A_SRAM_Done=FALSE;
    }*/            


    break;

  case Write_C :

    //need to ensure that output buffer fro SRAM is ready here. i.e need to check buffer at SRAM
    
    //std::cout << "In Write C " << std::endl;

   // printf(" buf sram size %d\n", Buf_from_SRAM->buf.size());
    //printf(" WriteC_Current %d\n", WriteC_Current);

#if 0    
    if (((Buf_from_SRAM->buf.size()>=Cache_Line/Element_Size)) ||
         ((Buf_from_SRAM->buf.size()>=(Panel_H%(Cache_Line/Element_Size))) && (Panel_H - (WriteC_Current%Panel_H))==(Panel_H%(Cache_Line/Element_Size)))
        )
#endif    
    int residue = (Panel_H%(Cache_Line/Element_Size));

     
    if ((Buf_from_SRAM->buf.size()>=Cache_Line/Element_Size) || 
        ((Buf_from_SRAM->buf.size()>= residue) && 
        (WriteC_Current == (Panel_V*Panel_H)- residue)))
    {

    //std::cout << "In Write C " << std::endl;
    
      
      //std::cout << " I am at write_C " <<std::endl;
      //getchar();
    
      Issue_Request_DRAM = TRUE;
      Write_DRAM = TRUE;

      WriteC_Next = 
      (WriteC_Current + (Cache_Line/Element_Size))%(Panel_V*Panel_H);// this should be improved if C is not square

#ifdef PATCHED
      if ((Panel_H- (WriteC_Current%Panel_H))<(Cache_Line/Element_Size)){
        WriteC_Next = (WriteC_Current + (Panel_H- (WriteC_Current%Panel_H)))%(Panel_V*Panel_H);
        Partial_Line = TRUE;
        Partial_Frac = Panel_H- WriteC_Current%Panel_H;
        
        printf("Partial Frac at Write C %d \n", Partial_Frac);
        printf("Write_C is %d \n", WriteC_Current);
      }
#endif

      if((WriteC_Current == (Panel_V*Panel_H)- (Cache_Line/Element_Size)) ||
       (WriteC_Current == (Panel_V*Panel_H)- Partial_Frac))
      {
        //test
        Pref_DRAM_Next = Pref_C;
        printf("Done storing C \n");
        //Pref_DRAM_Next = Wait;
        //exit(0);
        std::cout << "Moving to Pref_C from Write_C"<<std::endl;
        if (BigC_Current==NumofPartition)
          Pref_DRAM_Next = Wait;
      }
    }
    break;
    }
   }
  }
}

#if 0
void Accelerator::Address_Gen_DRAM(){

  switch(Pref_DRAM_Current){
  
  case Pref_C : //assuming for square

  y_offset = (BigC_Current==1 || BigC_Current==3)? Panel_Size: 0;  //(BYTE*8) is size of 1 element
  x_offset = (BigC_Current==2 || BigC_Current==3)? (Panel_Size*Panel_Size*2): 0;

  if(Issue_Request_DRAM){
      Address_DRAM = (IssueC_Current/Panel_Size)*(Panel_Size+Panel_Size)
      + IssueC_Current%Panel_Size + y_offset + x_offset; //need to add offset here which I believe should be BigC_Current*Panel_Size
      /*for(int j=0; j<8; j++)
        addrbuf[addridx+j] = Address_DRAM + j;

      addridx = addridx +8;*/
  
      Phys_Addr= Address_DRAM*(BYTE) + temp_phys_addr_C; // means that 1 elements consist of 8 Bytes
  
      Virt_Addr= Address_DRAM*(BYTE) + temp_virt_addr_C; // means that 1 elements consist of 8 Bytes
      /*cout << " I am at Address_Gen" <<endl;
      cout << " IssueC_Current is " << IssueC_Current<<endl;
      cout << " y_ofsett is " << y_offset<<endl;
      cout << " x_offset is " << x_offset<<endl;
      cout << " Panel_Size is " << Panel_Size<<endl;
      cout << " Address_DRAM is  " << Address_DRAM<<endl;
      cout << " Its value  is  " << DRAM[Address_DRAM]<<endl;*/
      
      //getchar();
      
      Address_SRAM = IssueC_Current;
    }
    //TODO : Confirm this addressing works

  break;

  case Pref_B_Init:
  
  y_offset = ((Chunk_B_Curr%2)==1)? Panel_Size: 0;
  x_offset = (BigB_Current*Kernel_Size)*Panel_Size*2;
  
  //offset_C = Panel_Size*Panel_Size*NumofPartition;
  offset_C = 0;

  if(Issue_Request_DRAM){
    Address_DRAM = offset_C + (IssueB_Current/Panel_Size)*(Panel_Size+Panel_Size)+ IssueB_Current%Panel_Size + y_offset + x_offset;
    Address_SRAM = IssueB_Current + Panel_Size*Panel_Size +((BigB_Current%2)*(Panel_Size*Kernel_Size));  //Panel_Size*Panel_Size is offset of C 
    
    Phys_Addr= Address_DRAM*(BYTE) + temp_phys_addr_B; // means that 1 elements consist of 8 Bytes
    
    Virt_Addr= Address_DRAM*(BYTE) + temp_virt_addr_B; // means that 1 elements consist of 8 Bytes
  
  }

 /* cout << " IssueB_Current is " << IssueB_Current<<endl;
  cout << " y_ofsett is " << y_offset<<endl;
  cout << " x_offset is " << x_offset<<endl;
  cout << " Address_DRAM in PrefB init is  " << Address_DRAM<<endl;
  cout << " Its value in PrefB is init  " << DRAM[Address_DRAM]<<endl;
  cout << " Address_SRAM in PrefB is init  " << Address_SRAM<<endl;
  *///getchar();

  break;

  case Pref_A_Init:
  
    x_offset = ((Chunk_A_Curr)>=2)? Panel_Size*Panel_Size*2: 0;
  y_offset = BigA_Current*Kernel_Size;
  
  //offset_B = Panel_Size*Panel_Size*NumofPartition*2;
  offset_B = 0;
 
  if(Issue_Request_DRAM){
    Address_DRAM = offset_B + (IssueA_Current/Kernel_Size)*(Panel_Size+Panel_Size) + IssueA_Current%Kernel_Size + x_offset + y_offset;
    Address_SRAM = IssueA_Current%(NumofCore*Kernel_Size*Kernel_Size) + (Panel_Size*Panel_Size + 2*Kernel_Size*Panel_Size) ;// Consider offset for SRAM also 
    
    Phys_Addr= Address_DRAM*(BYTE) + temp_phys_addr_A; // means that 1 elements consist of 8 Bytes
    
    Virt_Addr= Address_DRAM*(BYTE) + temp_virt_addr_A; // means that 1 elements consist of 8 Bytes
  }

  /*cout << " IssueA_Current in 1st is  " << IssueA_Current<<endl;
  cout << " Address_DRAM in PrefA init 1st is " << Address_DRAM<<endl;
  cout << " Its value in PrefA init is 1st  " << DRAM[Address_DRAM]<<endl;
  cout << " Address_SRAM in PrefA init is 1st " << Address_SRAM<<endl;
  *///getchar();

  break;
  
  case Pref_A_Init_Second:
  
  x_offset = ((Chunk_A_Curr)>=2)? Panel_Size*Panel_Size*2: 0;
  y_offset = BigA_Current*Kernel_Size;
  
  //offset_B = Panel_Size*Panel_Size*NumofPartition*2;
  offset_B = 0;
 
  if(Issue_Request_DRAM){
    Address_DRAM = offset_B + (IssueA_Current/Kernel_Size)*(Panel_Size+Panel_Size) + IssueA_Current%Kernel_Size + x_offset + y_offset;
    Address_SRAM = IssueA_Current%(NumofCore*Kernel_Size*Kernel_Size) + (Panel_Size*Panel_Size + 2*Kernel_Size*Panel_Size) ;// Consider offset for SRAM also 
    
    Phys_Addr= Address_DRAM*(BYTE) + temp_phys_addr_A; // means that 1 elements consist of 8 Bytes
    
    Virt_Addr= Address_DRAM*(BYTE) + temp_virt_addr_A; // means that 1 elements consist of 8 Bytes
  }

  /*cout << " IssueA_Current is 2nd " << IssueA_Current<<endl;
  cout << " Address_DRAM in PrefA init is 2nd " << Address_DRAM<<endl;
  cout << " Its value in PrefA init is  2nd " << DRAM[Address_DRAM]<<endl;
  cout << " Address_SRAM in PrefA init is 2nd" << Address_SRAM<<endl;
  *///getchar();

  break;

  case Pref_B:
  
  y_offset = ((Chunk_B_Curr%2)==1)? Panel_Size: 0;
  x_offset = (BigB_Current*Kernel_Size)*Panel_Size*2;
  
  //offset_C = Panel_Size*Panel_Size*NumofPartition;
  offset_C = 0; //for marss Masri
  
  /*cout << " IssueB_Current is " << IssueB_Current<<endl;
  cout << " y_ofsett is " << y_offset<<endl;
  cout << " x_offset is " << x_offset<<endl;
  cout << " Address_DRAM in PrefB init is  " << Address_DRAM<<endl;
  cout << " Its value in PrefB is init  " << DRAM[Address_DRAM]<<endl;
  cout << " Address_SRAM in PrefB is init  " << Address_SRAM<<endl;*/

  if(Issue_Request_DRAM){
    Address_DRAM = offset_C + (IssueB_Current/Panel_Size)*(Panel_Size+Panel_Size)+ IssueB_Current%Panel_Size + y_offset + x_offset;
  
    Phys_Addr= Address_DRAM*(BYTE) + temp_phys_addr_B; // means that 1 elements consist of 8 Bytes
  
    Virt_Addr = Address_DRAM*(BYTE) + temp_virt_addr_B; // means that 1 elements consist of 8 Bytes
    
    
    Address_SRAM = IssueB_Current + Panel_Size*Panel_Size +((BigB_Current%2)*(Panel_Size*Kernel_Size));  //Panel_Size*Panel_Size is offset of C 
  
  }
  break;

  case Pref_A:
  
  x_offset = ((Chunk_A_Curr)>=2)? Panel_Size*Panel_Size*2: 0;
  y_offset = BigA_Current*Kernel_Size;
  
  //offset_B = Panel_Size*Panel_Size*NumofPartition*2;
  offset_B = 0;
 
  if(Issue_Request_DRAM){
    Address_DRAM = offset_B + (IssueA_Current/Kernel_Size)*(Panel_Size+Panel_Size) + IssueA_Current%Kernel_Size + x_offset + y_offset;
    Address_SRAM = IssueA_Current%(NumofCore*Kernel_Size*Kernel_Size) + (Panel_Size*Panel_Size + 2*Kernel_Size*Panel_Size) ;// Consider offset for SRAM also 
    Phys_Addr= Address_DRAM*(BYTE) + temp_phys_addr_A; // means that 1 elements consist of 8 Bytes
    
    Virt_Addr= Address_DRAM*(BYTE) + temp_virt_addr_A; // means that 1 elements consist of 8 Bytes
  }

  
  /*cout << " y_offset is  is " << y_offset<<endl;
  cout << " Address_DRAM in PrefA is " << Address_DRAM<<endl;
  cout << " Its value in PrefA is  " << DRAM[Address_DRAM]<<endl;
  cout << " Address_SRAM in PrefA is " << Address_SRAM<<endl;*/
  //getchar();

  break;

  case Write_C:

   y_offset = ((BigC_Current-1)==1 || ((BigC_Current-1)==3))? Panel_Size: 0;
   x_offset = ((BigC_Current-1)==2 || ((BigC_Current-1)==3))? (Panel_Size*Panel_Size*2):0;

  if(Issue_Request_DRAM){
    Address_DRAM = (WriteC_Current/Panel_Size)*(Panel_Size+Panel_Size)
    + WriteC_Current%Panel_Size + y_offset + x_offset; //need to add offset here which I believe should be BigC_Current*Panel_Size
      Phys_Addr= Address_DRAM*(BYTE) + temp_phys_addr_C; // means that 1 elements consist of 8 Bytes
  
      Virt_Addr= Address_DRAM*(BYTE) + temp_virt_addr_C; // means that 1 elements consist of 8 Bytes
      
        //masri, Do I need below ?
      //Address_SRAM = WriteC_Current;
  }
  break;

  }


}
#endif

void Accelerator::Address_Gen_DRAM(){

  switch(Pref_DRAM_Current){
  
  case Pref_C : //assuming for square

#ifdef COL_MAJOR
  
  /************* COLUMN MAJOR ********************/

  x_offset = (BigC_Current==2 || BigC_Current==3)? Panel_V: 0;
  y_offset = (BigC_Current==1 || BigC_Current==3)? (Panel_H*LDC):0;
 
  //Panel_V for column major !!
  
  /*
    Address_DRAM =
    (IssueC_Current/Panel_V)*(LDC) + 
    IssueC_Current ;
  */

  if(Issue_Request_DRAM){
      Address_DRAM = (IssueC_Current/Panel_V)*(LDC)
      + IssueC_Current%Panel_V + y_offset + x_offset; //need to add offset here which I believe should be BigC_Current*Panel_Size

      Phys_Addr= Address_DRAM*(BYTE) + temp_phys_addr_C; // means that 1 elements consist of 8 Bytes
  
      Virt_Addr= Address_DRAM*(BYTE) + temp_virt_addr_C; // means that 1 elements consist of 8 Bytes
      Address_SRAM = IssueC_Current;

#if 0
      printf("C Now \n ");
      printf("IssueC_Current is %d\n", IssueC_Current);
      printf("Address_DRAM is %d\n", Address_DRAM);
      Check_Data(Virt_Addr);
      //exit(0);
#endif
    }
    //TODO : Confirm this addressing works

#else 

    /*************** ROW_MAJOR *******************/

  y_offset = (BigC_Current==1 || BigC_Current==3)? Panel_Size: 0;  //(BYTE*8) is size of 1 element
  x_offset = (BigC_Current==2 || BigC_Current==3)? (Panel_Size*Panel_Size*2): 0;

  if(Issue_Request_DRAM){
      Address_DRAM = (IssueC_Current/Panel_Size)*(Panel_Size+Panel_Size)
      + IssueC_Current%Panel_Size + y_offset + x_offset; //need to add offset here which I believe should be BigC_Current*Panel_Size
      /*for(int j=0; j<8; j++)
        addrbuf[addridx+j] = Address_DRAM + j;

      addridx = addridx +8;*/
  
      Phys_Addr= Address_DRAM*(BYTE) + temp_phys_addr_C; // means that 1 elements consist of 8 Bytes
  
      Virt_Addr= Address_DRAM*(BYTE) + temp_virt_addr_C; // means that 1 elements consist of 8 Bytes
      /*cout << " I am at Address_Gen" <<endl;
      cout << " IssueC_Current is " << IssueC_Current<<endl;
      cout << " y_ofsett is " << y_offset<<endl;
      cout << " x_offset is " << x_offset<<endl;
      cout << " Panel_Size is " << Panel_Size<<endl;
      cout << " Address_DRAM is  " << Address_DRAM<<endl;
      cout << " Its value  is  " << DRAM[Address_DRAM]<<endl;*/
      
      //getchar();
      
      Address_SRAM = IssueC_Current;
    }
    //TODO : Confirm this addressing works
#endif

  break;

  case Pref_B_Init:
  

#ifdef COL_MAJOR

  y_offset = ((Chunk_B_Curr%2)==1)? Panel_V*Panel_H*2: 0;
  //x_offset = (BigB_Current*KC)*Panel_V*2;
  x_offset = (BigB_Current*Kernel_H);

  if(Issue_Request_DRAM){
    Address_DRAM = (IssueB_Current/Kernel_H)*(LDC)+ IssueB_Current%Kernel_H + y_offset + x_offset;   
    //Address_SRAM = IssueB_Current + Panel_H*Panel_V +((BigB_Current%2)*(Panel_H*KC));  //Panel_Size*Panel_Size is offset of C 
    Address_SRAM = IssueB_Current;  //Panel_Size*Panel_Size is offset of C 

    Phys_Addr= Address_DRAM*(BYTE) + temp_phys_addr_B; // means that 1 elements consist of 8 Bytes
    
    Virt_Addr= Address_DRAM*(BYTE) + temp_virt_addr_B; // means that 1 elements consist of 8 Bytes
#if 0

    printf("B Now \n ");
    printf("IssueB_Current is %d\n", IssueB_Current);
    printf("Address_DRAM is %d\n", Address_DRAM);
    //Check_Data(Virt_Addr);
#endif  

  }

#else

  y_offset = ((Chunk_B_Curr%2)==1)? Panel_Size: 0;
  x_offset = (BigB_Current*Kernel_Size)*Panel_Size*2;
  
  //offset_C = Panel_Size*Panel_Size*NumofPartition;
  offset_C = 0;

  if(Issue_Request_DRAM){
    Address_DRAM = offset_C + (IssueB_Current/Panel_Size)*(Panel_Size+Panel_Size)+ IssueB_Current%Panel_Size + y_offset + x_offset;
    Address_SRAM = IssueB_Current + Panel_Size*Panel_Size +((BigB_Current%2)*(Panel_Size*Kernel_Size));  //Panel_Size*Panel_Size is offset of C 
    
    Phys_Addr= Address_DRAM*(BYTE) + temp_phys_addr_B; // means that 1 elements consist of 8 Bytes
    
    Virt_Addr= Address_DRAM*(BYTE) + temp_virt_addr_B; // means that 1 elements consist of 8 Bytes
    //Check_Data(Virt_Addr);
  
  }
#endif 



 /* cout << " IssueB_Current is " << IssueB_Current<<endl;
  cout << " y_ofsett is " << y_offset<<endl;
  cout << " x_offset is " << x_offset<<endl;
  cout << " Address_DRAM in PrefB init is  " << Address_DRAM<<endl;
  cout << " Its value in PrefB is init  " << DRAM[Address_DRAM]<<endl;
  cout << " Address_SRAM in PrefB is init  " << Address_SRAM<<endl;
  *///getchar();

  break;

  case Pref_A_Init:
 
#ifdef COL_MAJOR

  x_offset = ((Chunk_A_Curr)>=2)? Panel_H: 0;
  y_offset = BigA_Current*Kernel_H*LDA;
 
  if(Issue_Request_DRAM){
    Address_DRAM = ((IssueA_Current%(Kernel_V*Kernel_H))/Kernel_V)*(LDC) + (IssueA_Current%Kernel_V) + 
                    (IssueA_Current/(Kernel_V*Kernel_H))*Kernel_V + x_offset + y_offset;
    //Address_SRAM = IssueA_Current%(NumofCore*KC*KC) + (Panel_H*Panel_V + 2*KC*Panel_H) ;// Consider offset for SRAM also 
    Address_SRAM = IssueA_Current%(NumofCore*Kernel_V*Kernel_H);// Consider offset for SRAM also  
    Phys_Addr= Address_DRAM*(BYTE) + temp_phys_addr_A; // means that 1 elements consist of 8 Bytes
    
    Virt_Addr= Address_DRAM*(BYTE) + temp_virt_addr_A; // means that 1 elements consist of 8 Bytes
    //printf("A Now \n ");
    //Check_Data(Virt_Addr);
      
  }

#else 
    x_offset = ((Chunk_A_Curr)>=2)? Panel_Size*Panel_Size*2: 0;
  y_offset = BigA_Current*Kernel_Size;
  
  //offset_B = Panel_Size*Panel_Size*NumofPartition*2;
  offset_B = 0;
 
  if(Issue_Request_DRAM){
    Address_DRAM = (IssueA_Current/Kernel_Size)*(Panel_Size+Panel_Size) + IssueA_Current%Kernel_Size + x_offset + y_offset;
    Address_SRAM = IssueA_Current%(NumofCore*Kernel_Size*Kernel_Size) + (Panel_Size*Panel_Size + 2*Kernel_Size*Panel_Size) ;// Consider offset for SRAM also 
    
    Phys_Addr= Address_DRAM*(BYTE) + temp_phys_addr_A; // means that 1 elements consist of 8 Bytes
    
    Virt_Addr= Address_DRAM*(BYTE) + temp_virt_addr_A; // means that 1 elements consist of 8 Bytes
  }

  /*cout << " IssueA_Current in 1st is  " << IssueA_Current<<endl;
  cout << " Address_DRAM in PrefA init 1st is " << Address_DRAM<<endl;
  cout << " Its value in PrefA init is 1st  " << DRAM[Address_DRAM]<<endl;
  cout << " Address_SRAM in PrefA init is 1st " << Address_SRAM<<endl;
  *///getchar();

#endif

  break;
  
  case Pref_A_Init_Second:

#ifdef COL_MAJOR

  x_offset = ((Chunk_A_Curr)>=2)? Panel_H: 0;
  y_offset = BigA_Current*Kernel_H*LDA;
  
  if(Issue_Request_DRAM){
    Address_DRAM = ((IssueA_Current%(Kernel_V*Kernel_H))/Kernel_V)*(LDC) + (IssueA_Current%Kernel_V) + 
                    (IssueA_Current/(Kernel_V*Kernel_H))*Kernel_V + x_offset + y_offset;
  
  //Address_SRAM = IssueA_Current%(NumofCore*KC*KC) + (Panel_H*Panel_V + 2*KC*Panel_H) ;// Consider offset for SRAM also 
    Address_SRAM = IssueA_Current%(NumofCore*Kernel_V*Kernel_H);// Consider offset for SRAM also 
    
    Phys_Addr= Address_DRAM*(BYTE) + temp_phys_addr_A; // means that 1 elements consist of 8 Bytes
    
    Virt_Addr= Address_DRAM*(BYTE) + temp_virt_addr_A; // means that 1 elements consist of 8 Bytes
 
  }

#else 

  x_offset = ((Chunk_A_Curr)>=2)? Panel_Size*Panel_Size*2: 0;
  y_offset = BigA_Current*Kernel_Size;
  
  //offset_B = Panel_Size*Panel_Size*NumofPartition*2;
  offset_B = 0;
 
  if(Issue_Request_DRAM){
    Address_DRAM = offset_B + (IssueA_Current/Kernel_Size)*(Panel_Size+Panel_Size) + IssueA_Current%Kernel_Size + x_offset + y_offset;
    Address_SRAM = IssueA_Current%(NumofCore*Kernel_Size*Kernel_Size) + (Panel_Size*Panel_Size + 2*Kernel_Size*Panel_Size) ;// Consider offset for SRAM also 
    
    Phys_Addr= Address_DRAM*(BYTE) + temp_phys_addr_A; // means that 1 elements consist of 8 Bytes
    
    Virt_Addr= Address_DRAM*(BYTE) + temp_virt_addr_A; // means that 1 elements consist of 8 Bytes
  }

  /*cout << " IssueA_Current is 2nd " << IssueA_Current<<endl;
  cout << " Address_DRAM in PrefA init is 2nd " << Address_DRAM<<endl;
  cout << " Its value in PrefA init is  2nd " << DRAM[Address_DRAM]<<endl;
  cout << " Address_SRAM in PrefA init is 2nd" << Address_SRAM<<endl;
  *///getchar();
#endif


  break;

  case Pref_B:

#ifdef COL_MAJOR

  y_offset = ((Chunk_B_Curr%2)==1)? Panel_V*LDB: 0;
  //x_offset = (BigB_Current*KC)*Panel_V*2;
  x_offset = (BigB_Current*Kernel_H);

  if(Issue_Request_DRAM){
    Address_DRAM = (IssueB_Current/Kernel_H)*(LDC)+ IssueB_Current%Kernel_H + y_offset + x_offset;
    //Address_SRAM = IssueB_Current + Panel_H*Panel_V +((BigB_Current%2)*(Panel_H*KC));  //Panel_Size*Panel_Size is offset of C 
    Address_SRAM = IssueB_Current;  //Panel_Size*Panel_Size is offset of C 
    
    Phys_Addr= Address_DRAM*(BYTE) + temp_phys_addr_B; // means that 1 elements consist of 8 Bytes
  
    Virt_Addr = Address_DRAM*(BYTE) + temp_virt_addr_B; // means that 1 elements consist of 8 Bytes
  
  }

#else 
  y_offset = ((Chunk_B_Curr%2)==1)? Panel_Size: 0;
  x_offset = (BigB_Current*Kernel_Size)*Panel_Size*2;
  
  //offset_C = Panel_Size*Panel_Size*NumofPartition;
  offset_C = 0; //for marss Masri
  
  /*cout << " IssueB_Current is " << IssueB_Current<<endl;
  cout << " y_ofsett is " << y_offset<<endl;
  cout << " x_offset is " << x_offset<<endl;
  cout << " Address_DRAM in PrefB init is  " << Address_DRAM<<endl;
  cout << " Its value in PrefB is init  " << DRAM[Address_DRAM]<<endl;
  cout << " Address_SRAM in PrefB is init  " << Address_SRAM<<endl;*/

  if(Issue_Request_DRAM){
    Address_DRAM = offset_C + (IssueB_Current/Panel_Size)*(Panel_Size+Panel_Size)+ IssueB_Current%Panel_Size + y_offset + x_offset;
  
    Phys_Addr= Address_DRAM*(BYTE) + temp_phys_addr_B; // means that 1 elements consist of 8 Bytes
  
    Virt_Addr = Address_DRAM*(BYTE) + temp_virt_addr_B; // means that 1 elements consist of 8 Bytes
    
    
    Address_SRAM = IssueB_Current + Panel_Size*Panel_Size +((BigB_Current%2)*(Panel_Size*Kernel_Size));  //Panel_Size*Panel_Size is offset of C 
  
  }
#endif

  break;

  case Pref_A:
 
#ifdef COL_MAJOR

  x_offset = ((Chunk_A_Curr)>=2)? Panel_H: 0;
  y_offset = BigA_Current*Kernel_H*LDA;
  
  if(Issue_Request_DRAM){
    
    Address_DRAM = ((IssueA_Current%(Kernel_V*Kernel_H))/Kernel_V)*(LDC) + (IssueA_Current%Kernel_V) + 
                    (IssueA_Current/(Kernel_H*Kernel_V))*Kernel_V + x_offset + y_offset;
    //Address_DRAM = offset_B + (IssueA_Current/KC)*(LDC) + IssueA_Current%KC + x_offset + y_offset;
    //Address_SRAM = IssueA_Current%(NumofCore*KC*KC) + (Panel_H*Panel_V + 2*KC*Panel_H) ;// Consider offset for SRAM also 
    Address_SRAM = IssueA_Current%(NumofCore*Kernel_H*Kernel_V);// Consider offset for SRAM also 
    Phys_Addr= Address_DRAM*(BYTE) + temp_phys_addr_A; // means that 1 elements consist of 8 Bytes
    
    Virt_Addr= Address_DRAM*(BYTE) + temp_virt_addr_A; // means that 1 elements consist of 8 Bytes

  }

#else

  x_offset = ((Chunk_A_Curr)>=2)? Panel_Size*Panel_Size*2: 0;
  y_offset = BigA_Current*Kernel_Size;
  
  //offset_B = Panel_Size*Panel_Size*NumofPartition*2;
  offset_B = 0;
 
  if(Issue_Request_DRAM){
    Address_DRAM = offset_B + (IssueA_Current/Kernel_Size)*(Panel_Size+Panel_Size) + IssueA_Current%Kernel_Size + x_offset + y_offset;
    Address_SRAM = IssueA_Current%(NumofCore*Kernel_Size*Kernel_Size) + (Panel_Size*Panel_Size + 2*Kernel_Size*Panel_Size) ;// Consider offset for SRAM also 
    Phys_Addr= Address_DRAM*(BYTE) + temp_phys_addr_A; // means that 1 elements consist of 8 Bytes
    
    Virt_Addr= Address_DRAM*(BYTE) + temp_virt_addr_A; // means that 1 elements consist of 8 Bytes
  }

  
  /*cout << " y_offset is  is " << y_offset<<endl;
  cout << " Address_DRAM in PrefA is " << Address_DRAM<<endl;
  cout << " Its value in PrefA is  " << DRAM[Address_DRAM]<<endl;
  cout << " Address_SRAM in PrefA is " << Address_SRAM<<endl;*/
  //getchar();
#endif

  break;

  case Write_C:

#ifdef COL_MAJOR

  x_offset = (BigC_Current-1==2 || BigC_Current-1==3)? Panel_V: 0;
  y_offset = (BigC_Current-1==1 || BigC_Current-1==3)? (Panel_H*LDC):0;
 
  if(Issue_Request_DRAM){
      Address_DRAM = (WriteC_Current/Panel_V)*(LDC)
      + WriteC_Current%Panel_V + y_offset + x_offset; //need to add offset here which I believe should be BigC_Current*Panel_Size
#if 0
      if(NON_PART){
        Address_DRAM = (WriteC_Current/Panel_Size)*(LDC)
        + WriteC_Current%Panel_Size + y_offset + x_offset; //need to add offset here which I believe should be BigC_Current*Panel_Size
      }
#endif

      Phys_Addr= Address_DRAM*(BYTE) + temp_phys_addr_C; // means that 1 elements consist of 8 Bytes
  
      Virt_Addr= Address_DRAM*(BYTE) + temp_virt_addr_C; // means that 1 elements consist of 8 Bytes

  }

#else
   y_offset = ((BigC_Current-1)==1 || ((BigC_Current-1)==3))? Panel_Size: 0;
   x_offset = ((BigC_Current-1)==2 || ((BigC_Current-1)==3))? (Panel_Size*Panel_Size*2):0;

  if(Issue_Request_DRAM){
    Address_DRAM = (WriteC_Current/Panel_Size)*(Panel_Size+Panel_Size)
    + WriteC_Current%Panel_Size + y_offset + x_offset; //need to add offset here which I believe should be BigC_Current*Panel_Size
      Phys_Addr= Address_DRAM*(BYTE) + temp_phys_addr_C; // means that 1 elements consist of 8 Bytes
  
      Virt_Addr= Address_DRAM*(BYTE) + temp_virt_addr_C; // means that 1 elements consist of 8 Bytes
      
        //masri, Do I need below ?
      //Address_SRAM = WriteC_Current;
  }
#endif

  break;

  }

}


//Issuer function
void Accelerator::Pref_Issue(){
  
  //need to make sure that when I issue DRAM
  //request_count < MAX_REQUEST_COUNT
  
  W64 cur_phys_addr, cur_virt_addr;
  W64 send_addr;
  W64 base_virt_addr, base_phys_addr;
  
  base_virt_addr = temp_virt_addr;
  base_phys_addr = temp_phys_addr;

  //cur_phys_addr = Address_DRAM*(BYTE) + base_phys_addr; // means that 1 elements consist of 8 Bytes
  //cur_virt_addr = Address_DRAM*(BYTE) + base_virt_addr; // means that 1 elements consist of 8 Bytes
  //cur_phys_addr = Phys_Addr; // means that 1 elements consist of 8 Bytes
  //cur_virt_addr = Virt_Addr; // means that 1 elements consist of 8 Bytes

  //send_addr = tmp_addr & ~(Cache_Line-1);  // Align address to cache line

  if(Issue_Request_DRAM){
  
  cur_phys_addr = Phys_Addr; // means that 1 elements consist of 8 Bytes
  cur_virt_addr = Virt_Addr; // means that 1 elements consist of 8 Bytes
    //I dont think I need requested, as I only 
    //request one for each cache line
    if (request_count < MAX_REQUEST_COUNT) {
     if(Write_DRAM==FALSE){
        Memory::MemoryRequest *request = memoryHierarchy->get_free_request(id);
        assert(request != NULL);

        request->init(id, 0, cur_phys_addr, 0, sim_cycle,
                false, temp_rip ,
                temp_uuid ,
                Memory::MEMORY_OP_READ);
        request->set_coreSignal(&dcache_signal);
        memoryHierarchy->access_cache(request);
        dram_package->WE=FALSE;

     }
     else{
    
    /*ATOM_CORE_MODEL::StoreBufferEntry buf;

    buf.data = data;
    buf.addr = phys_addr;
    buf.virtaddr = virt_addr;
    // (1<<UOP_SIZE) is the number of bytes in the data
#define UOP_SIZE sizeshift
    buf.bytemask = ((1 << (1 << UOP_SIZE))-1);
    buf.size = UOP_SIZE;
    // Be careful not to use buf.op below.
    buf.op = NULL;
    buf.mmio = ctx->is_mmio_addr(virt_addr, true);*/

      Memory::MemoryRequest *request = memoryHierarchy->get_free_request(id);
      //printf("id = %d\n", id);
      assert(request != NULL);

      request->init(id, 0, cur_phys_addr, 0, sim_cycle,
            false, temp_rip /* What should be the RIP here? */,
            temp_uuid /* What should be the UUID here? */,
            Memory::MEMORY_OP_WRITE);
      request->set_coreSignal(&dcache_signal);
      memoryHierarchy->access_cache(request);      
      
      dram_package->WE=TRUE;

      int store_amount = (Partial_Line)? Partial_Frac : Cache_Line/Element_Size;

      for (int i=0; i<store_amount; i++){
        //Buffer->buf.push_back(i);
        dram_package->data.push_back(Buf_from_SRAM->buf.at(i));
        //printf("Store to DRAM %lf\n", Buf_from_SRAM->buf.at(i));
        //dram_package->data.push_back(i);

      //////////////////////////////////////////////
      
        /*std::cout << " buffer are " << Buffer->buf.at(i)<<std::endl;
        std::cout << " data are " << dram_package->data.at(i)<<std::endl;
        std::cout << " dram_package size is " << dram_package->data.size()<<std::endl;*/
      //getchar();
      }

      //We clear something on the output buffer of SRAM
      for (int j=0; j<store_amount; j++)
        Buf_from_SRAM->buf.pop_front();
    }

    //Need to put this request in the "waiting room"
      
      dram_package->Part_Line = Partial_Line;
      dram_package->Part_Frac = Partial_Frac;
     
     for (int i=0; i<Cache_Line/Element_Size; i++){
      dram_package->phys_addresses[i]= cur_phys_addr + i*BYTE;
      dram_package->virt_addresses[i]= cur_virt_addr + i*BYTE;

      //printf("Virt address to DRAM is %x \n", cur_phys_addr);
 
      dram_package->sram_addr[i]= Address_SRAM + i;
      
#ifdef COL_MAJOR
      int offset;
      if ((Pref_DRAM_Current==Pref_B_Init)||(Pref_DRAM_Current==Pref_B)){
        if(HowManyPanel==1)
          offset = Panel_H*Panel_V + SRAM_OFFSET*Panel_H + ((Chunk_B_Curr%2)*(Panel_H*Kernel_H));
        else 
          offset = Panel_H*Panel_V + SRAM_OFFSET*Panel_H + ((BigB_Current%2)*(Panel_H*Kernel_H));
      }

      else if ((Pref_DRAM_Current==Pref_A_Init)||(Pref_DRAM_Current==Pref_A) 
                || Pref_DRAM_Current==Pref_A_Init_Second){
        offset = (Panel_H*Panel_V + SRAM_OFFSET*Panel_H +  2*Kernel_H*Panel_H) ;// Consider offset for SRAM also 
      }
    
      else 
        offset = 0;

      //For column major
      dram_package->sram_addr[i]= ((Address_SRAM+i)/Panel_V + ((Address_SRAM+i)%Panel_V)*Panel_H) + offset;

      if ((Pref_DRAM_Current==Pref_B_Init)||(Pref_DRAM_Current==Pref_B))
        dram_package->sram_addr[i]= ((Address_SRAM+i)/Kernel_H + ((Address_SRAM+i)%Kernel_H)*Panel_V) + offset;

      else if ((Pref_DRAM_Current==Pref_A_Init)||(Pref_DRAM_Current==Pref_A) 
                || Pref_DRAM_Current==Pref_A_Init_Second){

        dram_package->sram_addr[i]= ((Address_SRAM+i)/Kernel_V + ((Address_SRAM+i)%Kernel_V)*Kernel_H) + offset;
      

      //debug 
      //v_addrbuf[v_addridx + i] = cur_virt_addr + i*BYTE;
      //addrbuf[addridx + i] = Address_SRAM + i;
      //cout << " addresses are " << dram_package->phys_addresses[i] <<endl;;
    }
 
#endif   
    }
    
    /*if (Write_DRAM){

    }*/

    if (last_pref){
      dram_package->last=TRUE;
      dram_package->last_count = last_req_count;
      dram_package->last_addr = Address_SRAM + Cache_Line/Element_Size;
      printf("last_pref is true \n");
      //exit(0);
    }
    else{ 
      dram_package->last=FALSE;
      dram_package->last_count = 0; 
      dram_package->last_addr = 0; 
    }


    dram_package->Serviced=false;
    v_addridx = v_addridx + 8;
   // addridx = addridx + 8;
  
    wait_buffer.push_back(*dram_package);
    ++request_count;
    //cout << "put request"<<endl;
    
    int store_amount = (Partial_Line)? Partial_Frac : Cache_Line/Element_Size;
    
    if(dram_package->data.size()>0)
      for (int i=0; i<store_amount; i++) 
        dram_package->data.pop_front();

  }
  
  //this means, we reached maximum request, can not
  //issue request to memory system
    else {
    
    }
  }
}

void Accelerator::Pref_Check_Ready(){

  bool load_now =false;
  W64 load_address;
  int index;
  W64 cur_virt_addr, cur_phys_addr;
  bool is_write=false;

  for(int i=0; i<wait_buffer.size(); i++){
    
    if (wait_buffer.at(i).ready==true){
      load_now = true;
      load_address=wait_buffer.at(i).virt_addresses[0];
      //for(int j=0; j<8; j++)
        //addrbuf[addridx+j] = load_address + j*8;
      is_write = wait_buffer.at(i).WE;
      index = i ;
      //addridx = addridx +8;
      //wait_buffer.erase(wait_buffer.begin() + i);
      break;
    }
  }

  if(load_now){
     int rc;
     cur_virt_addr = load_address;//virt is not always the same with phys
     cur_phys_addr = wait_buffer.at(index).phys_addresses[0];
     
     //bool is_partial_line = wait_buffer.at(index).Part_Line;
     //int howmanyelm = (is_partial_line)? wait_buffer.at(index).Part_Frac : Cache_Line/Element_Size;
     //Moch : need revisiting if the above is true 
     
     int howmanyelm = Cache_Line/Element_Size;
      
     if(!is_write){
        
#if 0
        printf("I am loading addr %d \n", wait_buffer.at(index).sram_addr[0]);
        printf("is_partial_line %d \n", is_partial_line);
        printf("howmanyelm %d \n", howmanyelm);
#endif

        this->Pref_Load(cur_virt_addr, cur_phys_addr, matrix_data_buf, temp_rip,
              temp_uuid, true, howmanyelm, index);
     }
    
     else{
        for(int i=0; i<howmanyelm; i++){
            store_data[i]=wait_buffer.at(index).data[i];
            //printf("From Wait Buffer %lf\n", store_data[i]);
        }
        data_pointer = store_data;
        this->Pref_Store(cur_virt_addr, cur_phys_addr, data_pointer, temp_rip,
              temp_uuid, true, howmanyelm, index);
        ++str_counter;
     }


     --request_count;    
      wait_buffer.erase(wait_buffer.begin() + index);
      //wait_buffer.pop_front();
  }

}

void Accelerator::Load(){
     
  W64 cur_phys_addr, cur_virt_addr;
  W64 send_addr;
  W64 base_virt_addr, base_phys_addr;
  
  base_virt_addr = temp_virt_addr;
  base_phys_addr = temp_phys_addr;

  /*cur_phys_addr = Address_DRAM*(BYTE) + base_phys_addr; // means that 1 elements consist of 8 Bytes
  cur_virt_addr = Address_DRAM*(BYTE) + base_virt_addr; // means that 1 elements consist of 8 Bytes*/
  cur_phys_addr = Phys_Addr; // means that 1 elements consist of 8 Bytes
  cur_virt_addr = Virt_Addr; // means that 1 elements consist of 8 Bytes
 
  this->Pref_Load(cur_virt_addr, cur_phys_addr, matrix_data_buf, temp_rip,
              temp_uuid, true, 6, 0);

}

void Accelerator::Pref_Load(W64 virt_addr, W64 phys_addr, void* data, W64 rip, W64 uuid, bool is_requested, int sizeshift, int index){

    W64 count = sizeshift;

    

    /*for(int i=0; i<Buf_DRAM.size(); i++){
        if (to_DRAM[i].dram_address==phys_addr)
          index=i;
        break;
    }*/

    for (int i = 0; i < count; ++i) {
        // Load 8 byte at a time


        //addrtrace.open(ADDR_TRACE, ofstream::app);
        
        PageFaultErrorCode pfec;
        int exception = 0;
        int mmio    = 0;

        //Waddr physaddr = ctx->check_and_translate(virt_addr, 3, false, false, exception, mmio, pfec);
        //addrtrace << "Addr :" <<(virt_addr + i*8) << " ; " << physaddr << " check_and_translate : " <<exception << endl;
        //addrtrace << "ctx->kernel_mode in accelerator : " << ctx->kernel_mode << endl;
        //addrtrace.close();

        bool old_kernel_mode = ctx->kernel_mode;
        ctx->kernel_mode = true;

        *(((W64*)data) + i) = ctx->loadvirt(virt_addr + i * 8, 3);
        
        //addrtrace.open(ADDR_TRACE, ofstream::app);
        //addrtrace << "After loadvirt" << endl;
        
        //if(i==count-1) addrtrace <<endl;
        //addrtrace.close();
        //testing
        //*(((W64*)data) + i) = 1;
        
        //testing in case of segementation fault
        //*(((W64*)data) + i) = i; 

        double test2 = W64toDouble (*(((W64*)data) + i));
        Buf_DRAM->buf.push_back(test2);
        Buf_DRAM->addr.push_back(wait_buffer.at(index).sram_addr[i]);
        int addr = wait_buffer.at(index).sram_addr[i];

        ctx->kernel_mode = old_kernel_mode;
        //testbuf[idxbuf+i]=test2;
        //addrbuf[idxbuf+i] = virt_addr + i*8;
#if 1
        //printf("temp_virt_addr is  %llx \n",temp_virt_addr );
        //printf("temp_curr_addr is  %llx \n",virt_addr + i*8 );
        //printf("Address is is %d \n", virt_addr-temp_virt_addr_C );
        //printf("in double test2 is %lf \n", test2);
        //printf("address SRAM is %d\n", addr);
        //getchar();
#endif
    }

#if 0
    if(wait_buffer.at(index).last){
        Buf_DRAM->last.push_back(wait_buffer.at(index).last);
        Buf_DRAM->last_issue_count.push_back(wait_buffer.at(index).last_count);
        Buf_DRAM->last_issue_addr.push_back(wait_buffer.at(index).last_addr);
      }
#endif

    idxbuf= idxbuf+8;
}

void Accelerator::Pref_Store(W64 virt_addr, W64 phys_addr, double * store_data, W64 rip, W64 uuid, bool is_requested, int sizeshift, int index){
    
  ATOM_CORE_MODEL::StoreBufferEntry buf[Cache_Line/Element_Size];

    int store_amnt = sizeshift;

#if 1
    for (int i =0; i<store_amnt; i++){

      buf[i].data = DoubleToW64(store_data[i]);
      buf[i].addr = phys_addr + i*8;
      buf[i].virtaddr = virt_addr + i*8;
      // (1<<UOP_SIZE) is the number of bytes in the data
#define UOP_SIZE sizeshift
      buf[i].bytemask = ((1 << (1 << UOP_SIZE))-1);
      buf[i].size = UOP_SIZE;
      // Be careful not to use buf.op below.
      buf[i].op = NULL;
      buf[i].mmio = ctx->is_mmio_addr(virt_addr + i*8, true);
    }

    bool old_kernel_mode = ctx->kernel_mode;
    ctx->kernel_mode = true;
    //bug
    //should bring for loop here

#if 1    
    for (int i =0; i<store_amnt; i++){ 
      buf[i].write_to_ram(*ctx);
      //double test2 = W64toDouble(ctx->loadvirt(virt_addr + i * 8, 3));
      //printf("in store\n");
      //printf("in double test2 is %lf \n", test2);
    }
#endif

    ctx->kernel_mode = old_kernel_mode;
#endif

}

#if 0

void Accelerator::PF_SM_SRAM(){

  Issue_Request_SRAM = FALSE;


  //need address_mapping DRAM_to_SRAM
  int tmp_addr = 0;

  int offset_C = Panel_Size*Panel_Size;
  int offset_B = offset_C + (2*Panel_Size*Kernel_Size);
  int residue_amount = 0;
  is_residue = FALSE;
  int amount = (A_SRAM_Init)? A_SRAM_Unit: NumofCore;
 
  //set the initial bandwidth amount
  send_amount = 0;
  if(Buf_DRAM->buf.size()>0){
    Buf_DRAM->buf_ready=TRUE;
    if (Buf_DRAM->buf.size()>= (Port_Bandwidth/Element_Size))
        send_amount=Port_Bandwidth/Element_Size;
    else 
        send_amount=Buf_DRAM->buf.size();
  }
  else
    Buf_DRAM->buf_ready=FALSE;


  if(wait_next_C && Buf_DRAM->buf_ready==FALSE)
      SRAM_Current=Idle; 

#if 0

  if(Buf_DRAM->buf.size()>=(Port_Bandwidth/Element_Size) ) //fix this !
    Buf_DRAM->buf_ready=TRUE;
  else{
    int bufready = (Sent_C_Current>=(Panel_Size*Panel_Size)-(Port_Bandwidth/Element_Size))||
                   (Sent_B_Current>=(Kernel_Size*Panel_Size)-(Port_Bandwidth/Element_Size))||
                   (Sent_A_Current>=(amount*Kernel_Size*Kernel_Size)-(Port_Bandwidth/Element_Size));
    
    //std::cout << "bufready is " << bufready<<std::endl;

    if(bufready && (Buf_DRAM->buf.size()!=0)){
      Buf_DRAM->buf_ready = TRUE;
      //std::cout << "bufready is " << bufready<<std::endl;
    }
    else 
      Buf_DRAM->buf_ready = FALSE;
  }
#endif


#if 0  
  if(Buf_DRAM->buf_ready){
    SRAM_Current = (tmp_addr< offset_C)? Send_C_SRAM: (tmp_addr<offset_B)? Send_B_SRAM : Send_A_SRAM;
  
    if(Buf_DRAM->last_issue_addr.size()!=0){
    
      bool last_one = ( (Sent_C_Current>=(Panel_Size*Panel_Size)-(Port_Bandwidth/Element_Size))  && SRAM_Current==Send_C_SRAM)||
                     ((Sent_B_Current>=(Kernel_Size*Panel_Size)-(Port_Bandwidth/Element_Size)) && SRAM_Current==Send_B_SRAM)||
                     ((Sent_A_Current>=(amount*Kernel_Size*Kernel_Size)-(Port_Bandwidth/Element_Size)) && SRAM_Current==Send_A_SRAM);

      residue_amount = (((Buf_DRAM->last_issue_count.at(0))%(Port_Bandwidth/Element_Size))==0)?
                    Port_Bandwidth/Element_Size: Buf_DRAM->last_issue_count.at(0)%(Port_Bandwidth/Element_Size);
      
      //std::cout << "residue amount is " << residue_amount<<std::endl;
      /*std::cout << "last issue addreess is " << Buf_DRAM->last_issue_addr.at(0)<<std::endl;*/
      
      if(last_one && residue_amount!=Port_Bandwidth/Element_Size){
    
      is_residue = TRUE;
      residue_amount = (((Buf_DRAM->last_issue_count.at(0))%(Port_Bandwidth/Element_Size))==0)?
                    Port_Bandwidth/Element_Size: Buf_DRAM->last_issue_count.at(0)%(Port_Bandwidth/Element_Size);
#if 0 

      std::cout << "last_issue count is" << Buf_DRAM->last_issue_count.at(0)<<std::endl;
      std::cout << "Residue amount is " << Port_Bandwidth/Element_Size<<std::endl;
      std::cout << "Sent_B_Current is " << Sent_B_Current<<std::endl;
      exit(0);
#endif
      
      }
    /*else
      residue_amount = 0;*/

    }
  
  }
  else {
    if(wait_next_C)
      SRAM_Current=Idle; 
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
    
    if(NON_PART){  
      write_ready = done && (fetched_Cin_total>=1) &&(fetched_B==1) && (fetched_A_total==howmanyA) && (Pref_DRAM_Current==Write_C);
    }
    else{  
    write_ready = done && (fetched_Cin_total>=1) &&(fetched_B==HowManyPanel) && (fetched_A_total==howmanyA*HowManyPanel) && (Pref_DRAM_Current==Write_C);
    }
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

  //Case for reading SRAM
  if(write_ready){
      
   switch(SRAM_Current){
    case Read_C_SRAM:

        static int resamount = (Panel_Size*Panel_Size)%(Port_Bandwidth/Element_Size);
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
#if 0
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
#endif

        if(Read_C_Current== (Panel_Size*Panel_Size)- (Port_Bandwidth/Element_Size) ||
            (Read_C_Current == (Panel_Size*Panel_Size)-resamount)){
        
          write_ready=FALSE;
          fetched_Cin_total=0;
          fetched_A_total=2*NumofCore;
          fetched_B=1;
          wait_next_C=TRUE;
          done =FALSE;
          std::cout << " READ_C is "<< Read_C_Current <<std::endl;
          std::cout << " READ SRAM LAST" <<std::endl;
          std::cout << " fetched_B " << fetched_B<<std::endl;
          std::cout << " fetched_A_total " << fetched_A_total<<std::endl;

          for(i=0; i<NumofCore; i++)
            to_LS->Cout[i]=FALSE;
          
          if ((Read_C_Current== (Panel_Size*Panel_Size) - resamount) && resamount!=0)
            is_residue = TRUE;

        }

        if (is_residue)
          Read_C_Current = 0;
        else 
          Read_C_Current = (Read_C_Current + (Port_Bandwidth/Element_Size)) %(Panel_Size*Panel_Size);


        break;
    }  
  }


  else if (Buf_DRAM->buf_ready){

  //std::cout << "SRAM_Current is " << SRAM_Current << std::endl;
  
  for(int i=0; i<send_amount; i++){

    tmp_addr=Buf_DRAM->addr[i];
    SRAM_Current = (tmp_addr< offset_C)? Send_C_SRAM: (tmp_addr<offset_B)? Send_B_SRAM : Send_A_SRAM;

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

#if 0       
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
#endif

       /*
       if((Sent_C_Current == (Panel_Size*Panel_Size-(Port_Bandwidth/Element_Size))) || 
           Sent_C_Current == (Panel_Size*Panel_Size)-(residue_amount) )
       */

       if(Sent_C_Current == (Panel_Size*Panel_Size)-1)
      {
        to_LS->C_All_Ready=TRUE;
        for(int i=0; i<NumofCore; i++){
          to_LS->Cin[i] = TRUE;
        }
        fetched_Cin_total++;
        //exit(0);

      }


      Sent_C_Current = (Sent_C_Current + 1) %(Panel_Size*Panel_Size);


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
      
#if 0    
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
#endif

      if(Sent_B_Current== (Kernel_Size*Panel_Size) -1) 
          {
        for(int i =0; i<NumofCore; i++)
          to_LS->B[i]=TRUE;  
        to_LS->currentB = (to_LS->currentB +1)%NumofMemB;
        fetched_B++;
#if 0
        static int county=0;
        county++;
        std::cout << " is residue " << is_residue<<std::endl;
        std::cout << "residue is" << residue_amount<<std::endl;
        std::cout << "Sent_B Current is" << Sent_B_Current<<std::endl;
        std::cout << "Buf DRAM is" << Buf_DRAM->addr[0]<<std::endl;
        std::cout << "Buf DRAM size is" << Buf_DRAM->addr.size()<<std::endl;
        if(county==2) exit(0);
#endif
        //exit(0);
        /*std::cout << " fetched_B is  "<<fetched_B << std::endl;
        std::cout << " in send_B_SRAM " << std::endl;
        std::cout << "to_LS->B is " << to_LS->B << std::endl;*/
        //exit(0);
        last_A_Init=TRUE;
        B_SRAM_Done=TRUE;

      }
          
      Sent_B_Current = (Sent_B_Current + 1) %(Kernel_Size*Panel_Size);


      break;

      case Send_A_SRAM:

        Issue_Request_SRAM = TRUE;
        Write_SRAM = TRUE;
        
        Track_SRAM(tmp_addr);
         for (int i=0; i<NumofCore; i++){
            if (fetched_A[i]==Kernel_Size*Kernel_Size){
              to_LS->A[i]=TRUE;
              //std::cout << "Sent_A_Current " << Sent_A_Current << std::endl;
              //getchar();
              fetched_A[i]=0;
              fetched_A_total++;
              //std::cout << "fetched_A_total " << fetched_A_total << std::endl;
            }
         }
      
        //std::cout << " amount is  "<< amount << std::endl;
      //  getchar(); 

#if 0
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
#endif

         if((Sent_A_Current == (amount*Kernel_Size*Kernel_Size) -1 )){
            if(A_SRAM_Init) A_SRAM_Init=FALSE;
            A_SRAM_Done=TRUE; 
            /*static int a ;
            a++;

            std::cout << "I am here for " << a <<"times" <<std::endl;
            to_LS->A[0]=TRUE;
            to_LS->A[1]=TRUE;*/
#if 0          
            static int county=0;
            county++;
            {
            std::cout << " is residue " << is_residue<<std::endl;
            std::cout << "residue is" << residue_amount<<std::endl;
            std::cout << "Sent_A Current is" << Sent_A_Current<<std::endl;
            std::cout << "Buf DRAM is" << Buf_DRAM->addr[0]<<std::endl;
            std::cout << "Buf DRAM size is" << Buf_DRAM->addr.size()<<std::endl;
            //if (county==3)exit(0);
            }
#endif
         }
        
         Sent_A_Current = (Sent_A_Current + 1) %(amount*Kernel_Size*Kernel_Size);

        break;

        case Idle:

        break;

      }
    }
  }
}
#endif


void Accelerator::PF_SM_SRAM(){

  Issue_Request_SRAM = FALSE;


  //need address_mapping DRAM_to_SRAM
  int tmp_addr = 0;

  int offset_C = Panel_V*Panel_H + SRAM_OFFSET*Panel_H;
  int offset_B = offset_C + (2*Panel_H*Kernel_H);
  int residue_amount = 0;
  is_residue = FALSE;
  int amount = (A_SRAM_Init)? A_SRAM_Unit: NumofCore;
 
  //set the initial bandwidth amount
  send_amount = 0;
  if(Buf_DRAM->buf.size()>0){
    Buf_DRAM->buf_ready=TRUE;
    if (Buf_DRAM->buf.size()>= (Port_Bandwidth/Element_Size))
        send_amount=Port_Bandwidth/Element_Size;
    else 
        send_amount=Buf_DRAM->buf.size();
  }
  else
    Buf_DRAM->buf_ready=FALSE;


  if(wait_next_C && Buf_DRAM->buf_ready==FALSE)
      SRAM_Current=Idle; 

#if 0

  if(Buf_DRAM->buf.size()>=(Port_Bandwidth/Element_Size) ) //fix this !
    Buf_DRAM->buf_ready=TRUE;
  else{
    int bufready = (Sent_C_Current>=(Panel_Size*Panel_Size)-(Port_Bandwidth/Element_Size))||
                   (Sent_B_Current>=(Kernel_Size*Panel_Size)-(Port_Bandwidth/Element_Size))||
                   (Sent_A_Current>=(amount*Kernel_Size*Kernel_Size)-(Port_Bandwidth/Element_Size));
    
    //std::cout << "bufready is " << bufready<<std::endl;

    if(bufready && (Buf_DRAM->buf.size()!=0)){
      Buf_DRAM->buf_ready = TRUE;
      //std::cout << "bufready is " << bufready<<std::endl;
    }
    else 
      Buf_DRAM->buf_ready = FALSE;
  }
#endif


#if 0  
  if(Buf_DRAM->buf_ready){
    SRAM_Current = (tmp_addr< offset_C)? Send_C_SRAM: (tmp_addr<offset_B)? Send_B_SRAM : Send_A_SRAM;
  
    if(Buf_DRAM->last_issue_addr.size()!=0){
    
      bool last_one = ( (Sent_C_Current>=(Panel_Size*Panel_Size)-(Port_Bandwidth/Element_Size))  && SRAM_Current==Send_C_SRAM)||
                     ((Sent_B_Current>=(Kernel_Size*Panel_Size)-(Port_Bandwidth/Element_Size)) && SRAM_Current==Send_B_SRAM)||
                     ((Sent_A_Current>=(amount*Kernel_Size*Kernel_Size)-(Port_Bandwidth/Element_Size)) && SRAM_Current==Send_A_SRAM);

      residue_amount = (((Buf_DRAM->last_issue_count.at(0))%(Port_Bandwidth/Element_Size))==0)?
                    Port_Bandwidth/Element_Size: Buf_DRAM->last_issue_count.at(0)%(Port_Bandwidth/Element_Size);
      
      //std::cout << "residue amount is " << residue_amount<<std::endl;
      /*std::cout << "last issue addreess is " << Buf_DRAM->last_issue_addr.at(0)<<std::endl;*/
      
      if(last_one && residue_amount!=Port_Bandwidth/Element_Size){
    
      is_residue = TRUE;
      residue_amount = (((Buf_DRAM->last_issue_count.at(0))%(Port_Bandwidth/Element_Size))==0)?
                    Port_Bandwidth/Element_Size: Buf_DRAM->last_issue_count.at(0)%(Port_Bandwidth/Element_Size);
#if 0 

      std::cout << "last_issue count is" << Buf_DRAM->last_issue_count.at(0)<<std::endl;
      std::cout << "Residue amount is " << Port_Bandwidth/Element_Size<<std::endl;
      std::cout << "Sent_B_Current is " << Sent_B_Current<<std::endl;
      exit(0);
#endif
      
      }
    /*else
      residue_amount = 0;*/

    }
  
  }
  else {
    if(wait_next_C)
      SRAM_Current=Idle; 
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

  //Case for reading SRAM
  if(write_ready){
      
   switch(SRAM_Current){
    case Read_C_SRAM:

        static int resamount = (Panel_V*Panel_H)%(Port_Bandwidth/Element_Size);
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
#if 0
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
#endif

        if(Read_C_Current== (Panel_V*Panel_H)- (Port_Bandwidth/Element_Size) ||
            (Read_C_Current == (Panel_V*Panel_H)-resamount)){
        
          write_ready=FALSE;
          fetched_Cin_total=0;
          fetched_A_total=2*NumofCore;
          fetched_B=1;
          wait_next_C=TRUE;
          done =FALSE;
          std::cout << " READ_C is "<< Read_C_Current <<std::endl;
          std::cout << " READ SRAM LAST" <<std::endl;
          std::cout << " fetched_B " << fetched_B<<std::endl;
          std::cout << " fetched_A_total " << fetched_A_total<<std::endl;

          for(i=0; i<NumofCore; i++)
            to_LS->Cout[i]=FALSE;
          
          if ((Read_C_Current== (Panel_H*Panel_V) - resamount) && resamount!=0)
            is_residue = TRUE;

        }

        if (is_residue)
          Read_C_Current = 0;
        else 
          Read_C_Current = (Read_C_Current + (Port_Bandwidth/Element_Size)) %(Panel_V*Panel_H);


        break;
    }  
  }


  else if (Buf_DRAM->buf_ready){

  //std::cout << "SRAM_Current is " << SRAM_Current << std::endl;
  
  for(int i=0; i<send_amount; i++){

    tmp_addr=Buf_DRAM->addr[i];
    SRAM_Current = (tmp_addr< offset_C)? Send_C_SRAM: (tmp_addr<offset_B)? Send_B_SRAM : Send_A_SRAM;

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

#if 0       
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
#endif

       /*
       if((Sent_C_Current == (Panel_Size*Panel_Size-(Port_Bandwidth/Element_Size))) || 
           Sent_C_Current == (Panel_Size*Panel_Size)-(residue_amount) )
       */

       if(Sent_C_Current == (Panel_H*Panel_V)-1)
      {
        to_LS->C_All_Ready=TRUE;
        for(int i=0; i<NumofCore; i++){
          to_LS->Cin[i] = TRUE;
        }
        fetched_Cin_total++;
        //exit(0);

      }


      Sent_C_Current = (Sent_C_Current + 1) %(Panel_H*Panel_V);


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
      
#if 0    
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
#endif

      if(Sent_B_Current== (Kernel_H*Panel_H) -1) 
          {
        for(int i =0; i<NumofCore; i++)
          to_LS->B[i]=TRUE;  
        to_LS->currentB = (to_LS->currentB +1)%NumofMemB;
        fetched_B++;
#if 0
        static int county=0;
        county++;
        std::cout << " is residue " << is_residue<<std::endl;
        std::cout << "residue is" << residue_amount<<std::endl;
        std::cout << "Sent_B Current is" << Sent_B_Current<<std::endl;
        std::cout << "Buf DRAM is" << Buf_DRAM->addr[0]<<std::endl;
        std::cout << "Buf DRAM size is" << Buf_DRAM->addr.size()<<std::endl;
        if(county==2) exit(0);
#endif
        //exit(0);
        /*std::cout << " fetched_B is  "<<fetched_B << std::endl;
        std::cout << " in send_B_SRAM " << std::endl;
        std::cout << "to_LS->B is " << to_LS->B << std::endl;*/
        //exit(0);
        last_A_Init=TRUE;
        B_SRAM_Done=TRUE;

      }
          
      Sent_B_Current = (Sent_B_Current + 1) %(Kernel_H*Panel_H);


      break;

      case Send_A_SRAM:

        Issue_Request_SRAM = TRUE;
        Write_SRAM = TRUE;
        
        Track_SRAM(tmp_addr);
         for (int i=0; i<NumofCore; i++){
            if (fetched_A[i]==Kernel_V*Kernel_H){
              to_LS->A[i]=TRUE;
              //std::cout << "Sent_A_Current " << Sent_A_Current << std::endl;
              //getchar();
              fetched_A[i]=0;
              fetched_A_total++;
              //std::cout << "fetched_A_total " << fetched_A_total << std::endl;
            }
         }
      
        //std::cout << " amount is  "<< amount << std::endl;
      //  getchar(); 

#if 0
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
#endif

         if((Sent_A_Current == (amount*Kernel_H*Kernel_V) -1 )){
            if(A_SRAM_Init) A_SRAM_Init=FALSE;
            A_SRAM_Done=TRUE; 
            /*static int a ;
            a++;

            std::cout << "I am here for " << a <<"times" <<std::endl;
            to_LS->A[0]=TRUE;
            to_LS->A[1]=TRUE;*/
#if 0          
            static int county=0;
            county++;
            {
            std::cout << " is residue " << is_residue<<std::endl;
            std::cout << "residue is" << residue_amount<<std::endl;
            std::cout << "Sent_A Current is" << Sent_A_Current<<std::endl;
            std::cout << "Buf DRAM is" << Buf_DRAM->addr[0]<<std::endl;
            std::cout << "Buf DRAM size is" << Buf_DRAM->addr.size()<<std::endl;
            //if (county==3)exit(0);
            }
#endif
         }
        
         Sent_A_Current = (Sent_A_Current + 1) %(amount*Kernel_H*Kernel_V);

        break;

        case Idle:

        break;

      }
    }
  }
}

#if 0

void Accelerator::Track_SRAM(int address){

  int offset_C = Panel_Size*Panel_Size;
  int offset_B = offset_C + (2*Panel_Size*Kernel_Size);
    
  which_matrix = (address< offset_C)? CIN: (address<offset_B)? B : A;

  switch(which_matrix){
  
    case CIN:

      for(int i=0; i<NumofCore; i++){
        if(address < (i+1)*Kernel_Size*Panel_Size){
         fetched_Cin[i] = fetched_Cin[i] + 1;
         break;
        }
      }

    break;
    
    case A:

      for(int i=0; i<NumofCore; i++){
        if(address < offset_B +  ((i+1)%(NumofCore+1))*Kernel_Size*Kernel_Size){
          //fetched_A[i] = fetched_A[i] + Port_Bandwidth/Element_Size; 
          fetched_A[i] = fetched_A[i] + 1; 
          
          /*int res = fetched_A[i];
          if(fetched_A[i]>Kernel_Size*Kernel_Size){
            fetched_A[i]=Kernel_Size*Kernel_Size;
            if(i!=(NumofCore-1))
              fetched_A[i+1]= fetched_A[i+1] + res-(Kernel_Size*Kernel_Size);
          
          }*/
          
          break;
        }
      }
    break; 
  }

}
#endif 

void Accelerator::Track_SRAM(int address){

  int offset_C = Panel_H*Panel_V+SRAM_OFFSET*Panel_H;
  int offset_B = offset_C + (2*Panel_H*Kernel_H);
    
  which_matrix = (address< offset_C)? CIN: (address<offset_B)? B : A;

  switch(which_matrix){
  
    case CIN:

      for(int i=0; i<NumofCore; i++){
        if(address < (i+1)*Kernel_H*Panel_H){
         fetched_Cin[i] = fetched_Cin[i] + 1;
         break;
        }
      }

    break;
    
    case A:

      for(int i=0; i<NumofCore; i++){
        if(address < offset_B +  ((i+1)%(NumofCore+1))*Kernel_H*Kernel_V){
          //fetched_A[i] = fetched_A[i] + Port_Bandwidth/Element_Size; 
          fetched_A[i] = fetched_A[i] + 1; 
          
          /*int res = fetched_A[i];
          if(fetched_A[i]>Kernel_Size*Kernel_Size){
            fetched_A[i]=Kernel_Size*Kernel_Size;
            if(i!=(NumofCore-1))
              fetched_A[i+1]= fetched_A[i+1] + res-(Kernel_Size*Kernel_Size);
          
          }*/
          
          break;
        }
      }
    break; 
  }

}

void Accelerator::Req_to_SRAM(){ // to write or to read

    sram_package->req=FALSE;
    sram_package->res_amnt=FALSE;
    sram_package->res=FALSE;
  if(Issue_Request_SRAM){
   
    sram_package->req=TRUE;
    int req_amount = 0;
    
      //Address_SRAM = Address_SRAM + Port_Bandwidth/Element_Size;
      
    sram_package->Serviced= FALSE;

#if 0
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

#endif

    if (Write_SRAM){
      
      sram_package->WE=TRUE;
      req_amount = send_amount;
      sram_package->res_amnt=req_amount;

      for (int i=0; i<req_amount; i++){
        sram_package->addresses[i]= Buf_DRAM->addr[i]; 
        sram_package->data[i]= Buf_DRAM->buf[i];     
        //printf("address to SRAM is %d\n", sram_package->addresses[i]);
        //printf("data to SRAM is %d\n", sram_package->data[i]);

      }
      //if Buf_DRAM->buf_ready should be called in SM_SRAM

      //Here is independent from req_amount
      for (int i=0; i<req_amount; i++){
        Buf_DRAM->buf.pop_front();
        Buf_DRAM->addr.pop_front();
      }

#if 0      
      if(is_residue){
        Buf_DRAM->last.pop_front();
        Buf_DRAM->last_issue_count.pop_front();
        Buf_DRAM->last_issue_addr.pop_front();
        sram_package->res=TRUE;
        sram_package->res_amnt=req_amount;
        static int count=0;
      }
#endif
      
      if (Buf_DRAM->buf.size()==0){
        Buf_DRAM->buf_ready=FALSE;
        /*std::cout << "Buf false at SRAM " << Buf_DRAM->buf_ready<<std::endl;
        //getchar();*/
      }
    }

    else {

      sram_package->WE=FALSE;
      sram_package->res_amnt= Port_Bandwidth/Element_Size;
      
      if(is_residue){
        static int writeamount = (Panel_Size*Panel_Size)%(Port_Bandwidth/Element_Size);
        sram_package->res=TRUE;
        sram_package->res_amnt=writeamount;
        //exit(0);
      }
      
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

void Accelerator::Update_SM(){

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

// Load the content of matrix based on row-major order
bool Accelerator::do_load_content_row_major(void *nothing)
{
    int rc;
    //bool all_ready = false;

    // The address of current cache line.
    W64 cur_virt_addr, cur_phys_addr;

    // The address of the first cache line (start of matrix)
    W64 base_virt_addr, base_phys_addr;

    // The difference between cur_addr and base_addr
    W64 offset;

    // The data
    W64 data[MEM_REQ_COUNT];

    //base_virt_addr = temp_virt_addr + 2 * sizeof(int);
    //base_phys_addr = temp_phys_addr + 2 * sizeof(int);
    base_virt_addr = temp_virt_addr;
    base_phys_addr = temp_phys_addr;

    // Row-major seek
    // j is the number of cache line within a row
    for (int j = 0; j < matrix_header.n / MEM_REQ_COUNT; ++j) {
        // Calculate the offset based on row and column count
        offset = (row_count*matrix_header.n + j * MEM_REQ_COUNT) * sizeof(int);
        cur_virt_addr = base_virt_addr + offset;
        cur_phys_addr = base_phys_addr + offset;

        // If the memory address has not been requested
        if unlikely (!requested[j] && request_count < MAX_REQUEST_COUNT) {
            // Magic number 6 indicates 2^6=64 bytes
            // Send out request
            rc = this->load(cur_virt_addr, cur_phys_addr, matrix_data_buf+offset, temp_rip,
                    temp_uuid, false, 6);
            printf("FIRST ATTEMP to load %p, result = %d, simcycle = %ld\n", cur_phys_addr, rc, sim_cycle);
            requested[j] = true;

            // If cache hit (impossible for no cache case), directly store it
            if (rc == ACCESS_OK) {
                ++wait_count;
            }

            // Set cache ready to false to wait for the result
            cache_ready_map[cur_phys_addr] = false;
            ++request_count;
        }
        // If the cache line is ready from the memory system
        else if unlikely (cache_ready_map[cur_phys_addr]) {
            // Magic number 6 indicates 2^6=64 bytes
            // Read the data
            rc = this->load(cur_virt_addr, cur_phys_addr, matrix_data_buf+offset, temp_rip,
                    temp_uuid, true, 6);
            printf("SECOND ATTEMP to load %p, result = %d, simcycle = %ld\n", cur_phys_addr, rc, sim_cycle);
            ++wait_count;

            // Set cache ready to false indicates no longer need to read the
            // data
            cache_ready_map[cur_phys_addr] = false;
            --request_count;
        }
    }

    //printf("wait_count = %d, row_count = %d\n", wait_count, row_count);
    // When all elements in the same row finishes, proceed to the next row.
    if unlikely (wait_count == matrix_header.n / MEM_REQ_COUNT) {
        printf("Row_count = %d, Wait_count = %d\n", row_count, wait_count);

        ++row_count;
        wait_count = 0;

        for (int j = 0; j < matrix_header.n; ++j) {
            requested[j] = false;
        }
    }

    // When all rows have been processed, return true indicates this stage has
    // finished.
    if (row_count >= matrix_header.m) {
        return true;
    } else {
        return false;
    }
}

void print_matrix(int *A) {
    for (int i = 0; i < matrix_header.m; ++i) {
        for (int j = 0; j < matrix_header.n; ++j) {
            printf("%d ", A[i*matrix_header.n+j]);
        }
        printf("\n");
    }
}

bool Accelerator::do_calculate(void *nothing)
{
    // Debugging: print out A, B
    //print_matrix(matrix_data.A);
    //print_matrix(matrix_data.B);

    // XXX Current dummy operation: C = A + B
#if 0
    for (int i = 0; i < matrix_header.m; ++i) {
        for (int j = 0; j < matrix_header.n; ++j) {
            matrix_data.C[i*matrix_header.n+j] =
                matrix_data.A[i*matrix_header.n+j] +
                matrix_data.B[i*matrix_header.n+j];
        }
    }
#endif

    //print_matrix(matrix_data.C);

    //return ((++cal_cycle_count) >= max_cal_cycle);
    return true;
}

bool Accelerator::do_store(void *nothing)
{
    int rc = ACCESS_OK;

    // DEBUG Print out data
#if 0
    for (int i = 0; i < matrix_data_buf_size/3; ++i) {
        printf("%d ", matrix_data_buf + i * sizeof(int));
    }
    printf("\n");

    rc = store(temp_virt_addr+2*sizeof(int), temp_phys_addr+2*sizeof(int),
            matrix_data_buf, matrix_data_buf_size, temp_rip, temp_uuid, true);
    printf("matrix_data_buf_size = %lu\n", matrix_data_buf_size);
    if (rc != ACCESS_OK) {
        return false;
    }
#endif

    rc = true;

    printf("Going to send finish signal\n");

    // XXX Memory leak, left for test right now
    //free(matrix_data_buf);

    //int bytemask = ((1 << (1 << 3))-1);
    //W64 reg = ctx->storemask(LAP_MMIO_ADDR, 0, bytemask); // sizeshift=3 for 64bit-data
    lap_mmio_reg = 0;

    // Testing with a delay here, see if the delay causes the bug.
    //int delay = 100;
    //marss_add_event(core_wakeup_signal, delay, NULL);


    // Send an interrupt to the CPU
    send_interrupt();
    return true;
}

void Accelerator::send_interrupt()
{
    //ctx->handle_interrupt = 1;
    //ctx->exception_is_int = 0;
    //ctx->interrupt_request |= CPU_INTERRUPT_HARD;

    lap_finish();
#if 0
    qemu_set_irq(lap_irq, 1);
    qemu_set_irq(lap_irq, 0);
#endif
}
void Accelerator::do_Pref(){

  //Steps to do :
  //1. Set-up Issue Request 
  //2. Generate address
  //3. Pref_Issue ---> Prefetcher will issue request to DRAM
  //4. Pref_Check_Ready --> Check whether there is any available cache line
  //5. Req to SRAM to send the data back

    PF_SM_Issuer_DRAM();
    Address_Gen_DRAM();
    //Load();
    Pref_Issue();
    Pref_Check_Ready();
#ifdef HALFWORD
    Issue_Request_SRAM = FALSE;
    sram_package->req=FALSE;
    sram_package->res_amnt=FALSE;
    sram_package->res=FALSE;
    if((counter_half%2)==0)
#endif     
    {
    PF_SM_SRAM();
    Req_to_SRAM();
    }
    Update_SM();
    counter_half = (counter_half+1)%2;

}



int Accelerator::load(W64 virt_addr, W64 phys_addr, void* data, W64 rip, W64 uuid, bool is_requested, int sizeshift)
{
    bool hit;

    Memory::MemoryRequest *request = memoryHierarchy->get_free_request(id);
    assert(request != NULL);

    request->init(id, 0, phys_addr, 0, sim_cycle,
            false, rip /* What should be the RIP here? */,
            uuid /* What should be the UUID here? */,
            Memory::MEMORY_OP_READ);
    request->set_coreSignal(&dcache_signal);

    if (!is_requested) {
        hit = memoryHierarchy->access_cache(request);

        if (!hit) {
            // Handle Cache Miss
            cache_ready = false;
            return ACCESS_CACHE_MISS;
        }
    }

    // On cache hit, retrieve data from the memory location.

    if (sizeshift <= 3) {
        bool old_kernel_mode = ctx->kernel_mode;
        ctx->kernel_mode = true;
        *((W64*) data) = ctx->loadvirt(virt_addr, sizeshift); // sizeshift=3 for 64bit-data
        ctx->kernel_mode = old_kernel_mode;
    } else {
        W64 count = 1 << (sizeshift - 3);

        bool old_kernel_mode = ctx->kernel_mode;
        ctx->kernel_mode = true;
        
        int index;

        /*for(int i=0; i<Buf_DRAM.size(); i++){
            if (to_DRAM[i].dram_address==phys_addr)
              index=i;
            break;
        }*/

        for (int i = 0; i < count; ++i) {
            // Load 8 byte at a time
            *(((W64*)data) + i) = ctx->loadvirt(virt_addr + i * 8, 3);
            //need to push_back, masri
            //Buf_DRAM.data.push_back(ctx->loadvirt(virt_addr + i * 8, 3));
            //Buf_DRAM.sram_address.push_back(to_DRAM[index].sram_address[i]);

        }
        ctx->kernel_mode = old_kernel_mode;
    }

    return ACCESS_OK;
}

/*void Accelerator::Pref_Store(W64 virt_addr, W64 phys_addr, double & data, W64 rip, W64 uuid, bool is_requested, int sizeshift, int index){
    
  ATOM_CORE_MODEL::StoreBufferEntry buf[Cache_Line/Element_Size];

    for (int i =0; i<Cache_Line/Element_Size; i++){

      buf[i].data = DoubleToW64(data[i]);
      buf[i].addr = phys_addr + i*8;
      buf[i].virtaddr = virt_addr + i*8;
      // (1<<UOP_SIZE) is the number of bytes in the data
#define UOP_SIZE sizeshift
      buf[i].bytemask = ((1 << (1 << UOP_SIZE))-1);
      buf[i].size = UOP_SIZE;
      // Be careful not to use buf.op below.
      buf[i].op = NULL;
      buf[i].mmio = ctx->is_mmio_addr(virt_addr + i*8, true);
    }

    bool old_kernel_mode = ctx->kernel_mode;
    ctx->kernel_mode = true;
    //bug
    //should bring for loop here
    for (int i =0; i<Cache_Line/Element_Size; i++){ 
      buf[i].write_to_ram(*ctx);
    }
    
    ctx->kernel_mode = old_kernel_mode;

}*/


int Accelerator::store(W64 virt_addr, W64 phys_addr, W64& data, W64 rip, W64 uuid, bool is_requested, int sizeshift)
{
    ATOM_CORE_MODEL::StoreBufferEntry buf;

    buf.data = data;
    buf.addr = phys_addr;
    buf.virtaddr = virt_addr;
    // (1<<UOP_SIZE) is the number of bytes in the data
#define UOP_SIZE sizeshift
    buf.bytemask = ((1 << (1 << UOP_SIZE))-1);
    buf.size = UOP_SIZE;
    // Be careful not to use buf.op below.
    buf.op = NULL;
    buf.mmio = ctx->is_mmio_addr(virt_addr, true);

    Memory::MemoryRequest *request = memoryHierarchy->get_free_request(id);
    //printf("id = %d\n", id);
    assert(request != NULL);

    request->init(id, 0, phys_addr, 0, sim_cycle,
            false, rip,
            uuid ,
            Memory::MEMORY_OP_WRITE);
    request->set_coreSignal(&dcache_signal);

    //memoryHierarchy->access_cache(request);

    //printf("Writing to RAM: virtaddr = %llu, data = %llu, bytemask = %d, size = %d\n",
    //        buf.virtaddr, buf.data, buf.bytemask, buf.size);

    //printf("STORE virtaddr=%llx, data=%ld, sizeshift=%d\n", virt_addr, data, sizeshift);
    bool old_kernel_mode = ctx->kernel_mode;
    ctx->kernel_mode = true;
    //bug
    //should bring for loop here
    
    buf.write_to_ram(*ctx);
    ctx->kernel_mode = old_kernel_mode;
    //ctx->storemask(buf.addr, buf.data, buf.bytemask);

    return ACCESS_OK;
}

int Accelerator::store(W64 virt_addr, W64 phys_addr, void *data, size_t size, W64 rip, W64 uuid, bool is_requested)
{
    int rc;
    int sizeshift;
    W64 word;
    int ret = ACCESS_OK;
    W64 cur_virt_addr, cur_phys_addr;

    for (int i = 0; i < size; i += (1<<sizeshift)) {
        if (size - i >= 8) {
            sizeshift = 3;
        } else if (size-i >= 4){
            sizeshift = 2;
        } else if (size-i >= 2){
            sizeshift = 1;
        } else if (size-i >= 1){
            sizeshift = 0;
        } else {
            assert(0);
        }

        // Page table?
        cur_virt_addr = virt_addr + i;
        cur_phys_addr = phys_addr + i;

        switch (sizeshift) {
            case 0: {
                byte *b = (byte *)((byte *)data + i);
                word = *b;
                break;
                    }

            case 1: {
                W16 *b = (W16 *)((byte *)data + i);
                word = *b;
                break;
                    }

            case 2: {
                W32 *b = (W32 *)((byte *)data + i);
                word = *b;
                break;
                    }

            case 3: {
                W64 *b = (W64 *)((byte *)data + i);
                word = *b;
                break;
                    }

            default:
                printf("Unsupported sizeshift: %d", sizeshift);
                break;
        }

        rc = store(cur_virt_addr, cur_phys_addr, word, rip, uuid, is_requested, sizeshift);
        if (rc < 0) {
            ret = ACCESS_CACHE_MISS;
        }
    }

    return ret;
}

int Accelerator::load_buf(W64 virt_addr, W64 phys_addr, void *data, size_t size, W64 rip, W64 uuid, bool is_requested)
{
    int rc;
    int sizeshift;
    W64 word;
    int ret = ACCESS_OK;
    W64 cur_virt_addr, cur_phys_addr;

    for (int i = 0; i < size; i += (1<<sizeshift)) {
        if (size - i >= 8) {
            sizeshift = 3;
        } else if (size-i >= 4){
            sizeshift = 2;
        } else if (size-i >= 2){
            sizeshift = 1;
        } else if (size-i >= 1){
            sizeshift = 0;
        } else {
            assert(0);
        }

        cur_virt_addr = virt_addr + i;
        cur_phys_addr = phys_addr + i;

        //printf("cur_virt_addr = %llu\n", cur_virt_addr);
        rc = load(cur_virt_addr, cur_phys_addr, &word, rip, uuid, is_requested, sizeshift);
        if (rc < 0) {
            ret = ACCESS_CACHE_MISS;
        } else {
            switch (sizeshift) {
                case 0: {
                    byte *b = (byte *)((byte *)data + i);
                    *b = word;
                    break;
                        }

                case 1: {
                    W16 *b = (W16 *)((byte *)data + i);
                    *b = word;
                    break;
                        }

                case 2: {
                    W32 *b = (W32 *)((byte *)data + i);
                    *b = word;
                    break;
                        }

                case 3: {
                    W64 *b = (W64 *)((byte *)data + i);
                    *b = word;
                    break;
                        }

                default:
                    printf("Unsupported sizeshift: %d", sizeshift);
                    break;
            }
        }


    }

    return ret;
}

// Handle data path when a load request finishes.
bool Accelerator::load_cb(void *arg)
{
    Memory::MemoryRequest* req = (Memory::MemoryRequest*)arg;
    // TODO: Set the flag correlate to the requested memory
  //  printf("Inside Accelerator dcache callback.\n");
    cache_ready = true;
    //cache_ready_map[req->get_physical_address()] = true;
    /*printf("Ready for physical address: %p\n", req->get_physical_address());
    printf("Ready for physical address: %llx\n", req->get_physical_address());*/
    //printf("In load cb\n");

    if(wait_buffer.size()>0){
      for(int i=0; i<wait_buffer.size(); i++){
        if (wait_buffer.at(i).phys_addresses[0]==req->get_physical_address()){
          wait_buffer.at(i).ready=true;
          //printf("Ready for physical address inside wait buffer: %p\n", req->get_physical_address());
          count_access++;
          break;
        }
      }
    }
    //printf("Ready for physical address: %p\n", req->get_physical_address());
    //printf("Current cycle: %ld\n", sim_cycle);

    return true;
}

W64 Accelerator::exec(AcceleratorArg& arg)
{

    // The following are commands to be executed.
    printf("Caught an accelerator exec!\n");
    printf("arg = %llu\n", arg.virt_addr);
    temp_virt_addr = arg.virt_addr;
    temp_phys_addr = arg.phys_addr;
    temp_rip = arg.rip;
    temp_uuid = arg.uuid;

    printf("virt_addr = %llu\n", arg.virt_addr);
    printf("phys_addr = %llu\n", arg.phys_addr);

    cache_ready = true;
    temp_state = Accel_Load_header;

    return 0;
}

//write dump memory here

void Accelerator::Check_Data(W64 Virt_Addr){

    bool old_kernel_mode = ctx->kernel_mode;
    ctx->kernel_mode = true;

    double * data = matrix_data_buf;

    for (int i = 0; i < Cache_Line/Element_Size; ++i) {
        // Load 8 byte at a time 
        
        *(((W64*)data) + i) = ctx->loadvirt(Virt_Addr + i* 8, 3);
        //need to push_back, masri
       
        double test2 = W64toDouble (*(((W64*)data) + i));
        

        printf("Mem is [%d] =  %lf \n",i , test2);
        //getchar();
    }
}

void Accelerator::Dump_DRAM(){

    bool old_kernel_mode = ctx->kernel_mode;
    ctx->kernel_mode = true;

    double * data = matrix_data_buf;

    //for (int i = 0; i < MC*NC; ++i) {
    for (int i = 0; i < LDC*LDC; ++i) {
        // Load 8 byte at a time 
        //int idx =  (i%MC)*MC + i/MC;
        int idx =  (i%LDC)*LDC + i/LDC;
        
        *(((W64*)data) + i) = ctx->loadvirt(temp_virt_addr_A- KC*sizeof(double) + idx * 8, 3);
        //need to push_back, masri
       
        double test2 = W64toDouble (*(((W64*)data) + i));
        

        printf("DRAM[%d] =  %lf \n",i , test2);
        //getchar();
    }
    
    ctx->kernel_mode = old_kernel_mode;

}


// Currently it only tests loading in the memory Hierarchy.
// Return true if exit to QEMU is requested.
bool Accelerator::runcycle(void *nothing)
{

  //printf("At begin run cycle\n");


  if(Freq_Ctr>=CPU_FREQ){
    
    Freq_Ctr -= CPU_FREQ;
    
    switch (temp_state) {
        case Accel_Load_header:
            if (cache_ready) {
                if (do_load_header(nothing)) {
                    printf("Load Header complete!\n");
                    row_count = 0;
                    column_count = 0;
                    block_count = 0;
                    request_count = 0;
                    wait_count = 0;
                    for (int i = 0; i < MAX_SIZE_IN_TEST; ++i)
                        requested[i] = false;
                    //temp_state = Accel_Load_content_row_major;
                    //temp_state = Accel_Cal;
                    //temp_state = Accel_Prefetching;
                      temp_state = Accel_Construct;
                      
                    //temp_state = Accel_Cal;

                      /*//test for new read
                      temp_state = Accel_Idle;
                      lap_mmio_reg = 0;
                      send_interrupt();*/

                      //temp_state = Accel_Store;
                    //printf("Current cycle: %ld\n", sim_cycle);
                    //exit(0);
                    PER_ITER_BEGIN_CYCLES = sim_cycle;
                }
                temp_state = Accel_Cal;
              }
            break;

        case Accel_Construct:

            do_construct();
            temp_state = Accel_Prefetching;

        break;
        
        case Accel_Intermediate:

            do_intermediate();
            
          for (int i=0; i<NumofCore; i++)
            CORES[i]->Reset();
            
            temp_state = Accel_Prefetching;

        break;

        case Accel_Prefetching :

        
        for (int i=0; i<NumofCore; i++){
      
          if (CORES[i]->GetKernelStatus()==FALSE) 
          CORES[i]->GEMM_Compute(0);
          //CORES[i]-> GEMM_Test_Pref();

          else{
          ;
          } 
            //finish = i; //notice which core has finished
        }

        //printf("Before Pref\n");

        do_Pref();
        
        //printf("After Pref\n");

#if 1        
        for (int j=0; j<NumofCore; j++){
          Sram[j]->run_every_cycle();
        }
#endif

        //printf("After SRAM\n");

        Sram_Pref->run_every_cycle();

        //Sram_Pref->run_every_cycle();
        //if(Pref_DRAM_Next==Pref_B){
        //if(count_access==28){
        //cout << " Sent_C_Current is " << Sent_C_Current<<endl;
        //if(Sent_B_Current==16){ 
#if 0
        cout << "Pref Current is " << Pref_DRAM_Current << endl;
        cout << " Sent_A_Current is " << Sent_A_Current<<endl;
        cout << " Sent_B_Current is " << Sent_B_Current<<endl;
        cout << " Sent_C_Current is " << Sent_C_Current<<endl;
        //cout << " count access is " << count_access<<endl;
        cout << " wait buffer size is  " << wait_buffer.size()<<endl;
        cout << " Buf DRAM size is  " << Buf_DRAM->addr.size()<<endl;
        //if(Pref_DRAM_Current==Wait && /*fetched_B==1*/ fetched_A_total==1/*&& Sent_A_Current==1200*/){ 
#endif

#if 0        
        if(/*BigC_Current==4 &&*/ Pref_DRAM_Current==Write_C){
        printf("Start printing\n");
        printf("Pref_DRAM_Current is  %d\n", Pref_DRAM_Current);
        printf("wait buffer size is  %d\n", wait_buffer.size());
        printf("WriteC_Current is %d\n ", WriteC_Current);
        printf("Read_C_Current is %d\n ", Read_C_Current);
        printf("fetched_B %d\n ", fetched_B);
        printf("fetched_A_total %d\n ", fetched_A_total);
        printf("done %d\n ", done);
        printf("fetched_Cin_total %d\n ", fetched_Cin_total);
    
        }  
#endif
        //printf("After Core\n");
        //exit(0);
        
        if(Pref_DRAM_Current==Wait && wait_buffer.size()==1){
          //LAST_STORE = 1;
           MOVE_TO_LAST = 1;
           LAST_COUNTER = 0;
        } 
        
        if(MOVE_TO_LAST && Pref_DRAM_Current==Wait && wait_buffer.size()==0){ 
          
#if 1
          static int ctr =0;
        
          LAST_STORE = 1;
         //for(int i=0; i<Panel_Size*Panel_Size + K_H*Panel_Size; i++)
            //printf("SRAM[%d] is %lf\n", i, SRAM[i]);
          
          if(ctr==1){
              printf("Finish Iteration Current cycle: %ld\n", sim_cycle);
              delete[]  SRAM;
              SRAM = NULL;

              temp_state = Accel_Idle;
              lap_mmio_reg = 0;
              send_interrupt();
          }

          ctr++;
         
#endif
        LAST_STORE = 1;        

          if(LAST_COUNTER==1){
              ITER_COUNT ++;
              PER_ITER_END_CYCLES = sim_cycle;
              W64 cycleSpent = (PER_ITER_END_CYCLES - PER_ITER_BEGIN_CYCLES);
              TOT_LAP_CYCLES += (PER_ITER_END_CYCLES - PER_ITER_BEGIN_CYCLES);

              ptl_logfile <<"/*****************LAP STATS****************/"<<endl;

              ptl_logfile << "LAP Iteration #" << ITER_COUNT << " : " <<endl;
              ptl_logfile << "Begin Cycle : " << PER_ITER_BEGIN_CYCLES << endl;
              ptl_logfile << "End Cycle : " << PER_ITER_END_CYCLES << endl;
              ptl_logfile << "Total Cycles Spent in this Iter : " << cycleSpent <<endl;
              ptl_logfile << "Total Cycles Spent on LAP Comp : " << TOT_LAP_CYCLES << endl;

              printf("Finish Iteration,  Start cycle: %ld\n", PER_ITER_BEGIN_CYCLES);
              printf("Finish Iteration,  End cycle: %ld\n", PER_ITER_END_CYCLES);
              ptl_logfile <<"/*****************END****************/"<<endl;
              
              delete[]  SRAM;
              SRAM = NULL;
                
              PER_ITER_BEGIN_CYCLES = 0;
              PER_ITER_END_CYCLES = 0;

              temp_state = Accel_Idle;
              lap_mmio_reg = 0;
              send_interrupt();
              MOVE_TO_LAST = 0;
              LAST_COUNTER = 0;
              LAST_STORE   = 0;
          }

          LAST_COUNTER++;
        
        }
        break;

        case Accel_Load_content_row_major:
            if (do_load_content_row_major(nothing)) {
                printf("Load Content row-major complete!\n");
                column_count = 0;
                row_count = 0;
                block_count = 0;
                request_count = 0;
                wait_count = 0;
                for (int i = 0; i < MAX_SIZE_IN_TEST; ++i)
                    requested[i] = false;
                //temp_state = Accel_Load_content_column_major;
                temp_state = Accel_Cal;
                printf("Current cycle: %ld\n", sim_cycle);
            }
            break;

        case Accel_Load_content_column_major:
            if (do_load_content_column_major(nothing)) {
                printf("Load Content column-major complete!\n");
                column_count = 0;
                row_count = 0;
                block_count = 0;
                request_count = 0;
                wait_count = 0;
                for (int i = 0; i < MAX_SIZE_IN_TEST; ++i)
                    requested[i] = false;
                //temp_state = Accel_Load_content_block_major;
                temp_state = Accel_Cal;
                printf("Current cycle: %ld\n", sim_cycle);
            }
            break;

        case Accel_Load_content_block_major:
            if (do_load_content_block_major(nothing)) {
                printf("Load Content block-major complete!\n");
                column_count = 0;
                row_count = 0;
                block_count = 0;
                request_count = 0;
                wait_count = 0;
                for (int i = 0; i < MAX_SIZE_IN_TEST; ++i)
                    requested[i] = false;
                temp_state = Accel_Cal;
                printf("Current cycle: %ld\n", sim_cycle);
            }
            break;

        case Accel_Cal:
            
            
            static long long cotr=0;
            cotr++;
            
            //printf("At Accel_Cal ! cotr is %d\n", cotr);
            //exit(0);

#if 0
            if(cotr!=1000){
                break;
            }
            
            if (do_calculate(nothing)) {   
                printf("Cal complete! cotr is %d\n", cotr);
                exit(0);
                //temp_state = Accel_Store;
                temp_state = Accel_Idle;
                lap_mmio_reg = 0;
                send_interrupt();
            }

#endif            
            break;

        case Accel_Store:
            printf("storing!\n");
            if (do_store(nothing)) {
                printf("Storing complete!\n");
                temp_state = Accel_Idle;
            }
            break;

        default:
        case Accel_Idle:
            if (do_idle(nothing)) {
                printf("LAP Request Received! switching to load_header.");
                temp_state = Accel_Load_header;
            }
            break;

    }

  }
  
  //printf("At end run cycle\n");
  
  Freq_Ctr += ACCEL_FREQ;
  return false;
    //Multicore->execute ?//masri
}
