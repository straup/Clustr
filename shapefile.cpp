#include "shapefile.h"
#include <algorithm>
#include <iterator>
#include <vector>
#include <stdexcept>
#include <stdlib.h>
#include <cassert>

using namespace std;

namespace Clustr {
    Shapefile::Shapefile (string const filename, GeometryType layer_type, bool append)
    {
        name = filename.substr(0, filename.find_last_of('.'));
        geom_type = layer_type;
        layer = NULL;

        OGRRegisterAll();
        driver = OGRGetDriverByName(driver_name.c_str());
        if( driver == NULL ) {
            throw runtime_error( driver_name + " driver not available." );
        }

        ds = OGR_Dr_Open(driver, filename.c_str(), NULL);
        if (ds != NULL && !append) {
            OGR_DS_Destroy(ds);
            unlink(filename.c_str());
        }
        if (ds == NULL || !append) {
            ds = OGR_Dr_CreateDataSource(driver, filename.c_str(), NULL);
            if( ds == NULL ) {
                throw runtime_error(filename + " datasource creation failed.");
            }
        }

        layer = OGR_DS_GetLayer(ds, 0);
        if (layer != NULL && !append) {
            if (OGR_DS_DeleteLayer(ds, 0) != OGRERR_NONE) {
                throw runtime_error(filename + " existing layer can't be deleted.");
            }
        }
        if (layer == NULL) {
            layer = OGR_DS_CreateLayer(ds, name.c_str(), NULL, layer_type, NULL);
            if( layer == NULL ) {
                throw runtime_error(filename + " layer creation failed.");
            }
        }
    }

    void Shapefile::add_field (const char *field_name, OGRFieldType type, int width, int precision) {
        OGRFieldDefnH fld = OGR_Fld_Create(field_name, type);
        OGR_Fld_SetWidth(fld, width);
        OGR_Fld_SetPrecision(fld, precision);
        if( OGR_L_CreateField( layer, fld, 1 ) != OGRERR_NONE ) {
            throw runtime_error(name + " field creation failed.");
        }
        OGR_Fld_Destroy(fld);
    }

    void Shapefile::add_feature (const Feature &feature) {
        if( OGR_L_CreateFeature( layer, feature.handle() ) != OGRERR_NONE ) {
            throw runtime_error(name + " feature creation failed.");
        }
    }

    Feature::Feature (const Shapefile &shape) {
        geom_type = shape.geometry_type();
        defn = shape.definition();
        feat = OGR_F_Create(defn);
    }

    int Feature::index (const char *name) {
        int i = OGR_FD_GetFieldIndex(defn, name);
        if (i == -1) {
            throw runtime_error(string(name) + " field not found on feature.");
        }
        return i;
    }

    void Feature::set (const Geometry &geom) {
        assert(geom.geometry_type() == geom_type);
        if (OGR_F_SetGeometry(feat, geom.handle()) != OGRERR_NONE) {
            throw runtime_error("Couldn't set geometry on feature.");
        }
    }

    void Feature::set (const char *name, const char *val) {
        OGR_F_SetFieldString(feat, index(name), val);
    }

    void Feature::set (const char *name, long int val) {
        OGR_F_SetFieldInteger(feat, index(name), val);
    }

    void Feature::set (const char *name, double val) {
        OGR_F_SetFieldDouble(feat, index(name), val);
    }

    Geometry::Geometry(GeometryType geom_type) : geom_type(geom_type) {
        outer = inner = OGR_G_CreateGeometry(geom_type);
        if (geom_type == wkbPolygon) add_ring();
    }

    void Geometry::add_ring (void) {
        assert(geom_type == wkbPolygon);
        OGRGeometryH ring = OGR_G_CreateGeometry(wkbLinearRing);
        if (OGR_G_AddGeometryDirectly(outer, ring) != OGRERR_NONE) {
            throw runtime_error("Couldn't add ring to polygon");
        }
        inner = ring;
    }

    void Geometry::push_back (double x, double y) {
        OGR_G_AddPoint_2D(inner, x, y);
    }
    
    void Geometry::push_back (const Point& pt) {
        OGR_G_AddPoint_2D(inner, pt.x(), pt.y());
    }

    template <typename Iterator>
    void Geometry::insert (Iterator begin, Iterator end) {
        for (Iterator it = begin; it != end; it++) {
            push_back(*it);
        }
        if (geom_type == wkbPolygon || geom_type == wkbMultiPolygon) {
            push_back(*begin);
        }
    }

    /* necessary to keep the linker from blowing up */
    template void Geometry::insert<Ring::iterator>(Ring::iterator, Ring::iterator);

    void Geometry::insert_rings (Polygon::iterator begin,  Polygon::iterator end) {
        assert(geom_type == wkbPolygon);
        for (Polygon::iterator it = begin; it != end; it++) {
            if (it != begin) add_ring();
            insert(it->begin(), it->end());
        }
    }

    void MultiGeometry::push_back (const Geometry& geom) {
        if (OGR_G_AddGeometry(outer, geom.handle()) != OGRERR_NONE) {
            throw runtime_error("Couldn't add geometry to multigeometry");
        }
    }

    void MultiGeometry::insert (Polygon::iterator begin, Polygon::iterator end) {
        Geometry geom(wkbPolygon);
        geom.insert_rings(begin, end);
        push_back(geom);
    }
}

