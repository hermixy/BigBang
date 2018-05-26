#include <map>

#include "sqlite3/dll.hpp"
#include "sqlite3/vsqlite3.hpp"
#include "win32.hpp"

#include "box.hpp"
#include "string.hpp"

using namespace WarGrey::SCADA;

typedef int (*_fun__wchar__sqlite3__int)(const wchar_t*, sqlite3_t**);
typedef const wchar_t* (*_fun__sqlite3__wchar)(sqlite3_t*);
typedef int (*_fun__sqlite3__int)(sqlite3_t*);
typedef long (*_fun__sqlite3__long)(sqlite3_t*);
typedef int(*_fun__sqlite3__uint__trace__void__int)(sqlite3_t*, unsigned int, sqlite3_trace_t, void*);

typedef int (*_fun__sqlite3__wchar__stmt__void__int)(sqlite3_t*, const wchar_t*, size_t, sqlite3_stmt_t**, const void**);
typedef int (*_fun__stmt__int)(sqlite3_stmt_t*);
typedef int (*_fun__stmt__int__int)(sqlite3_stmt_t*, int);
typedef int (*_fun__stmt__int__int32__int)(sqlite3_stmt_t*, int, int32);
typedef int (*_fun__stmt__int__int64__int)(sqlite3_stmt_t*, int, int64);
typedef int (*_fun__stmt__int__real__int)(sqlite3_stmt_t*, int, double);
typedef int (*_fun__stmt__int__blob__fptr__int)(sqlite3_stmt_t*, int, const char*, size_t, _fun_destructor);
typedef int (*_fun__stmt__int__wchar__fptr__int)(sqlite3_stmt_t*, int, const wchar_t*, size_t, _fun_destructor);

typedef const char* (*_fun__stmt__int__char)(sqlite3_stmt_t*, int);
typedef const wchar_t* (*_fun__stmt__int__wchar)(sqlite3_stmt_t*, int);
typedef int32 (*_fun__stmt__int__int32)(sqlite3_stmt_t*, int);
typedef int64 (*_fun__stmt__int__int64)(sqlite3_stmt_t*, int);
typedef double (*_fun__stmt__int__real)(sqlite3_stmt_t*, int);

typedef const char* (*_fun__stmt__char)(sqlite3_stmt_t*);

static HMODULE sqlite3 = nullptr;
static int references = 0;

static _fun__int sqlite3_libversion_number;
static _fun__wchar__sqlite3__int sqlite3_open16;
static _fun__sqlite3__int sqlite3_close;
static _fun__sqlite3__wchar sqlite3_errmsg16;
static _fun__sqlite3__uint__trace__void__int sqlite3_trace_v2;

static _fun__sqlite3__wchar__stmt__void__int sqlite3_prepare16_v2;
static _fun__stmt__int sqlite3_reset;
static _fun__stmt__int sqlite3_step;
static _fun__stmt__int sqlite3_finalize;

static _fun__stmt__int sqlite3_bind_parameter_count;
static _fun__stmt__int sqlite3_clear_bindings;
static _fun__stmt__int__int sqlite3_bind_null;
static _fun__stmt__int__int32__int sqlite3_bind_int;
static _fun__stmt__int__int64__int sqlite3_bind_int64;
static _fun__stmt__int__real__int sqlite3_bind_double;
static _fun__stmt__int__blob__fptr__int sqlite3_bind_blob;
static _fun__stmt__int__wchar__fptr__int sqlite3_bind_text16;

static _fun__stmt__int sqlite3_data_count;
static _fun__stmt__int sqlite3_column_count;
static _fun__stmt__int__wchar sqlite3_column_database_name16;
static _fun__stmt__int__wchar sqlite3_column_table_name16;
static _fun__stmt__int__wchar sqlite3_column_origin_name16;
static _fun__stmt__int__wchar sqlite3_column_decltype16;
static _fun__stmt__int__int sqlite3_column_bytes;
static _fun__stmt__int__int sqlite3_column_bytes16;
static _fun__stmt__int__int sqlite3_column_type;
static _fun__stmt__int__char sqlite3_column_blob;
static _fun__stmt__int__wchar sqlite3_column_text16;
static _fun__stmt__int__int32 sqlite3_column_int;
static _fun__stmt__int__int64 sqlite3_column_int64;
static _fun__stmt__int__real sqlite3_column_double;
static _fun__stmt__char sqlite3_sql;

static _fun__sqlite3__int sqlite3_changes;
static _fun__sqlite3__int sqlite3_total_changes;
static _fun__sqlite3__long sqlite3_last_insert_rowid;

static const int SQLITE_OK   = 0;
static const int SQLITE_ROW  = 100;
static const int SQLITE_DONE = 101;

static const _fun_destructor SQLITE_STATIC    = ((_fun_destructor)0);
static const _fun_destructor SQLITE_TRANSIENT = ((_fun_destructor)-1);

static void load_sqlite3(Syslog* logger) {
	if (sqlite3 == nullptr) {
		sqlite3 = win32_load_foreign_library("sqlite3", logger);

		if (sqlite3 != nullptr) {
			win32_fetch(sqlite3, sqlite3_libversion_number, _fun__int, logger);
			win32_fetch(sqlite3, sqlite3_open16, _fun__wchar__sqlite3__int, logger);
			win32_fetch(sqlite3, sqlite3_close, _fun__sqlite3__int, logger);
			win32_fetch(sqlite3, sqlite3_errmsg16, _fun__sqlite3__wchar, logger);
			win32_fetch(sqlite3, sqlite3_trace_v2, _fun__sqlite3__uint__trace__void__int, logger);

			win32_fetch(sqlite3, sqlite3_prepare16_v2, _fun__sqlite3__wchar__stmt__void__int, logger);
			win32_fetch(sqlite3, sqlite3_reset, _fun__stmt__int, logger);
			win32_fetch(sqlite3, sqlite3_step, _fun__stmt__int, logger);
			win32_fetch(sqlite3, sqlite3_finalize, _fun__stmt__int, logger);

			win32_fetch(sqlite3, sqlite3_bind_parameter_count, _fun__stmt__int, logger);
			win32_fetch(sqlite3, sqlite3_clear_bindings, _fun__stmt__int, logger);
			win32_fetch(sqlite3, sqlite3_bind_null, _fun__stmt__int__int, logger);
			win32_fetch(sqlite3, sqlite3_bind_int, _fun__stmt__int__int32__int, logger);
			win32_fetch(sqlite3, sqlite3_bind_int64, _fun__stmt__int__int64__int, logger);
			win32_fetch(sqlite3, sqlite3_bind_double, _fun__stmt__int__real__int, logger);
			win32_fetch(sqlite3, sqlite3_bind_blob, _fun__stmt__int__blob__fptr__int, logger);
			win32_fetch(sqlite3, sqlite3_bind_text16, _fun__stmt__int__wchar__fptr__int, logger);

			win32_fetch(sqlite3, sqlite3_data_count, _fun__stmt__int, logger);
			win32_fetch(sqlite3, sqlite3_column_count, _fun__stmt__int, logger);
			win32_fetch(sqlite3, sqlite3_column_database_name16, _fun__stmt__int__wchar, logger);
			win32_fetch(sqlite3, sqlite3_column_table_name16, _fun__stmt__int__wchar, logger);
			win32_fetch(sqlite3, sqlite3_column_origin_name16, _fun__stmt__int__wchar, logger);
			win32_fetch(sqlite3, sqlite3_column_decltype16, _fun__stmt__int__wchar, logger);
			win32_fetch(sqlite3, sqlite3_column_bytes, _fun__stmt__int__int, logger);
			win32_fetch(sqlite3, sqlite3_column_bytes16, _fun__stmt__int__int, logger);
			win32_fetch(sqlite3, sqlite3_column_type, _fun__stmt__int__int, logger);
			win32_fetch(sqlite3, sqlite3_column_blob, _fun__stmt__int__char, logger);
			win32_fetch(sqlite3, sqlite3_column_text16, _fun__stmt__int__wchar, logger);
			win32_fetch(sqlite3, sqlite3_column_int, _fun__stmt__int__int32, logger);
			win32_fetch(sqlite3, sqlite3_column_int64, _fun__stmt__int__int64, logger);
			win32_fetch(sqlite3, sqlite3_column_double, _fun__stmt__int__real, logger);
			win32_fetch(sqlite3, sqlite3_sql, _fun__stmt__char, logger);

			win32_fetch(sqlite3, sqlite3_changes, _fun__sqlite3__int, logger);
			win32_fetch(sqlite3, sqlite3_total_changes, _fun__sqlite3__int, logger);
			win32_fetch(sqlite3, sqlite3_last_insert_rowid, _fun__sqlite3__long, logger);

			references += 1;
		}
	}
}

static void unload_sqlite3(Syslog* logger) {
	if (sqlite3 != nullptr) {
		references -= 1;

		if (references <= 0) {
			win32_unload_foreign_library(sqlite3, logger);
			sqlite3 = nullptr;
		}
	}
}

static const wchar_t* sqlite3_statement_description(sqlite3_stmt_t* stmt) {
	static size_t buffer_size = 64;
	static wchar_t* sql = new wchar_t[buffer_size];
	const char* raw = sqlite3_sql(stmt);
	Platform::String^ desc = nullptr;
	size_t len = strlen(raw);

	if (len >= buffer_size) {
		delete[] sql;
		buffer_size = len + 1;
		sql = new wchar_t[buffer_size];
	}

	sql[len] = '\0';
	for (size_t i = 0; i < strlen(raw); i++) {
		sql[i] = raw[i];
	}

	return sql;
}

/*************************************************************************************************/
int WarGrey::SCADA::sqlite3_default_trace_callback(unsigned int type, void* pCxt, void* P, void* X) {
	SQLite3* self = static_cast<SQLite3*>(pCxt);

	switch (type) {
	case SQLITE_TRACE_STMT: {
		self->get_logger()->log_message(Log::Debug, L"EXEC: %s", sqlite3_statement_description(P));
	}; break;
	case SQLITE_TRACE_PROFILE: { /* TODO */ }; break;
	default: /* no useful information */;
	}
	
	return 0;
}

SQLite3::SQLite3(const wchar_t* dbfile, Syslog* logger, sqlite3_trace_t xCallback)
	: IDBSystem((logger == nullptr) ? make_system_logger(DBMS::SQLite3.ToString()) : logger, DBMS::SQLite3) {
	const wchar_t* database = ((dbfile == nullptr) ? L":memory:" : dbfile);

	load_sqlite3(this->get_logger());

	if (sqlite3_open16(database, &this->db) != SQLITE_OK) {
		this->report_error(L"failed to connect to [%s]", database);
	} else {
		unsigned int uMask = SQLITE_TRACE_STMT | SQLITE_TRACE_PROFILE | SQLITE_TRACE_ROW | SQLITE_TRACE_CLOSE;

		sqlite3_trace_v2(this->db, uMask, xCallback, this);

		this->get_logger()->log_message(Log::Debug, L"connected to [%s]", database);
	}
}

SQLite3::~SQLite3() {
	unload_sqlite3(this->get_logger());

	if (sqlite3_close(this->db) != SQLITE_OK) {
		this->report_warning("");
	}
}

IVirtualSQL* SQLite3::new_sql_factory(TableColumnInfo* columns, size_t count) {
	return new VirtualSQLite3(columns, count, this->libversion());
}

int SQLite3::libversion() {
	return sqlite3_libversion_number();
}

IPreparedStatement* SQLite3::prepare(Platform::String^ raw) {
	const wchar_t* sql = raw->Data();
	sqlite3_stmt_t* prepared_stmt;
	SQLiteStatement* stmt = nullptr;

	if (sqlite3_prepare16_v2(this->db, sql, wstrlen(sql), &prepared_stmt, nullptr) == SQLITE_OK) {
		stmt = new SQLiteStatement(this, prepared_stmt);
	} else {
		this->report_error("prepare");
	}

	return stmt;
}

std::list<SQliteTableInfo> SQLite3::table_info(const wchar_t* name) {
	SQLiteStatement* pragma = static_cast<SQLiteStatement*>(IDBSystem::prepare(L"PRAGMA table_info = %s;", name));
	std::list<SQliteTableInfo> infos;
	
	while (pragma->step()) {
		SQliteTableInfo info;
		SQLiteDataType type = pragma->column_type(2);

		info.cid = pragma->column_int32(0);
		info.name = pragma->column_text(1);
		info.type = pragma->column_text(2);
		info.notnull = (pragma->column_int32(3) != 0);
		info.dflt_value = pragma->column_text(4);
		info.pk = pragma->column_int32(5);
		
		this->get_logger()->log_message(Log::Debug, L"Column[%d] %s[%s]: %s, %s, %d; %s",
			info.cid, info.name->Data(), type.ToString()->Data(), info.type->Data(),
			info.notnull.ToString()->Data(), info.pk, info.dflt_value->Data());

		infos.push_back(info);
	}

	delete pragma;

	return infos;
}

int SQLite3::changes(bool total) {
	int changes = 0;
	
	if (total) {
		changes = sqlite3_total_changes(this->db);
	} else {
		changes = sqlite3_changes(this->db);
	}

	return changes;
}

long SQLite3::last_insert_rowid() {
	return sqlite3_last_insert_rowid(this->db);
}

const wchar_t* SQLite3::get_last_error_message() {
	return sqlite3_errmsg16(this->db);
}

/*************************************************************************************************/
SQLiteStatement::SQLiteStatement(SQLite3* db, sqlite3_stmt_t* stmt)
	: IPreparedStatement(DBMS::SQLite3), stmt(stmt), master(db) {}

SQLiteStatement::~SQLiteStatement() {
	// sqlite3_finalize() returns error code of last statement execution, not of the finalization.
	// so there is no need to check the return value;
	sqlite3_finalize(this->stmt);
}

void SQLiteStatement::reset(bool clear) {
	// sqlite3_finalize() returns error code of last statement execution, not of the finalization.
	// so there is no need to check the return value;
	sqlite3_reset(this->stmt);

	if (clear) {
		this->clear_bindings();
	}
}

void SQLiteStatement::clear_bindings() {
	sqlite3_clear_bindings(this->stmt);
}

Platform::String^ SQLiteStatement::description() {
	return ref new Platform::String(sqlite3_statement_description(this->stmt));
}

bool SQLiteStatement::step(int* data_count, const wchar_t* error_src) {
	bool notdone = true;
	
	switch (sqlite3_step(this->stmt)) {
	case SQLITE_DONE: SET_BOX(data_count, 0); notdone = false; break;
	case SQLITE_ROW: SET_BOX(data_count, this->column_data_count()); break;
	default: this->master->report_error(error_src);
	}

	return notdone;
}

int SQLiteStatement::column_data_count() {
	return sqlite3_data_count(this->stmt);
}

Platform::String^ SQLiteStatement::column_database_name(unsigned int cid) {
	return ref new Platform::String(sqlite3_column_database_name16(this->stmt, cid));
}

Platform::String^ SQLiteStatement::column_table_name(unsigned int cid) {
	return ref new Platform::String(sqlite3_column_table_name16(this->stmt, cid));
}

Platform::String^ SQLiteStatement::column_name(unsigned int cid) {
	return ref new Platform::String(sqlite3_column_origin_name16(this->stmt, cid));
}

Platform::String^ SQLiteStatement::column_decltype(unsigned int cid) {
	return ref new Platform::String(sqlite3_column_decltype16(this->stmt, cid));
}

SQLiteDataType SQLiteStatement::column_type(unsigned int cid) {
	return static_cast<SQLiteDataType>(sqlite3_column_type(this->stmt, cid));
}

std::string SQLiteStatement::column_blob(unsigned int cid) {
	return std::string(sqlite3_column_blob(this->stmt, cid));
}

Platform::String^ SQLiteStatement::column_text(unsigned int cid) {
	return ref new Platform::String(sqlite3_column_text16(this->stmt, cid));
}

int32 SQLiteStatement::column_int32(unsigned int cid) {
	return sqlite3_column_int(this->stmt, cid);
}

int64 SQLiteStatement::column_int64(unsigned int cid) {
	return sqlite3_column_int64(this->stmt, cid);
}

double SQLiteStatement::column_double(unsigned int cid) {
	return sqlite3_column_double(this->stmt, cid);
}

unsigned int SQLiteStatement::parameter_count() {
	return sqlite3_bind_parameter_count(this->stmt);
}

void SQLiteStatement::bind_parameter(unsigned int pid) {
	if (sqlite3_bind_null(this->stmt, pid + 1) != SQLITE_OK) {
		this->master->report_error("bind_null");
	}
}

void SQLiteStatement::bind_parameter(unsigned int pid, int32 v) {
	if (sqlite3_bind_int(this->stmt, pid + 1, v) != SQLITE_OK) {
		this->master->report_error("bind_int32");
	}
}

void SQLiteStatement::bind_parameter(unsigned int pid, int64 v) {
	if (sqlite3_bind_int64(this->stmt, pid + 1, v) != SQLITE_OK) {
		this->master->report_error("bind_int64");
	}
}

void SQLiteStatement::bind_parameter(unsigned int pid, double v) {
	if (sqlite3_bind_double(this->stmt, pid + 1, v) != SQLITE_OK) {
		this->master->report_error("bind_double");
	}
}

void SQLiteStatement::bind_parameter(unsigned int pid, const char* v) {
	if (sqlite3_bind_blob(this->stmt, pid + 1, v, strlen(v), SQLITE_STATIC) != SQLITE_OK) {
		this->master->report_error("bind_blob");
	}
}

void SQLiteStatement::bind_parameter(unsigned int pid, const wchar_t* v) {
	if (sqlite3_bind_text16(this->stmt, pid + 1, v, wstrlen(v), SQLITE_STATIC) != SQLITE_OK) {
		this->master->report_error("bind_text");
	}
}