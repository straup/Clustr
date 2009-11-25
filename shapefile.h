#include "clustr.h"
#include <ogr_api.h>
#include <string>

namespace Clustr {
    class Shapefile;
    const std::string driver_name = "ESRI Shapefile";
    typedef OGRwkbGeometryType GeometryType;

    class Geometry {
      protected:
        OGRGeometryH inner;
        OGRGeometryH outer;
        GeometryType geom_type;
      public:
        Geometry (GeometryType geom_type);
        ~Geometry () { OGR_G_DestroyGeometry(outer); };
        OGRGeometryH handle () const { return outer; };
        GeometryType geometry_type (void) const { return geom_type; };
        void add_ring (void);
        void push_back (double x, double y);
        void push_back (const Point& pt);
        template <typename Iterator>
        void insert (Iterator begin, Iterator end);
        void insert_rings (Polygon::iterator begin, Polygon::iterator end);
    };

    class MultiGeometry : public Geometry {
      public:
        MultiGeometry (GeometryType geom_type) : Geometry(geom_type) {};
        OGRGeometryH handle (void) const { return outer; };
        void push_back (const Geometry& geom);
        void insert (Polygon::iterator begin, Polygon::iterator end);
    };
   
    class Feature {
        OGRFeatureH feat;
        OGRFeatureDefnH defn;
        GeometryType geom_type;
        int index (const char *name);
      public:
        Feature (const Shapefile &shape);
        ~Feature () { OGR_F_Destroy(feat); };
        OGRFeatureH handle () const { return feat; };
        void set (const char *name, const char *val);
        void set (const char *name, long int val);
        void set (const char *name, double val);
        void set (const Geometry &geom);
    }; 

    class Shapefile {
        OGRSFDriverH driver;
        OGRDataSourceH ds;
        OGRLayerH layer;
        GeometryType geom_type;
        std::string name;
      public:
        Shapefile (std::string const filename, GeometryType layer_type, bool append=false);
        ~Shapefile () { OGR_DS_Destroy( ds ); };
        OGRFeatureDefnH definition (void) const { return OGR_L_GetLayerDefn(layer); };
        GeometryType geometry_type (void) const { return geom_type; };
        void add_field (const char *name, OGRFieldType type, int width, int precision=0);
        void add_feature (const Feature &feature);
    };
};
