// TopLevel.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "rrg.h"
#include "io.h"
#include <time.h>
#include "Compare.h"
#include "RrgStore.h"
using namespace std;
using namespace FDP::netlist;
using namespace FDP::archlib;
using namespace FDP::IO;
using namespace FDP::RRG;


namespace{
	const string xdlfile("D:\\project\\projects_of_c++\\routing\\routing\\xc4vlx15-ff668-10-pips.xdlrc");
	const int mainMaxX=72;
	const int mainMaxY=65;
}


int main(int argc, char* argv[])
{

	FPGADesign* fpga = new FPGADesign("something");
	ArchFile*	arch = new ArchFile(argv[1]);
	READER*		reader = new ArchReader(fpga,arch);
	const time_t starttime=time(NULL);
	Rrg*		rrg = new Rrg(fpga);
	cout<<"start to parse the arch file.."<<endl;
	reader->parse();
	rrg->build(); 
	const time_t endtime=time(NULL);
	cout<<"Rrg build finished! Cost "<<endtime-starttime<<"seconds. "<<endl;
	tr1::shared_ptr<MysqlInterface> sql_int(new MysqlInterface());
	sql_int->writeInitial();
	RrgStore rrgstore(sql_int,rrg);
	rrgstore.rrgStoreIntoDB();


	//Compare cmpr;
	//cmpr.initial(xdlfile,rrg,make_pair(100,100));
	//cmpr.readArchXDL();
	//int x,y=0;
	//for (x = 0;x < mainMaxX; ++x)
	//{
	//	for (y = 0; y < mainMaxY; ++y)
	//	{
	//		cmpr.TileMatching(x,y);
	//	}
	//}
	
	/*while(true)
	{
		HINSTANCE Hdll;
		Hdll=LoadLibrary(TEXT("D:\\project\\projects_of_c++\\routing\\CmprDll\\Release\\CmprDll.dll"));
		if(Hdll==NULL)cout<<"failed to load DLL!"<<endl;
		else cout<<"succeeded to load DLL!"<<endl;
		CreatRrgInterFace fuc_ptr=(CreatRrgInterFace)GetProcAddress(Hdll,"CreatObjectPtr");
		RrgInterFace *rrginterface= fuc_ptr();
	
		try{	
			rrginterface->setRrg(rrg);
			rrginterface->setFpag(fpga);
			rrginterface->starPoint();
		}
		catch(runtime_error e)
		{
			cout<<e.what()<<endl;
		}
		FreeLibrary(Hdll);
		system("pause");
	}*/
	//system("pause");
	return 0;
}

