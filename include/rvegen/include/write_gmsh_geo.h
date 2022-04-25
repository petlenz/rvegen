#ifndef WRITE_GMSH_GEO_H
#define WRITE_GMSH_GEO_H

#include <iostream>

#include "circle.h"
#include "ellipse.h"
#include "cylinder.h"
#include "ellipsoid.h"
#include "box_bounding.h"

namespace rvegen {

template<typename _T>
class write_gmsh_geo
{
public:
    using value_typ = _T;
    using size_type = std::size_t;

    write_gmsh_geo() {}

    template<typename _RVE>
    inline void write_file(std::fstream & __file, _RVE const& __rve){
        const auto dimension{__rve.dimension()};
        if(dimension == 2){
            write_file_2D(__file, __rve);
        }else if (dimension == 3) {
            write_file_3D(__file, __rve);
        }
    }


    template<typename _RVE>
    inline void write_bounding_boxes(std::fstream & __file, _RVE const& __rve){
        const auto dimension{__rve.dimension()};
        if(dimension == 2){
            write_bounding_boxes_2D(__file, __rve);
        }else if (dimension == 3) {
            write_bounding_boxes_3D(__file, __rve);
        }
    }

private:
    template<typename _RVE>
    inline void write_bounding_boxes_2D(std::fstream & __file, _RVE const& __rve){
        const auto& shapes{__rve.shapes()};
        const size_type start = 5 + shapes.size();
        for(size_t i{0}; i<shapes.size(); ++i){
            const auto& box_ptr{*static_cast<rectangle_bounding<value_typ>*>(shapes[i].get()->bounding_box())};
            const auto& top = box_ptr.top_point();
            const auto& bottom = box_ptr.bottom_point();
            __file<<"Rectangle("<<start+i<<") = {"<<bottom[0]<<", "<<bottom[1]<<", 0, "<<top[0]-bottom[0]<<", "<<top[1]-bottom[1]<<"};"<<std::endl;
        }
    }

    template<typename _RVE>
    inline void write_bounding_boxes_3D(std::fstream & __file, _RVE const& __rve){
        const auto& shapes{__rve.shapes()};
        const size_type start = 2 + shapes.size();
        for(size_t i{0}; i<shapes.size(); ++i){
            const auto& box_ptr{*static_cast<box_bounding<value_typ>*>(shapes[i].get()->bounding_box())};
            const auto& top = box_ptr.top_point();
            const auto& bottom = box_ptr.bottom_point();
            __file<<"Box("<<start+i<<") = {"<<bottom[0]<<", "<<bottom[1]<<", "<<bottom[2]<<", "<<top[0]-bottom[0]<<", "<<top[1]-bottom[1]<<", "<<top[2]-bottom[2]<<"};"<<std::endl;
        }
    }

    template<typename _RVE>
    inline void write_file_2D(std::fstream & __file, _RVE const& __rve){
        const auto [x_box, y_box, z_box]{__rve.box()};
        const auto& shapes{__rve.shapes()};

        __file<<"SetFactory(\"OpenCASCADE\");"<<std::endl;
        __file<<"Rectangle(1) = {0, 0, 0, "<<x_box<<", "<<y_box<<", 0};"<<std::endl;

        //Rectangle + 4 lines ???? idk...
        const size_type start = 5;

        for(size_t i{0}; i<shapes.size(); ++i){
            if(dynamic_cast<circle<value_typ>*>(shapes[i].get())){
                const auto& data{*static_cast<circle<value_typ>*>(shapes[i].get())};
                __file<<"Circle("<<start+i<<") = {"<<data(0)<<","<<data(1)<<", 0, "<<data.radius()<<", 0, 2*Pi};"<<std::endl;
            }else if(dynamic_cast<ellipse<value_typ>*>(shapes[i].get())){
                const auto& data{*static_cast<ellipse<value_typ>*>(shapes[i].get())};
                __file<<"Ellipse("<<start+i<<") = {"<<data(0)<<","<<data(1)<<", 0, "<<data.radius_a()<<","<<data.radius_b()<<", 0, 2*Pi};"<<std::endl;
                __file<<"Rotate {{0, 0, 1}, {"<<data(0)<<", "<<data(1)<<", 0}, "<<data.rotation()<<"*Pi} {Curve{"<<start+i<<"};}"<<std::endl;
            }else{
                throw std::runtime_error("write_gmsh_geo::write_file_2D(): no matching shape type");
            }
        }


        for(size_t i{0};i<shapes.size();++i){
            __file<<"Curve Loop("<<i+start<<") = {"<<start+i<<"};"<<std::endl;
        }

        for(size_t i{0};i<shapes.size();++i){
            __file<<"Plane Surface("<<i+start<<") = {"<<start+i<<"};"<<std::endl;
        }

        for(size_t i{0};i<shapes.size();++i){
            __file<<"BooleanIntersection{ Surface{1}; }{ Surface{"<<i+start<<"}; Delete; }"<<std::endl;
        }

        for(size_t i{0};i<shapes.size();++i){
            __file<<"BooleanDifference{ Surface{1}; Delete; }{ Surface{"<<i+start<<"}; }"<<std::endl;
        }

        for(size_t i{0};i<shapes.size();++i){
            __file<<"BooleanDifference{ Surface{1}; Delete; }{ Surface{"<<i+start<<"}; }"<<std::endl;
        }
    }

    template<typename _RVE>
    inline void write_file_3D(std::fstream & __file, _RVE const& __rve){
        const auto [x_box, y_box, z_box]{__rve.box()};
        const auto& shapes{__rve.shapes()};

        __file<<"SetFactory(\"OpenCASCADE\");"<<std::endl;
        __file<<"Mesh.CharacteristicLengthMin = 0;"<<std::endl;
        __file<<"Mesh.CharacteristicLengthMax = 0.05;"<<std::endl;
        __file<<"Box(1) = {0, 0, 0,"<<x_box<<","<<y_box<<", "<<z_box<<"};"<<std::endl;

        //Rectangle + 4 lines ???? idk...
        const size_type start = 2;

        for(size_t i{0}; i<shapes.size(); ++i){
            if(dynamic_cast<cylinder<value_typ>*>(shapes[i].get())){
                const auto& data{*static_cast<cylinder<value_typ>*>(shapes[i].get())};
                __file<<"Cylinder("<<start+i<<") = {"<<data(0)<<","<<data(1)<<","<<data(2)<<", 0, 0, "<<data.height()<<", "<<data.radius()<<", 2*Pi};"<<std::endl;

            }else if(dynamic_cast<ellipsoid<value_typ>*>(shapes[i].get())){
                const auto& data{*static_cast<ellipsoid<value_typ>*>(shapes[i].get())};
                __file<<"Sphere("<<start+i<<") = {0, 0, 0, 1};"<<std::endl;
                __file<<"Dilate{{0, 0, 0}, {"<<data.radius_a()<<", "<<data.radius_b()<<", "<<data.radius_c()<<"}} {Volume{"<<start+i<<"};}"<<std::endl;
                __file<<"Rotate {{1, 0, 0}, {0, 0, 0}, "<<data.rotation_x()<<"} {Volume{"<<start+i<<"};}"<<std::endl;
                __file<<"Rotate {{0, 1, 0}, {0, 0, 0}, "<<data.rotation_y()<<"} {Volume{"<<start+i<<"};}"<<std::endl;
                __file<<"Rotate {{0, 0, 1}, {0, 0, 0}, "<<data.rotation_z()<<"} {Volume{"<<start+i<<"};}"<<std::endl;
                __file<<"Translate{"<<data(0)<<", "<<data(1)<<", "<<data(2)<<"} {Volume{"<<start+i<<"};}"<<std::endl;
            }else{
                throw std::runtime_error("write_gmsh_geo::write_file_3D(): no matching shape type");
            }
        }

        for(size_t i{0};i<shapes.size();++i){
            __file<<"BooleanIntersection{ Volume{1}; }{ Volume{"<<i+2<<"}; Delete; }"<<std::endl;
        }

        for(size_t i{0};i<shapes.size();++i){
            __file<<"BooleanDifference{ Volume{1}; Delete; }{ Volume{"<<i+2<<"}; }"<<std::endl;
        }
    }
};

}

#endif // WRITE_GMSH_GEO_H
