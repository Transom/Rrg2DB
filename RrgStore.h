#pragma once
#include "MysqlInterface.h"
#include "SqlTalbe.h"
#include "rrg.h"
using namespace std;
using namespace FDP::RRG;
using namespace SQLVALTYPE;

const unsigned short ILLEGAL_WIRE_INDEX=10000;
class HopUnitStore{
public:
	typedef vector<HopUnit>::const_iterator cnst_itr;
	pair<unsigned int,unsigned int> hopUnit_insert(const vector<HopUnit> & temp_vec);
	pair<unsigned int,unsigned int> hopUnit_insert(cnst_itr itr_begin,cnst_itr itr_end);
	cnst_itr begin()const {return hop_vec.begin();}
	cnst_itr end()const {return hop_vec.end();}
private:
	vector<HopUnit> hop_vec;
};

class RrgStore
{
public:
	RrgStore(MySqlInt_sptr _sql_int,const Rrg *_rrg):sql_int(_sql_int),cur_rrg(_rrg){}
	~RrgStore(void);
	bool rrgStoreIntoDB();
private:
	sqlTable_sptr creatRrgTable();
	void loadRrgTable(sqlTable_sptr rrg_tabel);
	sqlTable_sptr creatWireStoreTable();
	void loadWireStoreTable(sqlTable_sptr);
	sqlTable_sptr createTileStoreTable();
	void loadTileStoreTable(sqlTable_sptr);
	void creatAndLoadTT(TileTemplate*,string TTName);
	void creatAndLoadSegmentStore(string tableName,const HopUnitStore& _HU);
	void creatAndLoadFanoutStore(string tableName,const HopUnitStore& _HU);
	void creatAndLordTileUnitTable(string tableName);
	void creatAndLoadwireIndexForTT(string tableName);
	void creatAndLoadPinwireList(string tableName);
private:
	const Rrg * cur_rrg;
	MySqlInt_sptr sql_int;
	HopUnitStore  fanouts,segments;
	vector<unsigned short> wireIndexForTTs;
	map<WireTemplate*,unsigned short> wireLUT;
	map<TileTemplate*,unsigned short> tileLUT;
	SQL_factory cur_factory;
};
