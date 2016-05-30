
#ifndef ACCELERATOR_H
#define ACCELERATOR_H

#include <ptlsim.h>
#include <ptl-qemu.h>
#include <machine.h>
#include <statsBuilder.h>
#include <memoryHierarchy.h>
#include "Parameters.h"
#include "LAPU.h"
#include "prefetcher.h"
#include "sram.h"
#include "sram_pref.h"
#include "dram.h"

namespace Core {
    enum {
        ACCESS_OK = 0,
        ACCESS_CACHE_MISS,
        ACCESS_FAIL,

        NUM_ACCESS_RESULTS
    };

    struct AcceleratorArg {
        W64 virt_addr;
        W64 phys_addr;
        W64 rip;
        W64 uuid;
    };

    struct Accelerator : public Statable {
        Accelerator(BaseMachine& machine, const char* name);
        virtual ~Accelerator() {}

        // TODO Pure Abstract function
        virtual void init();
        virtual bool runcycle(void*);

        virtual bool do_idle(void*);
        virtual bool do_load_header(void*);
        virtual bool do_load_content(void*);
        virtual bool do_load_content_row_major(void*);
        virtual bool do_load_content_column_major(void*);
        virtual bool do_load_content_block_major(void*);
        virtual bool do_store(void*);
        virtual bool do_calculate(void*);
        virtual void do_construct();
        virtual void do_intermediate();
        virtual void Check_Data(W64 Virt_Addr);
        virtual void send_interrupt();

        virtual void update_memory_hierarchy_ptr();
        virtual W64 exec(AcceleratorArg &arg);

        virtual int load(W64 virt_addr, W64 phys_addr, void* data, W64 rip, W64 uuid, bool is_requested, int sizeshift = 3);
        virtual int load_buf(W64 virt_addr, W64 phys_addr, void *data, size_t size, W64 rip, W64 uuid, bool is_requested);
        virtual int store(W64 virt_addr, W64 phys_addr, W64& data, W64 rip, W64 uuid, bool is_requested, int sizeshift = 3);
        virtual int store(W64 virt_addr, W64 phys_addr, void *data, size_t size, W64 rip, W64 uuid, bool is_requested);

        //virtual int load_blocked(W64 addr, W64& data);
        virtual bool load_cb(void *arg);
        
        virtual void PF_SM_Issuer_DRAM();
        virtual void Address_Gen_DRAM();
        virtual void Pref_Issue();
        virtual void Pref_Check_Ready();
        virtual void Load();
        virtual void Update_SM();
        virtual void Pref_Load(W64 virt_addr, W64 phys_addr, void* data, W64 rip, W64 uuid, bool is_requested, int sizeshift, int index);
        virtual void Pref_Store(W64 virt_addr, W64 phys_addr, double * data, W64 rip, W64 uuid, bool is_requested, int sizeshift, int index);
        virtual void PF_SM_SRAM();
        virtual void Req_to_SRAM();
        virtual void Track_SRAM(int addr);
        virtual void do_Pref();
        virtual void Dump_DRAM();
        virtual int  do_modify_kv(int threshold);

        int id;
        const char * name;

        BaseMachine& machine;
        Memory::MemoryHierarchy* memoryHierarchy;
        Context* ctx;
        Signal run_cycle;
        /* Core wakeup signal */
        Signal *core_wakeup_signal;

        /* Cache Access */
        Signal dcache_signal;

        /* Cache Miss Ready */
        bool cache_ready;


        /* MMIO control register value */
        W64 rc0;

        /* QEMU IRQ value */
        qemu_irq lap_irq;
        //masri define variables here
        int x_offset;
        int y_offset;
        int offset_C;
        int offset_B;


        int Chunk_B_Next;
        int Chunk_B_Curr;
        int Chunk_A_Next;
        int Chunk_A_Curr;
        //bool Req_to_DRAM;
        //bool Req_to_SRAM;
        bool WE_DRAM;
        bool WE_SRAM;
        bool DRAM_IN_DATA;
        bool DRAM_OUT_DATA;
        bool SRAM_IN_DATA;
        bool SRAM_OUT_DATA;
        bool ID_To_SRAM;

        typedef struct{
          int Addr;
          double Data[LAPU_Size];
          bool WE;
          int ID;
          bool Serviced;
        }DRAM_req_list;

        typedef struct{
          int Addr;
          double Data[LAPU_Size];
          bool WE;
        }SRAM_req_list;
        
        PREF_Package *sram_package;
        PF_Buffer *Buf_from_SRAM;
        
        DRAM_Package *dram_package ;
        PF_DRAM_Buffer *Buf_DRAM;
        
        bool Issue_Request_SRAM;
        bool Issue_Request_DRAM;
        bool Write_SRAM;
        bool Write_DRAM;
        int Address_SRAM;
        W64 Address_DRAM;

        enum Prefetch_SRAM {Send_C_SRAM, Send_B_SRAM, Send_A_SRAM, Read_C_SRAM,
                            Send_B_Init_SRAM, Idle} SRAM_Current, SRAM_Next;
        
        enum Prefetch_DRAM {Pref_C, Pref_B, Pref_A, Write_C, Pref_B_Init, Pref_A_Init,
                            Pref_Wait_for_Fetch_A_Again_Init, Wait, Pref_A_Init_Second} Pref_DRAM_Current, Pref_DRAM_Next;

        int Sent_C_Current;
        int Sent_C_Next;
        int Sent_C_Counter_Current;
        int Sent_C_Counter_Next;
        int Sent_B_Current;
        int Sent_B_Next;
        int Sent_B_Counter_Current;
        int Sent_B_Counter_Next;
        int Sent_A_Current;
        int Sent_A_Next;
        int Sent_A_Counter_Current;
        int Sent_A_Counter_Next;
        int Read_C_Current;
        int Read_C_Next;
        int Read_C_Counter_Current;
        int Read_C_Counter_Next;

        int SRAM_POS_Current;
        int SRAM_POS_Next;

        int IssueC_Current;
        int IssueC_Next;
        
        int IssueB_Current;
        int IssueB_Next;

        int IssueA_Current;
        int IssueA_Next;

        int BigC_Current;
        int BigC_Next;

        int BigB_Current;
        int BigB_Next;

        int BigA_Current;
        int BigA_Next;

        int WriteC_Current;
        int WriteC_Next;

        int done;
        int cout_ready;
        int count_cout;
        int cout_offset;
        int write_ready;

        int per_core_A_DRAM;
        int per_core_A_SRAM;
        int per_core_A_temp;
        int per_core_A;
        
        bool B_SRAM_Done;
        bool A_SRAM_Done;
        bool A_SRAM_Init;
        int A_SRAM_Unit;
        bool all_B_Ready;

        int wait_next_C;
        int fetched_Cin[NumofCore];
        int fetched_Cin_total;
        int fetched_B;
        int fetched_A[NumofCore];
        int fetched_A_total;

        int which_matrix;
        int last_A_Init;

        LAP_PREF_Sync *to_LS;

        int next_a;
        int A_Current;
        int A_Next;
        int counter_A;

        std::deque <DRAM_Package> wait_buffer;

        //for Multicore 
        /*************************************/
        //virtual int Execute();
        /*static int dump();

        int Assign_input_Matrix( double **& matrix_A, double **& matrix_B, double **& matrix_C);*/

        int PORT;
        int *Current_Arbiter;
        int *Next_Arbiter;
        int *Current_PORT;
        int *Next_PORT;
        int total_cycles;
        LAPU ** CORES;
        
        double **Matrix_A;
        double **Matrix_B;
        double **Matrix_C;

        S_RAM **Sram;
        PF    *Pref;
        LS_Buffer *LAP_Buf;
        S_RAM_PREF *Sram_Pref;
        PF_Buffer *PF_Buf;
        dram *Dram;
        LAP_Package *lap_req; 
        PREF_Package *pref_req; 
        DRAM_Package *pref_dram_req;
        PF_DRAM_Buffer *PF_DRAM_Buf;
        LAP_PREF_Sync *lap_pref; 

        bool Partial_Line;
        int  Partial_Frac;

        int Freq_Ctr;

    };
}

#endif // ACCELERATOR_H
