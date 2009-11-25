#ifndef CLUSTR_H
#define CLUSTR_H

#define VERSION     "0.1"
#define AUTHOR      "Schuyler Erle <schuyler@nocat.net>"
#define COPYRIGHT   "(c) 2008 Yahoo!, Inc."

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Filtered_kernel.h>
#include <CGAL/algorithm.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/Alpha_shape_2.h>
#include <CGAL/Polygon_2.h>

#include <cmath>
#include <vector>
#define KM_PER_DEGREE (40075.16/360)

namespace Clustr {
    typedef float coord_type;

    typedef CGAL::Simple_cartesian<coord_type>  SC;
    typedef CGAL::Filtered_kernel<SC> K;

    typedef K::Point_2  Point;
    typedef K::Segment_2  Segment;
    typedef K::Direction_2  Direction;

    typedef CGAL::Alpha_shape_vertex_base_2<K> Vb;
    typedef CGAL::Alpha_shape_face_base_2<K>  Fb;
    typedef CGAL::Triangulation_data_structure_2<Vb,Fb> Tds;
    typedef CGAL::Delaunay_triangulation_2<K,Tds> Triangulation_2;

    template < class Gt, class Vb = CGAL::Alpha_shape_vertex_base_2<Gt> >
    class Vertex_counter_base : public Vb {
        typedef typename Vb::Triangulation_data_structure  TDS;
        unsigned int counter;
      public:
        typedef TDS                             Triangulation_data_structure;
        typedef typename TDS::Vertex_handle     Vertex_handle;
        typedef typename TDS::Face_handle       Face_handle;
 
        typedef typename Gt::FT                 Coord_type;
        typedef std::pair< Coord_type, Coord_type >    Interval2;
        typedef typename Vb::Point              Point;

        template < typename TDS2 >
        struct Rebind_TDS {
            typedef typename Vb::template Rebind_TDS<TDS2>::Other Vb2;
            typedef Vertex_counter_base<Gt,Vb2> Other;
        };

        Vertex_counter_base() : Vb(), counter(0) {};
        Vertex_counter_base(const Point & p) : Vb(p), counter(0) {};
        Vertex_counter_base(const Point & p, Face_handle f) : Vb(f,p), counter(0) {};
        Vertex_counter_base(Face_handle f) : Vb(f), counter(0) {};

        unsigned int operator++ (void) { return ++counter; };
        unsigned int operator++ (int) { return counter++; };
        unsigned int count (void) { return counter; };
    };
    typedef CGAL::Triangulation_data_structure_2<Vertex_counter_base<K>,Fb> Tds_with_counter;
    typedef CGAL::Delaunay_triangulation_2<K,Tds_with_counter> Triangulation_with_counter;
    typedef CGAL::Alpha_shape_2<Triangulation_with_counter> Alpha_shape;

    typedef Alpha_shape::Face  Face;
    typedef Alpha_shape::Vertex Vertex;
    typedef Alpha_shape::Edge Edge;
    typedef Alpha_shape::Face_handle  Face_handle;
    typedef Alpha_shape::Vertex_handle Vertex_handle;

    typedef Alpha_shape::Face_iterator  Face_iterator;
    typedef Alpha_shape::Vertex_iterator  Vertex_iterator;
    typedef Alpha_shape::Edge_circulator  Edge_circulator;
    typedef Alpha_shape::Face_circulator  Face_circulator;

    typedef Alpha_shape::Alpha_iterator Alpha_iterator;
    typedef Alpha_shape::Alpha_shape_vertices_iterator Alpha_shape_vertices_iterator;

    class Mesh : public Alpha_shape {
      public:
        class vertex_circulator;
            
        class component {
            typedef Alpha_shape_vertices_iterator Mesh_iterator;
            Mesh *mesh;
            Mesh_iterator base;
            Mesh_iterator end;
          public:
            component (Mesh &mesh, Mesh_iterator b) : mesh(&mesh), base(b) {};
            component (Mesh &mesh) : mesh(&mesh) {
                base = mesh.alpha_shape_vertices_begin();
                end  = mesh.alpha_shape_vertices_end();
            };
            bool operator!=(const component &other) { return mesh!=other.mesh || base!=other.base; };  
            bool operator==(const component &other) { return mesh==other.mesh && base==other.base; }; 
            vertex_circulator operator*() { return vertex_circulator(*this); };
            component& operator++ (int) { return ++(*this); };
            component& operator++() { 
                if (base != end) {
                    do {++base;} while ((*base)->count() && base != end);
                }
                return *this;
            }; 
            friend class vertex_circulator;
        };

        class vertex_circulator {
            component *ring;
            Vertex_handle vertex;
            Edge previous;
            
            Vertex_handle target (const Edge &e);
            Vertex_handle origin (const Edge &e);
          public:
            vertex_circulator (component &c): ring(&c), vertex(*c.base), previous() {};
            Vertex_handle operator*() const { return vertex; };
            Vertex_handle operator->() const { return vertex; };
            vertex_circulator& operator++ (int) { return ++(*this); };
            vertex_circulator& operator++();
        };

      private:
        component end_component;
      public:
        Mesh(coord_type alpha = coord_type(0)) : Alpha_shape(alpha, Alpha_shape::REGULARIZED),
                                                 end_component(*this, alpha_shape_vertices_end()) {};

        template <class InputIterator>
        Mesh(const InputIterator& first, const InputIterator& last,
                const coord_type& alpha = coord_type(0))
                : Alpha_shape(first, last, alpha, Alpha_shape::REGULARIZED),
                  end_component(*this, alpha_shape_vertices_end()) {};

        component components_begin () { return component(*this); };
        component components_end () { return end_component; };
    };

    /* BIG ASSUMPTION: Point<K> is actually a lon/lat point
     * -- this underlies all of the following distance and
     * area calculations. */

    //using std::tr1::cos;
    //using std::tr1::acos;

    typedef CGAL::Polygon_2<K> Ring_base;
    class Ring : public Ring_base {
        coord_type area_;
        coord_type perimeter_;
        coord_type scale_x(coord_type lat) { return cos(lat*acos(-1.0)/180); };
      public:
        typedef Ring_base::Vertex_iterator iterator;
        Ring() : Ring_base(), area_(0), perimeter_(0) {};
        void reverse_orientation (void) { area_ *= -1.0; this->Ring_base::reverse_orientation(); };
        bool is_ccw (void) { return area_ >= 0; };
        coord_type area (void) { return area_*KM_PER_DEGREE*KM_PER_DEGREE; };
        coord_type perimeter (void) { return perimeter_*KM_PER_DEGREE; };
        void push_back (const Point &p);
        iterator begin (void) { return vertices_begin(); };
        iterator end (void) { return vertices_end(); };
    };

    typedef std::vector<Ring> Polygon_base;
    class Polygon : public Polygon_base {
      public:
        void push_back (Ring &ring);
        coord_type area (void);
        coord_type perimeter (void);
    };

    struct Config {
        std::string in_file;
        std::string out_file;
        coord_type alpha;
        bool points_only;
        bool verbose;

        Config() {  
            in_file = "-"; 
            out_file = "clustr.shp";
            alpha = 0;
            points_only = false;   
            verbose = false;   
        };
    };
};

#endif /* CLUSTR_H */
