/*
 * PE.cpp
 *
 *  Created on: Mar 4, 2010
 *      Author: Ardavan
 */

#include "PE.h"

PE::PE()
{
	// TODO Auto-generated constructor stub

}

PE::PE(int row, int col,  double * row_write_bus, double * row_read_bus, double * col_write_bus, double * col_read_bus, int CoreID)
{
	// TODO

  Kernel_V = K_V;
  Kernel_H = KC;
	
  Address_Reg_A_Curr=0;
	Address_Reg_A_Next=0;

	Address_A=0;
	Address_WA=0;

  MyCoreID=CoreID;

	Address_Reg_B_Curr=(Kernel_V*Kernel_H)/(LAPU_Size*LAPU_Size);
	Address_Reg_B_Next=(Kernel_H*Kernel_V)/(LAPU_Size*LAPU_Size);

	Address_B=(Kernel_V*Kernel_H)/(LAPU_Size*LAPU_Size);


	Address_Reg_WB_Curr=(Kernel_V*Kernel_H)/(LAPU_Size*LAPU_Size)+
						Kernel_H;  //is this kernel v or h?
	Address_Reg_WB_Next=(Kernel_H*Kernel_V)/(LAPU_Size*LAPU_Size)+
						Kernel_H	;

	Address_WB=(Kernel_V*Kernel_H)/(LAPU_Size*LAPU_Size)+
				Kernel_H	;


  toA = 'A';
  toB1= 'B';
  toB2= 'P';
  BMode = TRUE;
  AMode = false;
  fetch_C =true;
  fetch_A =false;
  fetch_B =false;
  send_C = false;


	My_Row= row;
	My_Column= col;

  Scratch_Regs_Curr= new double[Scratch_Size];
	Scratch_Regs_Next= new double[Scratch_Size];

	Read_My_Col_Bus= col_read_bus;
	Read_My_Row_Bus= row_read_bus;

	Write_My_Col_Bus= col_write_bus;
	Write_My_Row_Bus= row_write_bus;
	Cin_Counter=0;
	Cout_Counter=0;
	Bin_Counter=0;
  Ain_Counter=0;
	//std::cout<< "end of PE const"<<std::endl;
  Address_WB_Fetch = 0;
}

PE::~PE()
{
	// TODO Auto-generated destructor stub
}

void PE::Reset(){
  
  Counter_Curr = 0;
  Counter_Next = 0;
  
  Kernel_V = K_V;
  Kernel_H = KC;
	
  Address_Reg_A_Curr=0;
	Address_Reg_A_Next=0;

	Address_A=0;
	Address_WA=0;
	Address_WA_New=0;
  Address_WB_Fetch = 0;

	Address_Reg_B_Curr=(Kernel_V*Kernel_H)/(LAPU_Size*LAPU_Size);
	Address_Reg_B_Next=(Kernel_H*Kernel_V)/(LAPU_Size*LAPU_Size);

	Address_B=(Kernel_V*Kernel_H)/(LAPU_Size*LAPU_Size);


	Address_Reg_WB_Curr=(Kernel_V*Kernel_H)/(LAPU_Size*LAPU_Size)+
					Kernel_H;  //is this kernel v or h?
	Address_Reg_WB_Next=(Kernel_H*Kernel_V)/(LAPU_Size*LAPU_Size)+
						Kernel_H	;

	Address_WB=(Kernel_V*Kernel_H)/(LAPU_Size*LAPU_Size)+
				Kernel_H	;


  toA = 'A';
  toB1= 'B';
  toB2= 'P';
  BMode = TRUE;
  AMode = false;
  fetch_C =true;
  fetch_A =false;
  fetch_B =false;
  send_C = false;

	Cin_Counter=0;
	Cout_Counter=0;
	Bin_Counter=0;
  Ain_Counter=0;
  
  FcA = 0;
  newC = 0;

  ALU.Flush_Accumulator();

}

int PE::Cycle(){

	int i;
	for (i=0; i<Scratch_Size; i++)
		Scratch_Regs_Curr[i]=Scratch_Regs_Next[i];

	ALU.Cycle();
	//Local_Mem.Cycle();

	Counter_Curr=Counter_Next;
//	Address_Curr=Address_Next;

	Write_My_Col_Reg_Curr=Write_My_Col_Reg_Next;
	Write_My_Row_Reg_Curr=Write_My_Row_Reg_Next;


	Address_Reg_A_Curr=Address_Reg_A_Next;
	Address_Reg_B_Curr=Address_Reg_B_Next;
	Address_Reg_WB_Curr=Address_Reg_WB_Next;

}



int PE::Intialize_Local_Mem( double ** Input_matrix, int row_number, int column_number,int offset){

	Local_Mem.Initialize_Register_File(My_Row, My_Column, Input_matrix,row_number, column_number, offset);
	return 0;

}

int PE::Initialize_Local_Mem_New( double ** Input_matrix, int row_number, int column_number,int offset, char matr){

	Local_Mem.Initialize_Register_File_New(My_Row, My_Column, Input_matrix,row_number, column_number, offset, matr);
	return 0;

}

int PE::Flush_Local_Mem_New( double **& Input_matrix, int row_number, int column_number, int offset, char matr){

	Local_Mem.Flush_Register_File_New(Input_matrix,row_number, column_number, offset, matr);
	return 0;

}

int PE::Flush_Local_Mem( double **& Input_matrix, int row_number, int column_number, int offset){

	Local_Mem.Flush_Register_File(Input_matrix,row_number, column_number, offset);
	return 0;

}


int PE::Return_PE_Power(){


	return ALU.Return_FMA_Power_Consumed();

	// and register file and other stuff +State machine;



}



int PE::Dump_Regs(){

	int i;
	std::cout<<"PE("<<My_Row<<","<<My_Column<<"):"<<std::endl;

	for (i=0; i < Scratch_Size ;i++)
		std::cout << "Scratch["<<i<<"]"<<"="<<Scratch_Regs_Curr[i]<<std::endl;

	// What I am about to BC
	std::cout<<"Write_My_Col_Reg_Curr="<<Write_My_Col_Reg_Curr<<std::endl;
	std::cout<<"Write_My_Row_Reg_Curr="<<Write_My_Row_Reg_Curr<<std::endl;

	//What I see
	std::cout<<"Read_My_Col_Bus="<<*Read_My_Col_Bus<<std::endl;
	std::cout<<"Read_My_Row_Bus="<<*Read_My_Row_Bus<<std::endl;


	std::cout <<"Local_Mem_Address="<<Local_Mem_Address<<std::endl;
	std::cout<<"Accumulator="<<ALU.Return_ACC()<<std::endl;
	return 0 ;

}



int PE::Dump_ALU_Pipeline(ALU_op operation_type){


	std::cout<<"PE("<<My_Row<<","<<My_Column<<")"<<std::endl;
	std::cout<<"Dumping ALU_Pipeline ..,"<<std::endl;
	ALU.Dump_Pipeline(operation_type);


	}

int PE::Recurs_Gen_A(int Prev){
  
  if (Prev==0) return 1;
  else return (Recurs_Gen_A(Prev-1) +1)%((NumofA * Kernel_H*Kernel_V)/(LAPU_Size*LAPU_Size));
  ;

}

int PE::Recurs_Gen_ReadA(int Prev){
  
  if (Prev==0) return 1;
  else return (Recurs_Gen_A(Prev-1) +1)%((Kernel_H*Kernel_V)/(LAPU_Size*LAPU_Size));
  ;

}

int PE::Recurs_Gen_B(int Prev){
  
  if (!Prev) return 1;
  else return (Recurs_Gen_B(Prev-1) +1)%(Kernel_H);

}

int PE::Generate_Address_Signals(int Global_index, int Trsm_index, int Iter_Counter, int Latency_Counter,
								LAPU_Function routine,int LAPU_Current_State, int State_Start){
return 0;

}
//int PE::


//TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO
//TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO
//TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO
//TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO


void PE::Dump_PE_Mem (int amount){

#if 0
  //for(int i=0; i<Kernel_Size*Kernel_Size; i++){
  for(int i=0; i<Kernel_Size*Panel_Size; i++){
    
    
    //std::cout << "SRAM["<<i<<"]"<< "="<< SRAM[Panel_Size*Panel_Size + 2*Panel_Size*K_H + i]<<std::endl;
    //std::cout << "SRAM["<<i<<"]"<< "="<< SRAM[Panel_Size*Panel_Size + 2*Panel_Size*K_H + i]<<std::endl;

    /*if((i%(Kernel_Size+LAPU_Size))==0){
    //if((i%LAPU_Size)==LAPU_Size-1){
      std::cout << "SRAM["<<i<<"]"<< "="<< SRAM[i]<<std::endl;
      std::cout << "SRAM["<<i+1<<"]"<< "="<< SRAM[i+1]<<std::endl;
      std::cout << "SRAM["<<i+2<<"]"<< "="<< SRAM[i+2]<<std::endl;
      std::cout << "SRAM["<<i+3<<"]"<< "="<< SRAM[i+3]<<std::endl;
      std::cout<<std::endl; 
    
    }*/
  }

#endif

	for (int i=0; i<amount; i++)
      std::cout<<Local_Mem.Reg_Read_New(i, toB1)<<std::endl;
	std::cout<<"xxxxxxxxxxxxxx"<<std::endl;
	
  for (int i=0; i<amount; i++)
      std::cout<<Local_Mem.Reg_Read_New(i, toB2)<<std::endl;
	std::cout<<"xxxxxxxxxxxxxx"<<std::endl;
  
	std::cout<<"AAAAAAAAAAAAAAAAA"<<std::endl;
  for (int i=0; i<Mem_Size_A*2; i++)
      std::cout<<Local_Mem.Reg_Read_New(i, toA)<<std::endl;
	std::cout<<"xxxxxxxxxxxxxx"<<std::endl;

  //exit(0);
  
}




int PE::Execute				(int Global_index, int Trsm_index, int Iter_Counter, int Latency_Counter,
							LAPU_Function routine,int LAPU_Current_State, int State_Start){


 Local_Mem_Address=-1;
	// If I access memory with -1 Address I have had a Bug so it is good bug checcking scheme



		 switch (routine){

			case LAPU_Cholesky:

				//if ((My_Row==My_Column) && (My_Row==0)) std::cout<< "Exectuing CHOLESKY ..."<<std::endl;
				switch (LAPU_Current_State){

					case 0: // Initial

						//retrieve data from local memory
						//it should be the (0,0) PE  and iteration is automatically 0;
						// If iteration is 0
						//if (Global_Index==0 "I have not brought it here before to update" && iteration==0 "'it is not update within cholesky")
						//I have to fetch my local data,else I have already fetched it for Rank_1 Update before
						// Actually other than the first Cholesky data is always in scratch pad memory but I do not mention it here.


						//if (Global_Index==0)
						//if ((My_Row==My_Column) && (Iter_Counter==0))  // since it is the First Retrieve

						//	{
							Local_Mem_Address= Global_index * (Kernel_Size / LAPU_Size)+ Global_index;
							// find the place of the diagonal element in the diagonal PE

							// put it on the bus

						//	}

							if (Global_index==0) // write to Scratch_Pad since we had no Rank-K update
								Scratch_Regs_Next[0]=Local_Mem.Reg_Read(Local_Mem_Address);

							if ((My_Row==My_Column) && (My_Row==Iter_Counter))
								Write_My_Row_Reg_Next=Local_Mem.Reg_Read(Local_Mem_Address);

						// If iteration is 0 (Global_Index==0 "I have brought it here to update" && iteration==0) I have to fetch my local data, else I have already fetched it for Rank_1 Update before



						// TODO I put it on the Row bus

						//else Local_Mem_Address=-1;



					break;

					case 1:  // Feed_Sqrt





						if ((My_Row==My_Column) && (My_Row==Iter_Counter)){
								*Write_My_Row_Bus=Write_My_Row_Reg_Curr;
								//std::cout<<"I wrote on the Bus this"<< *Write_My_Row_Bus<<std::endl;
						}

						//PEs, Are Waiting for the Bus to pass the data to Sqrt;
						//Do nothing
						//Local_Mem_Address=-2;


					break;

					case 2: //Chol_Inv_Sqrt

						//PEs Are Waiting for the Sqrt to be computed
						Local_Mem_Address=-3;

					break;


					case 3: // Chol_BC_InvSqrt

						//PEs are waiting for Sqrt to come back

						Local_Mem_Address=-4;

					break;

					case 4:  // Chol_Multiply

						// If I am in the iteration_Numbers row or column I have to read from my Bus  and perform Multiply;
						// on my Local Data
						// If iteration is 0 and Global_index=0 I have to fetch it, else I have already fetched it for Rank_1 Update before

						// Lets do the fetching first in intialization if needed and get rid of this complexity


						// If I am the diagonal PE I have to save inv_Sqrt to a scratchpad register;

						if ((My_Row==Iter_Counter) && (Latency_Counter==0)) // Saving one BroadCast clock in Trsm :D
							Scratch_Regs_Next[1]=*Read_My_Row_Bus;

						if ( ( (My_Row==Iter_Counter)&&(My_Column>=My_Row) )  || ( (My_Column==Iter_Counter) && (My_Row>=My_Column) )   ){

							if (Latency_Counter<(Multiplication_Latency-1)){
								if (My_Row==Iter_Counter)
									ALU.Execute_Mul(*Read_My_Row_Bus,Scratch_Regs_Curr[0]);
								else
									ALU.Execute_Mul(*Read_My_Col_Bus,Scratch_Regs_Curr[0]);
							}

							else{

								Scratch_Regs_Next[0]=ALU.Execute_Mul(*Read_My_Row_Bus,Scratch_Regs_Curr[0]);

								if (My_Row==Iter_Counter)
									Write_My_Col_Reg_Next=Scratch_Regs_Next[0];
								if (My_Column==Iter_Counter)
									Write_My_Row_Reg_Next=Scratch_Regs_Next[0];

							}
						}

							//TODO  I do not care about what else I feed to it in other cycles right now



						// probably put the result of the multiply on the bus in the last cycle


					break;


					case 5: //  Chol_BC_Mul

						//PEs are just sending what they multiply to receivers


						if ((My_Column==Iter_Counter)  && (My_Column!=My_Row))
							*Write_My_Row_Bus=Write_My_Row_Reg_Curr;


						if	((My_Row==Iter_Counter)&& (My_Column!=My_Row))
							*Write_My_Col_Bus=Write_My_Col_Reg_Curr;



							//PEs are waiting to be brightened up by new data :D



					break;


					case 6: // Chol_Rank1_Update;


						// Those PEs that are supposed to be updated
						if ( (My_Column>Iter_Counter) && (My_Row>Iter_Counter)){

							if (Latency_Counter<(FMA_Latency-1)){
								ALU.Execute_MAD(*Read_My_Col_Bus, (-1)*(*Read_My_Row_Bus), Scratch_Regs_Curr[0]);

							}
							else {

								Scratch_Regs_Next[0]=ALU.Execute_MAD(*Read_My_Col_Bus, (-1.0)*(*Read_My_Row_Bus), Scratch_Regs_Curr[0]);
								if ((My_Column==(Iter_Counter+1))  && (My_Row==(Iter_Counter+1)))
									Write_My_Row_Reg_Next=Scratch_Regs_Next[0];
							}

						}


					break;


					case 7: // End

						Local_Mem_Address=-8;
						// Store them All so


						Local_Mem_Address=Global_index * (Kernel_Size / LAPU_Size)+ Global_index;
						//std::cout<<"PE("<<My_Row<<","<<My_Column<<") Regiters["<<Local_Mem_Address<<"]="<<Scratch_Regs_Curr[0];


						Local_Mem.Reg_Write(Local_Mem_Address,Scratch_Regs_Curr[0]);
					break;

				}





				break; // Final decision: Just fetch all from local register file in the initial state
						//and the rest of operations do not work with Local Stare
						// in the End State Store the data

			case LAPU_Trsm:
				//if ((My_Row==My_Column) && (My_Row==0)) std::cout<< "Exectuing TRSM ..."<<std::endl;
				switch (LAPU_Current_State){

					case 0:  //Initial
						// Just move the Data to the Bus if you are Diagonal PE
						// All other PEs Retrieve the Data to Accumulator of yourself
						//prepare for Multiplication





						//Local_Mem_Address=Global_index*(Kernel_Size / LAPU_Size)+Trsm_index ;

						Local_Mem_Address= (Trsm_index)* (Kernel_Size/LAPU_Size)  +  Global_index;
						//if ((My_Row==0)&&(My_Column==0))
						//	  Dump_Regs();


						//TODO
						//TODO
						//TODO if global_index==0
						//it is loaded by Rank_K Update already
						if (Global_index==0)
						Scratch_Regs_Next[2]=Local_Mem.Reg_Read(Local_Mem_Address);
						// Bring the target Vectors into Scratch_Pad memory
						// you have the inv_Sqrts in yourslev so no need to BC_Inv_Sqrt


					break;

					case 1:  //BC_Inv_Sqrt

						Local_Mem_Address=-2; //

					break;

					case 2:  //Multiply

						// If My_Row=Iteration_counter perform Multiply
						// in the last cycle If My_Row==Iteration_counter then Send the result on the Column bus
						//and save it to local store too

						if (My_Row==Iter_Counter)
							if (Latency_Counter<(Multiplication_Latency-1))
								ALU.Execute_Mul(Scratch_Regs_Curr[2],Scratch_Regs_Curr[1]);

							else{

									Scratch_Regs_Next[2]=ALU.Execute_Mul(Scratch_Regs_Curr[2],Scratch_Regs_Curr[1]);

										Write_My_Col_Reg_Next=Scratch_Regs_Next[2];

								}

						if ((My_Column == Iter_Counter) && (My_Row> Iter_Counter))
							if (Latency_Counter==(Multiplication_Latency-1))
								Write_My_Row_Reg_Next=Scratch_Regs_Curr[0];



						//If Iter_counter==My_Column and My_Row >=Iter_counter then send the "cholesky column" to the row bus


					break;

					case 3:  //BC_Multiply


						if (My_Row==Iter_Counter)
							*Write_My_Col_Bus=Write_My_Col_Reg_Curr;

						if ((My_Column == Iter_Counter) && (My_Row> Iter_Counter))
							*Write_My_Row_Bus=Write_My_Row_Reg_Curr;



					break;

					case 4:  //Partial_Rank_1 //Assumption MAC delay is at least 2 clocks

						Local_Mem_Address=-5;

						//everything is in the the scratch_pad memory; or received from buses

						//Computation Part
						if (My_Row>Iter_Counter){

							if (Latency_Counter<(FMA_Latency-1)){
								ALU.Execute_MAD(*Read_My_Col_Bus, (-1)*(*Read_My_Row_Bus), Scratch_Regs_Curr[2]);

							}
							else {

								Scratch_Regs_Next[2]=ALU.Execute_MAD(*Read_My_Col_Bus, (-1.0)*(*Read_My_Row_Bus), Scratch_Regs_Curr[2]);

							}

						}

						// Communication

						if (Latency_Counter==0)
							if (My_Row==My_Column)
								*Write_My_Row_Bus=*Read_My_Col_Bus; // TODO I assumed I can read and write in 1+1 cycles (By_Passed the Write Register)

						if (Latency_Counter==1)
							if  (My_Column==Iter_Counter){
								//Local_Mem_Address= (Trsm_index)* (Kernel_Size/LAPU_Size)  +  Global_index;
								Local_Mem_Address=Global_index*(Kernel_Size / LAPU_Size)+Trsm_index ;

								Local_Mem.Reg_Write(Local_Mem_Address, *Read_My_Row_Bus);
							}


						// if 1st clock cycle:
						// start MAc operations
						//if I am a diagonal PE get the data from column_bus and put it on the row_bus

						//if 2nd clock cycle:
						// If my column==iteration_Counter then save the row bus into local memory



						//if any other time just do continue MAc operation;


					break;



					case 5: //Trans

						if (Latency_Counter==0)
							if (My_Row==My_Column)
								*Write_My_Row_Bus=*Read_My_Col_Bus; // TODO I assumed I can read and write in 1+1 cycles (By_Passed the Write Register)

						if (Latency_Counter==1)
							if  (My_Column==Iter_Counter){
								//Local_Mem_Address= (Trsm_index)* (Kernel_Size/LAPU_Size)  +  Global_index;
								Local_Mem_Address=Global_index*(Kernel_Size / LAPU_Size)+Trsm_index ;

								Local_Mem.Reg_Write(Local_Mem_Address, *Read_My_Row_Bus);
							}
						break;


					case 6:  //End



						//Local_Mem_Address=Global_index*(Kernel_Size / LAPU_Size)+Trsm_index ;
						Local_Mem_Address= (Trsm_index)* (Kernel_Size/LAPU_Size)  +  Global_index;


						Local_Mem.Reg_Write(Local_Mem_Address,Scratch_Regs_Curr[2]);


					break;


				}
				//Dump_Regs();

			break;
			case LAPU_Rank_Update:

				switch(LAPU_Current_State){

					case 0: // Initial it takes two cycles for loading the data from local store and putting it on column or row buses


							// load the target matrix into the accumulator

							//Local_Mem_Address=Global_index*(Kernel_Size/LAPU_Size)+Trsm_index;

							Local_Mem_Address=Trsm_index*(Kernel_Size/LAPU_Size)+Global_index;


							//Scratch_Regs_Next[0]=Local_Mem.Reg_Read(Local_Mem_Address);
							//loading accumulator
							ALU.Load_Accumulator(Local_Mem.Reg_Read(Local_Mem_Address));


					break;

					case 1: //Pre_Fetch


						// in first cycle  Prefetch PE(0,0)
						//  PE(0,0) read from memory the  column supposed to be BCasted

						if (Latency_Counter==0){

							if (  (My_Row==0) && (My_Column==My_Row) ){
									//Local_Mem_Address= ( (Iter_Counter+2)/ LAPU_Size) * (Kernel_Size/LAPU_Size)+Trsm_index;
								Local_Mem_Address= ( (Iter_Counter+2)/ LAPU_Size) * (Kernel_Size/LAPU_Size)+Global_index;

								Write_My_Row_Reg_Next=Local_Mem.Reg_Read(Local_Mem_Address);
								}
						}


						//in second Cycle
						//1: Pre_Fetch  PE(1,1)
						//2: Prepare to BC the Column 0 and Row 0  (Diagonal PE does the row fetch now)

						else{ // Latency_Counter==1


							// Prefetch PE(1,1)
							if ( (My_Row==1) && (My_Column==My_Row) ){
									//Local_Mem_Address= ((Iter_Counter+2)/ LAPU_Size) * (Kernel_Size/LAPU_Size)+Trsm_index;
								Local_Mem_Address= ((Iter_Counter+2)/ LAPU_Size) * (Kernel_Size/LAPU_Size)+Global_index;

									Write_My_Row_Reg_Next=Local_Mem.Reg_Read(Local_Mem_Address);
								}


							//Prepare to BC the 0th column and 0th row  (Diagonal PE does the row fetch now)
							if (My_Row==0){
								//Local_Mem_Address=(Global_index)*(Kernel_Size/LAPU_Size) + (Iter_Counter+1)/LAPU_Size;
								Local_Mem_Address=(Trsm_index)*(Kernel_Size/LAPU_Size) + (Iter_Counter+1)/LAPU_Size;


								Write_My_Col_Reg_Next=Local_Mem.Reg_Read(Local_Mem_Address);
							}
							else if (My_Column==0){
								//Local_Mem_Address=((Iter_Counter+1)/LAPU_Size) * Kernel_Size/LAPU_Size + Trsm_index;
								Local_Mem_Address=((Iter_Counter+1)/LAPU_Size) * Kernel_Size/LAPU_Size + Global_index;
								Write_My_Row_Reg_Next=Local_Mem.Reg_Read(Local_Mem_Address);
							}
						}


					break;

					case 2: // BC

						//Iteration_counter=0;

						//Iteration_Counter Starting to count from now on
						//-> BC is the base: we are Broad_Casting (Iteration_Counter)th Row and Column


						//1- Pre_Fetch PE(2,2)
						//2-Prepare to BC the Column 1 and Row 1  (Diagonal PE does the row fetch now)
						//3- Drive the Bus for Row 0 and  Column 0



						// Pre_Fetch (2,2)
						if ( (My_Row==(Iter_Counter+2)) && (My_Column==My_Row) ){
									//Local_Mem_Address= ((Iter_Counter+2)/ LAPU_Size) * (Kernel_Size/LAPU_Size)+Trsm_index;
							Local_Mem_Address= ((Iter_Counter+2)/ LAPU_Size) * (Kernel_Size/LAPU_Size)+Global_index;


									Write_My_Row_Reg_Next=Local_Mem.Reg_Read(Local_Mem_Address);
								}

						// Load the next BC into Bus Registers  Row 1 and Column 1 PEs
						if (My_Row==(Iter_Counter+1)){
							//Local_Mem_Address=(Global_index)*(Kernel_Size/LAPU_Size) + (Iter_Counter+1)/LAPU_Size;
							Local_Mem_Address=(Trsm_index)*(Kernel_Size/LAPU_Size) + (Iter_Counter+1)/LAPU_Size;
							Write_My_Col_Reg_Next=Local_Mem.Reg_Read(Local_Mem_Address);
						}
						else if (My_Column==(Iter_Counter+1)){
							//Local_Mem_Address=((Iter_Counter+1)/LAPU_Size) * Kernel_Size/LAPU_Size + Trsm_index;
							Local_Mem_Address=((Iter_Counter+1)/LAPU_Size) * Kernel_Size/LAPU_Size + Global_index;
							Write_My_Row_Reg_Next=Local_Mem.Reg_Read(Local_Mem_Address);
						}

						//Driving the bus  Row 0 and Column 0
						if (My_Column==0)
							*Write_My_Row_Bus=Write_My_Row_Reg_Curr;

						if (My_Row==0)
							*Write_My_Col_Bus=Write_My_Col_Reg_Curr;





					break;

					case 3: // MAC_and_BC

						//1-Pre_Fetch PE(Iteration_Counter+ 2,Iteration_Counter+ 2)
						//2-Prepare to BC the Column Iteration_Counter+1 and Iteration_Counter+1  (Diagonal PE does the row fetch now)
						//3-Drive the Bus for Row Iteration_Counter and  Column Iteration_Counter
						//4- perform MAC on Row Iteration_Counter-1 and  Column Iteration_Counter-1

						////Communication part *****


						if ( (Iter_Counter+2) < (Global_index* LAPU_Size) )

							if (( My_Row==((Iter_Counter+2)%LAPU_Size) ) && (My_Column==My_Row)){
										//Local_Mem_Address= ((Iter_Counter+2)/ LAPU_Size) * (Kernel_Size/LAPU_Size)+Trsm_index;
									Local_Mem_Address= ((Iter_Counter+2)/ LAPU_Size) * (Kernel_Size/LAPU_Size)+Global_index;

										Write_My_Row_Reg_Next=Local_Mem.Reg_Read(Local_Mem_Address);
									}




						// Load the next BC into Bus Registers
						if ( (Iter_Counter+1) < (Global_index* LAPU_Size) ){

							if (My_Row==((Iter_Counter+1)%LAPU_Size)) {
								//Local_Mem_Address=(Global_index)*(Kernel_Size/LAPU_Size) + (Iter_Counter+1)/LAPU_Size;
								Local_Mem_Address=(Trsm_index)*(Kernel_Size/LAPU_Size) + (Iter_Counter+1)/LAPU_Size;


								Write_My_Col_Reg_Next=Local_Mem.Reg_Read(Local_Mem_Address);

							}

							else if  (My_Column==((Iter_Counter+1)%LAPU_Size)){
								//Local_Mem_Address=((Iter_Counter+1)/LAPU_Size) * Kernel_Size/LAPU_Size + Trsm_index;
								Local_Mem_Address=((Iter_Counter+1)/LAPU_Size) * Kernel_Size/LAPU_Size + Global_index;

								Write_My_Row_Reg_Next=Local_Mem.Reg_Read(Local_Mem_Address);

							}

						}


						// Driving Bus
						if (My_Column==(Iter_Counter%LAPU_Size))
							*Write_My_Row_Bus=Write_My_Row_Reg_Curr;

						if (My_Row==(Iter_Counter%LAPU_Size))
							*Write_My_Col_Bus=Write_My_Col_Reg_Curr;



						//	   Computation
						ALU.Execute_MAC(*Read_My_Col_Bus, (-1)*(*Read_My_Row_Bus));
						 //else
							// Scratch_Regs_Next[0]=ALU.Execute_MAD(*Read_My_Col_Bus, (-1)*(*Read_My_Row_Bus), Scratch_Regs_Curr[0]);

					break;

					case 4: // MAC
						// Nothing left to do;

						if (Latency_Counter<(FMA_Latency-1) )
							ALU.Execute_MAC(*Read_My_Col_Bus, (-1)*(*Read_My_Row_Bus));


						else{

							if (Global_index==Trsm_index){
								Scratch_Regs_Next[0]=ALU.Execute_MAC(*Read_My_Col_Bus, (-1)*(*Read_My_Row_Bus));
							}
							else
								Scratch_Regs_Next[2]=ALU.Execute_MAC(*Read_My_Col_Bus, (-1)*(*Read_My_Row_Bus));

						}



					break;

					case 5: // End
						// Write the matrix back into the Local_Store
						if (Global_index==Trsm_index){
							//Local_Mem_Address=Global_index*(Kernel_Size/LAPU_Size)+Trsm_index;
							Local_Mem_Address=Trsm_index*(Kernel_Size/LAPU_Size)+Global_index;
							Local_Mem.Reg_Write(Local_Mem_Address, Scratch_Regs_Curr[0]);
						}
					break;


				}

				//Dump_Regs();
			break;



		}


	//if ((My_Row==My_Column) && (My_Row==0))
		//Dump_Regs();

	return Local_Mem_Address;



}


// Load A Row Major  |||||||||
// Load B column major
/*
 *   _
 *   _
 *   _
 *   _
 */

int PE::Fetch(int IOisReady,int Comm_State, bool Stall, int Kc, int init_current){

			/*if (Kc==0){

				Cin_Counter=-1;
				Bin_Counter=-2;
				Cout_Counter=0;
        Ain_Counter = 0;

        fetch_C = true;
        fetch_B = false;
        fetch_A = false;
        send_C = false;

        if (N==0 && Mc==0 &&  Ma!=0)  FcA = 0; //if we are to count new Ai,p+1

			}*/
  //Dont forget the above control logic



  switch(Comm_State){
  

    case 3 :

    //fetch C_in
    
    //first, send request to mem_controller
      //std::cout << " Hey Ya " << std::endl;
      if (IOisReady){
        //
        //once ready signal from mem_controller, then do the below behavior
        if (Cin_Counter>=0)
          if (My_Row== Cin_Counter){
              
              if(init_current==3 /*|| init_current==16*/) ALU.Load_Accumulator(*Read_My_Col_Bus);
            
              Scratch_Regs_Next[1]=*Read_My_Col_Bus;
#ifdef PRINT_DEBUG
              if(My_Row==LAPU_Size-1)
                std::cout<<"SAVING CIN"<<*Read_My_Col_Bus<<std::endl;
#endif
            //Read the bus to Regfile
       //     if (Cin_Counter==LAPU_Size-1) getchar();
          }
        Cin_Counter++;
        
        if (Cin_Counter == LAPU_Size) {
          /*fetch_C = false;
          fetch_A = true;     //next fetch_A
          std::cout<<"end of fetch_C "<<std::endl;*/
          Cin_Counter=0;
//          FetchDone=CIN;
        }
      }

  
    break;

    case 4 :  
    //Send C_out
          
      //std::cout << "Cout_Counter is " << Cout_Counter << std::endl;

      if (Cout_Counter<LAPU_Size){
        //std::cout<<"Sending Cout"<<std::endl;
        // IOisReady means that the buffer in the memory side is ready to consume data
        if (My_Row==Cout_Counter){
          *Write_My_Col_Bus=Write_My_Col_Reg_Curr; //drive the bus
          //*Write_My_Col_Bus=Scratch_Regs_Curr[2];
#ifdef PRINT_DEBUG
          std::cout<<"send_C now "<<std::endl;
          std::cout<<"My_Col_Bus  "<< *Write_My_Col_Bus << std::endl;
#endif
          //if(My_Row==3 && My_Column==3) getchar();  
        }
        Cout_Counter++;
      }
      
      if (Cout_Counter==LAPU_Size){
        Cout_Counter=0;
      }
    
      //send_C = (Cout_Counter==4)? false: true;           
      //fetch_B = (Cout_Counter ==4)? true : false;
    
    break;


    case 5:

    //fetch B

      if(IOisReady){  

        if (Bin_Counter< LAPU_Size){
#ifdef PRINT_DEBUG
        std::cout<<"Fetch_B now "<<std::endl;
#endif
        //old address generation
        /*Address_WB=(Kernel_Size*Kernel_Size)/(LAPU_Size*LAPU_Size)+ //start of B
            (  (N/LAPU_Size +1) % 2  )*Kernel_Size +Mc	; // block of 1st or second B panel*/
        if (Bin_Counter>=0){

          
          if (!BMode) Local_Mem.Reg_Write_New(Address_WB_Fetch,(*Read_My_Col_Bus), toB2);
          else Local_Mem.Reg_Write_New(Address_WB_Fetch,(*Read_My_Col_Bus), toB1);
#ifdef PRINT_DEBUG          
          std::cout<<"SAVING B "<<*Read_My_Col_Bus<<"at ["<<Address_WB_Fetch<<"]"<<std::endl;
#endif
          Address_WB_Fetch = Recurs_Gen_B(Address_WB_Fetch);
          
        }

        Bin_Counter++;
        }
        
        if (Bin_Counter==4) {
          
          Bin_Counter =0;
     //     getchar();
        
        //debug
        /*if (My_Row==LAPU_Size-1 && My_Column==LAPU_Size-1)  
          getchar();*/
        }
      }
  
  break;

  //fetch A
  
  case 6:
      //TODO create state machine for A
    
  if(IOisReady){
     if(!Stall){
      if(Ain_Counter>=0 && (Kc%LAPU_Size!=3)){
      
        if(My_Row == Ain_Counter){
         Local_Mem.Reg_Write_New(Address_WA_New,(*Read_My_Col_Bus), toA); 
        
          Address_WA_New = Recurs_Gen_A(Address_WA_New);
#ifdef PRINT_DEBUG
          std::cout<<"SAVING AIN now "<< *Read_My_Col_Bus <<std::endl;
          std::cout<<"Address_WA is  "<< Address_WA_New <<std::endl;
#endif
         //getchar();
      
       FcA = FcA +1;
      
        }
       
      Ain_Counter++; 
      //std::cout<<"Ain_Counter is  "<< Ain_Counter <<std::endl;
     }

    }
    else {
    
      if(Ain_Counter>=0){
       if(My_Row == Ain_Counter){
         Local_Mem.Reg_Write_New(Address_WA_New,(*Read_My_Col_Bus), toA); 
          Address_WA_New = Recurs_Gen_A(Address_WA_New);
#ifdef PRINT_DEBUG
        std::cout<<"SAVING AIN now "<< *Read_My_Col_Bus <<std::endl;

        std::cout<<"Address_WA is  "<< Address_WA_New <<std::endl;
#endif
        //if (Ain_Counter==LAPU_Size-1) getchar();
        /*std::cout<<"FcA is  "<< FcA <<std::endl;*/
       FcA = FcA +1;
      

        }
       
      Ain_Counter++; 
      //std::cout<<"Ain_Counter is  "<< Ain_Counter <<std::endl;
     }
    
    
    }

    
    if (Ain_Counter==4) {
    
      Ain_Counter = 0;
    
    }
  }
  
  break;

  }

}



int PE::Execute_Matmul (int Global_index, int N, int Mc, int Kc, int Ma, int Matmul_Current_State,int Latency_Counter, int BigA, int BigC){

switch(Matmul_Current_State){



  case 0: //Fetch C in RegFile and ACC

    if (My_Row==(Kc-1) ){ // also K>0 stands
      ALU.Load_Accumulator(*Read_My_Col_Bus);
      Scratch_Regs_Next[1]=*Read_My_Col_Bus;
    }

  break;

  case 1://Fetch first Panel B

    if (Kc==0)
    if (My_Row==(LAPU_Size-1)){
      ALU.Load_Accumulator(*Read_My_Col_Bus);
      Scratch_Regs_Next[1]=*Read_My_Col_Bus;
      Address_WB_New = 0;
    }
      // load the target matrix into the accumulator

			// KC should go from 0 to Kernel_Size + LAPU_Size (For blocks of C)
			if(Kc>=1){ // 1 here is the bus cycles
				
        // old part of address generation
        /*Address_WB=(Kernel_Size*Kernel_Size)/(LAPU_Size*LAPU_Size)+(Kc-1);
				Local_Mem.Reg_Write(Address_WB,(*Read_My_Col_Bus));*/
				
        //new address recursion
        //Address_WB_New = Kc-1;
        Local_Mem.Reg_Write_New(Address_WB_New,(*Read_My_Col_Bus), toB1);
        Address_WB_New = Recurs_Gen_B(Address_WB_New);
			}





		break;


		case 2://Fetch A


			// Bring the last row of B into SRAM of All PEs
			if ( (Kc==0)&& (Mc==0)){

        //old part of address generation

				/*Address_WB=(Kernel_Size*Kernel_Size)/(LAPU_Size*LAPU_Size)+(Kernel_Size-1);
				Local_Mem.Reg_Write(Address_WB,(*Read_My_Col_Bus));
				Address_WA=0;*/
			
        //new part Mochamad 
        Local_Mem.Reg_Write_New(Address_WB_New,(*Read_My_Col_Bus), toB1);
				
        Address_WA_New = 0; //0 technically
      
      }
				/// All should write it to SRAM

			// KC should go from 0 to Kernel_Size + LAPU_Size (For blocks of C)






				//if(!( (Kc==0)&& (Mc==0)))
			else{ // 1 here is the bus cycles
					//if (My_Column==0) std::cout<<"ROW"<<My_Row<<"mode"<<((Kc-1)%LAPU_Size)<<std::endl;


					if (Kc%Kernel_H==0){   // Mochamad --> this means in the end of Kernel

					if (My_Row==((Mc+LAPU_Size-1)%LAPU_Size)){
				
            //new part    Mochamad
            
            Local_Mem.Reg_Write_New(Address_WA_New,(*Read_My_Col_Bus), toA);
            
            //std::cout << "address WA rec is " << Recurs_Gen_A(Address_WA_New)<< std::endl;
						//Address_WA_New++;
            
            //std::cout << "address WA is " << Address_WA_New << std::endl;
            //getchar();
            
            Address_WA_New = Recurs_Gen_A(Address_WA_New);

            //Address_WA+=Kernel_Size/LAPU_Size;
						//if (Address_WA>((Kernel_Size/LAPU_Size)^2))
						//	Address_WA=(Mc-1)/LAPU_Size;
						//Address_WA=(Kc/LAPU_Size)*(Kernel_Size/LAPU_Size)+Mc/LAPU_Size;
					}
					}

					else if (My_Row==Mc%LAPU_Size){

						// old part of address generation
            /*Local_Mem.Reg_Write(Address_WA,(*Read_My_Col_Bus));
						Address_WA++;*/

						Local_Mem.Reg_Write_New(Address_WA_New,(*Read_My_Col_Bus), toA);
						//Address_WA_New++;
            //getchar();
            //std::cout << "address WA is " << Address_WA_New << std::endl;
            Address_WA_New = Recurs_Gen_A(Address_WA_New);

					}
			}




		break;




		case 3:// BC0

      // we dont need below anymore, right ?
      /*
			if (My_Row==(LAPU_Size-1)){

				//new part Mochamad 
        //Address_WA_New= (Kernel_Size*Kernel_Size)/(LAPU_Size*LAPU_Size)-1;
				Local_Mem.Reg_Write_New(Address_WA_New,(*Read_My_Col_Bus), toA);
        Address_WA_New = Recurs_Gen_A(Address_WA_New);
        std::cout<< "Address_WA_New is" << Address_WA_New << std::endl;
        //getchar();

				if (My_Column==0) std::cout <<"ADDRESS"<<Address_WA<<std::endl;
			}*/

			  //if ((My_Row==0)&& (My_Column==0)){

				  /*std::cout<<"Calling Dumper"<<std::endl;
				  //this->Dump_PE_Mem(Mem_Size_A*2);
          this->Dump_PE_Mem(Mem_Size_B1);
          getchar();
			  }*/

			//Address_A=0;

      Address_A_New = 0;

				//2: Prepare to BC the Column 0 and Row 0  (Diagonal PE does the row fetch now)
					//Prepare to BC the 0th column and 0th row  (Diagonal PE does the row fetch now)

			/*if (My_Column==0)*/ //Now I need to put every As of PEs in the row_reg_next, not only based on column
      
      
      {

        //new part Mochamad
        
				Write_My_Row_Reg_Next=Local_Mem.Reg_Read_New(Address_A_New, toA);
			  //Address_A_New++;
        Address_A_New = Recurs_Gen_A(Address_A_New);

			}


			//ALU.Load_Accumulator(0);
		break;

		case 4: // BC

			//Kc=0;

			//Kc Starting to count from now on
			//-> BC is the base: (Kc)th Column of A is on the Bus

			//Prepare to BC the 1st column
			//Drive the row Buses for column  0

			//Driving the bus Column 0 and all rows
			if (My_Column==0)
				*Write_My_Row_Bus=Write_My_Row_Reg_Curr;


			//Read B into Reg_File for iteration 0
			//Address_Reg_B_Next=Address_Reg_B_Curr +1;
			
      Address_B_New= 0;    
      Scratch_Regs_Next[0]=Local_Mem.Reg_Read_New(Address_B_New, toB1);
      
      //std::cout << "Scratch_Regs[0] at bc is " << Scratch_Regs_Next[0]<<std::endl;
      //getchar();

      ALU.Flush_Accumulator();

			//		if ((My_Row==0)&& (My_Column==0))
			//		std::cout<< "what I read"<<Scratch_Regs_Next[0]<<std::endl;
			
      /*
      if ((My_Row==0)&& (My_Column==0)){

				std::cout<<"Calling Dumper"<<std::endl;
				this->Dump_PE_Mem(4);
			}*/

		break;

		case 5: // MAC_and_BC


			// Scratch[0] for B
			//Scratch[1] for Cin
			//Scratch[2] for Cout


			//1-Prepare to BC the Column Iteration_Counter+1
			//2-Drive the Bus for Column Iteration_Counter
			//3-perform MAC on Row Iteration_Counter-1 and  Column Iteration_Counter-1
      

      //this is for dumping A
      
      if(Ma==0 && Kc==1 && Mc==0 && N==0 && ITER_COUNT==0 /*&& Ma!=0*/){
      //if(Ma==0 && Kc==Kernel_Size-1 && Mc==Kernel_Size-LAPU_Size && N==0/*&& Ma!=0*/){
      //if(Ma== (howmanyA/NumofCore)-1 && Kc==Kernel_Size-1 && Mc==Kernel_Size-LAPU_Size && N==Panel_Size-LAPU_Size/*&& Ma!=0*/){
			  
#if 0
        if ((My_Row==0)&& (My_Column==0)){

				  //std::cout<<"Calling Dumper"<<std::endl;
				  //this->Dump_PE_Mem(Mem_Size_A*2);
          this->Dump_PE_Mem(Mem_Size_B1);
      //    getchar();
			  }
#endif
      }

      

      if (((howmanyA/NumofCore)%2) && howmanyA!=NumofCore){
        if((HowManyPanel%2)!=0){
          if ( ((Ma+BigA+BigC)%2)==0 )
            AMode=false;
          else 
            AMode=true;
        }
        else{ 

          if ( ((Ma+BigA)%2)==0 )
            AMode=false;
          else 
            AMode=true;
        }
      }
      
      else if ( (((Ma%2)==0) && howmanyA!=NumofCore) || (Ma==0 && howmanyA==NumofCore && (BigA%2)==0)) AMode = false; //to decide the writeback pos, prefetch or demand
      else AMode = true;


      if (!AMode){
        
        //if (Address_A_New==64) Address_A_New=0;

         if(N!=0 && Mc==0 && Kc==0){ //repeat for every starting computation of N
          Address_A_New = 1; 
        }
      }


      else {
        
        //if (Address_A_New==0) Address_A_New=64;
        
       if(N!=0 && Mc==0 && Kc==0) //repeat for every starting computation of N
        Address_A_New = (Kernel_V * Kernel_H)/(LAPU_Size*LAPU_Size) + 1;
      }

      
      if (Kc%LAPU_Size==LAPU_Size-1){
				
        //if(AMode) Address_A_New= Address
        
        Write_My_Row_Reg_Next=Local_Mem.Reg_Read_New(Address_A_New, toA);
			  //Address_A_New++;
        //if (My_Row==1) std::cout << " Address_A is " << Address_A_New << std::endl;
        //getchar();

        
        /*if (My_Row ==0 && My_Column==0){
        std::cout <<"Address_A " << Address_A_New <<std::endl;
        std::cout << "Value " << Write_My_Row_Reg_Next<<std::endl;
        if (Ma==2) getchar(); 
        }*/
        
        Address_A_New = Recurs_Gen_A(Address_A_New);


        /*if(Kc==27 && Mc==28 && !My_Column && !My_Row) {

          std::cout<< "Address_A_New is " <<Address_A_New<<std::endl;
          getchar();
        }*/
        
        if ( (Kc== Kernel_H-LAPU_Size-1) && (Mc == Kernel_V-LAPU_Size)){

          if (N==Panel_Size-LAPU_Size /*&&  Address_A_New== (Kernel_Size * Kernel_Size)/(LAPU_Size*LAPU_Size)*/)
               Address_A_New = Address_A_New;
            
          else Address_A_New= (!AMode)? 0:(Kernel_V * Kernel_H)/(LAPU_Size*LAPU_Size);

        }

      }

			// Driving Bus
			if (My_Column==(Kc%LAPU_Size))
				*Write_My_Row_Bus=Write_My_Row_Reg_Curr;

      //For All read B to Register[0]
			
      // new part mochamad

      //Address_B_New = Kc;
      Address_B_New = Recurs_Gen_B(Address_B_New);

      //if in cblass call mode

      if(((Panel_Size/LAPU_Size)%2)==1){

        if(((howmanyA/NumofCore)%2)==1){        
          if( (((N/LAPU_Size) + Ma + BigC)%2)==0) BMode= false;
          else BMode = true;
        } 

        else {

          if( (((N/LAPU_Size) + Ma)%2)==0) BMode= false;
          else BMode = true;
        }
      }

      else {
        if (((N/LAPU_Size)%2)==0) BMode = false;
        else BMode = true;
      }

      if (!BMode){ 
			  Scratch_Regs_Next[0]=Local_Mem.Reg_Read_New(Address_B_New, toB1)	;
        //std::cout << "Scratch_Regs_Next[0] is " << Scratch_Regs_Next[0]<<std::endl;
        //getchar();
      }
      
      else 
			  Scratch_Regs_Next[0]=Local_Mem.Reg_Read_New(Address_B_New, toB2)	;



			//EXECUTION


			//TODO does it cover all?
			// reloading Accumulator
			// getting the old result
			// doing MAC
      
			//std::cout<<"MAC_IN_A"<<(*Read_My_Row_Bus)<<std::endl;
			//std::cout<<"MAC_IN_B"<<Scratch_Regs_Curr[0]<<std::endl;
      //getchar();
      
      
      
      
      /*if(Mc==0 && N==0 && Ma!=0){
			  if (My_Row==1 && My_Column==1){
				  std::cout<<"MAC_IN_A"<<(*Read_My_Row_Bus)<<std::endl;
				  std::cout<<"MAC_IN_B"<<Scratch_Regs_Curr[0]<<std::endl;
          std::cout<<"Adress_B"<<Address_B_New-1<<std::endl;
          //getchar();
			  }
      }*/

      double b;

			if ((Kc==FMA_Latency-1)&&(Mc!=0 ||  N!=0 || Ma!=0 || BigA!=0)){ // Latency is correct think the last one goes in when Kc=0
        //std::cout << " Kc, Mc, N are "<< Kc <<" " << Mc<< std::endl;
				//getchar();
        
        Scratch_Regs_Next[3]=ALU.Execute_MAC( Scratch_Regs_Curr[0], (*Read_My_Row_Bus));
				Write_My_Col_Reg_Next=Scratch_Regs_Next[3];
        
        /*if ((N==4 || N==0)  && Mc) {
          
          std::cout << "result is " << Write_My_Col_Reg_Next<<std::endl;
          std::cout << "Scratch_Regs_curr[0] is " << Scratch_Regs_Curr[0]<<std::endl;
          std::cout << "Read my row bus is " << *Read_My_Row_Bus<<std::endl;
          getchar();

        }*/
			}
			else if (Kc==0){ // Loading accumulator takes a cycle, and we start new panel with kc=1;
				//std::cout<<"LOADING ACC"<<Scratch_Regs_Curr[1]<<std::endl;
				if(!newC){
          //std::cout << "Multiply A and B ati Kc==0 is "<<std::endl;
          ALU.Execute_MAC( Scratch_Regs_Curr[0], (*Read_My_Row_Bus));
        }

        if(newC)
          newC=FALSE;

				//std::cout <<"LOADIN ACC WITH"<<Scratch_Regs_Curr[1]<<std::endl;
				ALU.Load_Accumulator(Scratch_Regs_Curr[1]);
        
        //if (My_Row>=3)
          //std::cout<<"Accumulator is " << Scratch_Regs_Curr[1] <<std::endl;
        
        //if (My_Row==0 && My_Column==0) getchar();

			}

			else
      { 
        double a =ALU.Execute_MAC( Scratch_Regs_Curr[0], (*Read_My_Row_Bus));
#ifdef PRINT_DEBUG
       /*if (My_Row==0 && My_Column==0)
        std::cout << "Multiply A and B is " << a <<std::endl;
        //if (My_Row==0 && My_Column==0) getchar();*/
#endif
       b=a;
      }
      
     // if(Mc==4 && Kc==18 && N==0 && Ma!=0){

#if 0      
        if (My_Row==3 && My_Column==0 &&MyCoreID==0 && ITER_COUNT==0){
				  std::cout<<"MAC_IN_A "<<(*Read_My_Row_Bus)<<std::endl;
				  std::cout<<"MAC_IN_B "<<Scratch_Regs_Curr[0]<<std::endl;
          std::cout<<"MAC_Res "<<b<<std::endl;
          std::cout<<"KC is "<< Kc<<std::endl;
          //std::cout<<"Adress_A "<<Address_A_New-1<<std::endl;
          
          //if(Mc==4 && Kc==18 && N==0 && Ma==0)
            //getchar();
			  }
#endif

#if 0

        if(Kc==1 && Mc==0 && BigA==2 && N==0){
       
          if(MyCoreID==0 && My_Row==0 && My_Column==0){
            Dump_PE_Mem(0);
            exit(0);
          }
        
        }
#endif

#ifdef PRINT_DEBUG
#endif
      //}

		break;

		case 6: // MAC_Flush
			// Nothing left to do;

			if (Latency_Counter<(FMA_Latency-1) )
				ALU.Execute_MAC(Scratch_Regs_Curr[0], (*Read_My_Row_Bus));


			else{

					Scratch_Regs_Next[2]=ALU.Execute_MAC(Scratch_Regs_Curr[0], (*Read_My_Row_Bus));
          
          

          //updated by Mochamad  Execution
           
					Scratch_Regs_Next[3]= Scratch_Regs_Next[2];
				  Write_My_Col_Reg_Next=Scratch_Regs_Next[3];
          Cout_Counter = -1;// set bus delay

          //Now 


			}



		break;

    case 7:

			if (Latency_Counter<FMA_Latency-1)
				ALU.Execute_MAC(Scratch_Regs_Curr[0], (*Read_My_Row_Bus));


			else if(Latency_Counter==FMA_Latency-1){

					Scratch_Regs_Next[2]=ALU.Execute_MAC(Scratch_Regs_Curr[0], (*Read_My_Row_Bus));          
          
          if(My_Row==LAPU_Size-1)
            std::cout<<"Last result " << Scratch_Regs_Next[2] << std::endl;

          //updated by Mochamad  Execution
           
					Scratch_Regs_Next[3]= Scratch_Regs_Next[2];
				  Write_My_Col_Reg_Next=Scratch_Regs_Next[3];
          ALU.Flush_Accumulator();
          newC=TRUE;

          //Now 
			}


    break;

		case 8: // End
		  		
      if (Cout_Counter<LAPU_Size){
        
			  if(Cout_Counter>=0){

          if (My_Row==Cout_Counter)
	  					*Write_My_Col_Bus=Write_My_Col_Reg_Curr; //drive the bus
		  				//*Write_My_Col_Bus=Scratch_Regs_Curr[2];
          }
			  Cout_Counter++;
      }

			std::cout<< "PE("<<My_Row<<","<<My_Column<<")="<<Scratch_Regs_Curr[2]<<std::endl  ;// Write the matrix back into the Local_Store

		break;

    case 9 : //idle;

    break;


	}
        
}


