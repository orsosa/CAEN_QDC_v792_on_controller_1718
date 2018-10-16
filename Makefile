########################################################################

EXE	=	getData

##CC	=	gcc
CC	=	g++

COPTS	=	-fPIC -DLINUX -Wall
#COPTS	=	-g -fPIC -DLINUX -Wall 

FLAGS	=	-Wall -s
#FLAGS	=	-Wall

DEPLIBS	=       -l CAENVME -l ncurses -lc -lm

LIBS	=	

INCLUDEDIR =	-I. 

OBJS	=	getData.o CAENVMEenvironment.o ProgramParameters.o

#INCLUDES =	CAENVMElib.h CAENVMEtypes.h CAENVMEoslib.h CAENVMEenvironment.h ProgramParameters.h

#########################################################################

all	:	$(EXE)

clean	:
		/bin/rm -f $(OBJS) $(EXE)

$(EXE)	:	$(OBJS)
#		/bin/rm -f $(EXE)
		$(CC) $(FLAGS) -o $(EXE) $(OBJS) $(DEPLIBS)

#$(OBJS)	:	$(INCLUDES)
#$(OBJS)	:	$(INCLUDES) Makefile

%.o	:	%.cpp
		$(CC) -c $(COPTS) $(INCLUDEDIR) -o $@ $^

