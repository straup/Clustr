# include <numeric>
# include <cmath>
# include "clustr.h"

using namespace Clustr;
//using std::tr1::sqrt;

void Ring::push_back (const Point &p2) {
    if (!is_empty()) {
        Point p1 = vertex(size()-1);
        coord_type fx = scale_x((p1.y() + p2.y())/2.0),
                   dx = p2.x() - p1.x(),
                   dy = p2.y() - p1.y();
        perimeter_ += sqrt(dx*dx*fx*fx + dy*dy);
        area_ += fx*fx*(p1.x() * p2.y() - p2.x() * p1.y()) / 2;
    }
    this->Ring_base::push_back(p2);
}

void Polygon::push_back (Ring &ring) {
    if ((empty() && !ring.is_ccw()) ||
        (!empty() && ring.is_ccw()))
        ring.reverse_orientation();
    this->Polygon_base::push_back(ring); 
}

coord_type Polygon::area (void) {
    coord_type total = 0;
    for (Polygon::iterator ring = begin(); ring != end(); ring++) {
        total += ring->area();
    }
    return total;
}

coord_type Polygon::perimeter (void) {
    coord_type total = 0;
    for (Polygon::iterator ring = begin(); ring != end(); ring++) {
        total += ring->perimeter();
    }
    return total;
}
