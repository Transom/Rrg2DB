#pragma once
#include <WinSock2.h>
#include"mysql.h"
#include <iostream>
#include <string>
#include<vector>
#include "utils.h"
#include "MysqlInterface.h"
using namespace std;
using namespace FDP;
typedef  string SQL_CWD;
namespace SQLVALTYPE{

enum SqlValueType{INT,SMALLINT,TINYINT,VARCHAR};
class SqlTalbe;
class Sqlcolumn_base{
public:
	Sqlcolumn_base(const string &_name):name(_name){}
	~Sqlcolumn_base(){}
	virtual string getSqlTypeCwd()const{return "called the base virtual func error!";}
	virtual SqlValueType getType()const{ cout<<"getType wrong!"<<endl; return INT;}
	string  getcolumnName(void)const { return name;}
private:
string name;
};

class Sqlcolumn_Small: public Sqlcolumn_base{
	public:
	Sqlcolumn_Small(const string &_name):Sqlcolumn_base(_name){}
	~Sqlcolumn_Small(){}
	string getSqlTypeCwd()const {return getcolumnName()+" SMALLINT UNSIGNED";}
	SqlValueType getType()const{return SMALLINT;}
};

class Sqlcolumn_tiny:public Sqlcolumn_base{
public:
	~Sqlcolumn_tiny(){}
	Sqlcolumn_tiny(const string &_name):Sqlcolumn_base(_name){}
	string getSqlTypeCwd()const {return getcolumnName()+" TINYINT";}
	SqlValueType getType()const{return TINYINT;}
};

class Sqlcolumn_int:public Sqlcolumn_base{
public:
	~Sqlcolumn_int(){}
	Sqlcolumn_int(const string &_name):Sqlcolumn_base(_name){}
	string getSqlTypeCwd()const {return getcolumnName()+" INT UNSIGNED";}
	SqlValueType getType()const{return INT;}
};
class Sqlcolumn_var_Long:public Sqlcolumn_base{
public:
	~Sqlcolumn_var_Long(){}
	Sqlcolumn_var_Long(const string &_name):Sqlcolumn_base(_name){}
	string getSqlTypeCwd()const {return getcolumnName()+" VARCHAR(100)";}
	SqlValueType getType()const{return VARCHAR;}
};

class Sqlcolumn_var_Short:public Sqlcolumn_base{
public:
	~Sqlcolumn_var_Short(){}
	Sqlcolumn_var_Short(const string &_name):Sqlcolumn_base(_name){}
	string getSqlTypeCwd()const {return getcolumnName()+" VARCHAR(50)";}
	SqlValueType getType()const{return VARCHAR;}
};

class SqlValue_base{
public:
	SqlValue_base(){}
	virtual ~SqlValue_base(){}
	virtual string getSqlValCwd() const =0;
	virtual SqlValueType getType()const =0;
};
class SqlVal_Small:public SqlValue_base{
public:
	SqlVal_Small(short _val=0):val(_val){}
	~SqlVal_Small(){}
	string getSqlValCwd()const{ return num2str(val);}
	SqlValueType getType()const{return SMALLINT;}
private:
	short val;
};

class SqlVal_Tiny:public SqlValue_base{
public:
	SqlVal_Tiny(short _val=0):val(_val){}
	~SqlVal_Tiny(){}
	string getSqlValCwd()const{ return num2str(val);}
	SqlValueType getType()const{return TINYINT;}
private:
	short val;
};

class SqlVal_Int:public SqlValue_base{
public:
	SqlVal_Int(int _val=0):val(_val){}
	string getSqlValCwd()const { 
		char buff[16];
		 _itoa_s(val,buff,10);
		 return string(buff);
	}
	SqlValueType getType()const{return INT;}
private:
	int val;
};

class SqlVal_Var:public SqlValue_base{
public:
	SqlVal_Var(string _val=""):val(_val){}
	~SqlVal_Var(){}
	string getSqlValCwd()const{ return "'"+val+"'";}
	SqlValueType getType()const{return VARCHAR;}
private:
	string val;
};

class SQL_factory{
public:
	SqlValue_base * makeSQLValInt(int _val){ return new SqlVal_Int(_val);}
	SqlValue_base * makeSQLValSmallInt(short _val){return new SqlVal_Small(_val);}
	SqlValue_base * makSQLValTinyInt(short _val){return new SqlVal_Tiny(_val);}
	SqlValue_base * makeSQLValVarchar(string _val){return new SqlVal_Var(_val);}
};
Sqlcolumn_base* const null_ptr=NULL;
typedef tr1::shared_ptr<Sqlcolumn_base>		SqlColumn_sptr;
typedef tr1::shared_ptr<SqlValue_base>		SqlValue_sptr;
typedef tr1::shared_ptr<MysqlInterface>     MySqlInt_sptr;
typedef vector<SqlValue_sptr>				recordType;
typedef tr1::shared_ptr<SqlTalbe>	sqlTable_sptr;
class SqlTalbe
{
public:
	SqlTalbe(string _name,MySqlInt_sptr _sqlInt,SqlColumn_sptr _p_key=SqlColumn_sptr(null_ptr)):name(_name),sqlInt(_sqlInt),primary_key(_p_key){};
	~SqlTalbe(void);
	void pushARecord(const recordType& _record);
	void addAColumn(SqlColumn_sptr _colum){ column_list.push_back(_colum);}
	void setPrimaryKey(SqlColumn_sptr key){primary_key=key;}
	void StoreTableToDB();
	void StoreTableToDB_ByStep();
	void StoreTableToDB_ByStepS();
	size_t record_vec_size()const{ return records_vec.size();}
private:
	string getAInsertCWD(const recordType & record);
	string getValueList(const recordType & record);
	string getCreateTableCWD();
	string getTotalInsertCWD(const vector<recordType> &_temp_vec);
private:
	string	  name;
	vector<SqlColumn_sptr> column_list; 
	vector< recordType > records_vec;
	SqlColumn_sptr primary_key;
	MySqlInt_sptr sqlInt;
};



}