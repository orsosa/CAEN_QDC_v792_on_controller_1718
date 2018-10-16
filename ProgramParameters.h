#ifndef __PROGRAM_PARAMETERS__
#define __PROGRAM_PARAMETERS__

#include <string.h>
#include <iostream>
#include <fstream>


class ProgramParameters 
{
private:
	int debug;
	unsigned long int mask;
	unsigned long int mask2;
	unsigned long int mask3;
	int	samples;
	int	Iped;
	char*	file_name_format;
	char*	controllerDevice;
	char*	logfilename;
	bool	verbose;
	std::ofstream logfile;

public:
	ProgramParameters(int argc,char** argv);
	~ProgramParameters();
	void	Presentation();
	int	checkImportantParameters();
	void	usage(char* argv);
	void	Help(char* argv);
	bool	Debug(int level);
	bool	Mask(int i);
	int	GetSamples();
	void	SaveData(int *Data, int *Hits, unsigned long int *GateCounter, int board, std::ofstream & file, int samples);
	void	SaveDataCosmic(int *Data, int *Hits, unsigned long int *GateCounter, int board , std::string &fname, std::string &t_stamp);
	char*	GetFName();
	std::string GetTime();
	std::string CurrentDateTime(char* format);
	void	SetFName(char* fname);
	char*	GetControllerDevice();
	bool	Verbose();
	void	SetIped(int Iped);
	int	GetIped();
	void	OpenLogFile();
	void	Log(const char *message);
	void	CloseLogFile();
	void	SetPermissions(char *filename);
	void	CheckFilePath();
};

#endif
