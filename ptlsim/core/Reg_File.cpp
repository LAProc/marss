/*
 * Reg_File.cpp
 *
 *  Created on: Mar 6, 2010
 *      Author: ardavan
 */


#include "Reg_File.h"

Reg_File::Reg_File()
{
	// TODO Auto-generated constructor stub
	Size=Mem_Size;  // is this dummy code ?

#if 0
	Registers= (double *) malloc ( sizeof(double)* (Mem_Size));
//	std::cout<< "end of  Reg_file const"<<std::endl;
	Registers_A= (double *) malloc ( sizeof(double)* (Mem_Size_A*NumofA));
	Registers_B1= (double *) malloc ( sizeof(double)* (Mem_Size_B1));
	Registers_B2= (double *) malloc ( sizeof(double)* (Mem_Size_B2));
#endif

	Registers= new double [Mem_Size];
	Registers_A= new double [(Mem_Size_A*NumofA)];
	Registers_B1= new double [(Mem_Size_B1)];
	Registers_B2= new double[(Mem_Size_B2)];
}

Reg_File::~Reg_File()
{
	// TODO Auto-generated destructor stub


}


double Reg_File::Reg_Read(int address){

	//std::cout<<"*********************"<<std::endl;
	return Registers[address];

	// TODO
}

double Reg_File::Reg_Read_New(int address, char mem){

  if (mem=='A'){
  	return Registers_A[address];
  }

  if (mem=='B'){
  	return Registers_B1[address];
  }

  if (mem=='P'){
  	return Registers_B2[address];
  }
	// TODO
}

int Reg_File::Reg_Write_New(int address, double data, char mem){


  if (mem=='A'){
    Registers_A[address]=data;
  }

  if (mem=='B'){
    Registers_B1[address]=data;
  }
  
  if (mem=='P'){
    Registers_B2[address]=data;
  }
}

int Reg_File::Reg_Write(int address, double data){

	//std::cout<<"*********************"<<std::endl;
	//std::cout<<"writing "<<data<<"in "<<address<<std::endl;
	Registers[address]=data;

}

/*
int Reg_File::Cycle(){


	int i;
	for (i=0; i < Size; i++)
		Registers_Curr[i]=Registers_Next[i];

}
*/
int Reg_File::Initialize_Register_File (int row, int column, double ** A, int row_number, int column_number, int offset){

	// Storing the matrix in the Register file column order

	 int i, j;
	 int l=offset;
	// instead of using the constructor I intialize my_row and my_column here.
	my_row=row;
	my_column=column;



	for (j= my_column; j< column_number; j=j+LAPU_Size)
		for (i=my_row; i< row_number; i=i+LAPU_Size){
			Registers[l]=A[i][j];
			l++;
			if (l>Mem_Size) std::cout << "Erorr: Memory overload"<<std::endl;
		}



}

int Reg_File::Initialize_Register_File_New (int row, int column, double ** A, int row_number, int column_number, int offset, char matr){

	// Storing the matrix in the Register file column order

	 int i, j;
	 int l=offset;
	// instead of using the constructor I intialize my_row and my_column here.
	my_row=row;
	my_column=column;

  if(matr=='A'){

  	for (j= my_column; j< column_number; j=j+LAPU_Size)
	  	for (i=my_row; i< row_number; i=i+LAPU_Size){
		  	Registers_A[l]=A[i][j];
			  l++;
			  if (l>Mem_Size_A) std::cout << "Erorr: Memory overload"<<std::endl;
		  }

    }
  
  if(matr=='B'){

  	for (j= my_column; j< column_number; j=j+LAPU_Size)
	  	for (i=my_row; i< row_number; i=i+LAPU_Size){
		  	Registers_B1[l]=A[i][j];
			  l++;
			  if (l>Mem_Size_B1) std::cout << "Erorr: Memory overload"<<std::endl;
		  }

    }
  
  if(matr=='P'){

  	for (j= my_column; j< column_number; j=j+LAPU_Size)
	  	for (i=my_row; i< row_number; i=i+LAPU_Size){
		  	Registers_B2[l]=A[i][j];
			  l++;
			  if (l>Mem_Size_B2) std::cout << "Erorr: Memory overload"<<std::endl;
		  }

    }

}

int Reg_File::Flush_Register_File ( double **& B, int row_number, int column_number,int offset){

	// Storing the matrix in the Register file column order

	 int i, j;
	 int l=offset;

	// std::cout<< "flushing register file"<<std::endl;
	// std::cout<<"l="<<l<<std::endl;
	 for (i=my_row; i< row_number; i=i+LAPU_Size)
	for (j= my_column; j< column_number; j=j+LAPU_Size)
		{
			//std::cout<<"l="<<l<<std::endl;
			//std::cout<<Registers[++l]<<std::endl;
			B[i][j]=Registers[l] ;
			l++;
			if (l>Mem_Size) std::cout << "Error: Memory Underload"<<std::endl;
		}

}

int Reg_File::Flush_Register_File_New ( double **& B, int row_number, int column_number,int offset, char matr){

	// Storing the matrix in the Register file column order

	 int i, j;
	 int l=offset;

	// std::cout<< "flushing register file"<<std::endl;
	// std::cout<<"l="<<l<<std::endl;
	
   if(matr=='A'){
   
   for (i=my_row; i< row_number; i=i+LAPU_Size)
  	for (j= my_column; j< column_number; j=j+LAPU_Size)
		{
			//std::cout<<"l="<<l<<std::endl;
			//std::cout<<Registers[++l]<<std::endl;
			B[i][j]=Registers_A[l] ;
			l++;
			if (l>Mem_Size_A*howmanyA) std::cout << "Error: Memory Underload"<<std::endl;
		}
   }
   
   if(matr=='B'){
   
   for (i=my_row; i< row_number; i=i+LAPU_Size)
  	for (j= my_column; j< column_number; j=j+LAPU_Size)
		{
			//std::cout<<"l="<<l<<std::endl;
			//std::cout<<Registers[++l]<<std::endl;
			B[i][j]=Registers_B1[l] ;
			l++;
			if (l>Mem_Size_B1) std::cout << "Error: Memory Underload"<<std::endl;
		}
   }

   if(matr=='P'){
   
   for (i=my_row; i< row_number; i=i+LAPU_Size)
  	for (j= my_column; j< column_number; j=j+LAPU_Size)
		{
			//std::cout<<"l="<<l<<std::endl;
			//std::cout<<Registers[++l]<<std::endl;
			B[i][j]=Registers_B2[l] ;
			l++;
			if (l>Mem_Size_B2) std::cout << "Error: Memory Underload"<<std::endl;
		}
   }

  
}
