// QDCDataAqcisitionv1.0
//
// Developer:   Pedro F. Toledo
//              <pedrotoledocorrea@gmail.com>
//
// CAENVMEenvironment.cpp
//
// Description: This file contains the class methodes required for the QDCDataAcqui
//              sition program, been designed for the version 1.0
//
// Libraries-----------------------------------------------------------------------
#include <stdio.h>
//nclude <stdarg.h>
//#include <stdint.h> 
//#include "CAENVMElib.h"
#include <stdlib.h>
//#include <string.h>
#include "CAENVMEenvironment.h"         //Custom Environment
#include <fstream>
#include <iostream>

unsigned int QDCConnection::num = 0;
int32_t QDCConnection::BHandle = 0;

QDCConnection::QDCConnection(int QDCAddressToConnect){
	int Data,result;
	CVErrorCodes errcode;
	this->debug = 0;
	if(this->num == 0){
		this->BHandle = 0;
		short Device = atoi("USB");
		result = CAENVME_Init(cvV1718, 0, Device, &BHandle);
		if(result != cvSuccess ){
			std::cerr << "\e[0;31mERROR:\e[0m ";
			switch(result){
				case -1:
					std::cerr << "VME bus error during the cycle";
					break;
				case -2:
					std::cerr << "Communication error";
					break;
				case -3:
					std::cerr << "Unspecified error";
					break;
				case -4:
					std::cerr << "Invalid parameter";
					break;
				case -5:
					std::cerr << "Timeout error";
					break;
			}
			std::cerr << ", opening the USB Link to V1718 device" << std::endl;
			exit(1);
		}
		if(this->Debug())
			std::cout << "Connection with CAEN-V1718 Established through USB" << std::endl;
	}
	this->QDCAddress = QDCAddressToConnect;
	if(QDCAddress%0x10000 != 0){
		std::cerr << "\e[0;31mERROR:\e[0m Invalid QDC address" << std::endl;
		exit(7);
	}else{
//		errcode = CAENVME_ReadCycle(QDCConnection::BHandle, this->QDCAddress + 0x1000, &Data, cvA24_U_DATA, cvD16);
		errcode = CAENVME_ReadCycle(QDCConnection::BHandle, this->QDCAddress + 0x1000, &Data, cvA32_U_DATA, cvD16);
		if(errcode != cvSuccess){
			std::cerr << "\e[0;31mERROR:\e[0m opening the QDC link on address " << std::hex << this->QDCAddress << std::dec << "; Error code: " << errcode << std::endl;
                	exit(7) ;	
		}
	}
	if(this->Debug()){
		std::cout << "QDC Connected at VME Address " << std::hex << this->QDCAddress;
		std::cout << ": Firmware v" << std::dec << ((Data&0xF000)>>12)*10+((Data&0xF00)>>8);
		std::cout << "." << ((Data&0xF0)>>4);
		std::cout << (Data&0xF) << std::endl;
		}
	this->num++;
}

QDCConnection::~QDCConnection(){
	this->num--;
	if(this->num==0)
		CAENVME_End(this->BHandle);
}

void QDCConnection::QDCWrite(int Address, int Value){
	CVErrorCodes errcode;
	if(
		Address%2 == 0    and 
		Address >= 0x1002 and 
		Address <= 0x10BF and 
		Address != 0x100E and
		Address != 0x1022 and
		Address != 0x1024 and
		Address != 0x1026 and
		Address != 0x1070 and
		Address != 0x1072 and
		!(Address < 0x1000 and Address > 0x07FC)){
//		errcode = CAENVME_WriteCycle(this->BHandle, this->QDCAddress + Address, &Value, cvA24_U_DATA, cvD16);
		errcode = CAENVME_WriteCycle(this->BHandle, this->QDCAddress + Address, &Value, cvA32_U_DATA, cvD16);
		if(errcode != cvSuccess){
			std::cerr << "\e[0;31mERROR:\e[0m writing QDC at address 0x" << std::hex << QDCAddress << std::dec << std::endl;
			std::cerr << "Error code: " << errcode << std::endl;
			exit(2);
		}
	}else{
		std::cerr << "\e[0;31mERROR:\e[0m Invalid Writing QDC Address" << std::endl;
                exit(2) ;
	}
}

void QDCConnection::QDCRead(int Address, int *Value){
	unsigned int Data = 0 ;
	if( Address%2 == 0 and Address >= 0 and 
	Address <= 0x10BF and Address != 0x1016 and
	Address != 0x1028 and Address != 0x102A and
	Address != 0x1034 and Address != 0x1036 and
	Address != 0x1038 and Address != 0x103A and
	Address != 0x103E and Address != 0x1040 and
	Address != 0x1064 and Address != 0x1068 and
	!(Address < 0x1000 and Address > 0x07FC)){
		if(Address <= 0x07FC){
//			if(CAENVME_ReadCycle(this->BHandle, this->QDCAddress + Address, &Data, cvA24_U_DATA, cvD32) != cvSuccess){
			if(CAENVME_ReadCycle(this->BHandle, this->QDCAddress + Address, &Data, cvA32_U_DATA, cvD32) != cvSuccess){
				std::cerr << "\e[0;31mERROR:\e[0m reading QDC at address 0x" << std::hex << QDCAddress << std::dec << std::endl;
				exit(3);
			}
		}else{
//			if(CAENVME_ReadCycle(this->BHandle, this->QDCAddress + Address, &Data, cvA24_U_DATA, cvD16) != cvSuccess){
			if(CAENVME_ReadCycle(this->BHandle, this->QDCAddress + Address, &Data, cvA32_U_DATA, cvD16) != cvSuccess){
				std::cerr << "\e[0;31mERROR:\e[0m reading QDC at address 0x" << std::hex << this->QDCAddress << std::endl;
				exit(3);
			}
                }
		*Value = Data;	
	}else{
		std::cerr << "\e[0;31mERROR:\e[0m Invalid Reading QDC Address" << std::endl;
                exit(3) ;
	}
}

int QDCConnection::QDCReadBLT(int Size, int *Data){
	int Total;
        unsigned char *Datai;
	if(Size <= 4096 and Size > 0){
		//Datai = (unsigned char*)malloc(Size*sizeof(unsigned char)*4) ;
		Datai = new unsigned char [Size*4*sizeof(unsigned char)];
//	        CAENVME_BLTReadCycle(this->BHandle, this->QDCAddress, Datai, 4*4096, cvA24_U_DATA, cvD32, &Total) ;
	        CAENVME_BLTReadCycle(this->BHandle, this->QDCAddress, Datai, 4*4096, cvA32_U_DATA, cvD32, &Total) ;
	      	for(int i=0;i<Size;i++){
			if(i<Total/4){
	               		Data[i] = (Datai[i*4+3]<<24)+(Datai[i*4+2]<<16)+(Datai[i*4+1]<<8)+Datai[i*4+0] ;
        		}else{
				Data[i] = -1;
			}
		}
		delete Datai;
        	return Total/4;
	}else{
		std::cerr << "\e[0;31mERROR:\e[0m Invalid BLT Reading Length" << std::endl;
		exit(4) ;
	}
	return 0;
}

void QDCConnection::EnableChannel(int i){
	this->QDCWrite(0x1080+i*2,0);
}

void QDCConnection::DisableChannel(int i){
//	this->QDCWrite(0x1080+i*2,0x1F0);
	this->QDCWrite(0x1080+i*2,0x100);
}

int QDCConnection::ReadThresholdValue(int i){
	int Value;
	this->QDCRead(0x1080+i*2,&Value);
	return Value;
}

void QDCConnection::Reset(){
//	this->QDCWrite(0x1016,1);
	unsigned int value = this->GetBitSet1Register();
	value  |= 1<<7;
	// Writing a 1 on bit 7 on this register let resetted the module
	this->QDCWrite(0x1006,0x80);
	value = 0 | 1<<7;
	// Writing a 1 on bit 7 on 0x1008 release the reset signal
	this->QDCWrite(0x1008,0x80);
}

void QDCConnection::DataReset(){
	this->QDCWrite(0x1032,0x4);
	this->QDCWrite(0x1034,0x4);
}

void QDCConnection::OverFlowEnable(bool value){
	if(value) this->QDCWrite(0x1032,8);
	else 	  this->QDCWrite(0x1034,8);
}

int QDCConnection::GetBitSet2Register(){
	int Value;
	this->QDCRead(0x1032, &Value);
	return Value;
}

int QDCConnection::GetBitSet1Register(){
	int Value;
	this->QDCRead(0x1006, &Value);
	return Value;
}

int QDCConnection::ReadStatusRegister(){
        int Value;
        this->QDCRead(0x100E, &Value);
        return Value;
}

void QDCConnection::SetEventCounterMode(int mode){
	if(mode!=1 && mode!=0){
		std::cerr << "\e[0;31mERROR:\e[0m QDCConnection::SetEventCounterMode: writing in the port." << std::endl;
		std::cout << "Mode:" << std::endl;
		std::cout << "\t0\t: Event Counter increment always." << std::endl;
		std::cout << "\t1\t: Event Counter increment only in accepted events." << std::endl;
		exit(1);
	}
	if(mode){
		this->QDCWrite(0x1032,0x4000);
	}else{
		this->QDCWrite(0x1034,0x4000);
	}
}

bool QDCConnection::Debug(){
	return (bool) this->debug;
}

void QDCConnection::Debug(int debug){
	this->debug = debug;
}

int QDCConnection::IpedRegister(){
	int Value;
	this->QDCRead(0x1060, &Value);
	return Value;
}

void QDCConnection::SetIpedRegister(int Value){
	this->QDCWrite(0x1060,Value);
}

int QDCConnection::StepThreshold(){
	int Value;
	this->QDCRead(0x1032, &Value);
	return Value&0x100;
}

int QDCConnection::Enable1us(){
	CAENVME_SetOutputConf(BHandle,cvOutput0,cvDirect,cvActiveHigh,cvManualSW);
	CAENVME_SetOutputConf(BHandle,cvOutput1,cvInverted,cvActiveHigh,cvManualSW);
	return (int) CAENVME_SetOutputRegister(QDCConnection::BHandle,0xC0);
}

int QDCConnection::Enable120ns(){
	CAENVME_SetOutputConf(BHandle,cvOutput0,cvDirect,cvActiveHigh,cvManualSW);
        CAENVME_SetOutputConf(BHandle,cvOutput1,cvInverted,cvActiveHigh,cvManualSW);
	return (int) CAENVME_SetOutputRegister(QDCConnection::BHandle,0x0);
}


int QDCConnection::Version(){
	int Data;
//	if(CAENVME_ReadCycle(BHandle, this->QDCAddress + 0x1000, &Data, cvA24_U_DATA, cvD16) != cvSuccess){
	if(CAENVME_ReadCycle(BHandle, this->QDCAddress + 0x1000, &Data, cvA32_U_DATA, cvD16) != cvSuccess){ 
                        std::cerr << "\e[0;31mERROR:\e[0m opening the QDC link on address " << std::hex << this->QDCAddress << std::dec << std::endl;
                        return 1;
	}
	std::cout << "QDC Connected at VME Address " << std::hex << this->QDCAddress;
	std::cout << ": Firmware v" << std::dec << ((Data&0xF000)>>12)*10+((Data&0xF00)>>8);
	std::cout << "." << ((Data&0xF0)>>4);
	std::cout << (Data&0xF) << std::endl;
	return 0;
}

/*
Setting MCST/CBLT Control Register sets board order
to use in CBLT or MCST chain as:
0 -> Intermediate inactive Board
1 -> Last Board
2 -> First Board
3 -> Intermediate active Board
*/
void QDCConnection::SetMCSTControlRegister(int value){
	if(value <=3 and value >=0) this->QDCWrite(0x101A,value);
	else std::cerr << "\e[0;31mERROR:\e[0m SetMCSTControlRegister: Value out of range" << std::endl;
}

void QDCConnection::SetGeoAddress(int address){
	this->QDCWrite(0x1002,address);
}

	



