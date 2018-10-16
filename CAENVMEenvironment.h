// QDCDataAqcisitionv1.0
//
// Developer: 	Pedro F. Toledo
//		<pedrotoledocorrea@gmail.com>
//
// CAENVMEenvironment.h
//
// Description:	This file contains the class definitions required for the QDCDataAc
//		quisition program, been designed for the version 1.0
//
// Libraries-----------------------------------------------------------------------
//#include <stdarg.h>                     //From CAENVMEDemoi

#ifndef __CAEN_QDC_Connection__
#define __CAEN_QDC_Connection__

#include "CAENVMElib.h"                 //v1718 Lib

class QDCConnection{
	private:
		int 		QDCAddress ;
		short		Link;
		int		debug;
		static int32_t	BHandle;
		static	unsigned int num;
	public:
				QDCConnection(int QDCAddressToConnect);
				~QDCConnection();
		void 		QDCWrite(int Address, int Value);
		void		QDCRead(int Address, int *Value);
		int		QDCReadBLT(int Size, int *Data);
		void		EnableChannel(int i);
		void		DisableChannel(int i);
		void		Reset();
		void		DataReset();
		void		OverFlowEnable(bool value);
		int		ReadStatusRegister();
		int		GetBitSet2Register();
		int             GetBitSet1Register();
		void		SetEventCounterMode(int mode);
		bool		Debug();
		void		Debug(int debug);
		int		IpedRegister();
		int		StepThreshold();
		void		SetIpedRegister(int Value);
		int		ReadThresholdValue(int i);
		int		Enable1us();
		int		Enable120ns();
		int		Version();
		void		SetMCSTControlRegister(int value);
		void		SetGeoAddress(int address);
};

#endif
