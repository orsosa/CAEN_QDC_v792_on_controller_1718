#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string>

#include "CAENVMEenvironment.h"         //For this setup communication
#include "ProgramParameters.h"

#define Nqdc 3

ProgramParameters *setup;
QDCConnection *QDC[Nqdc];

void sighandler(int sig)
{
	int j;
        std::cout << "Signal " << sig << " caught..." << std::endl;
        delete setup;
	for(j=0;j<Nqdc;j++)
	{
		delete QDC[j];
	}
	//delete QDC[1];
	//delete QDC[2];
	exit(1);
}

int main(int argc, char *argv[])
{
	setup = new ProgramParameters(argc,argv);
	bool QDC_inplace[Nqdc]={1,1,1};
	int i, k, j, QDC_STAT[Nqdc];//,i[Nqdc];
	int Data[Nqdc][4096];//, bads[Nqdc][32];
	int Hits[3]={0,0,0};
	unsigned long int GateCounter[3] = {0,0,0};
	int samples = setup->GetSamples();
	std::string fname, t_stamp;
	std::ofstream file;
	
	for(j=0;j<Nqdc;j++)
	{
		GateCounter[j] = 0;

		std::ostringstream stringStream;
		stringStream << "Setting QDC_" << j << " at address " << 0x00110000*(j+5);
                std::string copyOfStr = stringStream.str();
 		setup->Log((const char*)copyOfStr.c_str());
		stringStream.clear();
		
		QDC[j] = new QDCConnection(0x00110000*(j+5));
		for(k=0;k<32;k++){
			if(setup->Mask(k+32*j)){
				QDC[j]->EnableChannel(k);
				//std::cout << "Habilitando canal: " << k+32*j << std::endl;
			}else{
				QDC[j]->DisableChannel(k);
			}
		}
		QDC[j]->SetGeoAddress(j+5); //Number at VME Crate
		QDC[j]->Reset();
		QDC[j]->SetEventCounterMode(1);
		QDC[j]->OverFlowEnable(true);
		QDC[j]->SetIpedRegister(setup->GetIped());
	}
	//Setting MCST/CBLT order
	QDC[0]->SetMCSTControlRegister(2);	//First
	QDC[1]->SetMCSTControlRegister(3);	//Intermediate
	QDC[2]->SetMCSTControlRegister(1);	//Last

	signal(SIGABRT, &sighandler);
	signal(SIGTERM, &sighandler);
	signal(SIGINT, &sighandler);
	fname.assign(setup->GetFName());
	fname.append(setup->CurrentDateTime("%Y%m%d_%H%M%S"));
	fname.append(".dat");
	file.open(fname.c_str());
	file << "#Gate\t#CH\t#ADC\t#OV\t#Date\t\t#Time\n";
	file.close();
	for(j=0;j<Nqdc;j++) QDC[j]->DataReset();

	std::cout << "Storing in file " << fname.c_str() << std::endl;
	while(1)
	{

		t_stamp.assign(setup->CurrentDateTime("%Y%m%d\t%H%M%S"));
		int status = 0x1;
		for(int i=0;i<Nqdc;i++){
			if(QDC_inplace[i]) QDC[i]->QDCRead(0x100E,&QDC_STAT[i]);
			else continue;
			status &= QDC_STAT[i];
		}
		if (status)
			for(int j=0;j<Nqdc;j++)
			{
				if (!QDC_inplace[j]) continue;
				for(i=0;i<4096;i++) Data[j][i]=0;
				QDC[j]->QDCReadBLT(4096,Data[j]);
				setup->SaveDataCosmic(Data[j],&Hits[j],&GateCounter[j],j,fname, t_stamp);
				
			}

			
		std::cout << "Counts:  ";
		if(setup->Verbose())
			for(int j=0;j<Nqdc;j++)
			{
				if (QDC_inplace[j]) std::cout << "QDC[" << j <<"] = " << Hits[j] << "\tstat: " << QDC_STAT[j] << "\t" << status << "\t";
			}
		std::cout << "\r";
		if(Hits[0]>samples-1){ 
			fname.assign(setup->GetFName());
			fname.append(setup->CurrentDateTime("%Y%m%d_%H%M%S"));
			fname.append(".dat");
			for(i=0;i<3;i++) Hits[i]=0;
			std::cout << "Storing in new file " << fname.c_str() << std::endl;
		}
		sleep(0.5);
	
	}
//	file.close();
	delete setup;
	for(j=0;j<Nqdc;j++){
	        delete QDC[j];
	}
        //delete QDC[1];
        //delete QDC[2];
	return 0;
}

