#include "my_extension.h"

#include <sqlite3ext.h>
#include <QByteArray>
#include <QString>
#include <iostream>
//Methods
#include "cgalgeometryhelper.h"
#include  "my_extension.h"
extern "C"{

SQLITE_EXTENSION_INIT1


#ifdef _WIN32
__declspec(dllexport)
#endif

/* calculate percentage filled */
void poly_percentage_filled(sqlite3_context *context, int argc, sqlite3_value **argv) {
	const unsigned char * geometry = sqlite3_value_text(argv[0]);
	double res = CGALGeometryHelper::percentageFilled(geometry);
	if (res < 0) {
		sqlite3_result_null(context);
		return;
	}
	sqlite3_result_double(context, res);        // save the result
}

/* calculate aspect ratio */
void polygon_aspect_ratio(sqlite3_context *context, int argc, sqlite3_value **argv) {
	const unsigned char * geometry = sqlite3_value_text(argv[0]);
	double res = CGALGeometryHelper::aspectRationBB(geometry);
	if (res < 0) {
		sqlite3_result_null(context);
		return;
	}
	sqlite3_result_double(context, res);        // save the result
}


/* calculate aspect ratio */
void mutiply_vector(sqlite3_context *context, int argc, sqlite3_value **argv) {
	const unsigned char * vec = sqlite3_value_text(argv[0]);
	double value = sqlite3_value_double(argv[1]);


	std::string ress = my_extension::mutiply_vector(vec, value);
	sqlite3_result_text(context, ress.c_str(),ress.size(),SQLITE_TRANSIENT);

	//sqlite3_result_double(context, res);        // save the result
}

int sqlite3_dmsqliteplugin_init(
		sqlite3 *db,
		char **pzErrMsg,
		const sqlite3_api_routines *pApi
		){
	int rc = SQLITE_OK;
	SQLITE_EXTENSION_INIT2(pApi);


	sqlite3_create_function(db, "dm_poly_percentage_filled", 1, SQLITE_UTF8, 0, &poly_percentage_filled, 0, 0);
	sqlite3_create_function(db, "dm_poly_aspect_ratio", 1, SQLITE_UTF8, 0, &polygon_aspect_ratio, 0, 0);
	sqlite3_create_function(db, "dm_mutiply_vector", 2, SQLITE_UTF8, 0, &mutiply_vector, 0, 0);


	return rc;
}

}
