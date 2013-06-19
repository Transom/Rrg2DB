#include "RrgStore.h"


using namespace SQLVALTYPE;

RrgStore::~RrgStore(void)
{
}

const string tileUnit_T_name="TileUnitArray";
const string PinWire_T_name="pinWireMap";
const string wireStore_T_name="wireStore";
const string tileStroe_T_name="tileStore";

bool RrgStore::rrgStoreIntoDB()
{
	
	sqlTable_sptr rrg_table=creatRrgTable();
	loadRrgTable(rrg_table);

	sqlTable_sptr wireStore_table=creatWireStoreTable();
	loadWireStoreTable(wireStore_table);

	sqlTable_sptr tileStore_talbe=createTileStoreTable();
	loadTileStoreTable(tileStore_talbe);

	creatAndLordTileUnitTable(tileUnit_T_name);

	creatAndLoadwireIndexForTT("wireIndexForTTs");

	creatAndLoadFanoutStore("fanouts_hopUnit",fanouts);

	creatAndLoadSegmentStore("segments_hopUnit",segments);

	creatAndLoadPinwireList("PinWireList");
	return true;
}	

SQLVALTYPE::sqlTable_sptr RrgStore::creatRrgTable()
{
	string tableName="rrg";
	sqlTable_sptr rrg_table(new SqlTalbe(tableName,sql_int));

	SqlColumn_sptr fpga_package(new Sqlcolumn_var_Long("fpga_pachage"));
	SqlColumn_sptr tileUnit_table(new Sqlcolumn_var_Short("tileUnit_table"));
	SqlColumn_sptr pinWireMap_table(new Sqlcolumn_var_Short("pinWireMap_table"));
	SqlColumn_sptr wireStore_tabale(new Sqlcolumn_var_Short("wireStore_table"));
	SqlColumn_sptr tileStore_tabale(new Sqlcolumn_var_Short("tileStore_table"));
	SqlColumn_sptr fpga_scale_x(new Sqlcolumn_Small("fpga_scale_x"));
	SqlColumn_sptr fpga_scale_y(new Sqlcolumn_Small("fpga_scale_y"));
	rrg_table->addAColumn(fpga_package);
	rrg_table->addAColumn(tileUnit_table);
	rrg_table->addAColumn(pinWireMap_table);
	rrg_table->addAColumn(wireStore_tabale);
	rrg_table->addAColumn(tileStore_tabale);
	rrg_table->addAColumn(fpga_scale_x);
	rrg_table->addAColumn(fpga_scale_y);
	return rrg_table;
}

void RrgStore::loadRrgTable(sqlTable_sptr rrg_tabel)
{	
	
	recordType  record;
	string pakageName=cur_rrg->getfpga()->getpackage();
	short  fpga_scale_x=cur_rrg->get_fpga_scale().getx();
	short  fpga_scale_y=cur_rrg->get_fpga_scale().gety();

	record.push_back(SqlValue_sptr(cur_factory.makeSQLValVarchar(pakageName)));
	record.push_back(SqlValue_sptr(cur_factory.makeSQLValVarchar(tileUnit_T_name)));
	record.push_back(SqlValue_sptr(cur_factory.makeSQLValVarchar(PinWire_T_name)));
	record.push_back(SqlValue_sptr(cur_factory.makeSQLValVarchar(wireStore_T_name)));
	record.push_back(SqlValue_sptr(cur_factory.makeSQLValVarchar(tileStroe_T_name)));
	record.push_back(SqlValue_sptr(cur_factory.makeSQLValSmallInt(fpga_scale_x)));
	record.push_back(SqlValue_sptr(cur_factory.makeSQLValSmallInt(fpga_scale_y)));
	rrg_tabel->pushARecord(record);
	rrg_tabel->StoreTableToDB();
}

SQLVALTYPE::sqlTable_sptr RrgStore::creatWireStoreTable()
{
	sqlTable_sptr wireStore_table(new SqlTalbe(wireStore_T_name,sql_int));

	SqlColumn_sptr wirename(new Sqlcolumn_var_Short("wireName"));
	SqlColumn_sptr index(new Sqlcolumn_Small("wire_index"));
	SqlColumn_sptr fanouts_l(new Sqlcolumn_int("fanouts_l"));
	SqlColumn_sptr fanouts_h(new Sqlcolumn_int("fanouts_h"));
	SqlColumn_sptr segments_l(new Sqlcolumn_int("segments_l"));
	SqlColumn_sptr segments_h(new Sqlcolumn_int ("segments_h"));
	wireStore_table->addAColumn(wirename);
	wireStore_table->addAColumn(index);
	wireStore_table->addAColumn(fanouts_l);
	wireStore_table->addAColumn(fanouts_h);
	wireStore_table->addAColumn(segments_l);
	wireStore_table->addAColumn(segments_h);
	return wireStore_table;
}

void RrgStore::loadWireStoreTable(sqlTable_sptr wireStore_tabel)
{	
	multimap<string,WireTemplate*>::const_iterator itr;
	unsigned index=0;
	cout<<"wire store size: "<<cur_rrg->wire_size()<<endl;
	for (itr=cur_rrg->wire_begin();itr!=cur_rrg->wire_end();itr++){
		wireLUT.insert(make_pair(itr->second,index++));
		recordType cur_record;
		cur_record.push_back(SqlValue_sptr(cur_factory.makeSQLValVarchar(itr->first)));
		cur_record.push_back(SqlValue_sptr(cur_factory.makeSQLValSmallInt(itr->second->getIndex())));
		pair<unsigned,unsigned> fanouts_pair=fanouts.hopUnit_insert(itr->second->pip_begin(),itr->second->pip_end());
		cur_record.push_back(SqlValue_sptr(cur_factory.makeSQLValInt(fanouts_pair.first)));
		cur_record.push_back(SqlValue_sptr(cur_factory.makeSQLValInt(fanouts_pair.second)));
		pair<unsigned,unsigned> segments_pair=segments.hopUnit_insert(itr->second->seg_begin(),itr->second->seg_end());
		cur_record.push_back(SqlValue_sptr(cur_factory.makeSQLValInt(segments_pair.first)));
		cur_record.push_back(SqlValue_sptr(cur_factory.makeSQLValInt(segments_pair.second)));
		wireStore_tabel->pushARecord(cur_record);
	}
	wireStore_tabel->StoreTableToDB();
}

SQLVALTYPE::sqlTable_sptr RrgStore::createTileStoreTable()
{
	sqlTable_sptr tileStore_table(new SqlTalbe(tileStroe_T_name,sql_int));
	SqlColumn_sptr wireList_l(new Sqlcolumn_int("wireList_l"));
	SqlColumn_sptr wireList_h(new Sqlcolumn_int("wireList_h"));
	tileStore_table->addAColumn(wireList_l);
	tileStore_table->addAColumn(wireList_h);
	return tileStore_table;
}


void RrgStore::loadTileStoreTable( sqlTable_sptr tileStore_table )
{
	unsigned	TT_num=0;
	multimap<size_t,TileTemplate*>::const_iterator itr;
	for(itr=cur_rrg->tile_begin();itr!=cur_rrg->tile_end();itr++){
		vector<WireTemplate*>::const_iterator itr_wire;
		unsigned wireIndex_l=wireIndexForTTs.size();
		unsigned counter=0;
		for(itr_wire=itr->second->begin();itr_wire!=itr->second->end();itr_wire++){
			if((*itr_wire)==NULL) wireIndexForTTs.push_back(ILLEGAL_WIRE_INDEX);
			else if(wireLUT.count(*itr_wire))
			wireIndexForTTs.push_back(wireLUT[*itr_wire]);
			else {
				cout<<"something wrong!"<<endl;
				cout<<(*itr_wire)->getname()<<endl;
				system("pause");
			}
			counter++;
		}
		unsigned wireIndex_h=wireIndexForTTs.size();
		tileLUT.insert(make_pair(itr->second,TT_num++));
		recordType  cur_record;
		cur_record.push_back(SqlValue_sptr(cur_factory.makeSQLValInt(wireIndex_l)));
		cur_record.push_back(SqlValue_sptr(cur_factory.makeSQLValInt(wireIndex_h)));
		tileStore_table->pushARecord(cur_record);
	}
	tileStore_table->StoreTableToDB_ByStep();
}


void RrgStore::creatAndLoadTT( TileTemplate* tile,string TTName)
{
	sqlTable_sptr tile_table(new SqlTalbe(TTName,sql_int));
	SqlColumn_sptr wire_index(new Sqlcolumn_int("wire_index"));
	tile_table->addAColumn(wire_index);
	vector<WireTemplate*>::const_iterator itr;
	for(itr=tile->begin();itr!=tile->end();itr++){
		recordType cur_record;
		SqlValue_sptr cur_index(cur_factory.makeSQLValInt(wireLUT[*itr]));
		cur_record.push_back(cur_index);
		tile_table->pushARecord(cur_record);
	}
	tile_table->StoreTableToDB();
}

void RrgStore::creatAndLoadSegmentStore( string tableName,const HopUnitStore& _HU )
{
	sqlTable_sptr hu_table(new SqlTalbe(tableName,sql_int));
	SqlColumn_sptr hu_x(new Sqlcolumn_tiny("x"));
	SqlColumn_sptr hu_y(new Sqlcolumn_tiny("y"));
	SqlColumn_sptr hu_i(new Sqlcolumn_Small("i"));
	hu_table->addAColumn(hu_x);
	hu_table->addAColumn(hu_y);
	hu_table->addAColumn(hu_i);
	vector<HopUnit>::const_iterator itr;
	for(itr=_HU.begin();itr!=_HU.end();itr++)
	{
		SqlValue_sptr val_x(cur_factory.makSQLValTinyInt(itr->x));
		SqlValue_sptr val_y(cur_factory.makSQLValTinyInt(itr->y));
		SqlValue_sptr val_i(cur_factory.makeSQLValSmallInt(itr->index));
		recordType cur_record;
		cur_record.push_back(val_x);
		cur_record.push_back(val_y);
		cur_record.push_back(val_i);
		hu_table->pushARecord(cur_record);
	}
	hu_table->StoreTableToDB();
}
void RrgStore::creatAndLoadFanoutStore( string tableName,const HopUnitStore& _HU )
{
	sqlTable_sptr hu_table(new SqlTalbe(tableName,sql_int));
	SqlColumn_sptr hu_i(new Sqlcolumn_Small("i"));
	hu_table->addAColumn(hu_i);
	vector<HopUnit>::const_iterator itr;
	for(itr=_HU.begin();itr!=_HU.end();itr++)
	{
		SqlValue_sptr val_i(cur_factory.makeSQLValSmallInt(itr->index));
		recordType cur_record;
		cur_record.push_back(val_i);
		hu_table->pushARecord(cur_record);
	}
	hu_table->StoreTableToDB();
}
void RrgStore::creatAndLoadwireIndexForTT( string tableName)
{
	vector<unsigned short> compressed_vec;
	vector<unsigned short> ::const_iterator itr;
	unsigned short compressed_val=ILLEGAL_WIRE_INDEX;
	for(itr=wireIndexForTTs.begin();itr!=wireIndexForTTs.end();++itr){
		if(*itr==ILLEGAL_WIRE_INDEX){
			++compressed_val;
		}
		else{
			if(compressed_val>ILLEGAL_WIRE_INDEX){
				compressed_vec.push_back(compressed_val);
				compressed_val=ILLEGAL_WIRE_INDEX;
			}
			compressed_vec.push_back(*itr);
		}
	}
	if(compressed_val>ILLEGAL_WIRE_INDEX)
		compressed_vec.push_back(compressed_val);

	cout<<wireIndexForTTs.size()<<" | "<<compressed_vec.size()<<endl;
	cout<<compressed_vec.at(323903)<<endl;


	sqlTable_sptr wireIndex_table(new SqlTalbe(tableName,sql_int));
	int column_num=8;
	for(int ii=0;ii<column_num;ii++)
	{
		char buff[4];
		_itoa_s(ii,buff,10);
		string post_fix(buff);
		wireIndex_table->addAColumn(SqlColumn_sptr(new Sqlcolumn_Small("wireT_index_"+post_fix)));
	}
	for(itr=compressed_vec.begin();itr!=compressed_vec.end();)
	{
		recordType cur_record;
		for(int jj=0;jj<column_num;jj++){
			if(itr!=compressed_vec.end()){// 这里的bug调了一个下午！ 序列首位很容易存在错误！
				SqlValue_sptr val_A(cur_factory.makeSQLValSmallInt(*itr));
				cur_record.push_back(val_A);
				itr++;
			}
			else {
				SqlValue_sptr val_B(cur_factory.makeSQLValSmallInt(0));
				cur_record.push_back(val_B);
				//if(jj==column_num-1)itr++;
			}
		}
		wireIndex_table->pushARecord(cur_record);
	}
	cout<<" creatAndLoadwireIndexForTT record_vec size: "<<wireIndex_table->record_vec_size()<<endl;
	wireIndex_table->StoreTableToDB_ByStepS();
}

void RrgStore::creatAndLordTileUnitTable( string tableName )
{
	sqlTable_sptr	table(new  SqlTalbe(tableName,sql_int));
	table->addAColumn(SqlColumn_sptr(new Sqlcolumn_var_Short("tile_name")));
	table->addAColumn(SqlColumn_sptr(new Sqlcolumn_Small("TT_index")));
	table->addAColumn(SqlColumn_sptr(new Sqlcolumn_int("longWireIndex_l")));
	table->addAColumn(SqlColumn_sptr(new Sqlcolumn_int("longWireIndex_h")));
	
	short x=0,y=0;
	for(x=0;x<cur_rrg->get_fpga_scale().getx();x++)
		for(y=0;y<cur_rrg->get_fpga_scale().gety();y++){
			const TileUnit& curTileUnit=cur_rrg->get_tile_by_pos(Point(x,y,0));
			string name=curTileUnit.gettilename();
			TileTemplate* TT_ptr=curTileUnit.getTileTemplate();
			hash_map<short,WireTemplate*>::const_iterator itr,itr_begin,itr_end;
			itr_begin=curTileUnit.long_global_nets.begin();
			itr_end=curTileUnit.long_global_nets.end();
			recordType cur_record;
			//SqlValue_sptr tile_name(cur_factory.makeSQLValVarchar(name));
			cur_record.push_back(SqlValue_sptr(cur_factory.makeSQLValVarchar(name)));
			if(tileLUT.count(TT_ptr)==0){cout<<"something wrong! "<<__LINE__<<endl;system("pause");}
			cur_record.push_back(SqlValue_sptr(cur_factory.makeSQLValSmallInt(tileLUT[TT_ptr])));
			unsigned wireIndex_l=wireIndexForTTs.size();
			for(itr=itr_begin;itr!=itr_end;itr++){
				if(wireLUT.count(itr->second)==0){cout<<"something wrong! "<<__LINE__<<endl;system("pause");}
				wireIndexForTTs.push_back(wireLUT[itr->second]);
			}
			unsigned wireIndex_h=wireIndexForTTs.size();

			//if(x==71&&y==64){
			//	cout<<"longWireIndex_h-1: "<<wireIndex_h-1<<endl;
			//	cout<<"wireIndexList "<<wireIndexForTTs.at(wireIndex_h-1)<<endl;
			//	//cout<<"wireTstore: "<<wireTstore.at(wireIndexForTTs.at(wireIndex_h-1))->getIndex()<<endl;
			//	system("pause");
			//}
			cur_record.push_back(SqlValue_sptr(cur_factory.makeSQLValInt(wireIndex_l)));
			cur_record.push_back(SqlValue_sptr(cur_factory.makeSQLValInt(wireIndex_h)));
			table->pushARecord(cur_record);
		}
	table->StoreTableToDB();
}

void RrgStore::creatAndLoadPinwireList( string tableName )
{
	sqlTable_sptr	table(new  SqlTalbe(tableName,sql_int));
	table->addAColumn(SqlColumn_sptr(new Sqlcolumn_var_Short("sitePinName")));
	table->addAColumn(SqlColumn_sptr(new Sqlcolumn_var_Short("siteName")));
	table->addAColumn(SqlColumn_sptr(new Sqlcolumn_var_Short("pinWireName")));
	table->addAColumn(SqlColumn_sptr(new Sqlcolumn_Small("x")));
	table->addAColumn(SqlColumn_sptr(new Sqlcolumn_Small("y")));
	table->addAColumn(SqlColumn_sptr(new Sqlcolumn_Small("z")));
	
	short x,y;
	const vector<sitePinToPinwire>* pinWire_vec=NULL;
	for(x=0;x<cur_rrg->get_fpga_scale().getx();x++)
		for(y=0;y<cur_rrg->get_fpga_scale().gety();y++){
			pinWire_vec=&cur_rrg->getpinwire_vec(x,y);
			vector<sitePinToPinwire>::const_iterator itr;
			for (itr=pinWire_vec->begin();itr!=pinWire_vec->end();itr++){
				recordType cur_record;
				cur_record.push_back(SqlValue_sptr(cur_factory.makeSQLValVarchar(itr->sitePinName)));
				cur_record.push_back(SqlValue_sptr(cur_factory.makeSQLValVarchar(itr->siteName)));
				cur_record.push_back(SqlValue_sptr(cur_factory.makeSQLValVarchar(itr->pinWireName)));
				cur_record.push_back(SqlValue_sptr(cur_factory.makeSQLValSmallInt(x)));
				cur_record.push_back(SqlValue_sptr(cur_factory.makeSQLValSmallInt(y)));
				cur_record.push_back(SqlValue_sptr(cur_factory.makeSQLValSmallInt(itr->z)));
				table->pushARecord(cur_record);
			}
		}
	table->StoreTableToDB();
}
//
//void RrgStore::creatAndLoadwireIndexForTU( string tableName )
//{
//	sqlTable_sptr wireIndex_table(new SqlTalbe(tableName,sql_int));
//	wireIndex_table->addAColumn(SqlColumn_sptr(new Sqlcolumn_Small("indexInTile")));
//	wireIndex_table->addAColumn(SqlColumn_sptr(new Sqlcolumn_Small("indexForLUT")));
//	vector<pair<unsigned short,unsigned short> >::const_iterator itr;
//	for(itr=longWireIndexForTUs.begin();itr!=longWireIndexForTUs.end();itr++)
//	{
//		recordType cur_record;
//		SqlValue_sptr val_A(cur_factory.makeSQLValSmallInt(itr->first));
//		cur_record.push_back(val_A);
//		SqlValue_sptr val_B(cur_factory.makeSQLValSmallInt(itr->second));
//		cur_record.push_back(val_B);
//		wireIndex_table->pushARecord(cur_record);
//	}
//	cout<<" creatAndLoadwireIndexForTU record_vec size: "<<wireIndex_table->record_vec_size()<<endl;
//	wireIndex_table->StoreTableToDB();
//}
pair<unsigned int,unsigned int> HopUnitStore::hopUnit_insert( const vector<HopUnit> & temp_vec )
{
	unsigned int begin_pos=hop_vec.size();
	for (vector<HopUnit>::const_iterator itr=temp_vec.begin();itr!=temp_vec.end();itr++){
		hop_vec.push_back(*itr);
	}
	unsigned int end_pos=hop_vec.size();
	return make_pair(begin_pos,end_pos);
}

pair<unsigned int,unsigned int> HopUnitStore::hopUnit_insert( cnst_itr itr_begin,cnst_itr itr_end )
{
	unsigned int begin_pos=hop_vec.size();
	for (vector<HopUnit>::const_iterator itr=itr_begin;itr!=itr_end;itr++){
		hop_vec.push_back(*itr);
	}
	unsigned int end_pos=hop_vec.size();
	return make_pair(begin_pos,end_pos);
}