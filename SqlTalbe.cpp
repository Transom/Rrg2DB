#include "SqlTalbe.h"
using namespace SQLVALTYPE;


SqlTalbe::~SqlTalbe(void)
{
}

string SQLVALTYPE::SqlTalbe::getCreateTableCWD()
{
	string temp_cwd="CREATE TABLE IF NOT EXISTS "+name+"(";
	vector<SqlColumn_sptr>::iterator itr=column_list.begin();
	temp_cwd+=(*itr)->getSqlTypeCwd();++itr;
	for(;itr!=column_list.end();itr++){
		temp_cwd+=",";
		temp_cwd+=(*itr)->getSqlTypeCwd();
	}
	if(primary_key!=NULL){ 
		temp_cwd+=", PRIMARY KEY(";
		temp_cwd+=primary_key->getcolumnName();
		temp_cwd+=")";		
	}
	temp_cwd+=")";
	return temp_cwd;
}

string SQLVALTYPE::SqlTalbe::getAInsertCWD(const recordType & record)
{
	string temp_cwd="INSERT INTO "+name+" VALUES (";
	vector<SqlValue_sptr>::const_iterator itr=record.begin();
	temp_cwd+=(*itr)->getSqlValCwd(); ++itr;
	for(;itr!=record.end();itr++){
		temp_cwd+=",";
		temp_cwd+=(*itr)->getSqlValCwd();
	}
	temp_cwd+=")";
	return temp_cwd;
}
string SQLVALTYPE::SqlTalbe::getValueList( const recordType & record )
{
	string temp_cwd=" (";
	vector<SqlValue_sptr>::const_iterator itr=record.begin();
	temp_cwd+=(*itr)->getSqlValCwd(); ++itr;
	for(;itr!=record.end();itr++){
		temp_cwd+=",";
		temp_cwd+=(*itr)->getSqlValCwd();
	}
	temp_cwd+=")";
	return temp_cwd;
}

void SQLVALTYPE::SqlTalbe::pushARecord( const recordType& _record )
{
	if(_record.size()!=column_list.size()) cout<<"record_size and column_size don't match!"<<endl;
	for(size_t ii=0;ii<column_list.size();ii++){
		if(column_list.at(ii)->getType()!=_record.at(ii)->getType()) cout<<"record type doesn't match!"<<endl;
	}
	records_vec.push_back(_record);
}

void SQLVALTYPE::SqlTalbe::StoreTableToDB()
{
	string sqlcwd=getCreateTableCWD();
	sqlInt->mysqlRealQuery(sqlcwd);
	time_t star_t=time(NULL);
	cout<<name<<" size: "<<records_vec.size()<<endl;
	string allRecordCwd=getTotalInsertCWD(records_vec);
	sqlInt->mysqlRealQuery(allRecordCwd);
	time_t end_t=time(NULL);
	cout<<"cost "<<end_t-star_t<<" seconds."<<endl;
}
void SQLVALTYPE::SqlTalbe::StoreTableToDB_ByStep()
{
	string sqlcwd=getCreateTableCWD();
	sqlInt->mysqlRealQuery(sqlcwd);
	time_t star_t=time(NULL);
	cout<<name<<" size: "<<records_vec.size()<<endl;
	vector<recordType>::iterator itr;
	int val=0;
	for (itr=records_vec.begin();itr!=records_vec.end();itr++){
		sqlcwd=getAInsertCWD(*itr);
		sqlInt->mysqlRealQuery(sqlcwd);
	}
	time_t end_t=time(NULL);
	cout<<"cost "<<end_t-star_t<<" seconds."<<endl;
}
void SQLVALTYPE::SqlTalbe::StoreTableToDB_ByStepS()
{
	unsigned step_num=records_vec.size()/4+50;
	string sqlcwd=getCreateTableCWD();
	sqlInt->mysqlRealQuery(sqlcwd);
	time_t star_t=time(NULL);
	cout<<name<<" size: "<<records_vec.size()<<endl;
	unsigned val=0;
	vector<recordType>::iterator itr;
	string sql_steps;
	for (itr=records_vec.begin();itr!=records_vec.end();itr++){
		sql_steps+=getValueList(*itr);
		val++;
		if((val%step_num)==0&&val!=0){
			cout<<val<<endl;
			string tempCwd="INSERT INTO "+name+" VALUES ";
			tempCwd+=sql_steps;
			sqlInt->mysqlRealQuery(tempCwd);
			sql_steps.clear();
		}
		else if(itr!=records_vec.end()-1){
			sql_steps+=", ";
		}
	}
	string tempCwd="INSERT INTO "+name+" VALUES ";
	tempCwd+=sql_steps;
	sqlInt->mysqlRealQuery(tempCwd);
	time_t end_t=time(NULL);
	cout<<"cost "<<end_t-star_t<<" seconds."<<endl;
}
string SQLVALTYPE::SqlTalbe::getTotalInsertCWD( const vector<recordType> &_temp_vec )
{
	string sqlcwd="INSERT INTO "+name+" VALUES ";
	vector<recordType>::iterator itr=records_vec.begin();
	sqlcwd+=getValueList(*itr); 
	itr++;
	for(;itr!=records_vec.end();itr++){
		sqlcwd+=",";
		sqlcwd+=getValueList(*itr);
	}
	return sqlcwd;
}
