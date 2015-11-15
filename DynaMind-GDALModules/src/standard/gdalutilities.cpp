#include "gdalutilities.h"
#include "dm.h"

#include <ogrsf_frmts.h>

namespace DM {

int GDALUtilities::DMToOGRGeometry(int dm_geometry)
{
	int type = wkbNone;
	switch(dm_geometry)
	{
	case DM::NODE:
		type = wkbPoint;
		break;
	case DM::FACE:
		type = wkbPolygon;
		break;
	case DM::EDGE:
		type = wkbLineString;
		break;
	case DM::COMPONENT:
		type = wkbNone;
		break;
	}
	return type;
}

int GDALUtilities::OGRtoDMGeometry(OGRFeatureDefn *def)
{
	int type = -1;
	OGRwkbGeometryType ogrType = def->GetGeomType();
	std::string strType = OGRGeometryTypeToName(ogrType);
	switch(wkbFlatten(ogrType))
	{
	case wkbNone:
		type = DM::COMPONENT;
		break;
	case wkbPoint:
		type = DM::NODE;
		break;
	case wkbMultiPoint:
		type = DM::NODE;
		break;
	case wkbPolygon:
		type = DM::FACE;
		break;
	case wkbMultiPolygon:
		type = DM::FACE;
		break;
	case wkbLineString:
		type = DM::EDGE;
		break;
	case wkbMultiLineString:
		type = DM::EDGE;
		break;
	case wkbUnknown:
		type = DM::COMPONENT;
		break;
	default:
		DM::Logger(DM::Warning) << "Geometry type not implemented: " << strType;
		return -1;
	}
	DM::Logger(DM::Debug) << "Found: Geometry type " << strType;
	return type;
}

DM::Attribute::AttributeType GDALUtilities::OGRToDMAttribute(OGRFieldDefn * fdef) {
	DM::Attribute::AttributeType type = DM::Attribute::NOTYPE;
	switch (fdef->GetType()) {
	case OFTInteger:
		type = DM::Attribute::INT;
		break;
	case OFTReal:
		type = DM::Attribute::DOUBLE;
		break;
	case OFTString:
		type = DM::Attribute::STRING;
		break;
	case OFTDate:
		type = DM::Attribute::DATE;
		break;
	default:
		DM::Logger(DM::Warning) << "Type not supported attributed " << fdef->GetNameRef() << " not loaded";
		break;
	}
	return type;
}

DM::Attribute::AttributeType DM::GDALUtilities::AttributeTypeStringToType(const std::string &type)
{

	if (type == "INTEGER")
		return DM::Attribute::INT;
	if (type == "DOUBLE")
		return DM::Attribute::DOUBLE;
	if (type == "STRINGVECTOR")
		return DM::Attribute::STRINGVECTOR;
	if (type == "DOUBLEVECTOR")
		return DM::Attribute::DOUBLEVECTOR;
	if (type == "STRING")
		return DM::Attribute::STRING;
	if (type == "DATE")
		return DM::Attribute::DATE;
	if (type == "LINK")
		return DM::Attribute::LINK;
	return DM::Attribute::NOTYPE;


}
}

