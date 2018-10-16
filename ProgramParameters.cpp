#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "ProgramParameters.h"
#include <string.h>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#ifndef __SUPER_TRANSLATOR_INT2STRING__
#define __SUPER_TRANSLATOR_INT2STRING__
#include <sstream>
#include <string>

std::string int2str(int i)
{
	std::ostringstream conv;
	conv << i;
	return conv.str();
}
#endif

ProgramParameters::ProgramParameters(int argc,char** argv)
{
	int i,separator;
	std::string param,temp,option,paramName,paramValue;
	this->debug   = false;
	this->mask    = 0;
	this->mask2   = 0;
	this->mask3   = 0;
	this->samples = 0;
	this->verbose = true;
	this->file_name_format = NULL;
	this->controllerDevice = new char[13];
	strcpy(this->controllerDevice, "/dev/ttyUSB0");
	this->logfilename = new char[27];
	strcpy(this->logfilename,"/home/shashlik/Documents/Log/Logfile.log");
	this->Iped = 250;

	this->OpenLogFile();
	for(i=1;i<argc;i++)
	{
		if(argv[i][0]=='-' && argv[i][1]=='-' && argv[i][2]!='\0')
		{
			param.assign(argv[i]);
			separator  = param.find_last_of("="); 
			paramName  = param.substr(2,separator-2);
			paramValue = param.substr(1+separator);
			
			if(paramName=="debug")
			{
				this->debug = atoi(paramValue.c_str());
			}
			else if(paramName=="mask")
			{
				if(paramValue.length()<=8){
					this->mask = 0;
					this->mask2 = 0;
					this->mask3 = strtoul(paramValue.c_str(),NULL,16);
				}else if(paramValue.length()<=16){
					this->mask = 0;
					this->mask2 = strtoul(paramValue.substr(0,8).c_str(),NULL,16);
					this->mask3 = strtoul(paramValue.substr(8,paramValue.length()-8).c_str(),NULL,16);
				}else{
					this->mask = strtoul(paramValue.substr(0,8).c_str(),NULL,16);
					this->mask2 = strtoul(paramValue.substr(8,8).c_str(),NULL,16);
					this->mask3 = strtoul(paramValue.substr(16,paramValue.length()-16).c_str(),NULL,16);
				}
			}
			else if(paramName=="samples"){
				this->samples = atoi(paramValue.c_str());
			}
			else if(paramName=="file-format"){
				this->file_name_format = (char*) memchr(argv[i],'=',strlen(argv[i]));
				this->file_name_format++;
				this->CheckFilePath();
			}
			else if(paramName=="controller-device"){
				delete[] this->controllerDevice;
				this->controllerDevice = new char[paramValue.length()+1];
				strcpy(this->controllerDevice,paramValue.c_str());
			}
			else if(paramName=="Iped"){
				this->Iped = atoi(paramValue.c_str());
			}
			else if(paramName=="help"){
				this->Help(argv[0]);
			}
			else{
				this->Log((char*)std::string("Unrecognized option "+paramName).c_str());
				//std::cout << "Unrecognized option\t\t: " << paramName << std::endl;
				exit(EXIT_FAILURE);
			}
		}
		else if(argv[i][0]=='-' && argv[i][1]!='-'){
			if(argv[i][1]=='\0')
				continue;
			if(argv[i][1]=='q'){
				this->verbose = false;
			}else if(argv[i][1]=='v'){
				this->verbose = true;
			}
		}
	}
	this->Log("Options loaded");
	if(this->verbose){
		this->Presentation();
		std::cout << std::endl;
		
	}
	if(this->checkImportantParameters()){
		this->usage(argv[0]);
		exit(5);
	}
}

void ProgramParameters::Presentation(){
	std::cout << "------------------------------------------------------------------------------" << std::endl;
	/*std::cout << "\t\tDataAcquisitionSystem v2.0" << std::endl;
	std::cout << "Contact     :" << std::endl;
	std::cout << "QDC System  : Pedro Toledo <pedrotoledocorrea@gmail.com>" << std::endl;
	std::cout << "Power Supply: Orlando Soto <orlando.soto@usm.cl>" << std::endl;
	std::cout << "Manager     : Alam Toro S. <alam.toro@usm.cl>" << std::endl;
	std::cout << "------------------------------------------------------------------------------" << std::endl;*/
	
}

int ProgramParameters::checkImportantParameters(){
	if(this->debug < 0){
		return 1;
	}
	if(this->mask3 > 0 || this->mask2 > 0 || this->mask > 0){
		if(this->mask > 0xFFFFFFFF || this->mask2 > 0xFFFFFFFF || this->mask3 > 0xFFFFFFFF || (this->mask == 0 && this->mask2 == 0 && this->mask3 == 0)){
			std::cerr << "\e[0;31mERROR:\e[0m Each part of the Mask must be lower than 0xFFFFFFFF" << std::endl;
			return EXIT_FAILURE;
		}
		if(this->verbose)
			std::cout << "Mask set to\t\t\t: " << std::hex << this->mask << " " << this->mask2 << " " << this->mask3 << std::dec << std::endl;
	}else{
		std::cerr << "\e[0;31mERROR:\e[0m Mask must be a value over 0" << std::endl;
		return EXIT_FAILURE;
	}
	if(this->samples<1){
		std::cerr << "\e[0;31mERROR:\e[0m Samples must be setted" << std::endl;
		return EXIT_FAILURE;
	}else{
		if(this->verbose)
			std::cout << "Samples set to\t\t\t: " << this->samples << std::endl;
	}
	if(this->file_name_format == NULL){
		std::cerr << "\e[0;31mERROR:\e[0m A name format is needed" << std::endl;
		return EXIT_FAILURE;
	}else{
		if(this->verbose)
			std::cout << "File name format set to\t\t: " << this->file_name_format << ".dat" << std::endl;
	}
	if(strcmp(this->controllerDevice,"") == 0){
		std::cerr << "\e[0;31mERROR:\e[0m A device for the controller must be specifyed" << std::endl;
		return EXIT_FAILURE;
	}else{
		if(this->verbose)
			std::cout << "Controller device set to\t: " << this->controllerDevice << std::endl;
	}
	return 0;
}

void ProgramParameters::usage(char* argv){
	std::cout << std::endl << "Format is: " << std::endl;
	std::cout << "\t" << argv << "\t--mask=<Mask MPPC:32-1> --samples=<Samples> " << std::endl;
	std::cout << "\t\t--file-format=</path/Filename> [--debug=<Debug Level>]" << std::endl << std::endl;
	std::cout << "Example:" << std::endl;
	std::cout << "\t" << argv << "\t--mask=FFFF0000 --samples=1000 --file-format=/home/example" << std::endl << std::endl;
	std::cout << "This will acquire 1000 samples from MPPC position 32-17" << std::endl;
	std::cout << ", store them on a set of data files which names begin with " << std::endl;
	std::cout << "the word example." << std::endl;
}

void ProgramParameters::Help(char* argv){
	this->usage(argv);
	std::cout << std::endl << "Also available options are:" << std::endl;
	std::cout << "--controller-device:\tSpecify the device that represent the Controller" << std::endl;
	std::cout << "\t\t\tin the Bias Voltage Power Supply" << std::endl;
	exit(0);
}

ProgramParameters::~ProgramParameters(){
	this->CloseLogFile();
	delete[] this->logfilename;
	delete[] this->controllerDevice;
}

bool ProgramParameters::Debug(int level){
	return this->debug&(((unsigned long int)0x1)<<level);
}

bool ProgramParameters::Mask(int i){
	if(i<0 || i>95){
		std::cerr << "\e[0;31mERROR:\e[0m ProgramParamedters::Mask: Mask parameter must be between 0 and 31" << std::endl;
		exit(1);
	}
	unsigned long int temp;
	if(i<32){
		temp = (((unsigned long int)0x1)<<i);
		return this->mask3&temp;
	}else if(i<64){
		temp = (((unsigned long int)0x1)<<(i-32));
		return this->mask2&temp;
	}else{
		temp = (((unsigned long int)0x1)<<(i-64));
		return this->mask&temp;
	}
}

int ProgramParameters::GetSamples(){
	return this->samples;
}

char* ProgramParameters::GetFName(){
	return this->file_name_format;
}

void ProgramParameters::SetFName(char* fname){
	this->file_name_format = fname;
}

void ProgramParameters::SaveData(int *Data, int *Hits, unsigned long int *GateCounter, int board , std::ofstream & file,int samples){
        int InHeader = 0;
        int i=0,j=0;
	int Channel = 0;
	int GoodCH = 0;
	int OV;
	bool saveData = false;
	std::string buffer;

	//Data Kinds
	int DataKind;
	int Header = 2;
	int Datum  = 0;
	int EOB	   = 4;
	int NVD	   = 6;
	
        while(i<4096){
                //Cheking for the information Header

		// This case show the end of the information sended by QDC board
		// Look on CAENVMEenvironment.cpp for further information
		if(Data[i]==-1){
			//std::cout << "Data[i] = -1" << std::endl;		
			return;
		}

		DataKind = (Data[i]&0x7000000)>>24;
		//std::cout <<"i = " << i << "\tDataKind = " << DataKind << std::endl;

                if(InHeader==0 && DataKind==Header){
                        //Header read and expected
			GoodCH = (Data[i]&0x3f00)>>8;
			InHeader=1;
			i += GoodCH+1;	//Move to the EOB Word
			std::cout << "::Good Channels: " << GoodCH <<"::" << std::endl;
			continue;
		}else if(InHeader==1 && DataKind==Header){
                        std::cout << "::Unexpected Header::" << std::endl;
			return;
		}else if(InHeader==1 && DataKind==EOB){
                        std::cout << "::End of Block Reached::" << std::endl;
                        InHeader = 0;
			*GateCounter = Data[i]&0xFFFFFF;
			i++;
			if(true){//GoodCH==32){ 
				saveData = true; 
				std::cout<< "::saveData true::" << std::endl;
			}/*else{
				bads[k] = *GateCounter;
				k++;
			}*/                   
                }else if(InHeader==0 && DataKind==EOB){
			std::cout << "::Unexpected EOB::" << std::endl;
			return;
		}else if(DataKind==NVD){
			std::cout << "::Not Valid Datum Reached::" << std::endl;
			return;
		}else if((DataKind!=Header) and
		  	 (DataKind!=Datum ) and 
			 (DataKind!=EOB   ) and
			 (DataKind!=NVD   )){
			std::cout << "::Reserved Word::" << std::endl;
			i++;
			continue;
		}

                //When Im inside a valid information section
                if(saveData){
			
			j=i-1;
			do{
				j--;
				DataKind = (Data[j]&0x7000000)>>24;
	                        if(DataKind!=Datum){
					std::cerr << "ERROR: Something is wrong with data: no valid data detected" << std::endl; 
					return;
        	                }

				Channel = 32*board + ((Data[j]&0x1F0000)>>16);
				OV = (Data[j]&0x1000)>>12;

				if(OV) std::cout << "::OverFlow on CH " << Channel << " Gate " << *GateCounter << "::" << std::endl;
								
				if(this->Mask(Channel) == false){
					std::cout << "::Disabled Channel Stored::" << std::endl;
					continue;
				}
				
				if((file!=NULL) and Hits[0]< samples){
					buffer.assign(int2str(*GateCounter)+"\t"+int2str(Channel)+"\t");
					//buffer.append(int2str(Data[j]&0xFFF)+"\n");
					buffer.append(int2str(Data[j]&0xFFF)+"\t"+int2str(OV)+"\n");
					file << buffer.c_str();
				}

			}while(Channel-32*board >0);
			Hits[0]++;
			std::cout << "Hits: " << Hits[0] << std::endl;
			saveData = false;
                }
        }
	return;
}

void ProgramParameters::SaveDataCosmic(int *Data, int *Hits, unsigned long int *GateCounter, int board , std::string &fname, std::string &t_stamp){
        int InHeader = 0;
        int i=0,j=0;
	int Channel = 0;
	int GoodCH = 0;
	int OV;
	bool saveData = false;
	std::string buffer;
	std::ofstream file;

	//Data Kinds
	int DataKind;
	int Header = 2;
	int Datum  = 0;
	int EOB	   = 4;
	int NVD	   = 6;
	
        while(i<4096){
                //Cheking for the information Header

		// This case shows the end of the information sended by QDC board
		// Look on CAENVMEenvironment.cpp for further information
		if(Data[i]==-1){
			//std::cout << "Data[i] = -1" << std::endl;		
			return;
		}

		DataKind = (Data[i]&0x7000000)>>24;
		//std::cout <<"i = " << i << "\tDataKind = " << DataKind << std::endl;

                if(InHeader==0 && DataKind==Header){
                        //Header read and expected
			GoodCH = (Data[i]&0x3f00)>>8;
			InHeader=1;
			i += GoodCH+1;	//Move to the EOB Word
			//std::cout << "::Good Channels: " << GoodCH <<"::" << std::endl;
			continue;
		}else if(InHeader==1 && DataKind==Header){
                        std::cout << "::Unexpected Header::" << std::endl;
			return;
		}else if(InHeader==1 && DataKind==EOB){
                        //std::cout << "::End of Block Reached::" << std::endl;
                        InHeader = 0;
			*GateCounter = Data[i]&0xFFFFFF;
			i++;
			if(true){//GoodCH==32){ 
				saveData = true; 
				//std::cout<< "::saveData true::" << std::endl;
			}/*else{
				bads[k] = *GateCounter;
				k++;
			}*/                   
                }else if(InHeader==0 && DataKind==EOB){
			std::cout << "::Unexpected EOB::" << std::endl;
			return;
		}else if(DataKind==NVD){
			//std::cout << "::Not Valid Datum Reached::" << std::endl;
			i++;
			continue;
		}else if((DataKind!=Header) and
		  	 (DataKind!=Datum ) and 
			 (DataKind!=EOB   ) and
			 (DataKind!=NVD   )){
			std::cout << "::Reserved Word::" << std::endl;
			i++;
			continue;
		}

                //When inside a valid information section
                if(saveData){
			
			j=i-1;
			file.open(fname.c_str(),std::ios_base::app);
			do{
				j--;
				DataKind = (Data[j]&0x7000000)>>24;
	                        if(DataKind!=Datum){
					std::cerr << "ERROR: Something is wrong with data: no valid data detected" << std::endl; 
					return;
        	                }

				Channel = 32*board + ((Data[j]&0x1F0000)>>16);
				OV = (Data[j]&0x1000)>>12;

				//if(OV) std::cout << "::OverFlow on CH " << Channel << " Gate " << *GateCounter << "::" << std::endl;
								
				if(this->Mask(Channel) == false){
					std::cout << "::Disabled Channel Stored::" << std::endl;
					continue;
				}
				
				if((file!=NULL)){				
					buffer.assign(int2str(*GateCounter)+"\t"+int2str(Channel)+"\t");
					//buffer.append(int2str(Data[j]&0xFFF)+"\n");
					buffer.append(int2str(Data[j]&0xFFF)+"\t"+int2str(OV)+"\t"+t_stamp.c_str()+"\n");
					file << buffer.c_str();					
				}

			}while(Channel-32*board >0);
			Hits[0]++;
			//std::cout << "Hits: " << Hits[0] << std::endl;
			saveData = false;
			file.close();
                }
        }
	return;
}



char* ProgramParameters::GetControllerDevice(){
	return this->controllerDevice;
}

bool ProgramParameters::Verbose(){
	return this->verbose;
}

int ProgramParameters::GetIped(){
	return this->Iped;
}

void ProgramParameters::SetIped(int Iped){
	this->Iped = Iped;
}

void ProgramParameters::OpenLogFile(){
	struct stat logfilestatus;
	stat(this->logfilename, &logfilestatus);
	// Not open the file if log file's size 
	// bigger than 10M
	if(logfilestatus.st_size < 10000000){
		this->logfile.open(this->logfilename,std::ios_base::out|std::ios_base::app);
		if(this->logfile.is_open() == false)
			std::cerr << "WARNING: Logfile not opened" << std::endl;
		this->logfile << "starting" << std::endl;
	}else{
		std::cerr << "ERROR: Log file has a size bigger than 10MB. This will not be used." << std::endl;
	}
	return;
}

void ProgramParameters::Log(const char *message){
	
	if(this->logfile.is_open()){
		logfile << this->GetTime() << "\t" << message << std::endl;
	}else{
		std::cerr << "WARNING: Logfile is not opened" << std::endl;
	}
	return;
}

std::string ProgramParameters::GetTime(){
	time_t rawtime;
	struct tm * timeinfo;
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	return std::string(asctime(timeinfo));
}

std::string ProgramParameters::CurrentDateTime(char* format) {
	time_t     now = time(NULL);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), format, &tstruct);
	return buf;
}

void ProgramParameters::CloseLogFile(){
	if(this->logfile.is_open()){
		this->Log("closing.");
		this->logfile.close();
	}
	return;
}

void ProgramParameters::SetPermissions(char *filename){
	if(access(filename,F_OK)==-1){
		std::cerr << "ERROR: file does not exist" << std::endl;
	}
	if(chmod(filename, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH)==-1){
		std::cerr << "ERROR: bringing permissions to: " << filename << std::endl;
				
		std::ostringstream stringStream;
		stringStream << "ERROR: bringing permissions to: " << filename;
		std::string copyOfStr = stringStream.str();
		this->Log((const char*)copyOfStr.c_str());
		stringStream.clear();

		//this->Log(Form("ERROR: bringing permissions to: %s",filename));
	}
	return;
}

void ProgramParameters::CheckFilePath(){
	std::string fname(this->file_name_format);
	std::string path;
	path = fname.substr(0, fname.find_last_of("/"));
	if(access(path.c_str(),F_OK) == -1){
		std::cerr << "ERROR: Path to: " << this->file_name_format << " does not exist." << std::endl;
		
		std::ostringstream stringStream;
                stringStream << "ERROR: Path does to" << this->file_name_format << "not exist";
                std::string copyOfStr = stringStream.str();
                this->Log((const char*)copyOfStr.c_str());
		stringStream.clear();		
		
		//this->Log(Form("ERROR: Path does to %s not exist",this->file_name_format));
		exit(1);
	}
	return;
}

