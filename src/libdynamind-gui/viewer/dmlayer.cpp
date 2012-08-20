#include "dmlayer.h"

#include "dmcomponent.h"
#include "dmnode.h"
#include "dmface.h"
#include "dmedge.h"
#include "dmattribute.h"
#include "dmsystemiterators.h"

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/partition_2.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Partition_traits_2<K>                         Traits;
typedef Traits::Point_2                                     Point_2;
typedef Traits::Polygon_2                                   Polygon_2;
typedef std::vector<Polygon_2>                              Polygon_list;

#include <QImage>
#include <QPainter>
#include <cassert>

namespace DM {

template<int SD_GL_PRIMITIVE>
struct SimpleDrawer {

    void operator()(DM::System *s, DM::View v, void *f_e, DM::Node *n, iterator_pos pos) {
        if (pos == before) {
            glBegin(SD_GL_PRIMITIVE);
            return;
        }
        if (pos == after) {
            glEnd();
            return;
        }
        glColor3f(0, 0, 0);
        glVertex3dv(n->get());
    }
};


struct TesselatedFaceDrawer {
    double height_scale;
    double attr_span;
    const Layer &l;
    Polygon_2 polygon;
    double current_height;
    double current_tex[2];
    int name_start;
    
    TesselatedFaceDrawer(const Layer &l)
        : l(l), height_scale(0.0), name_start(l.getNameStart()) {
        if (l.getAttribute() == "") {
            return;
        }
        if (l.getHeightInterpretation() > 0.0) {
            const ViewMetaData &vmd = l.getViewMetaData();
            this->height_scale = 1.0/vmd.attr_max*vmd.radius() * l.getHeightInterpretation();
            this->attr_span = vmd.attr_max - vmd.attr_min;
        }
        
    }
    
    void operator()(DM::System *s, DM::View v, DM::Face *f, DM::Node *n, iterator_pos pos) {
        if (pos == after) {
            render();
            polygon.clear();
            name_start++;
            return;
        }
        if (pos == before) {
            if (height_scale > 0) {
                Attribute *a = f->getAttribute(l.getAttribute());
                if (a->getType() == Attribute::DOUBLE) {
                    current_height = f->getAttribute(l.getAttribute())->getDouble() * height_scale;
                } else {
                    double attr_value = a->getDoubleVector()[l.getAttributeVectorName()];
                    current_height = attr_value * height_scale;
                }
            }
            if (glIsTexture(l.getColorInterpretation())) {
                const ViewMetaData &vmd = l.getViewMetaData();
                Attribute *a = f->getAttribute(l.getAttribute());
                if (a->getType() == Attribute::DOUBLE) {
                    current_tex[0] = (a->getDouble() - vmd.attr_min) / attr_span;
                } else {
                    current_tex[0] = (a->getDoubleVector()[l.getAttributeVectorName()] - vmd.attr_min) / attr_span;
                }
                current_tex[1] = 0.5;
            } else {
                current_tex[0] = current_tex[1] = 0.0;
            }
            return;
        }
        
        if (pos != in_between) return;
        
        Point_2 p(n->getX(), n->getY());
        polygon.push_back(p);
    }
    
    void render() {
        if (glIsTexture(l.getColorInterpretation())) {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, l.getColorInterpretation());
        }
        assert(glGetError() == GL_NO_ERROR);
        if (polygon.is_clockwise_oriented())
            polygon.reverse_orientation();
        Polygon_list tesselated;
        CGAL::y_monotone_partition_2(polygon.vertices_begin(), polygon.vertices_end(),
                                     std::back_inserter(tesselated));
        foreach(Polygon_2 poly, tesselated) {
            glBegin(GL_POLYGON);
            
            foreach(Point_2 p, poly.container()) {
                if (glIsTexture(l.getColorInterpretation())) {
                    glColor3f(1.0, 1.0, 1.0);
                    glTexCoord2dv(current_tex);
                } else {
                    glColor3f(0.0, 0.0, 0.0);
                }
                glVertex3d(p.x(), p.y(), current_height);
            }
            
            glEnd();
        }
        if (glIsTexture(l.getColorInterpretation())) glDisable(GL_TEXTURE_2D);
    }
};


Layer::Layer(System *s, View v, const std::string &a) 
    : system(s), view(v), 
      attribute(a), vmd(a),
      attribute_vector_name(0), texture(-1),
      scale_height(-1) {
}

void Layer::draw() {
    if (lists.size() <= attribute_vector_name) {
        lists.resize(attribute_vector_name+1, -1);
    }
    if (!glIsList(lists[attribute_vector_name])) {
        lists[attribute_vector_name] = glGenLists(1);
        glNewList(lists[attribute_vector_name], GL_COMPILE);
        
        if (view.getType() == DM::FACE) {
            TesselatedFaceDrawer drawer(*this);
            iterate_faces(system, view, drawer);
        }
        if (view.getType() == DM::EDGE) {
            SimpleDrawer<GL_LINES> drawer;
            iterate_edges(system, view, drawer);
        }
        if (view.getType() == DM::NODE) {
            SimpleDrawer<GL_POINTS> drawer;
            iterate_nodes(system, view, drawer);
        }
        
        glEndList();
    }
    
    glPushMatrix();
    glTranslated(x_off, y_off, z_off);
    assert(glIsList(lists[attribute_vector_name]));
    glCallList(lists[attribute_vector_name]);
    glPopMatrix();
}

void Layer::drawWithNames() {
    draw();
}

void Layer::systemChanged() {
    vmd = ViewMetaData(attribute);

    if (view.getType() == DM::FACE) {
        iterate_faces(system, view, vmd);
    }
    if (view.getType() == DM::EDGE) {
        iterate_edges(system, view, vmd);
    }
    if (view.getType() == DM::NODE) {
        iterate_nodes(system, view, vmd);
    }
    
    foreach (GLuint list, lists) {
        if (glIsList(list)) {
            glDeleteLists(list, 1);
        }
    }
}

} // namespace DM
