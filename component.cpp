#include <iostream>
#include "clustr.h"

using namespace Clustr;

Vertex_handle Mesh::vertex_circulator::target (const Edge &e) {
    return e.first->vertex(Mesh::ccw(e.second));
}

Vertex_handle Mesh::vertex_circulator::origin (const Edge &e) {
    return e.first->vertex(Mesh::cw(e.second));
}

Mesh::vertex_circulator&
Mesh::vertex_circulator::operator++(void)
{
    Mesh *mesh = ring->mesh;
    Edge_circulator edge = mesh->incident_edges(vertex), edge0 = edge;
    Edge selected;
    int min_count = ~0;

    if (previous.first == NULL) previous = *edge;
    Direction angle_in(mesh->segment(previous)), angle_out;

    (*vertex)++;
    do {
        if (mesh->classify(*edge) == Mesh::REGULAR) {
            Vertex_handle next_v = target(*edge);
            if (next_v == origin(previous)) continue;
            if (next_v->count() <= min_count) {
                Direction next_angle = mesh->segment(*edge).direction();
                if (next_v->count() == min_count && 
                    !next_angle.counterclockwise_in_between(angle_in,angle_out))
                    continue;
                selected  = *edge;
                angle_out = mesh->segment(selected).direction();
                min_count = next_v->count();
            }
        } 
    } while (++edge != edge0);
    //std::cerr << mesh->segment(previous) << " -> " << mesh->segment(selected) << std::endl;
    previous = selected;
    vertex = target(selected);
    return *this;
}
