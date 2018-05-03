/*######################################################################
#
# FILE.......:  MINUTES
#
# PORPOUSE...:  Utility gestione dei minuti per scripts
#
# AUTHOR.....: Cremonini
#
# CREATED....: 15/06/2008 V 1.0
#
# COPYRIGHT..:
#
######################################################################*/

// Include 
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <QDateTime>

using namespace std;

int main(int argc,char *argv[])
{
	setlocale(LC_ALL,"C");

	if (argc < 3) { 
	  cout << endl;
	  cout << "Usage: minutes [YYYYMMDDHHMI] [minuti]" << endl;
	  cout << endl;
	  return -1;
	}

	QDateTime indate = QDateTime::fromString(argv[1],"yyyyMMddHHmm");
	int minute =  atoi(argv[2]);

	indate = indate.addSecs(minute * 60);

	std::string str(indate.toString("yyyyMMddHHmm").toAscii());

	cout << str.c_str() << endl;

        return 0;

}

