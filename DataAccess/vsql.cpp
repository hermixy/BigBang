#include "vsql.hpp"

using namespace WarGrey::SCADA;

IVirtualSQL::IVirtualSQL(TableColumnInfo* columns, size_t count) {
	this->columns = new TableColumnInfo[count];

	for (size_t idx = 0; idx < count; idx++) {
		this->columns[idx] = columns[idx];
	}
}

IVirtualSQL::~IVirtualSQL() {
	delete[] this->columns;
}

bool WarGrey::SCADA::db_column_primary(TableColumnInfo& info) {
	return ((info.flags & DB_PRIMARY_KEY) == DB_PRIMARY_KEY);
}

bool WarGrey::SCADA::db_column_notnull(TableColumnInfo& info) {
	return ((info.flags & DB_NOT_NULL) == DB_NOT_NULL);
}

bool WarGrey::SCADA::db_column_unique(TableColumnInfo& info) {
	return ((info.flags & DB_UNIQUE) == DB_UNIQUE);
}
