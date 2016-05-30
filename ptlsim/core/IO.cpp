/*
 * IO.cpp
 *
 *  Created on: May 21, 2010
 *      Author: ardavan
 *///

#include "IO.h"
IO::IO()
{
	// TODO Auto-generated constructor stub
	data_transfer_total=0;
}

IO::~IO()
{
	// TODO Auto-generated destructor stub
}


IO::IO(double *& Row_Buses_Write,double *& Row_Buses_Read,double *& Column_Buses_Write,double *& Column_Buses_Read){


	Read_Col_Buses= Column_Buses_Read;
	Read_Row_Buses= Row_Buses_Read;
	Write_Col_Buses=Column_Buses_Write;
	Write_Row_Buses=Row_Buses_Write;

	Cin_Counter=0;
	Cout_Counter=-2;
	Bin_Counter=0;
  Ain_Counter = 0;

  last_x =0;
  last_y =0;

  fetch_C = true;
  fetch_B = false;
  fetch_A = false;
  send_C = false;

  FcA = 0;

  MyCoreID;

}

int IO::Assign_input_Matrix( double **& matrix_A, double **& matrix_B, double **& matrix_C){

	Matrix_A=matrix_A;
	Matrix_B=matrix_B;
	Matrix_C=matrix_C;


}

int IO::PassMyCoreID(int ID){

  MyCoreID = ID;
}



int IO::IO_Execute_Matmul (int Global_index, int N, int Mc, int Kc, int Ma, int  Matmul_Current_State, int Latency_Counter_Curr){

	int i,j,k;

	switch( Matmul_Current_State){

		case 0: //Init: fetch C in Regfile


			for (i=0;i<LAPU_Size;i++) // write on all buses the rows in steps
				Write_Col_Buses[i]=Matrix_C[Kc][i];

      //moch : New for multicore
			for (i=0;i<LAPU_Size;i++) {// write on all buses the rows in steps
				Write_Col_Buses[i]=Matrix_C[Kc+ MyCoreID*Kernel_Size][i];
      }

      A_per_Core = (howmanyA/NumofCore);
      Residue = (howmanyA%NumofCore);
        
      for (k=0; k<Residue; k++){
        if (MyCoreID==k) A_per_Core++; 
      }

		break;


		case 1: // Fetch B

			for (i=0;i<LAPU_Size;i++)
				Write_Col_Buses[i]=Matrix_B[Kc][i];


		break;

		case 2: // Fetch A

			/*for (i=0;i<LAPU_Size;i++)
				//Write_Col_Buses[i]=Matrix_A[Mc+Kc%LAPU_Size][(Kc/LAPU_Size)*LAPU_Size+i];
				Write_Col_Buses[i]=Matrix_A[Mc][(Kc/LAPU_Size)*LAPU_Size+i];*/
			
      for (i=0;i<LAPU_Size;i++)
				//Write_Col_Buses[i]=Matrix_A[Mc+Kc%LAPU_Size][(Kc/LAPU_Size)*LAPU_Size+i];
				Write_Col_Buses[i]=Matrix_A[Mc + MyCoreID*Kernel_Size][(Kc/LAPU_Size)*LAPU_Size+i];

			//std::cout <<"on the bus"<<Matrix_A[Mc+Kc%LAPU_Size][(Kc/LAPU_Size)*LAPU_Size+i-1]<<std::endl;
		break;


		case 3:

		break;

		case 4:

		break;
		case 5:

			/*if ((Kc>=0) && (Kc< (Kernel_Size/3))){
				//reset all counters
				Cin_Counter=0;
				Cout_Counter=-2; // -2*(delay of bus);
				Bin_Counter=0;


					// read Cin instantly (just now)
					for (i=0;i<LAPU_Size;i++)
						for (j=0;j<LAPU_Size;j++){

							Buffer_Cin[i][j]=Matrix_C[((Mc+LAPU_Size)%Kernel_Size)+i]
													 [(N+ ((Mc+LAPU_Size)/Kernel_Size)*LAPU_Size)% Panel_Size+j]; //TODO;

					}
				Done=0;
			}*/

      if (Kc==0 || (Kc==1 && Mc==0 && N==0 && Ma==0)){
				//reset all counters
				Cin_Counter=0;
				Cout_Counter=-2; // -2*(delay of bus);
				Bin_Counter=0;
        Ain_Counter = 0;

        fetch_C = true;
        fetch_B = false;
        fetch_A = false;
        send_C = false;

        if (N==0 && Mc==0 && Ma!=0) FcA = 0;


        /*if (Ma && !Kc && !Mc && !N){
        
          for (i=0; i<Kernel_Size;i++){
            for (j=0; j<Kernel_Size;j++){
            Matrix_C[i][j] = 0;
            
            }
          }
          //getchar();
        }*/




					// read Cin instantly (just now)
					for (i=0;i<LAPU_Size;i++)
						for (j=0;j<LAPU_Size;j++){
              if(N==Panel_Size-LAPU_Size && Mc==Kernel_Size-LAPU_Size){

                if (Ma!= A_per_Core-1)
							  Buffer_Cin[i][j]=Matrix_C[((Mc+LAPU_Size)%Kernel_Size)+i + MyCoreID*Kernel_Size 
                                 + (Ma+1)*NumofCore*Kernel_Size]
													      [(N+ ((Mc+LAPU_Size)/Kernel_Size)*LAPU_Size)% Panel_Size+j]; //TODO;
              
                }

              else{   //general case

							  Buffer_Cin[i][j]=Matrix_C[((Mc+LAPU_Size)%Kernel_Size)+i + MyCoreID*Kernel_Size 
                                 + Ma*NumofCore*Kernel_Size]
													 [(N+ ((Mc+LAPU_Size)/Kernel_Size)*LAPU_Size)% Panel_Size+j]; //TODO;
              //A
              //if (Ma==1) getchar();
              }

					}

				
         /* for (i=0;i<LAPU_Size; i++)
						Write_Col_Buses[i]=Buffer_Cin[Cin_Counter][i];
        
        Cin_Counter++;*/

				Done=0;
			}

			if (fetch_C ){


				//if ( (Kc== ceil(Kernel_Size/3)) && ((Kc%3)==0) ){
				//std::cout<<"DONE"<<Done<<std::endl;
				if (Done==0){
					Done++;
					// read A instantly in the beginning of the second period
				  
         /*if(FcA!= (Kernel_Size * Kernel_Size)/(LAPU_Size*LAPU_Size))*/{  
          for (i=0;i<LAPU_Size;i++)
						for (j=0;j<LAPU_Size;j++){

              if (N<Kernel_Size){
							Buffer_A[i][j]=Matrix_A[((Ma+1)%A_per_Core)*NumofCore*Kernel_Size + MyCoreID*Kernel_Size + N + i][Mc+j]; //TODO;
              std::cout << "Buffer A is " << Buffer_A[i][j] << std::endl;
              //getchar();
              }
						}
          //FcA++;

          }

				}

				//Send the Cin on the buses cycle by cycle;
				if (Cin_Counter< LAPU_Size){

					std::cout<<"Sending CIN "<<std::endl;
					for (i=0;i<LAPU_Size; i++){
						Write_Col_Buses[i]=Buffer_Cin[Cin_Counter][i];
					  std::cout<<Write_Col_Buses[i]<<std::endl;
          }
					Cin_Counter++;
				}

        if (Cin_Counter==LAPU_Size) {
          fetch_A = true;
          fetch_C = false;
          Done = 2;
        }
      }

			else if (fetch_A){      //fetching A
          
        std::cout << "Sending AIN " <<std::endl;

        if (Ain_Counter<LAPU_Size && ((Kc%LAPU_Size)!=2)){
					//std::cout <<"Cout_Counter"<<Cout_Counter<<std::endl;
				 if(FcA!= (Kernel_Size*Kernel_Size)/(LAPU_Size * LAPU_Size)){
          if (Ain_Counter>=0){
					  for (i=0;i<LAPU_Size; i++){
						  Write_Col_Buses[i]=Buffer_A[Ain_Counter][i];
              std::cout <<"Ain from IO is"<< Write_Col_Buses[i] << std::endl;
            }
					  Ain_Counter++;
            std::cout<<"fetch_A now "<<std::endl;
            std::cout<<"FcA_IO now is " << FcA <<std::endl;
            //getchar();
				  }

          else FcA = FcA + 1; //increment FcA after all As have been saved in all PEs
         }

         else Ain_Counter++;
          
         if(Ain_Counter==LAPU_Size){
         
            send_C = true;
            Ain_Counter = 0;
            fetch_A = false;
         }

        }
			}


			else if(send_C) { // if in the third period

        if(Done==2){
          Done =0;
          
					for (i=0;i<LAPU_Size;i++)
						for (j=0;j<LAPU_Size;j++){
              Buffer_B[i][j]=Matrix_B[Mc+i][(N+LAPU_Size)% Panel_Size+j];
              //B_Latency[i][j] = rand()%LATENCY_RANGE + 1
              //if B_Latency[i][j]==0 {
              //B_ready[i][j] = 1;
              //Buffer_B[i][j] = Matrix_B[address][address] 
              //if all ready, send signal to PE indicating this is ready, and what data is this (A, B, or C)
              //
              //      }
          } 
        }

        if(Cout_Counter<LAPU_Size){

          if(Cout_Counter>=0){
            std::cout << "Saving COUT " <<std::endl;
            for(i=0; i<LAPU_Size; i++)
              Buffer_Cout[Cout_Counter][i] = Read_Col_Buses[i];
          }

          Cout_Counter++;

        }

       if(Cout_Counter==LAPU_Size) {
          fetch_B = true;
          send_C = false;
          Done = 3;
        }
      }


      else if(fetch_B){
      
        if(Done==3){
        
          Done = 0;
          
          for (i=0; i<LAPU_Size;i++)
            for (j=0; j<LAPU_Size; j++){

            if ((Mc==0) && (N!=0) && ((N%Panel_Size)!=0) /*&& (N==Panel_Size-LAPU_Size)*/){
              Matrix_C[((Kernel_Size-LAPU_Size)%Kernel_Size)+i + MyCoreID*Kernel_Size + Ma*NumofCore*Kernel_Size]
              [(N-LAPU_Size)+j] =Buffer_Cout[i][j];
              
              if ((((Kernel_Size-LAPU_Size)%Kernel_Size)+i)==(N-LAPU_Size)+j){
              std::cout << "Mc 0 but N is not, Buffer_Cout[i][j] is " <<  Buffer_Cout[i][j] << std::endl;
              //getchar();
              }
            }
            else if ( Ma!=0 && Mc==0 && N==0){ 
              Matrix_C[((Kernel_Size-LAPU_Size)%Kernel_Size)+i + (Ma-1)*NumofCore*Kernel_Size + MyCoreID*Kernel_Size]
              [((Panel_Size-LAPU_Size)%Panel_Size)+j] =Buffer_Cout[i][j]; 
              //chuui

              if ((((Kernel_Size-LAPU_Size)%Kernel_Size)+i)==((Kernel_Size-LAPU_Size)%Kernel_Size)+j){
              std::cout << "last 4X4 Buffer_Cout[i][j] is j " << Buffer_Cout[i][j] << std::endl;
              //getchar();
              }
            
            }
            
            else if (Mc>0){
              Matrix_C[Mc-LAPU_Size+i + Ma*NumofCore*Kernel_Size + MyCoreID*Kernel_Size][N+j] =Buffer_Cout[i][j];
              /*if((Mc-LAPU_Size+i)==N+j){ 
              std::cout << "Buffer_Cout[i][j] is j " << Buffer_Cout[i][j] << std::endl;
              //B
              //getchar();
              }*/
              /*if (N==4 && Mc){ 
                std::cout << "Buffer_Cout[i][j] is j " << Buffer_Cout[i][j] << std::endl;
                getchar();
              }*/
            }

            else if (Mc==0 && (N%Kernel_Size)==0 && N!=0){
            Matrix_C[Kernel_Size-LAPU_Size + i + MyCoreID*Kernel_Size + Ma*NumofCore*Kernel_Size][N-LAPU_Size+j] = Buffer_Cout[i][j];

            std::cout << "Mc 0 but N is not, Buffer_Cout[i][j] is " <<  Buffer_Cout[i][j] << std::endl;
              //getchar();


            
            }
          }

        }


				if (Bin_Counter< LAPU_Size){
					//std::cout << "GGGGGGGGGGGGGGG"<<std::endl;

					std::cout<<"Sending B"<<std::endl;
					for (i=0;i<LAPU_Size; i++)
						Write_Col_Buses[i]=Buffer_B[Bin_Counter][i];

					Bin_Counter++;
				}

        if (Bin_Counter ==4){
          fetch_C = false;
          fetch_B = false;
          fetch_A = false;
          send_C = false;
        }

			}
		break;

		case 6: // Mac_Flush
        
      Cout_Counter = -2;
      last_x = Kernel_Size - LAPU_Size;
      last_y = Panel_Size - LAPU_Size;

    break;

    case 7: //The end

    if (Cout_Counter<LAPU_Size){

      if (Cout_Counter>=0){

	  	  for (i=0;i<LAPU_Size;i++)
		  		Matrix_C[last_x+ MyCoreID*Kernel_Size + (A_per_Core-1)*NumofCore*Kernel_Size][last_y+i] = Read_Col_Buses[i];
   
        std::cout<< "last_x " << last_x <<std::endl;
        std::cout<< "Ma and Kernelsize " << Ma  << Kernel_Size <<std::endl;
        getchar();

        last_x++;
      }

      Cout_Counter++;

    }

					/*for (i=0;i<LAPU_Size;i++)
							for (j=0;j<LAPU_Size;j++){

								// if we  have to write to previous column and we are not the 0th column
								if ((Mc==0)&&(N!=0))
									Matrix_C[((Kernel_Size-LAPU_Size)%Kernel_Size)+i]
											 [(N-LAPU_Size)+j] =Buffer_Cout[i][j];


								else if (Mc>0)
									Matrix_C[Mc-LAPU_Size+i]
											[N+j] =Buffer_Cout[i][j];


				      }*/

    break;

    

	}





}
