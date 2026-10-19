#include "../support/folia_test.hpp"
import folia.math.util.bbox;
import folia.math.output.common.placement.radical;

using namespace boost::ut;
using folia::math::BBox;
using namespace folia::math::output::common::placement;
namespace {
BBox make_box(double w,double h,double d,double scale,double rscale,double L,double R,double ic,double oc,double sk,double dx,char const* pwidth) { BBox b; b.w=w;b.h=h;b.d=d;b.scale=scale;b.rscale=rscale;b.L=L;b.R=R;b.ic=ic;b.oc=oc;b.sk=sk;b.dx=dx;b.pwidth=pwidth;return b; }
void check_box(BBox const& a,BBox const& e) { expect(a.w==e.w);expect(a.h==e.h);expect(a.d==e.d);expect(a.scale==e.scale);expect(a.rscale==e.rscale);expect(a.L==e.L);expect(a.R==e.R);expect(a.ic==e.ic);expect(a.oc==e.oc);expect(a.sk==e.sk);expect(a.dx==e.dx);expect(a.pwidth==e.pwidth); }
void check_font(RadicalFontParameters const& a,double rule,double surd,double xheight) { expect(a.rule_thickness==rule);expect(a.surd_height==surd);expect(a.x_height==xheight); }
void check_point(RadicalPoint const& a,double x,double y) { expect(a.x==x);expect(a.y==y); }
void check_rect(RadicalRect const& a,double x,double y,double w,double h) { expect(a.x==x);expect(a.y==y);expect(a.width==w);expect(a.height==h); }
}
suite native_radical_source_pairs = [] {
    "all_generated_source_radical_pairs"_test = [] {
        { // Source-paired radical case 1: sqrt, inline, size=-1, wrapper=0.75
            RadicalInput input{make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"),{},RadicalFontParameters{0.06,0.12,0.43},false,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==false);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==1.015);expect(request.stretch_height==0.835);expect(request.stretch_depth==0.18);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.46, 0.83, 0.17, 1, 0.9, 0.07, 0.02, 0.02, 0.01, 0.03, 0.01,""),-1);
            check_box(layout.bbox,make_box(1.66, 0.955, 0.18, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.06);expect(layout.q==0.135);expect(layout.required_surd_height==1.015);
            expect(layout.measured_height==0.835);expect(layout.placed_height==0.865);expect(layout.placed_surd_height==0.09);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.03500000000000003);
            check_point(layout.base_origin,0.46,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.46,0.775,1.2,0.09);
        }
        { // Source-paired radical case 2: root, inline, size=-1, wrapper=0.75
            RadicalInput input{make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"),make_box(0.3, 0.39, 0.11, 0.8, 1.3, 0.05, 0.02, 0.01, 0.02, 0.03, 0.04,"21%"),RadicalFontParameters{0.06,0.12,0.43},false,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.3, 0.39, 0.11, 0.8, 1.3, 0.05, 0.02, 0.01, 0.02, 0.03, 0.04,"21%"));
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==false);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==1.015);expect(request.stretch_height==0.835);expect(request.stretch_depth==0.18);expect(request.root_width==0.79);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.46, 0.83, 0.17, 1, 0.9, 0.07, 0.02, 0.02, 0.01, 0.03, 0.01,""),-1);
            check_box(layout.bbox,make_box(1.8199999999999998, 2.385, 0.18, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.06);expect(layout.q==0.135);expect(layout.required_surd_height==1.015);
            expect(layout.measured_height==0.835);expect(layout.placed_height==0.865);expect(layout.placed_surd_height==0.09);
            expect(layout.root_offset==0.16);expect(layout.root_height==1.908);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0.16,0.03500000000000003);
            check_point(layout.base_origin,0.62,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0,1.908);else expect(false);
            check_rect(layout.overbar,0.62,0.775,1.2,0.09);
        }
        { // Source-paired radical case 3: sqrt, inline, size=2, wrapper=0.75
            RadicalInput input{make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"),{},RadicalFontParameters{0.06,0.12,0.43},false,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==false);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==1.015);expect(request.stretch_height==0.835);expect(request.stretch_depth==0.18);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.46, 0.83, 0.17, 1, 0.9, 0.07, 0.02, 0.02, 0.01, 0.03, 0.01,""),2);
            check_box(layout.bbox,make_box(1.66, 0.955, 0.18, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.06);expect(layout.q==0.135);expect(layout.required_surd_height==1.015);
            expect(layout.measured_height==0.835);expect(layout.placed_height==0.865);expect(layout.placed_surd_height==0.09);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.03500000000000003);
            check_point(layout.base_origin,0.46,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.46,0.775,1.2,0.09);
        }
        { // Source-paired radical case 4: root, inline, size=2, wrapper=0.75
            RadicalInput input{make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"),make_box(0.3, 0.39, 0.11, 0.8, 1.3, 0.05, 0.02, 0.01, 0.02, 0.03, 0.04,"21%"),RadicalFontParameters{0.06,0.12,0.43},false,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.3, 0.39, 0.11, 0.8, 1.3, 0.05, 0.02, 0.01, 0.02, 0.03, 0.04,"21%"));
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==false);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==1.015);expect(request.stretch_height==0.835);expect(request.stretch_depth==0.18);expect(request.root_width==0.79);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.46, 0.83, 0.17, 1, 0.9, 0.07, 0.02, 0.02, 0.01, 0.03, 0.01,""),2);
            check_box(layout.bbox,make_box(1.774, 1.0350000000000001, 0.18, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.06);expect(layout.q==0.135);expect(layout.required_surd_height==1.015);
            expect(layout.measured_height==0.835);expect(layout.placed_height==0.865);expect(layout.placed_surd_height==0.09);
            expect(layout.root_offset==0.11399999999999999);expect(layout.root_height==0.558);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0.11399999999999999,0.03500000000000003);
            check_point(layout.base_origin,0.5740000000000001,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0,0.558);else expect(false);
            check_rect(layout.overbar,0.5740000000000001,0.775,1.2,0.09);
        }
        { // Source-paired radical case 5: sqrt, inline, size=-1, wrapper=1
            RadicalInput input{make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"),{},RadicalFontParameters{0.06,0.12,0.43},false,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==1.015);expect(request.stretch_height==0.835);expect(request.stretch_depth==0.18);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.46, 0.83, 0.17, 1, 0.9, 0.07, 0.02, 0.02, 0.01, 0.03, 0.01,""),-1);
            check_box(layout.bbox,make_box(1.66, 0.955, 0.18, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.06);expect(layout.q==0.135);expect(layout.required_surd_height==1.015);
            expect(layout.measured_height==0.835);expect(layout.placed_height==0.895);expect(layout.placed_surd_height==0.12);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.06500000000000006);
            check_point(layout.base_origin,0.46,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.46,0.775,1.2,0.12);
        }
        { // Source-paired radical case 6: root, inline, size=-1, wrapper=1
            RadicalInput input{make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"),make_box(0.3, 0.39, 0.11, 0.8, 1.3, 0.05, 0.02, 0.01, 0.02, 0.03, 0.04,"21%"),RadicalFontParameters{0.06,0.12,0.43},false,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.3, 0.39, 0.11, 0.8, 1.3, 0.05, 0.02, 0.01, 0.02, 0.03, 0.04,"21%"));
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==1.015);expect(request.stretch_height==0.835);expect(request.stretch_depth==0.18);expect(request.root_width==0.79);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.46, 0.83, 0.17, 1, 0.9, 0.07, 0.02, 0.02, 0.01, 0.03, 0.01,""),-1);
            check_box(layout.bbox,make_box(1.8199999999999998, 2.385, 0.18, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.06);expect(layout.q==0.135);expect(layout.required_surd_height==1.015);
            expect(layout.measured_height==0.835);expect(layout.placed_height==0.895);expect(layout.placed_surd_height==0.12);
            expect(layout.root_offset==0.16);expect(layout.root_height==1.938);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0.16,0.06500000000000006);
            check_point(layout.base_origin,0.62,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0,1.938);else expect(false);
            check_rect(layout.overbar,0.62,0.775,1.2,0.12);
        }
        { // Source-paired radical case 7: sqrt, inline, size=2, wrapper=1
            RadicalInput input{make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"),{},RadicalFontParameters{0.06,0.12,0.43},false,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==1.015);expect(request.stretch_height==0.835);expect(request.stretch_depth==0.18);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.46, 0.83, 0.17, 1, 0.9, 0.07, 0.02, 0.02, 0.01, 0.03, 0.01,""),2);
            check_box(layout.bbox,make_box(1.66, 0.955, 0.18, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.06);expect(layout.q==0.135);expect(layout.required_surd_height==1.015);
            expect(layout.measured_height==0.835);expect(layout.placed_height==0.895);expect(layout.placed_surd_height==0.12);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.06500000000000006);
            check_point(layout.base_origin,0.46,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.46,0.775,1.2,0.12);
        }
        { // Source-paired radical case 8: root, inline, size=2, wrapper=1
            RadicalInput input{make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"),make_box(0.3, 0.39, 0.11, 0.8, 1.3, 0.05, 0.02, 0.01, 0.02, 0.03, 0.04,"21%"),RadicalFontParameters{0.06,0.12,0.43},false,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.3, 0.39, 0.11, 0.8, 1.3, 0.05, 0.02, 0.01, 0.02, 0.03, 0.04,"21%"));
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==1.015);expect(request.stretch_height==0.835);expect(request.stretch_depth==0.18);expect(request.root_width==0.79);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.46, 0.83, 0.17, 1, 0.9, 0.07, 0.02, 0.02, 0.01, 0.03, 0.01,""),2);
            check_box(layout.bbox,make_box(1.774, 1.0350000000000001, 0.18, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.06);expect(layout.q==0.135);expect(layout.required_surd_height==1.015);
            expect(layout.measured_height==0.835);expect(layout.placed_height==0.895);expect(layout.placed_surd_height==0.12);
            expect(layout.root_offset==0.11399999999999999);expect(layout.root_height==0.5880000000000001);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0.11399999999999999,0.06500000000000006);
            check_point(layout.base_origin,0.5740000000000001,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0,0.5880000000000001);else expect(false);
            check_rect(layout.overbar,0.5740000000000001,0.775,1.2,0.12);
        }
        { // Source-paired radical case 9: sqrt, inline, size=-1, wrapper=1.35
            RadicalInput input{make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"),{},RadicalFontParameters{0.06,0.12,0.43},false,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==1.015);expect(request.stretch_height==0.835);expect(request.stretch_depth==0.18);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.46, 0.83, 0.17, 1, 0.9, 0.07, 0.02, 0.02, 0.01, 0.03, 0.01,""),-1);
            check_box(layout.bbox,make_box(1.66, 0.955, 0.18, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.06);expect(layout.q==0.135);expect(layout.required_surd_height==1.015);
            expect(layout.measured_height==0.835);expect(layout.placed_height==0.937);expect(layout.placed_surd_height==0.162);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.1070000000000001);
            check_point(layout.base_origin,0.46,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.46,0.775,1.2,0.162);
        }
        { // Source-paired radical case 10: root, inline, size=-1, wrapper=1.35
            RadicalInput input{make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"),make_box(0.3, 0.39, 0.11, 0.8, 1.3, 0.05, 0.02, 0.01, 0.02, 0.03, 0.04,"21%"),RadicalFontParameters{0.06,0.12,0.43},false,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.3, 0.39, 0.11, 0.8, 1.3, 0.05, 0.02, 0.01, 0.02, 0.03, 0.04,"21%"));
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==1.015);expect(request.stretch_height==0.835);expect(request.stretch_depth==0.18);expect(request.root_width==0.79);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.46, 0.83, 0.17, 1, 0.9, 0.07, 0.02, 0.02, 0.01, 0.03, 0.01,""),-1);
            check_box(layout.bbox,make_box(1.8199999999999998, 2.385, 0.18, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.06);expect(layout.q==0.135);expect(layout.required_surd_height==1.015);
            expect(layout.measured_height==0.835);expect(layout.placed_height==0.937);expect(layout.placed_surd_height==0.162);
            expect(layout.root_offset==0.16);expect(layout.root_height==1.98);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0.16,0.1070000000000001);
            check_point(layout.base_origin,0.62,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0,1.98);else expect(false);
            check_rect(layout.overbar,0.62,0.775,1.2,0.162);
        }
        { // Source-paired radical case 11: sqrt, inline, size=2, wrapper=1.35
            RadicalInput input{make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"),{},RadicalFontParameters{0.06,0.12,0.43},false,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==1.015);expect(request.stretch_height==0.835);expect(request.stretch_depth==0.18);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.46, 0.83, 0.17, 1, 0.9, 0.07, 0.02, 0.02, 0.01, 0.03, 0.01,""),2);
            check_box(layout.bbox,make_box(1.66, 0.955, 0.18, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.06);expect(layout.q==0.135);expect(layout.required_surd_height==1.015);
            expect(layout.measured_height==0.835);expect(layout.placed_height==0.937);expect(layout.placed_surd_height==0.162);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.1070000000000001);
            check_point(layout.base_origin,0.46,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.46,0.775,1.2,0.162);
        }
        { // Source-paired radical case 12: root, inline, size=2, wrapper=1.35
            RadicalInput input{make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"),make_box(0.3, 0.39, 0.11, 0.8, 1.3, 0.05, 0.02, 0.01, 0.02, 0.03, 0.04,"21%"),RadicalFontParameters{0.06,0.12,0.43},false,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.3, 0.39, 0.11, 0.8, 1.3, 0.05, 0.02, 0.01, 0.02, 0.03, 0.04,"21%"));
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==1.015);expect(request.stretch_height==0.835);expect(request.stretch_depth==0.18);expect(request.root_width==0.79);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.46, 0.83, 0.17, 1, 0.9, 0.07, 0.02, 0.02, 0.01, 0.03, 0.01,""),2);
            check_box(layout.bbox,make_box(1.774, 1.0350000000000001, 0.18, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.06);expect(layout.q==0.135);expect(layout.required_surd_height==1.015);
            expect(layout.measured_height==0.835);expect(layout.placed_height==0.937);expect(layout.placed_surd_height==0.162);
            expect(layout.root_offset==0.11399999999999999);expect(layout.root_height==0.6300000000000001);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0.11399999999999999,0.1070000000000001);
            check_point(layout.base_origin,0.5740000000000001,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0,0.6300000000000001);else expect(false);
            check_rect(layout.overbar,0.5740000000000001,0.775,1.2,0.162);
        }
        { // Source-paired radical case 13: sqrt, display, size=-1, wrapper=0.75
            RadicalInput input{make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"),{},RadicalFontParameters{0.06,0.12,0.43},true,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==true);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==1.1075);expect(request.stretch_height==0.9275);expect(request.stretch_depth==0.18);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.46, 0.83, 0.17, 1, 0.9, 0.07, 0.02, 0.02, 0.01, 0.03, 0.01,""),-1);
            check_box(layout.bbox,make_box(1.66, 1.0474999999999999, 0.18, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.43);expect(layout.q==0.22749999999999998);expect(layout.required_surd_height==1.1075);
            expect(layout.measured_height==0.9275);expect(layout.placed_height==0.9574999999999999);expect(layout.placed_surd_height==0.09);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.12749999999999995);
            check_point(layout.base_origin,0.46,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.46,0.8674999999999999,1.2,0.09);
        }
        { // Source-paired radical case 14: root, display, size=-1, wrapper=0.75
            RadicalInput input{make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"),make_box(0.3, 0.39, 0.11, 0.8, 1.3, 0.05, 0.02, 0.01, 0.02, 0.03, 0.04,"21%"),RadicalFontParameters{0.06,0.12,0.43},true,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.3, 0.39, 0.11, 0.8, 1.3, 0.05, 0.02, 0.01, 0.02, 0.03, 0.04,"21%"));
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==true);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==1.1075);expect(request.stretch_height==0.9275);expect(request.stretch_depth==0.18);expect(request.root_width==0.79);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.46, 0.83, 0.17, 1, 0.9, 0.07, 0.02, 0.02, 0.01, 0.03, 0.01,""),-1);
            check_box(layout.bbox,make_box(1.8199999999999998, 2.4775, 0.18, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.43);expect(layout.q==0.22749999999999998);expect(layout.required_surd_height==1.1075);
            expect(layout.measured_height==0.9275);expect(layout.placed_height==0.9574999999999999);expect(layout.placed_surd_height==0.09);
            expect(layout.root_offset==0.16);expect(layout.root_height==2.0004999999999997);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0.16,0.12749999999999995);
            check_point(layout.base_origin,0.62,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0,2.0004999999999997);else expect(false);
            check_rect(layout.overbar,0.62,0.8674999999999999,1.2,0.09);
        }
        { // Source-paired radical case 15: sqrt, display, size=2, wrapper=0.75
            RadicalInput input{make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"),{},RadicalFontParameters{0.06,0.12,0.43},true,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==true);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==1.1075);expect(request.stretch_height==0.9275);expect(request.stretch_depth==0.18);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.46, 0.83, 0.17, 1, 0.9, 0.07, 0.02, 0.02, 0.01, 0.03, 0.01,""),2);
            check_box(layout.bbox,make_box(1.66, 1.0474999999999999, 0.18, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.43);expect(layout.q==0.22749999999999998);expect(layout.required_surd_height==1.1075);
            expect(layout.measured_height==0.9275);expect(layout.placed_height==0.9574999999999999);expect(layout.placed_surd_height==0.09);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.12749999999999995);
            check_point(layout.base_origin,0.46,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.46,0.8674999999999999,1.2,0.09);
        }
        { // Source-paired radical case 16: root, display, size=2, wrapper=0.75
            RadicalInput input{make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"),make_box(0.3, 0.39, 0.11, 0.8, 1.3, 0.05, 0.02, 0.01, 0.02, 0.03, 0.04,"21%"),RadicalFontParameters{0.06,0.12,0.43},true,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.3, 0.39, 0.11, 0.8, 1.3, 0.05, 0.02, 0.01, 0.02, 0.03, 0.04,"21%"));
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==true);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==1.1075);expect(request.stretch_height==0.9275);expect(request.stretch_depth==0.18);expect(request.root_width==0.79);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.46, 0.83, 0.17, 1, 0.9, 0.07, 0.02, 0.02, 0.01, 0.03, 0.01,""),2);
            check_box(layout.bbox,make_box(1.774, 1.1275, 0.18, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.43);expect(layout.q==0.22749999999999998);expect(layout.required_surd_height==1.1075);
            expect(layout.measured_height==0.9275);expect(layout.placed_height==0.9574999999999999);expect(layout.placed_surd_height==0.09);
            expect(layout.root_offset==0.11399999999999999);expect(layout.root_height==0.6505);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0.11399999999999999,0.12749999999999995);
            check_point(layout.base_origin,0.5740000000000001,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0,0.6505);else expect(false);
            check_rect(layout.overbar,0.5740000000000001,0.8674999999999999,1.2,0.09);
        }
        { // Source-paired radical case 17: sqrt, display, size=-1, wrapper=1
            RadicalInput input{make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"),{},RadicalFontParameters{0.06,0.12,0.43},true,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==1.1075);expect(request.stretch_height==0.9275);expect(request.stretch_depth==0.18);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.46, 0.83, 0.17, 1, 0.9, 0.07, 0.02, 0.02, 0.01, 0.03, 0.01,""),-1);
            check_box(layout.bbox,make_box(1.66, 1.0474999999999999, 0.18, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.43);expect(layout.q==0.22749999999999998);expect(layout.required_surd_height==1.1075);
            expect(layout.measured_height==0.9275);expect(layout.placed_height==0.9874999999999999);expect(layout.placed_surd_height==0.12);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.15749999999999997);
            check_point(layout.base_origin,0.46,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.46,0.8674999999999999,1.2,0.12);
        }
        { // Source-paired radical case 18: root, display, size=-1, wrapper=1
            RadicalInput input{make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"),make_box(0.3, 0.39, 0.11, 0.8, 1.3, 0.05, 0.02, 0.01, 0.02, 0.03, 0.04,"21%"),RadicalFontParameters{0.06,0.12,0.43},true,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.3, 0.39, 0.11, 0.8, 1.3, 0.05, 0.02, 0.01, 0.02, 0.03, 0.04,"21%"));
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==1.1075);expect(request.stretch_height==0.9275);expect(request.stretch_depth==0.18);expect(request.root_width==0.79);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.46, 0.83, 0.17, 1, 0.9, 0.07, 0.02, 0.02, 0.01, 0.03, 0.01,""),-1);
            check_box(layout.bbox,make_box(1.8199999999999998, 2.4775, 0.18, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.43);expect(layout.q==0.22749999999999998);expect(layout.required_surd_height==1.1075);
            expect(layout.measured_height==0.9275);expect(layout.placed_height==0.9874999999999999);expect(layout.placed_surd_height==0.12);
            expect(layout.root_offset==0.16);expect(layout.root_height==2.0305);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0.16,0.15749999999999997);
            check_point(layout.base_origin,0.62,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0,2.0305);else expect(false);
            check_rect(layout.overbar,0.62,0.8674999999999999,1.2,0.12);
        }
        { // Source-paired radical case 19: sqrt, display, size=2, wrapper=1
            RadicalInput input{make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"),{},RadicalFontParameters{0.06,0.12,0.43},true,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==1.1075);expect(request.stretch_height==0.9275);expect(request.stretch_depth==0.18);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.46, 0.83, 0.17, 1, 0.9, 0.07, 0.02, 0.02, 0.01, 0.03, 0.01,""),2);
            check_box(layout.bbox,make_box(1.66, 1.0474999999999999, 0.18, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.43);expect(layout.q==0.22749999999999998);expect(layout.required_surd_height==1.1075);
            expect(layout.measured_height==0.9275);expect(layout.placed_height==0.9874999999999999);expect(layout.placed_surd_height==0.12);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.15749999999999997);
            check_point(layout.base_origin,0.46,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.46,0.8674999999999999,1.2,0.12);
        }
        { // Source-paired radical case 20: root, display, size=2, wrapper=1
            RadicalInput input{make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"),make_box(0.3, 0.39, 0.11, 0.8, 1.3, 0.05, 0.02, 0.01, 0.02, 0.03, 0.04,"21%"),RadicalFontParameters{0.06,0.12,0.43},true,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.3, 0.39, 0.11, 0.8, 1.3, 0.05, 0.02, 0.01, 0.02, 0.03, 0.04,"21%"));
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==1.1075);expect(request.stretch_height==0.9275);expect(request.stretch_depth==0.18);expect(request.root_width==0.79);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.46, 0.83, 0.17, 1, 0.9, 0.07, 0.02, 0.02, 0.01, 0.03, 0.01,""),2);
            check_box(layout.bbox,make_box(1.774, 1.1275, 0.18, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.43);expect(layout.q==0.22749999999999998);expect(layout.required_surd_height==1.1075);
            expect(layout.measured_height==0.9275);expect(layout.placed_height==0.9874999999999999);expect(layout.placed_surd_height==0.12);
            expect(layout.root_offset==0.11399999999999999);expect(layout.root_height==0.6805);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0.11399999999999999,0.15749999999999997);
            check_point(layout.base_origin,0.5740000000000001,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0,0.6805);else expect(false);
            check_rect(layout.overbar,0.5740000000000001,0.8674999999999999,1.2,0.12);
        }
        { // Source-paired radical case 21: sqrt, display, size=-1, wrapper=1.35
            RadicalInput input{make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"),{},RadicalFontParameters{0.06,0.12,0.43},true,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==1.1075);expect(request.stretch_height==0.9275);expect(request.stretch_depth==0.18);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.46, 0.83, 0.17, 1, 0.9, 0.07, 0.02, 0.02, 0.01, 0.03, 0.01,""),-1);
            check_box(layout.bbox,make_box(1.66, 1.0474999999999999, 0.18, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.43);expect(layout.q==0.22749999999999998);expect(layout.required_surd_height==1.1075);
            expect(layout.measured_height==0.9275);expect(layout.placed_height==1.0294999999999999);expect(layout.placed_surd_height==0.162);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.1994999999999999);
            check_point(layout.base_origin,0.46,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.46,0.8674999999999998,1.2,0.162);
        }
        { // Source-paired radical case 22: root, display, size=-1, wrapper=1.35
            RadicalInput input{make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"),make_box(0.3, 0.39, 0.11, 0.8, 1.3, 0.05, 0.02, 0.01, 0.02, 0.03, 0.04,"21%"),RadicalFontParameters{0.06,0.12,0.43},true,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.3, 0.39, 0.11, 0.8, 1.3, 0.05, 0.02, 0.01, 0.02, 0.03, 0.04,"21%"));
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==1.1075);expect(request.stretch_height==0.9275);expect(request.stretch_depth==0.18);expect(request.root_width==0.79);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.46, 0.83, 0.17, 1, 0.9, 0.07, 0.02, 0.02, 0.01, 0.03, 0.01,""),-1);
            check_box(layout.bbox,make_box(1.8199999999999998, 2.4775, 0.18, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.43);expect(layout.q==0.22749999999999998);expect(layout.required_surd_height==1.1075);
            expect(layout.measured_height==0.9275);expect(layout.placed_height==1.0294999999999999);expect(layout.placed_surd_height==0.162);
            expect(layout.root_offset==0.16);expect(layout.root_height==2.0725);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0.16,0.1994999999999999);
            check_point(layout.base_origin,0.62,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0,2.0725);else expect(false);
            check_rect(layout.overbar,0.62,0.8674999999999998,1.2,0.162);
        }
        { // Source-paired radical case 23: sqrt, display, size=2, wrapper=1.35
            RadicalInput input{make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"),{},RadicalFontParameters{0.06,0.12,0.43},true,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==1.1075);expect(request.stretch_height==0.9275);expect(request.stretch_depth==0.18);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.46, 0.83, 0.17, 1, 0.9, 0.07, 0.02, 0.02, 0.01, 0.03, 0.01,""),2);
            check_box(layout.bbox,make_box(1.66, 1.0474999999999999, 0.18, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.43);expect(layout.q==0.22749999999999998);expect(layout.required_surd_height==1.1075);
            expect(layout.measured_height==0.9275);expect(layout.placed_height==1.0294999999999999);expect(layout.placed_surd_height==0.162);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.1994999999999999);
            check_point(layout.base_origin,0.46,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.46,0.8674999999999998,1.2,0.162);
        }
        { // Source-paired radical case 24: root, display, size=2, wrapper=1.35
            RadicalInput input{make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"),make_box(0.3, 0.39, 0.11, 0.8, 1.3, 0.05, 0.02, 0.01, 0.02, 0.03, 0.04,"21%"),RadicalFontParameters{0.06,0.12,0.43},true,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.3, 0.39, 0.11, 0.8, 1.3, 0.05, 0.02, 0.01, 0.02, 0.03, 0.04,"21%"));
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==1.1075);expect(request.stretch_height==0.9275);expect(request.stretch_depth==0.18);expect(request.root_width==0.79);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.46, 0.83, 0.17, 1, 0.9, 0.07, 0.02, 0.02, 0.01, 0.03, 0.01,""),2);
            check_box(layout.bbox,make_box(1.774, 1.1275, 0.18, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.43);expect(layout.q==0.22749999999999998);expect(layout.required_surd_height==1.1075);
            expect(layout.measured_height==0.9275);expect(layout.placed_height==1.0294999999999999);expect(layout.placed_surd_height==0.162);
            expect(layout.root_offset==0.11399999999999999);expect(layout.root_height==0.7224999999999999);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0.11399999999999999,0.1994999999999999);
            check_point(layout.base_origin,0.5740000000000001,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0,0.7224999999999999);else expect(false);
            check_rect(layout.overbar,0.5740000000000001,0.8674999999999998,1.2,0.162);
        }
        { // Source-paired radical case 25: sqrt, inline, size=-1, wrapper=0.75
            RadicalInput input{make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"),{},RadicalFontParameters{0.15,0.08,0.27},false,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==false);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==1.0875000000000001);expect(request.stretch_height==0.9075000000000002);expect(request.stretch_depth==0.18);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.46, 0.83, 0.17, 1, 0.9, 0.07, 0.02, 0.02, 0.01, 0.03, 0.01,""),-1);
            check_box(layout.bbox,make_box(1.66, 0.9875, 0.18, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.15);expect(layout.q==0.1175);expect(layout.required_surd_height==1.0875000000000001);
            expect(layout.measured_height==0.9075000000000001);expect(layout.placed_height==0.8175000000000001);expect(layout.placed_surd_height==0.06);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,-0.012499999999999845);
            check_point(layout.base_origin,0.46,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.46,0.7575000000000001,1.2,0.06);
        }
        { // Source-paired radical case 26: root, inline, size=-1, wrapper=0.75
            RadicalInput input{make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"),make_box(0.3, 0.39, 0.11, 0.8, 1.3, 0.05, 0.02, 0.01, 0.02, 0.03, 0.04,"21%"),RadicalFontParameters{0.15,0.08,0.27},false,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.3, 0.39, 0.11, 0.8, 1.3, 0.05, 0.02, 0.01, 0.02, 0.03, 0.04,"21%"));
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==false);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==1.0875000000000001);expect(request.stretch_height==0.9075000000000002);expect(request.stretch_depth==0.18);expect(request.root_width==0.79);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.46, 0.83, 0.17, 1, 0.9, 0.07, 0.02, 0.02, 0.01, 0.03, 0.01,""),-1);
            check_box(layout.bbox,make_box(1.8199999999999998, 2.4575, 0.18, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.15);expect(layout.q==0.1175);expect(layout.required_surd_height==1.0875000000000001);
            expect(layout.measured_height==0.9075000000000001);expect(layout.placed_height==0.8175000000000001);expect(layout.placed_surd_height==0.06);
            expect(layout.root_offset==0.16);expect(layout.root_height==1.8605);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0.16,-0.012499999999999845);
            check_point(layout.base_origin,0.62,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0,1.8605);else expect(false);
            check_rect(layout.overbar,0.62,0.7575000000000001,1.2,0.06);
        }
        { // Source-paired radical case 27: sqrt, inline, size=2, wrapper=0.75
            RadicalInput input{make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"),{},RadicalFontParameters{0.15,0.08,0.27},false,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==false);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==1.0875000000000001);expect(request.stretch_height==0.9075000000000002);expect(request.stretch_depth==0.18);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.46, 0.83, 0.17, 1, 0.9, 0.07, 0.02, 0.02, 0.01, 0.03, 0.01,""),2);
            check_box(layout.bbox,make_box(1.66, 0.9875, 0.18, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.15);expect(layout.q==0.1175);expect(layout.required_surd_height==1.0875000000000001);
            expect(layout.measured_height==0.9075000000000001);expect(layout.placed_height==0.8175000000000001);expect(layout.placed_surd_height==0.06);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,-0.012499999999999845);
            check_point(layout.base_origin,0.46,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.46,0.7575000000000001,1.2,0.06);
        }
        { // Source-paired radical case 28: root, inline, size=2, wrapper=0.75
            RadicalInput input{make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"),make_box(0.3, 0.39, 0.11, 0.8, 1.3, 0.05, 0.02, 0.01, 0.02, 0.03, 0.04,"21%"),RadicalFontParameters{0.15,0.08,0.27},false,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.3, 0.39, 0.11, 0.8, 1.3, 0.05, 0.02, 0.01, 0.02, 0.03, 0.04,"21%"));
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==false);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==1.0875000000000001);expect(request.stretch_height==0.9075000000000002);expect(request.stretch_depth==0.18);expect(request.root_width==0.79);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.46, 0.83, 0.17, 1, 0.9, 0.07, 0.02, 0.02, 0.01, 0.03, 0.01,""),2);
            check_box(layout.bbox,make_box(1.774, 1.1075000000000002, 0.18, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.15);expect(layout.q==0.1175);expect(layout.required_surd_height==1.0875000000000001);
            expect(layout.measured_height==0.9075000000000001);expect(layout.placed_height==0.8175000000000001);expect(layout.placed_surd_height==0.06);
            expect(layout.root_offset==0.11399999999999999);expect(layout.root_height==0.5105000000000002);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0.11399999999999999,-0.012499999999999845);
            check_point(layout.base_origin,0.5740000000000001,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0,0.5105000000000002);else expect(false);
            check_rect(layout.overbar,0.5740000000000001,0.7575000000000001,1.2,0.06);
        }
        { // Source-paired radical case 29: sqrt, inline, size=-1, wrapper=1
            RadicalInput input{make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"),{},RadicalFontParameters{0.15,0.08,0.27},false,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==1.0875000000000001);expect(request.stretch_height==0.9075000000000002);expect(request.stretch_depth==0.18);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.46, 0.83, 0.17, 1, 0.9, 0.07, 0.02, 0.02, 0.01, 0.03, 0.01,""),-1);
            check_box(layout.bbox,make_box(1.66, 0.9875, 0.18, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.15);expect(layout.q==0.1175);expect(layout.required_surd_height==1.0875000000000001);
            expect(layout.measured_height==0.9075000000000001);expect(layout.placed_height==0.8375);expect(layout.placed_surd_height==0.08);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.007500000000000062);
            check_point(layout.base_origin,0.46,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.46,0.7575000000000001,1.2,0.08);
        }
        { // Source-paired radical case 30: root, inline, size=-1, wrapper=1
            RadicalInput input{make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"),make_box(0.3, 0.39, 0.11, 0.8, 1.3, 0.05, 0.02, 0.01, 0.02, 0.03, 0.04,"21%"),RadicalFontParameters{0.15,0.08,0.27},false,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.3, 0.39, 0.11, 0.8, 1.3, 0.05, 0.02, 0.01, 0.02, 0.03, 0.04,"21%"));
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==1.0875000000000001);expect(request.stretch_height==0.9075000000000002);expect(request.stretch_depth==0.18);expect(request.root_width==0.79);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.46, 0.83, 0.17, 1, 0.9, 0.07, 0.02, 0.02, 0.01, 0.03, 0.01,""),-1);
            check_box(layout.bbox,make_box(1.8199999999999998, 2.4575, 0.18, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.15);expect(layout.q==0.1175);expect(layout.required_surd_height==1.0875000000000001);
            expect(layout.measured_height==0.9075000000000001);expect(layout.placed_height==0.8375);expect(layout.placed_surd_height==0.08);
            expect(layout.root_offset==0.16);expect(layout.root_height==1.8804999999999998);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0.16,0.007500000000000062);
            check_point(layout.base_origin,0.62,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0,1.8804999999999998);else expect(false);
            check_rect(layout.overbar,0.62,0.7575000000000001,1.2,0.08);
        }
        { // Source-paired radical case 31: sqrt, inline, size=2, wrapper=1
            RadicalInput input{make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"),{},RadicalFontParameters{0.15,0.08,0.27},false,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==1.0875000000000001);expect(request.stretch_height==0.9075000000000002);expect(request.stretch_depth==0.18);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.46, 0.83, 0.17, 1, 0.9, 0.07, 0.02, 0.02, 0.01, 0.03, 0.01,""),2);
            check_box(layout.bbox,make_box(1.66, 0.9875, 0.18, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.15);expect(layout.q==0.1175);expect(layout.required_surd_height==1.0875000000000001);
            expect(layout.measured_height==0.9075000000000001);expect(layout.placed_height==0.8375);expect(layout.placed_surd_height==0.08);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.007500000000000062);
            check_point(layout.base_origin,0.46,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.46,0.7575000000000001,1.2,0.08);
        }
        { // Source-paired radical case 32: root, inline, size=2, wrapper=1
            RadicalInput input{make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"),make_box(0.3, 0.39, 0.11, 0.8, 1.3, 0.05, 0.02, 0.01, 0.02, 0.03, 0.04,"21%"),RadicalFontParameters{0.15,0.08,0.27},false,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.3, 0.39, 0.11, 0.8, 1.3, 0.05, 0.02, 0.01, 0.02, 0.03, 0.04,"21%"));
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==1.0875000000000001);expect(request.stretch_height==0.9075000000000002);expect(request.stretch_depth==0.18);expect(request.root_width==0.79);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.46, 0.83, 0.17, 1, 0.9, 0.07, 0.02, 0.02, 0.01, 0.03, 0.01,""),2);
            check_box(layout.bbox,make_box(1.774, 1.1075000000000002, 0.18, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.15);expect(layout.q==0.1175);expect(layout.required_surd_height==1.0875000000000001);
            expect(layout.measured_height==0.9075000000000001);expect(layout.placed_height==0.8375);expect(layout.placed_surd_height==0.08);
            expect(layout.root_offset==0.11399999999999999);expect(layout.root_height==0.5305000000000001);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0.11399999999999999,0.007500000000000062);
            check_point(layout.base_origin,0.5740000000000001,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0,0.5305000000000001);else expect(false);
            check_rect(layout.overbar,0.5740000000000001,0.7575000000000001,1.2,0.08);
        }
        { // Source-paired radical case 33: sqrt, inline, size=-1, wrapper=1.35
            RadicalInput input{make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"),{},RadicalFontParameters{0.15,0.08,0.27},false,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==1.0875000000000001);expect(request.stretch_height==0.9075000000000002);expect(request.stretch_depth==0.18);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.46, 0.83, 0.17, 1, 0.9, 0.07, 0.02, 0.02, 0.01, 0.03, 0.01,""),-1);
            check_box(layout.bbox,make_box(1.66, 0.9875, 0.18, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.15);expect(layout.q==0.1175);expect(layout.required_surd_height==1.0875000000000001);
            expect(layout.measured_height==0.9075000000000001);expect(layout.placed_height==0.8655);expect(layout.placed_surd_height==0.10800000000000001);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.03550000000000009);
            check_point(layout.base_origin,0.46,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.46,0.7575000000000001,1.2,0.10800000000000001);
        }
        { // Source-paired radical case 34: root, inline, size=-1, wrapper=1.35
            RadicalInput input{make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"),make_box(0.3, 0.39, 0.11, 0.8, 1.3, 0.05, 0.02, 0.01, 0.02, 0.03, 0.04,"21%"),RadicalFontParameters{0.15,0.08,0.27},false,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.3, 0.39, 0.11, 0.8, 1.3, 0.05, 0.02, 0.01, 0.02, 0.03, 0.04,"21%"));
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==1.0875000000000001);expect(request.stretch_height==0.9075000000000002);expect(request.stretch_depth==0.18);expect(request.root_width==0.79);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.46, 0.83, 0.17, 1, 0.9, 0.07, 0.02, 0.02, 0.01, 0.03, 0.01,""),-1);
            check_box(layout.bbox,make_box(1.8199999999999998, 2.4575, 0.18, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.15);expect(layout.q==0.1175);expect(layout.required_surd_height==1.0875000000000001);
            expect(layout.measured_height==0.9075000000000001);expect(layout.placed_height==0.8655);expect(layout.placed_surd_height==0.10800000000000001);
            expect(layout.root_offset==0.16);expect(layout.root_height==1.9084999999999999);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0.16,0.03550000000000009);
            check_point(layout.base_origin,0.62,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0,1.9084999999999999);else expect(false);
            check_rect(layout.overbar,0.62,0.7575000000000001,1.2,0.10800000000000001);
        }
        { // Source-paired radical case 35: sqrt, inline, size=2, wrapper=1.35
            RadicalInput input{make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"),{},RadicalFontParameters{0.15,0.08,0.27},false,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==1.0875000000000001);expect(request.stretch_height==0.9075000000000002);expect(request.stretch_depth==0.18);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.46, 0.83, 0.17, 1, 0.9, 0.07, 0.02, 0.02, 0.01, 0.03, 0.01,""),2);
            check_box(layout.bbox,make_box(1.66, 0.9875, 0.18, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.15);expect(layout.q==0.1175);expect(layout.required_surd_height==1.0875000000000001);
            expect(layout.measured_height==0.9075000000000001);expect(layout.placed_height==0.8655);expect(layout.placed_surd_height==0.10800000000000001);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.03550000000000009);
            check_point(layout.base_origin,0.46,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.46,0.7575000000000001,1.2,0.10800000000000001);
        }
        { // Source-paired radical case 36: root, inline, size=2, wrapper=1.35
            RadicalInput input{make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"),make_box(0.3, 0.39, 0.11, 0.8, 1.3, 0.05, 0.02, 0.01, 0.02, 0.03, 0.04,"21%"),RadicalFontParameters{0.15,0.08,0.27},false,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.3, 0.39, 0.11, 0.8, 1.3, 0.05, 0.02, 0.01, 0.02, 0.03, 0.04,"21%"));
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==1.0875000000000001);expect(request.stretch_height==0.9075000000000002);expect(request.stretch_depth==0.18);expect(request.root_width==0.79);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.46, 0.83, 0.17, 1, 0.9, 0.07, 0.02, 0.02, 0.01, 0.03, 0.01,""),2);
            check_box(layout.bbox,make_box(1.774, 1.1075000000000002, 0.18, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.15);expect(layout.q==0.1175);expect(layout.required_surd_height==1.0875000000000001);
            expect(layout.measured_height==0.9075000000000001);expect(layout.placed_height==0.8655);expect(layout.placed_surd_height==0.10800000000000001);
            expect(layout.root_offset==0.11399999999999999);expect(layout.root_height==0.5585000000000001);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0.11399999999999999,0.03550000000000009);
            check_point(layout.base_origin,0.5740000000000001,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0,0.5585000000000001);else expect(false);
            check_rect(layout.overbar,0.5740000000000001,0.7575000000000001,1.2,0.10800000000000001);
        }
        { // Source-paired radical case 37: sqrt, display, size=-1, wrapper=0.75
            RadicalInput input{make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"),{},RadicalFontParameters{0.15,0.08,0.27},true,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==true);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==1.1175000000000002);expect(request.stretch_height==0.9375000000000002);expect(request.stretch_depth==0.18);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.46, 0.83, 0.17, 1, 0.9, 0.07, 0.02, 0.02, 0.01, 0.03, 0.01,""),-1);
            check_box(layout.bbox,make_box(1.66, 1.0175, 0.18, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.27);expect(layout.q==0.14750000000000002);expect(layout.required_surd_height==1.1175000000000002);
            expect(layout.measured_height==0.9375000000000001);expect(layout.placed_height==0.8475000000000001);expect(layout.placed_surd_height==0.06);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.017500000000000182);
            check_point(layout.base_origin,0.46,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.46,0.7875000000000001,1.2,0.06);
        }
        { // Source-paired radical case 38: root, display, size=-1, wrapper=0.75
            RadicalInput input{make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"),make_box(0.3, 0.39, 0.11, 0.8, 1.3, 0.05, 0.02, 0.01, 0.02, 0.03, 0.04,"21%"),RadicalFontParameters{0.15,0.08,0.27},true,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.3, 0.39, 0.11, 0.8, 1.3, 0.05, 0.02, 0.01, 0.02, 0.03, 0.04,"21%"));
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==true);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==1.1175000000000002);expect(request.stretch_height==0.9375000000000002);expect(request.stretch_depth==0.18);expect(request.root_width==0.79);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.46, 0.83, 0.17, 1, 0.9, 0.07, 0.02, 0.02, 0.01, 0.03, 0.01,""),-1);
            check_box(layout.bbox,make_box(1.8199999999999998, 2.4875, 0.18, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.27);expect(layout.q==0.14750000000000002);expect(layout.required_surd_height==1.1175000000000002);
            expect(layout.measured_height==0.9375000000000001);expect(layout.placed_height==0.8475000000000001);expect(layout.placed_surd_height==0.06);
            expect(layout.root_offset==0.16);expect(layout.root_height==1.8905);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0.16,0.017500000000000182);
            check_point(layout.base_origin,0.62,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0,1.8905);else expect(false);
            check_rect(layout.overbar,0.62,0.7875000000000001,1.2,0.06);
        }
        { // Source-paired radical case 39: sqrt, display, size=2, wrapper=0.75
            RadicalInput input{make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"),{},RadicalFontParameters{0.15,0.08,0.27},true,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==true);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==1.1175000000000002);expect(request.stretch_height==0.9375000000000002);expect(request.stretch_depth==0.18);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.46, 0.83, 0.17, 1, 0.9, 0.07, 0.02, 0.02, 0.01, 0.03, 0.01,""),2);
            check_box(layout.bbox,make_box(1.66, 1.0175, 0.18, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.27);expect(layout.q==0.14750000000000002);expect(layout.required_surd_height==1.1175000000000002);
            expect(layout.measured_height==0.9375000000000001);expect(layout.placed_height==0.8475000000000001);expect(layout.placed_surd_height==0.06);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.017500000000000182);
            check_point(layout.base_origin,0.46,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.46,0.7875000000000001,1.2,0.06);
        }
        { // Source-paired radical case 40: root, display, size=2, wrapper=0.75
            RadicalInput input{make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"),make_box(0.3, 0.39, 0.11, 0.8, 1.3, 0.05, 0.02, 0.01, 0.02, 0.03, 0.04,"21%"),RadicalFontParameters{0.15,0.08,0.27},true,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.3, 0.39, 0.11, 0.8, 1.3, 0.05, 0.02, 0.01, 0.02, 0.03, 0.04,"21%"));
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==true);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==1.1175000000000002);expect(request.stretch_height==0.9375000000000002);expect(request.stretch_depth==0.18);expect(request.root_width==0.79);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.46, 0.83, 0.17, 1, 0.9, 0.07, 0.02, 0.02, 0.01, 0.03, 0.01,""),2);
            check_box(layout.bbox,make_box(1.774, 1.1375000000000002, 0.18, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.27);expect(layout.q==0.14750000000000002);expect(layout.required_surd_height==1.1175000000000002);
            expect(layout.measured_height==0.9375000000000001);expect(layout.placed_height==0.8475000000000001);expect(layout.placed_surd_height==0.06);
            expect(layout.root_offset==0.11399999999999999);expect(layout.root_height==0.5405000000000002);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0.11399999999999999,0.017500000000000182);
            check_point(layout.base_origin,0.5740000000000001,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0,0.5405000000000002);else expect(false);
            check_rect(layout.overbar,0.5740000000000001,0.7875000000000001,1.2,0.06);
        }
        { // Source-paired radical case 41: sqrt, display, size=-1, wrapper=1
            RadicalInput input{make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"),{},RadicalFontParameters{0.15,0.08,0.27},true,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==1.1175000000000002);expect(request.stretch_height==0.9375000000000002);expect(request.stretch_depth==0.18);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.46, 0.83, 0.17, 1, 0.9, 0.07, 0.02, 0.02, 0.01, 0.03, 0.01,""),-1);
            check_box(layout.bbox,make_box(1.66, 1.0175, 0.18, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.27);expect(layout.q==0.14750000000000002);expect(layout.required_surd_height==1.1175000000000002);
            expect(layout.measured_height==0.9375000000000001);expect(layout.placed_height==0.8675);expect(layout.placed_surd_height==0.08);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.03750000000000009);
            check_point(layout.base_origin,0.46,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.46,0.7875000000000001,1.2,0.08);
        }
        { // Source-paired radical case 42: root, display, size=-1, wrapper=1
            RadicalInput input{make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"),make_box(0.3, 0.39, 0.11, 0.8, 1.3, 0.05, 0.02, 0.01, 0.02, 0.03, 0.04,"21%"),RadicalFontParameters{0.15,0.08,0.27},true,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.3, 0.39, 0.11, 0.8, 1.3, 0.05, 0.02, 0.01, 0.02, 0.03, 0.04,"21%"));
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==1.1175000000000002);expect(request.stretch_height==0.9375000000000002);expect(request.stretch_depth==0.18);expect(request.root_width==0.79);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.46, 0.83, 0.17, 1, 0.9, 0.07, 0.02, 0.02, 0.01, 0.03, 0.01,""),-1);
            check_box(layout.bbox,make_box(1.8199999999999998, 2.4875, 0.18, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.27);expect(layout.q==0.14750000000000002);expect(layout.required_surd_height==1.1175000000000002);
            expect(layout.measured_height==0.9375000000000001);expect(layout.placed_height==0.8675);expect(layout.placed_surd_height==0.08);
            expect(layout.root_offset==0.16);expect(layout.root_height==1.9105);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0.16,0.03750000000000009);
            check_point(layout.base_origin,0.62,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0,1.9105);else expect(false);
            check_rect(layout.overbar,0.62,0.7875000000000001,1.2,0.08);
        }
        { // Source-paired radical case 43: sqrt, display, size=2, wrapper=1
            RadicalInput input{make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"),{},RadicalFontParameters{0.15,0.08,0.27},true,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==1.1175000000000002);expect(request.stretch_height==0.9375000000000002);expect(request.stretch_depth==0.18);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.46, 0.83, 0.17, 1, 0.9, 0.07, 0.02, 0.02, 0.01, 0.03, 0.01,""),2);
            check_box(layout.bbox,make_box(1.66, 1.0175, 0.18, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.27);expect(layout.q==0.14750000000000002);expect(layout.required_surd_height==1.1175000000000002);
            expect(layout.measured_height==0.9375000000000001);expect(layout.placed_height==0.8675);expect(layout.placed_surd_height==0.08);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.03750000000000009);
            check_point(layout.base_origin,0.46,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.46,0.7875000000000001,1.2,0.08);
        }
        { // Source-paired radical case 44: root, display, size=2, wrapper=1
            RadicalInput input{make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"),make_box(0.3, 0.39, 0.11, 0.8, 1.3, 0.05, 0.02, 0.01, 0.02, 0.03, 0.04,"21%"),RadicalFontParameters{0.15,0.08,0.27},true,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.3, 0.39, 0.11, 0.8, 1.3, 0.05, 0.02, 0.01, 0.02, 0.03, 0.04,"21%"));
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==1.1175000000000002);expect(request.stretch_height==0.9375000000000002);expect(request.stretch_depth==0.18);expect(request.root_width==0.79);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.46, 0.83, 0.17, 1, 0.9, 0.07, 0.02, 0.02, 0.01, 0.03, 0.01,""),2);
            check_box(layout.bbox,make_box(1.774, 1.1375000000000002, 0.18, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.27);expect(layout.q==0.14750000000000002);expect(layout.required_surd_height==1.1175000000000002);
            expect(layout.measured_height==0.9375000000000001);expect(layout.placed_height==0.8675);expect(layout.placed_surd_height==0.08);
            expect(layout.root_offset==0.11399999999999999);expect(layout.root_height==0.5605000000000001);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0.11399999999999999,0.03750000000000009);
            check_point(layout.base_origin,0.5740000000000001,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0,0.5605000000000001);else expect(false);
            check_rect(layout.overbar,0.5740000000000001,0.7875000000000001,1.2,0.08);
        }
        { // Source-paired radical case 45: sqrt, display, size=-1, wrapper=1.35
            RadicalInput input{make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"),{},RadicalFontParameters{0.15,0.08,0.27},true,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==1.1175000000000002);expect(request.stretch_height==0.9375000000000002);expect(request.stretch_depth==0.18);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.46, 0.83, 0.17, 1, 0.9, 0.07, 0.02, 0.02, 0.01, 0.03, 0.01,""),-1);
            check_box(layout.bbox,make_box(1.66, 1.0175, 0.18, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.27);expect(layout.q==0.14750000000000002);expect(layout.required_surd_height==1.1175000000000002);
            expect(layout.measured_height==0.9375000000000001);expect(layout.placed_height==0.8955000000000001);expect(layout.placed_surd_height==0.10800000000000001);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.06550000000000011);
            check_point(layout.base_origin,0.46,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.46,0.7875000000000001,1.2,0.10800000000000001);
        }
        { // Source-paired radical case 46: root, display, size=-1, wrapper=1.35
            RadicalInput input{make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"),make_box(0.3, 0.39, 0.11, 0.8, 1.3, 0.05, 0.02, 0.01, 0.02, 0.03, 0.04,"21%"),RadicalFontParameters{0.15,0.08,0.27},true,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.3, 0.39, 0.11, 0.8, 1.3, 0.05, 0.02, 0.01, 0.02, 0.03, 0.04,"21%"));
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==1.1175000000000002);expect(request.stretch_height==0.9375000000000002);expect(request.stretch_depth==0.18);expect(request.root_width==0.79);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.46, 0.83, 0.17, 1, 0.9, 0.07, 0.02, 0.02, 0.01, 0.03, 0.01,""),-1);
            check_box(layout.bbox,make_box(1.8199999999999998, 2.4875, 0.18, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.27);expect(layout.q==0.14750000000000002);expect(layout.required_surd_height==1.1175000000000002);
            expect(layout.measured_height==0.9375000000000001);expect(layout.placed_height==0.8955000000000001);expect(layout.placed_surd_height==0.10800000000000001);
            expect(layout.root_offset==0.16);expect(layout.root_height==1.9385000000000001);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0.16,0.06550000000000011);
            check_point(layout.base_origin,0.62,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0,1.9385000000000001);else expect(false);
            check_rect(layout.overbar,0.62,0.7875000000000001,1.2,0.10800000000000001);
        }
        { // Source-paired radical case 47: sqrt, display, size=2, wrapper=1.35
            RadicalInput input{make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"),{},RadicalFontParameters{0.15,0.08,0.27},true,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==1.1175000000000002);expect(request.stretch_height==0.9375000000000002);expect(request.stretch_depth==0.18);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.46, 0.83, 0.17, 1, 0.9, 0.07, 0.02, 0.02, 0.01, 0.03, 0.01,""),2);
            check_box(layout.bbox,make_box(1.66, 1.0175, 0.18, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.27);expect(layout.q==0.14750000000000002);expect(layout.required_surd_height==1.1175000000000002);
            expect(layout.measured_height==0.9375000000000001);expect(layout.placed_height==0.8955000000000001);expect(layout.placed_surd_height==0.10800000000000001);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.06550000000000011);
            check_point(layout.base_origin,0.46,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.46,0.7875000000000001,1.2,0.10800000000000001);
        }
        { // Source-paired radical case 48: root, display, size=2, wrapper=1.35
            RadicalInput input{make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"),make_box(0.3, 0.39, 0.11, 0.8, 1.3, 0.05, 0.02, 0.01, 0.02, 0.03, 0.04,"21%"),RadicalFontParameters{0.15,0.08,0.27},true,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.2, 0.64, 0.18, 1.1, 0.85, 0.08, 0.03, 0.04, 0.02, 0.01, 0,"73%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.3, 0.39, 0.11, 0.8, 1.3, 0.05, 0.02, 0.01, 0.02, 0.03, 0.04,"21%"));
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==1.1175000000000002);expect(request.stretch_height==0.9375000000000002);expect(request.stretch_depth==0.18);expect(request.root_width==0.79);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.46, 0.83, 0.17, 1, 0.9, 0.07, 0.02, 0.02, 0.01, 0.03, 0.01,""),2);
            check_box(layout.bbox,make_box(1.774, 1.1375000000000002, 0.18, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.27);expect(layout.q==0.14750000000000002);expect(layout.required_surd_height==1.1175000000000002);
            expect(layout.measured_height==0.9375000000000001);expect(layout.placed_height==0.8955000000000001);expect(layout.placed_surd_height==0.10800000000000001);
            expect(layout.root_offset==0.11399999999999999);expect(layout.root_height==0.5885000000000001);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0.11399999999999999,0.06550000000000011);
            check_point(layout.base_origin,0.5740000000000001,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0,0.5885000000000001);else expect(false);
            check_rect(layout.overbar,0.5740000000000001,0.7875000000000001,1.2,0.10800000000000001);
        }
        { // Source-paired radical case 49: sqrt, inline, size=-1, wrapper=0.75
            RadicalInput input{make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"),{},RadicalFontParameters{0.06,0.12,0.43},false,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==false);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==0.49500000000000005);expect(request.stretch_height==0.42500000000000004);expect(request.stretch_depth==0.07);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.24, 0.29, 0.09, 0.88, 1.17, -0.06, 0.11, 0.03, 0.02, -0.01, 0.05,""),-1);
            check_box(layout.bbox,make_box(0.72, 0.5449999999999999, 0.07, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.06);expect(layout.q==0.135);expect(layout.required_surd_height==0.49500000000000005);
            expect(layout.measured_height==0.425);expect(layout.placed_height==0.45499999999999996);expect(layout.placed_surd_height==0.09);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.16499999999999998);
            check_point(layout.base_origin,0.24,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.24,0.365,0.48,0.09);
        }
        { // Source-paired radical case 50: root, inline, size=-1, wrapper=0.75
            RadicalInput input{make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"),make_box(0.11, 0.2, 0.28, 1.2, 0.64, -0.07, 0.09, 0.04, -0.01, 0.02, -0.04,"9%"),RadicalFontParameters{0.06,0.12,0.43},false,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.11, 0.2, 0.28, 1.2, 0.64, -0.07, 0.09, 0.04, -0.01, 0.02, -0.04,"9%"));
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==false);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==0.49500000000000005);expect(request.stretch_height==0.42500000000000004);expect(request.stretch_depth==0.07);expect(request.root_width==0.47040000000000004);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.24, 0.29, 0.09, 0.88, 1.17, -0.06, 0.11, 0.03, 0.02, -0.01, 0.05,""),-1);
            check_box(layout.bbox,make_box(0.72, 2.2522, 0.07, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.06);expect(layout.q==0.135);expect(layout.required_surd_height==0.49500000000000005);
            expect(layout.measured_height==0.425);expect(layout.placed_height==0.45499999999999996);expect(layout.placed_surd_height==0.09);
            expect(layout.root_offset==0);expect(layout.root_height==2.1542);expect(layout.root_dx==0.0775);
            check_point(layout.surd_origin,0,0.16499999999999998);
            check_point(layout.base_origin,0.24,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0.0496,2.1542);else expect(false);
            check_rect(layout.overbar,0.24,0.365,0.48,0.09);
        }
        { // Source-paired radical case 51: sqrt, inline, size=2, wrapper=0.75
            RadicalInput input{make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"),{},RadicalFontParameters{0.06,0.12,0.43},false,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==false);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==0.49500000000000005);expect(request.stretch_height==0.42500000000000004);expect(request.stretch_depth==0.07);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.24, 0.29, 0.09, 0.88, 1.17, -0.06, 0.11, 0.03, 0.02, -0.01, 0.05,""),2);
            check_box(layout.bbox,make_box(0.72, 0.5449999999999999, 0.07, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.06);expect(layout.q==0.135);expect(layout.required_surd_height==0.49500000000000005);
            expect(layout.measured_height==0.425);expect(layout.placed_height==0.45499999999999996);expect(layout.placed_surd_height==0.09);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.16499999999999998);
            check_point(layout.base_origin,0.24,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.24,0.365,0.48,0.09);
        }
        { // Source-paired radical case 52: root, inline, size=2, wrapper=0.75
            RadicalInput input{make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"),make_box(0.11, 0.2, 0.28, 1.2, 0.64, -0.07, 0.09, 0.04, -0.01, 0.02, -0.04,"9%"),RadicalFontParameters{0.06,0.12,0.43},false,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.11, 0.2, 0.28, 1.2, 0.64, -0.07, 0.09, 0.04, -0.01, 0.02, -0.04,"9%"));
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==false);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==0.49500000000000005);expect(request.stretch_height==0.42500000000000004);expect(request.stretch_depth==0.07);expect(request.root_width==0.47040000000000004);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.24, 0.29, 0.09, 0.88, 1.17, -0.06, 0.11, 0.03, 0.02, -0.01, 0.05,""),2);
            check_box(layout.bbox,make_box(0.72, 0.5612, 0.07, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.06);expect(layout.q==0.135);expect(layout.required_surd_height==0.49500000000000005);
            expect(layout.measured_height==0.425);expect(layout.placed_height==0.45499999999999996);expect(layout.placed_surd_height==0.09);
            expect(layout.root_offset==0);expect(layout.root_height==0.4632);expect(layout.root_dx==0.11499999999999998);
            check_point(layout.surd_origin,0,0.16499999999999998);
            check_point(layout.base_origin,0.24,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0.07359999999999998,0.4632);else expect(false);
            check_rect(layout.overbar,0.24,0.365,0.48,0.09);
        }
        { // Source-paired radical case 53: sqrt, inline, size=-1, wrapper=1
            RadicalInput input{make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"),{},RadicalFontParameters{0.06,0.12,0.43},false,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==0.49500000000000005);expect(request.stretch_height==0.42500000000000004);expect(request.stretch_depth==0.07);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.24, 0.29, 0.09, 0.88, 1.17, -0.06, 0.11, 0.03, 0.02, -0.01, 0.05,""),-1);
            check_box(layout.bbox,make_box(0.72, 0.5449999999999999, 0.07, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.06);expect(layout.q==0.135);expect(layout.required_surd_height==0.49500000000000005);
            expect(layout.measured_height==0.425);expect(layout.placed_height==0.485);expect(layout.placed_surd_height==0.12);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.195);
            check_point(layout.base_origin,0.24,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.24,0.365,0.48,0.12);
        }
        { // Source-paired radical case 54: root, inline, size=-1, wrapper=1
            RadicalInput input{make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"),make_box(0.11, 0.2, 0.28, 1.2, 0.64, -0.07, 0.09, 0.04, -0.01, 0.02, -0.04,"9%"),RadicalFontParameters{0.06,0.12,0.43},false,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.11, 0.2, 0.28, 1.2, 0.64, -0.07, 0.09, 0.04, -0.01, 0.02, -0.04,"9%"));
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==0.49500000000000005);expect(request.stretch_height==0.42500000000000004);expect(request.stretch_depth==0.07);expect(request.root_width==0.47040000000000004);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.24, 0.29, 0.09, 0.88, 1.17, -0.06, 0.11, 0.03, 0.02, -0.01, 0.05,""),-1);
            check_box(layout.bbox,make_box(0.72, 2.2522, 0.07, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.06);expect(layout.q==0.135);expect(layout.required_surd_height==0.49500000000000005);
            expect(layout.measured_height==0.425);expect(layout.placed_height==0.485);expect(layout.placed_surd_height==0.12);
            expect(layout.root_offset==0);expect(layout.root_height==2.1841999999999997);expect(layout.root_dx==0.0775);
            check_point(layout.surd_origin,0,0.195);
            check_point(layout.base_origin,0.24,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0.0496,2.1841999999999997);else expect(false);
            check_rect(layout.overbar,0.24,0.365,0.48,0.12);
        }
        { // Source-paired radical case 55: sqrt, inline, size=2, wrapper=1
            RadicalInput input{make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"),{},RadicalFontParameters{0.06,0.12,0.43},false,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==0.49500000000000005);expect(request.stretch_height==0.42500000000000004);expect(request.stretch_depth==0.07);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.24, 0.29, 0.09, 0.88, 1.17, -0.06, 0.11, 0.03, 0.02, -0.01, 0.05,""),2);
            check_box(layout.bbox,make_box(0.72, 0.5449999999999999, 0.07, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.06);expect(layout.q==0.135);expect(layout.required_surd_height==0.49500000000000005);
            expect(layout.measured_height==0.425);expect(layout.placed_height==0.485);expect(layout.placed_surd_height==0.12);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.195);
            check_point(layout.base_origin,0.24,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.24,0.365,0.48,0.12);
        }
        { // Source-paired radical case 56: root, inline, size=2, wrapper=1
            RadicalInput input{make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"),make_box(0.11, 0.2, 0.28, 1.2, 0.64, -0.07, 0.09, 0.04, -0.01, 0.02, -0.04,"9%"),RadicalFontParameters{0.06,0.12,0.43},false,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.11, 0.2, 0.28, 1.2, 0.64, -0.07, 0.09, 0.04, -0.01, 0.02, -0.04,"9%"));
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==0.49500000000000005);expect(request.stretch_height==0.42500000000000004);expect(request.stretch_depth==0.07);expect(request.root_width==0.47040000000000004);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.24, 0.29, 0.09, 0.88, 1.17, -0.06, 0.11, 0.03, 0.02, -0.01, 0.05,""),2);
            check_box(layout.bbox,make_box(0.72, 0.5612, 0.07, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.06);expect(layout.q==0.135);expect(layout.required_surd_height==0.49500000000000005);
            expect(layout.measured_height==0.425);expect(layout.placed_height==0.485);expect(layout.placed_surd_height==0.12);
            expect(layout.root_offset==0);expect(layout.root_height==0.4932);expect(layout.root_dx==0.11499999999999998);
            check_point(layout.surd_origin,0,0.195);
            check_point(layout.base_origin,0.24,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0.07359999999999998,0.4932);else expect(false);
            check_rect(layout.overbar,0.24,0.365,0.48,0.12);
        }
        { // Source-paired radical case 57: sqrt, inline, size=-1, wrapper=1.35
            RadicalInput input{make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"),{},RadicalFontParameters{0.06,0.12,0.43},false,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==0.49500000000000005);expect(request.stretch_height==0.42500000000000004);expect(request.stretch_depth==0.07);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.24, 0.29, 0.09, 0.88, 1.17, -0.06, 0.11, 0.03, 0.02, -0.01, 0.05,""),-1);
            check_box(layout.bbox,make_box(0.72, 0.5449999999999999, 0.07, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.06);expect(layout.q==0.135);expect(layout.required_surd_height==0.49500000000000005);
            expect(layout.measured_height==0.425);expect(layout.placed_height==0.527);expect(layout.placed_surd_height==0.162);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.23700000000000004);
            check_point(layout.base_origin,0.24,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.24,0.365,0.48,0.162);
        }
        { // Source-paired radical case 58: root, inline, size=-1, wrapper=1.35
            RadicalInput input{make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"),make_box(0.11, 0.2, 0.28, 1.2, 0.64, -0.07, 0.09, 0.04, -0.01, 0.02, -0.04,"9%"),RadicalFontParameters{0.06,0.12,0.43},false,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.11, 0.2, 0.28, 1.2, 0.64, -0.07, 0.09, 0.04, -0.01, 0.02, -0.04,"9%"));
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==0.49500000000000005);expect(request.stretch_height==0.42500000000000004);expect(request.stretch_depth==0.07);expect(request.root_width==0.47040000000000004);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.24, 0.29, 0.09, 0.88, 1.17, -0.06, 0.11, 0.03, 0.02, -0.01, 0.05,""),-1);
            check_box(layout.bbox,make_box(0.72, 2.2522, 0.07, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.06);expect(layout.q==0.135);expect(layout.required_surd_height==0.49500000000000005);
            expect(layout.measured_height==0.425);expect(layout.placed_height==0.527);expect(layout.placed_surd_height==0.162);
            expect(layout.root_offset==0);expect(layout.root_height==2.2261999999999995);expect(layout.root_dx==0.0775);
            check_point(layout.surd_origin,0,0.23700000000000004);
            check_point(layout.base_origin,0.24,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0.0496,2.2261999999999995);else expect(false);
            check_rect(layout.overbar,0.24,0.365,0.48,0.162);
        }
        { // Source-paired radical case 59: sqrt, inline, size=2, wrapper=1.35
            RadicalInput input{make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"),{},RadicalFontParameters{0.06,0.12,0.43},false,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==0.49500000000000005);expect(request.stretch_height==0.42500000000000004);expect(request.stretch_depth==0.07);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.24, 0.29, 0.09, 0.88, 1.17, -0.06, 0.11, 0.03, 0.02, -0.01, 0.05,""),2);
            check_box(layout.bbox,make_box(0.72, 0.5449999999999999, 0.07, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.06);expect(layout.q==0.135);expect(layout.required_surd_height==0.49500000000000005);
            expect(layout.measured_height==0.425);expect(layout.placed_height==0.527);expect(layout.placed_surd_height==0.162);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.23700000000000004);
            check_point(layout.base_origin,0.24,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.24,0.365,0.48,0.162);
        }
        { // Source-paired radical case 60: root, inline, size=2, wrapper=1.35
            RadicalInput input{make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"),make_box(0.11, 0.2, 0.28, 1.2, 0.64, -0.07, 0.09, 0.04, -0.01, 0.02, -0.04,"9%"),RadicalFontParameters{0.06,0.12,0.43},false,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.11, 0.2, 0.28, 1.2, 0.64, -0.07, 0.09, 0.04, -0.01, 0.02, -0.04,"9%"));
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==0.49500000000000005);expect(request.stretch_height==0.42500000000000004);expect(request.stretch_depth==0.07);expect(request.root_width==0.47040000000000004);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.24, 0.29, 0.09, 0.88, 1.17, -0.06, 0.11, 0.03, 0.02, -0.01, 0.05,""),2);
            check_box(layout.bbox,make_box(0.72, 0.5612, 0.07, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.06);expect(layout.q==0.135);expect(layout.required_surd_height==0.49500000000000005);
            expect(layout.measured_height==0.425);expect(layout.placed_height==0.527);expect(layout.placed_surd_height==0.162);
            expect(layout.root_offset==0);expect(layout.root_height==0.5352000000000001);expect(layout.root_dx==0.11499999999999998);
            check_point(layout.surd_origin,0,0.23700000000000004);
            check_point(layout.base_origin,0.24,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0.07359999999999998,0.5352000000000001);else expect(false);
            check_rect(layout.overbar,0.24,0.365,0.48,0.162);
        }
        { // Source-paired radical case 61: sqrt, display, size=-1, wrapper=0.75
            RadicalInput input{make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"),{},RadicalFontParameters{0.06,0.12,0.43},true,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==true);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==0.5875);expect(request.stretch_height==0.5175000000000001);expect(request.stretch_depth==0.07);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.24, 0.29, 0.09, 0.88, 1.17, -0.06, 0.11, 0.03, 0.02, -0.01, 0.05,""),-1);
            check_box(layout.bbox,make_box(0.72, 0.6375000000000001, 0.07, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.43);expect(layout.q==0.22749999999999998);expect(layout.required_surd_height==0.5875);
            expect(layout.measured_height==0.5175000000000001);expect(layout.placed_height==0.5475);expect(layout.placed_surd_height==0.09);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.2575);
            check_point(layout.base_origin,0.24,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.24,0.4575,0.48,0.09);
        }
        { // Source-paired radical case 62: root, display, size=-1, wrapper=0.75
            RadicalInput input{make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"),make_box(0.11, 0.2, 0.28, 1.2, 0.64, -0.07, 0.09, 0.04, -0.01, 0.02, -0.04,"9%"),RadicalFontParameters{0.06,0.12,0.43},true,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.11, 0.2, 0.28, 1.2, 0.64, -0.07, 0.09, 0.04, -0.01, 0.02, -0.04,"9%"));
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==true);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==0.5875);expect(request.stretch_height==0.5175000000000001);expect(request.stretch_depth==0.07);expect(request.root_width==0.47040000000000004);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.24, 0.29, 0.09, 0.88, 1.17, -0.06, 0.11, 0.03, 0.02, -0.01, 0.05,""),-1);
            check_box(layout.bbox,make_box(0.72, 2.3447000000000005, 0.07, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.43);expect(layout.q==0.22749999999999998);expect(layout.required_surd_height==0.5875);
            expect(layout.measured_height==0.5175000000000001);expect(layout.placed_height==0.5475);expect(layout.placed_surd_height==0.09);
            expect(layout.root_offset==0);expect(layout.root_height==2.2466999999999997);expect(layout.root_dx==0.0775);
            check_point(layout.surd_origin,0,0.2575);
            check_point(layout.base_origin,0.24,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0.0496,2.2466999999999997);else expect(false);
            check_rect(layout.overbar,0.24,0.4575,0.48,0.09);
        }
        { // Source-paired radical case 63: sqrt, display, size=2, wrapper=0.75
            RadicalInput input{make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"),{},RadicalFontParameters{0.06,0.12,0.43},true,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==true);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==0.5875);expect(request.stretch_height==0.5175000000000001);expect(request.stretch_depth==0.07);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.24, 0.29, 0.09, 0.88, 1.17, -0.06, 0.11, 0.03, 0.02, -0.01, 0.05,""),2);
            check_box(layout.bbox,make_box(0.72, 0.6375000000000001, 0.07, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.43);expect(layout.q==0.22749999999999998);expect(layout.required_surd_height==0.5875);
            expect(layout.measured_height==0.5175000000000001);expect(layout.placed_height==0.5475);expect(layout.placed_surd_height==0.09);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.2575);
            check_point(layout.base_origin,0.24,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.24,0.4575,0.48,0.09);
        }
        { // Source-paired radical case 64: root, display, size=2, wrapper=0.75
            RadicalInput input{make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"),make_box(0.11, 0.2, 0.28, 1.2, 0.64, -0.07, 0.09, 0.04, -0.01, 0.02, -0.04,"9%"),RadicalFontParameters{0.06,0.12,0.43},true,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.11, 0.2, 0.28, 1.2, 0.64, -0.07, 0.09, 0.04, -0.01, 0.02, -0.04,"9%"));
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==true);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==0.5875);expect(request.stretch_height==0.5175000000000001);expect(request.stretch_depth==0.07);expect(request.root_width==0.47040000000000004);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.24, 0.29, 0.09, 0.88, 1.17, -0.06, 0.11, 0.03, 0.02, -0.01, 0.05,""),2);
            check_box(layout.bbox,make_box(0.72, 0.6537000000000001, 0.07, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.43);expect(layout.q==0.22749999999999998);expect(layout.required_surd_height==0.5875);
            expect(layout.measured_height==0.5175000000000001);expect(layout.placed_height==0.5475);expect(layout.placed_surd_height==0.09);
            expect(layout.root_offset==0);expect(layout.root_height==0.5557000000000001);expect(layout.root_dx==0.11499999999999998);
            check_point(layout.surd_origin,0,0.2575);
            check_point(layout.base_origin,0.24,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0.07359999999999998,0.5557000000000001);else expect(false);
            check_rect(layout.overbar,0.24,0.4575,0.48,0.09);
        }
        { // Source-paired radical case 65: sqrt, display, size=-1, wrapper=1
            RadicalInput input{make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"),{},RadicalFontParameters{0.06,0.12,0.43},true,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==0.5875);expect(request.stretch_height==0.5175000000000001);expect(request.stretch_depth==0.07);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.24, 0.29, 0.09, 0.88, 1.17, -0.06, 0.11, 0.03, 0.02, -0.01, 0.05,""),-1);
            check_box(layout.bbox,make_box(0.72, 0.6375000000000001, 0.07, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.43);expect(layout.q==0.22749999999999998);expect(layout.required_surd_height==0.5875);
            expect(layout.measured_height==0.5175000000000001);expect(layout.placed_height==0.5775);expect(layout.placed_surd_height==0.12);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.28750000000000003);
            check_point(layout.base_origin,0.24,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.24,0.4575,0.48,0.12);
        }
        { // Source-paired radical case 66: root, display, size=-1, wrapper=1
            RadicalInput input{make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"),make_box(0.11, 0.2, 0.28, 1.2, 0.64, -0.07, 0.09, 0.04, -0.01, 0.02, -0.04,"9%"),RadicalFontParameters{0.06,0.12,0.43},true,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.11, 0.2, 0.28, 1.2, 0.64, -0.07, 0.09, 0.04, -0.01, 0.02, -0.04,"9%"));
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==0.5875);expect(request.stretch_height==0.5175000000000001);expect(request.stretch_depth==0.07);expect(request.root_width==0.47040000000000004);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.24, 0.29, 0.09, 0.88, 1.17, -0.06, 0.11, 0.03, 0.02, -0.01, 0.05,""),-1);
            check_box(layout.bbox,make_box(0.72, 2.3447000000000005, 0.07, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.43);expect(layout.q==0.22749999999999998);expect(layout.required_surd_height==0.5875);
            expect(layout.measured_height==0.5175000000000001);expect(layout.placed_height==0.5775);expect(layout.placed_surd_height==0.12);
            expect(layout.root_offset==0);expect(layout.root_height==2.2767);expect(layout.root_dx==0.0775);
            check_point(layout.surd_origin,0,0.28750000000000003);
            check_point(layout.base_origin,0.24,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0.0496,2.2767);else expect(false);
            check_rect(layout.overbar,0.24,0.4575,0.48,0.12);
        }
        { // Source-paired radical case 67: sqrt, display, size=2, wrapper=1
            RadicalInput input{make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"),{},RadicalFontParameters{0.06,0.12,0.43},true,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==0.5875);expect(request.stretch_height==0.5175000000000001);expect(request.stretch_depth==0.07);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.24, 0.29, 0.09, 0.88, 1.17, -0.06, 0.11, 0.03, 0.02, -0.01, 0.05,""),2);
            check_box(layout.bbox,make_box(0.72, 0.6375000000000001, 0.07, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.43);expect(layout.q==0.22749999999999998);expect(layout.required_surd_height==0.5875);
            expect(layout.measured_height==0.5175000000000001);expect(layout.placed_height==0.5775);expect(layout.placed_surd_height==0.12);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.28750000000000003);
            check_point(layout.base_origin,0.24,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.24,0.4575,0.48,0.12);
        }
        { // Source-paired radical case 68: root, display, size=2, wrapper=1
            RadicalInput input{make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"),make_box(0.11, 0.2, 0.28, 1.2, 0.64, -0.07, 0.09, 0.04, -0.01, 0.02, -0.04,"9%"),RadicalFontParameters{0.06,0.12,0.43},true,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.11, 0.2, 0.28, 1.2, 0.64, -0.07, 0.09, 0.04, -0.01, 0.02, -0.04,"9%"));
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==0.5875);expect(request.stretch_height==0.5175000000000001);expect(request.stretch_depth==0.07);expect(request.root_width==0.47040000000000004);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.24, 0.29, 0.09, 0.88, 1.17, -0.06, 0.11, 0.03, 0.02, -0.01, 0.05,""),2);
            check_box(layout.bbox,make_box(0.72, 0.6537000000000001, 0.07, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.43);expect(layout.q==0.22749999999999998);expect(layout.required_surd_height==0.5875);
            expect(layout.measured_height==0.5175000000000001);expect(layout.placed_height==0.5775);expect(layout.placed_surd_height==0.12);
            expect(layout.root_offset==0);expect(layout.root_height==0.5857000000000001);expect(layout.root_dx==0.11499999999999998);
            check_point(layout.surd_origin,0,0.28750000000000003);
            check_point(layout.base_origin,0.24,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0.07359999999999998,0.5857000000000001);else expect(false);
            check_rect(layout.overbar,0.24,0.4575,0.48,0.12);
        }
        { // Source-paired radical case 69: sqrt, display, size=-1, wrapper=1.35
            RadicalInput input{make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"),{},RadicalFontParameters{0.06,0.12,0.43},true,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==0.5875);expect(request.stretch_height==0.5175000000000001);expect(request.stretch_depth==0.07);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.24, 0.29, 0.09, 0.88, 1.17, -0.06, 0.11, 0.03, 0.02, -0.01, 0.05,""),-1);
            check_box(layout.bbox,make_box(0.72, 0.6375000000000001, 0.07, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.43);expect(layout.q==0.22749999999999998);expect(layout.required_surd_height==0.5875);
            expect(layout.measured_height==0.5175000000000001);expect(layout.placed_height==0.6195);expect(layout.placed_surd_height==0.162);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.32950000000000007);
            check_point(layout.base_origin,0.24,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.24,0.4575,0.48,0.162);
        }
        { // Source-paired radical case 70: root, display, size=-1, wrapper=1.35
            RadicalInput input{make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"),make_box(0.11, 0.2, 0.28, 1.2, 0.64, -0.07, 0.09, 0.04, -0.01, 0.02, -0.04,"9%"),RadicalFontParameters{0.06,0.12,0.43},true,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.11, 0.2, 0.28, 1.2, 0.64, -0.07, 0.09, 0.04, -0.01, 0.02, -0.04,"9%"));
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==0.5875);expect(request.stretch_height==0.5175000000000001);expect(request.stretch_depth==0.07);expect(request.root_width==0.47040000000000004);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.24, 0.29, 0.09, 0.88, 1.17, -0.06, 0.11, 0.03, 0.02, -0.01, 0.05,""),-1);
            check_box(layout.bbox,make_box(0.72, 2.3447000000000005, 0.07, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.43);expect(layout.q==0.22749999999999998);expect(layout.required_surd_height==0.5875);
            expect(layout.measured_height==0.5175000000000001);expect(layout.placed_height==0.6195);expect(layout.placed_surd_height==0.162);
            expect(layout.root_offset==0);expect(layout.root_height==2.3186999999999998);expect(layout.root_dx==0.0775);
            check_point(layout.surd_origin,0,0.32950000000000007);
            check_point(layout.base_origin,0.24,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0.0496,2.3186999999999998);else expect(false);
            check_rect(layout.overbar,0.24,0.4575,0.48,0.162);
        }
        { // Source-paired radical case 71: sqrt, display, size=2, wrapper=1.35
            RadicalInput input{make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"),{},RadicalFontParameters{0.06,0.12,0.43},true,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==0.5875);expect(request.stretch_height==0.5175000000000001);expect(request.stretch_depth==0.07);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.24, 0.29, 0.09, 0.88, 1.17, -0.06, 0.11, 0.03, 0.02, -0.01, 0.05,""),2);
            check_box(layout.bbox,make_box(0.72, 0.6375000000000001, 0.07, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.43);expect(layout.q==0.22749999999999998);expect(layout.required_surd_height==0.5875);
            expect(layout.measured_height==0.5175000000000001);expect(layout.placed_height==0.6195);expect(layout.placed_surd_height==0.162);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.32950000000000007);
            check_point(layout.base_origin,0.24,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.24,0.4575,0.48,0.162);
        }
        { // Source-paired radical case 72: root, display, size=2, wrapper=1.35
            RadicalInput input{make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"),make_box(0.11, 0.2, 0.28, 1.2, 0.64, -0.07, 0.09, 0.04, -0.01, 0.02, -0.04,"9%"),RadicalFontParameters{0.06,0.12,0.43},true,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.11, 0.2, 0.28, 1.2, 0.64, -0.07, 0.09, 0.04, -0.01, 0.02, -0.04,"9%"));
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==0.5875);expect(request.stretch_height==0.5175000000000001);expect(request.stretch_depth==0.07);expect(request.root_width==0.47040000000000004);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.24, 0.29, 0.09, 0.88, 1.17, -0.06, 0.11, 0.03, 0.02, -0.01, 0.05,""),2);
            check_box(layout.bbox,make_box(0.72, 0.6537000000000001, 0.07, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.43);expect(layout.q==0.22749999999999998);expect(layout.required_surd_height==0.5875);
            expect(layout.measured_height==0.5175000000000001);expect(layout.placed_height==0.6195);expect(layout.placed_surd_height==0.162);
            expect(layout.root_offset==0);expect(layout.root_height==0.6277000000000001);expect(layout.root_dx==0.11499999999999998);
            check_point(layout.surd_origin,0,0.32950000000000007);
            check_point(layout.base_origin,0.24,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0.07359999999999998,0.6277000000000001);else expect(false);
            check_rect(layout.overbar,0.24,0.4575,0.48,0.162);
        }
        { // Source-paired radical case 73: sqrt, inline, size=-1, wrapper=0.75
            RadicalInput input{make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"),{},RadicalFontParameters{0.15,0.08,0.27},false,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==false);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==0.5675);expect(request.stretch_height==0.4975);expect(request.stretch_depth==0.07);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.24, 0.29, 0.09, 0.88, 1.17, -0.06, 0.11, 0.03, 0.02, -0.01, 0.05,""),-1);
            check_box(layout.bbox,make_box(0.72, 0.5775, 0.07, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.15);expect(layout.q==0.1175);expect(layout.required_surd_height==0.5675);
            expect(layout.measured_height==0.49750000000000005);expect(layout.placed_height==0.40750000000000003);expect(layout.placed_surd_height==0.06);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.11750000000000005);
            check_point(layout.base_origin,0.24,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.24,0.34750000000000003,0.48,0.06);
        }
        { // Source-paired radical case 74: root, inline, size=-1, wrapper=0.75
            RadicalInput input{make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"),make_box(0.11, 0.2, 0.28, 1.2, 0.64, -0.07, 0.09, 0.04, -0.01, 0.02, -0.04,"9%"),RadicalFontParameters{0.15,0.08,0.27},false,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.11, 0.2, 0.28, 1.2, 0.64, -0.07, 0.09, 0.04, -0.01, 0.02, -0.04,"9%"));
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==false);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==0.5675);expect(request.stretch_height==0.4975);expect(request.stretch_depth==0.07);expect(request.root_width==0.47040000000000004);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.24, 0.29, 0.09, 0.88, 1.17, -0.06, 0.11, 0.03, 0.02, -0.01, 0.05,""),-1);
            check_box(layout.bbox,make_box(0.72, 2.3247, 0.07, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.15);expect(layout.q==0.1175);expect(layout.required_surd_height==0.5675);
            expect(layout.measured_height==0.49750000000000005);expect(layout.placed_height==0.40750000000000003);expect(layout.placed_surd_height==0.06);
            expect(layout.root_offset==0);expect(layout.root_height==2.1067);expect(layout.root_dx==0.0775);
            check_point(layout.surd_origin,0,0.11750000000000005);
            check_point(layout.base_origin,0.24,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0.0496,2.1067);else expect(false);
            check_rect(layout.overbar,0.24,0.34750000000000003,0.48,0.06);
        }
        { // Source-paired radical case 75: sqrt, inline, size=2, wrapper=0.75
            RadicalInput input{make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"),{},RadicalFontParameters{0.15,0.08,0.27},false,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==false);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==0.5675);expect(request.stretch_height==0.4975);expect(request.stretch_depth==0.07);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.24, 0.29, 0.09, 0.88, 1.17, -0.06, 0.11, 0.03, 0.02, -0.01, 0.05,""),2);
            check_box(layout.bbox,make_box(0.72, 0.5775, 0.07, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.15);expect(layout.q==0.1175);expect(layout.required_surd_height==0.5675);
            expect(layout.measured_height==0.49750000000000005);expect(layout.placed_height==0.40750000000000003);expect(layout.placed_surd_height==0.06);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.11750000000000005);
            check_point(layout.base_origin,0.24,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.24,0.34750000000000003,0.48,0.06);
        }
        { // Source-paired radical case 76: root, inline, size=2, wrapper=0.75
            RadicalInput input{make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"),make_box(0.11, 0.2, 0.28, 1.2, 0.64, -0.07, 0.09, 0.04, -0.01, 0.02, -0.04,"9%"),RadicalFontParameters{0.15,0.08,0.27},false,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.11, 0.2, 0.28, 1.2, 0.64, -0.07, 0.09, 0.04, -0.01, 0.02, -0.04,"9%"));
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==false);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==0.5675);expect(request.stretch_height==0.4975);expect(request.stretch_depth==0.07);expect(request.root_width==0.47040000000000004);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.24, 0.29, 0.09, 0.88, 1.17, -0.06, 0.11, 0.03, 0.02, -0.01, 0.05,""),2);
            check_box(layout.bbox,make_box(0.72, 0.6337, 0.07, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.15);expect(layout.q==0.1175);expect(layout.required_surd_height==0.5675);
            expect(layout.measured_height==0.49750000000000005);expect(layout.placed_height==0.40750000000000003);expect(layout.placed_surd_height==0.06);
            expect(layout.root_offset==0);expect(layout.root_height==0.41570000000000007);expect(layout.root_dx==0.11499999999999998);
            check_point(layout.surd_origin,0,0.11750000000000005);
            check_point(layout.base_origin,0.24,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0.07359999999999998,0.41570000000000007);else expect(false);
            check_rect(layout.overbar,0.24,0.34750000000000003,0.48,0.06);
        }
        { // Source-paired radical case 77: sqrt, inline, size=-1, wrapper=1
            RadicalInput input{make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"),{},RadicalFontParameters{0.15,0.08,0.27},false,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==0.5675);expect(request.stretch_height==0.4975);expect(request.stretch_depth==0.07);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.24, 0.29, 0.09, 0.88, 1.17, -0.06, 0.11, 0.03, 0.02, -0.01, 0.05,""),-1);
            check_box(layout.bbox,make_box(0.72, 0.5775, 0.07, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.15);expect(layout.q==0.1175);expect(layout.required_surd_height==0.5675);
            expect(layout.measured_height==0.49750000000000005);expect(layout.placed_height==0.42750000000000005);expect(layout.placed_surd_height==0.08);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.13750000000000007);
            check_point(layout.base_origin,0.24,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.24,0.34750000000000003,0.48,0.08);
        }
        { // Source-paired radical case 78: root, inline, size=-1, wrapper=1
            RadicalInput input{make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"),make_box(0.11, 0.2, 0.28, 1.2, 0.64, -0.07, 0.09, 0.04, -0.01, 0.02, -0.04,"9%"),RadicalFontParameters{0.15,0.08,0.27},false,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.11, 0.2, 0.28, 1.2, 0.64, -0.07, 0.09, 0.04, -0.01, 0.02, -0.04,"9%"));
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==0.5675);expect(request.stretch_height==0.4975);expect(request.stretch_depth==0.07);expect(request.root_width==0.47040000000000004);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.24, 0.29, 0.09, 0.88, 1.17, -0.06, 0.11, 0.03, 0.02, -0.01, 0.05,""),-1);
            check_box(layout.bbox,make_box(0.72, 2.3247, 0.07, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.15);expect(layout.q==0.1175);expect(layout.required_surd_height==0.5675);
            expect(layout.measured_height==0.49750000000000005);expect(layout.placed_height==0.42750000000000005);expect(layout.placed_surd_height==0.08);
            expect(layout.root_offset==0);expect(layout.root_height==2.1267);expect(layout.root_dx==0.0775);
            check_point(layout.surd_origin,0,0.13750000000000007);
            check_point(layout.base_origin,0.24,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0.0496,2.1267);else expect(false);
            check_rect(layout.overbar,0.24,0.34750000000000003,0.48,0.08);
        }
        { // Source-paired radical case 79: sqrt, inline, size=2, wrapper=1
            RadicalInput input{make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"),{},RadicalFontParameters{0.15,0.08,0.27},false,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==0.5675);expect(request.stretch_height==0.4975);expect(request.stretch_depth==0.07);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.24, 0.29, 0.09, 0.88, 1.17, -0.06, 0.11, 0.03, 0.02, -0.01, 0.05,""),2);
            check_box(layout.bbox,make_box(0.72, 0.5775, 0.07, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.15);expect(layout.q==0.1175);expect(layout.required_surd_height==0.5675);
            expect(layout.measured_height==0.49750000000000005);expect(layout.placed_height==0.42750000000000005);expect(layout.placed_surd_height==0.08);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.13750000000000007);
            check_point(layout.base_origin,0.24,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.24,0.34750000000000003,0.48,0.08);
        }
        { // Source-paired radical case 80: root, inline, size=2, wrapper=1
            RadicalInput input{make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"),make_box(0.11, 0.2, 0.28, 1.2, 0.64, -0.07, 0.09, 0.04, -0.01, 0.02, -0.04,"9%"),RadicalFontParameters{0.15,0.08,0.27},false,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.11, 0.2, 0.28, 1.2, 0.64, -0.07, 0.09, 0.04, -0.01, 0.02, -0.04,"9%"));
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==0.5675);expect(request.stretch_height==0.4975);expect(request.stretch_depth==0.07);expect(request.root_width==0.47040000000000004);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.24, 0.29, 0.09, 0.88, 1.17, -0.06, 0.11, 0.03, 0.02, -0.01, 0.05,""),2);
            check_box(layout.bbox,make_box(0.72, 0.6337, 0.07, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.15);expect(layout.q==0.1175);expect(layout.required_surd_height==0.5675);
            expect(layout.measured_height==0.49750000000000005);expect(layout.placed_height==0.42750000000000005);expect(layout.placed_surd_height==0.08);
            expect(layout.root_offset==0);expect(layout.root_height==0.4357000000000001);expect(layout.root_dx==0.11499999999999998);
            check_point(layout.surd_origin,0,0.13750000000000007);
            check_point(layout.base_origin,0.24,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0.07359999999999998,0.4357000000000001);else expect(false);
            check_rect(layout.overbar,0.24,0.34750000000000003,0.48,0.08);
        }
        { // Source-paired radical case 81: sqrt, inline, size=-1, wrapper=1.35
            RadicalInput input{make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"),{},RadicalFontParameters{0.15,0.08,0.27},false,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==0.5675);expect(request.stretch_height==0.4975);expect(request.stretch_depth==0.07);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.24, 0.29, 0.09, 0.88, 1.17, -0.06, 0.11, 0.03, 0.02, -0.01, 0.05,""),-1);
            check_box(layout.bbox,make_box(0.72, 0.5775, 0.07, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.15);expect(layout.q==0.1175);expect(layout.required_surd_height==0.5675);
            expect(layout.measured_height==0.49750000000000005);expect(layout.placed_height==0.4555);expect(layout.placed_surd_height==0.10800000000000001);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.16550000000000004);
            check_point(layout.base_origin,0.24,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.24,0.34750000000000003,0.48,0.10800000000000001);
        }
        { // Source-paired radical case 82: root, inline, size=-1, wrapper=1.35
            RadicalInput input{make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"),make_box(0.11, 0.2, 0.28, 1.2, 0.64, -0.07, 0.09, 0.04, -0.01, 0.02, -0.04,"9%"),RadicalFontParameters{0.15,0.08,0.27},false,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.11, 0.2, 0.28, 1.2, 0.64, -0.07, 0.09, 0.04, -0.01, 0.02, -0.04,"9%"));
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==0.5675);expect(request.stretch_height==0.4975);expect(request.stretch_depth==0.07);expect(request.root_width==0.47040000000000004);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.24, 0.29, 0.09, 0.88, 1.17, -0.06, 0.11, 0.03, 0.02, -0.01, 0.05,""),-1);
            check_box(layout.bbox,make_box(0.72, 2.3247, 0.07, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.15);expect(layout.q==0.1175);expect(layout.required_surd_height==0.5675);
            expect(layout.measured_height==0.49750000000000005);expect(layout.placed_height==0.4555);expect(layout.placed_surd_height==0.10800000000000001);
            expect(layout.root_offset==0);expect(layout.root_height==2.1547);expect(layout.root_dx==0.0775);
            check_point(layout.surd_origin,0,0.16550000000000004);
            check_point(layout.base_origin,0.24,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0.0496,2.1547);else expect(false);
            check_rect(layout.overbar,0.24,0.34750000000000003,0.48,0.10800000000000001);
        }
        { // Source-paired radical case 83: sqrt, inline, size=2, wrapper=1.35
            RadicalInput input{make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"),{},RadicalFontParameters{0.15,0.08,0.27},false,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==0.5675);expect(request.stretch_height==0.4975);expect(request.stretch_depth==0.07);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.24, 0.29, 0.09, 0.88, 1.17, -0.06, 0.11, 0.03, 0.02, -0.01, 0.05,""),2);
            check_box(layout.bbox,make_box(0.72, 0.5775, 0.07, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.15);expect(layout.q==0.1175);expect(layout.required_surd_height==0.5675);
            expect(layout.measured_height==0.49750000000000005);expect(layout.placed_height==0.4555);expect(layout.placed_surd_height==0.10800000000000001);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.16550000000000004);
            check_point(layout.base_origin,0.24,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.24,0.34750000000000003,0.48,0.10800000000000001);
        }
        { // Source-paired radical case 84: root, inline, size=2, wrapper=1.35
            RadicalInput input{make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"),make_box(0.11, 0.2, 0.28, 1.2, 0.64, -0.07, 0.09, 0.04, -0.01, 0.02, -0.04,"9%"),RadicalFontParameters{0.15,0.08,0.27},false,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.11, 0.2, 0.28, 1.2, 0.64, -0.07, 0.09, 0.04, -0.01, 0.02, -0.04,"9%"));
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==0.5675);expect(request.stretch_height==0.4975);expect(request.stretch_depth==0.07);expect(request.root_width==0.47040000000000004);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.24, 0.29, 0.09, 0.88, 1.17, -0.06, 0.11, 0.03, 0.02, -0.01, 0.05,""),2);
            check_box(layout.bbox,make_box(0.72, 0.6337, 0.07, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.15);expect(layout.q==0.1175);expect(layout.required_surd_height==0.5675);
            expect(layout.measured_height==0.49750000000000005);expect(layout.placed_height==0.4555);expect(layout.placed_surd_height==0.10800000000000001);
            expect(layout.root_offset==0);expect(layout.root_height==0.46370000000000006);expect(layout.root_dx==0.11499999999999998);
            check_point(layout.surd_origin,0,0.16550000000000004);
            check_point(layout.base_origin,0.24,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0.07359999999999998,0.46370000000000006);else expect(false);
            check_rect(layout.overbar,0.24,0.34750000000000003,0.48,0.10800000000000001);
        }
        { // Source-paired radical case 85: sqrt, display, size=-1, wrapper=0.75
            RadicalInput input{make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"),{},RadicalFontParameters{0.15,0.08,0.27},true,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==true);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==0.5975);expect(request.stretch_height==0.5275000000000001);expect(request.stretch_depth==0.07);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.24, 0.29, 0.09, 0.88, 1.17, -0.06, 0.11, 0.03, 0.02, -0.01, 0.05,""),-1);
            check_box(layout.bbox,make_box(0.72, 0.6075, 0.07, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.27);expect(layout.q==0.14750000000000002);expect(layout.required_surd_height==0.5975);
            expect(layout.measured_height==0.5275000000000001);expect(layout.placed_height==0.43750000000000006);expect(layout.placed_surd_height==0.06);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.14750000000000008);
            check_point(layout.base_origin,0.24,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.24,0.37750000000000006,0.48,0.06);
        }
        { // Source-paired radical case 86: root, display, size=-1, wrapper=0.75
            RadicalInput input{make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"),make_box(0.11, 0.2, 0.28, 1.2, 0.64, -0.07, 0.09, 0.04, -0.01, 0.02, -0.04,"9%"),RadicalFontParameters{0.15,0.08,0.27},true,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.11, 0.2, 0.28, 1.2, 0.64, -0.07, 0.09, 0.04, -0.01, 0.02, -0.04,"9%"));
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==true);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==0.5975);expect(request.stretch_height==0.5275000000000001);expect(request.stretch_depth==0.07);expect(request.root_width==0.47040000000000004);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.24, 0.29, 0.09, 0.88, 1.17, -0.06, 0.11, 0.03, 0.02, -0.01, 0.05,""),-1);
            check_box(layout.bbox,make_box(0.72, 2.3547000000000002, 0.07, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.27);expect(layout.q==0.14750000000000002);expect(layout.required_surd_height==0.5975);
            expect(layout.measured_height==0.5275000000000001);expect(layout.placed_height==0.43750000000000006);expect(layout.placed_surd_height==0.06);
            expect(layout.root_offset==0);expect(layout.root_height==2.1367000000000003);expect(layout.root_dx==0.0775);
            check_point(layout.surd_origin,0,0.14750000000000008);
            check_point(layout.base_origin,0.24,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0.0496,2.1367000000000003);else expect(false);
            check_rect(layout.overbar,0.24,0.37750000000000006,0.48,0.06);
        }
        { // Source-paired radical case 87: sqrt, display, size=2, wrapper=0.75
            RadicalInput input{make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"),{},RadicalFontParameters{0.15,0.08,0.27},true,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==true);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==0.5975);expect(request.stretch_height==0.5275000000000001);expect(request.stretch_depth==0.07);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.24, 0.29, 0.09, 0.88, 1.17, -0.06, 0.11, 0.03, 0.02, -0.01, 0.05,""),2);
            check_box(layout.bbox,make_box(0.72, 0.6075, 0.07, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.27);expect(layout.q==0.14750000000000002);expect(layout.required_surd_height==0.5975);
            expect(layout.measured_height==0.5275000000000001);expect(layout.placed_height==0.43750000000000006);expect(layout.placed_surd_height==0.06);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.14750000000000008);
            check_point(layout.base_origin,0.24,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.24,0.37750000000000006,0.48,0.06);
        }
        { // Source-paired radical case 88: root, display, size=2, wrapper=0.75
            RadicalInput input{make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"),make_box(0.11, 0.2, 0.28, 1.2, 0.64, -0.07, 0.09, 0.04, -0.01, 0.02, -0.04,"9%"),RadicalFontParameters{0.15,0.08,0.27},true,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.11, 0.2, 0.28, 1.2, 0.64, -0.07, 0.09, 0.04, -0.01, 0.02, -0.04,"9%"));
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==true);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==0.5975);expect(request.stretch_height==0.5275000000000001);expect(request.stretch_depth==0.07);expect(request.root_width==0.47040000000000004);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.24, 0.29, 0.09, 0.88, 1.17, -0.06, 0.11, 0.03, 0.02, -0.01, 0.05,""),2);
            check_box(layout.bbox,make_box(0.72, 0.6637000000000001, 0.07, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.27);expect(layout.q==0.14750000000000002);expect(layout.required_surd_height==0.5975);
            expect(layout.measured_height==0.5275000000000001);expect(layout.placed_height==0.43750000000000006);expect(layout.placed_surd_height==0.06);
            expect(layout.root_offset==0);expect(layout.root_height==0.4457000000000001);expect(layout.root_dx==0.11499999999999998);
            check_point(layout.surd_origin,0,0.14750000000000008);
            check_point(layout.base_origin,0.24,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0.07359999999999998,0.4457000000000001);else expect(false);
            check_rect(layout.overbar,0.24,0.37750000000000006,0.48,0.06);
        }
        { // Source-paired radical case 89: sqrt, display, size=-1, wrapper=1
            RadicalInput input{make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"),{},RadicalFontParameters{0.15,0.08,0.27},true,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==0.5975);expect(request.stretch_height==0.5275000000000001);expect(request.stretch_depth==0.07);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.24, 0.29, 0.09, 0.88, 1.17, -0.06, 0.11, 0.03, 0.02, -0.01, 0.05,""),-1);
            check_box(layout.bbox,make_box(0.72, 0.6075, 0.07, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.27);expect(layout.q==0.14750000000000002);expect(layout.required_surd_height==0.5975);
            expect(layout.measured_height==0.5275000000000001);expect(layout.placed_height==0.4575000000000001);expect(layout.placed_surd_height==0.08);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.1675000000000001);
            check_point(layout.base_origin,0.24,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.24,0.37750000000000006,0.48,0.08);
        }
        { // Source-paired radical case 90: root, display, size=-1, wrapper=1
            RadicalInput input{make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"),make_box(0.11, 0.2, 0.28, 1.2, 0.64, -0.07, 0.09, 0.04, -0.01, 0.02, -0.04,"9%"),RadicalFontParameters{0.15,0.08,0.27},true,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.11, 0.2, 0.28, 1.2, 0.64, -0.07, 0.09, 0.04, -0.01, 0.02, -0.04,"9%"));
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==0.5975);expect(request.stretch_height==0.5275000000000001);expect(request.stretch_depth==0.07);expect(request.root_width==0.47040000000000004);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.24, 0.29, 0.09, 0.88, 1.17, -0.06, 0.11, 0.03, 0.02, -0.01, 0.05,""),-1);
            check_box(layout.bbox,make_box(0.72, 2.3547000000000002, 0.07, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.27);expect(layout.q==0.14750000000000002);expect(layout.required_surd_height==0.5975);
            expect(layout.measured_height==0.5275000000000001);expect(layout.placed_height==0.4575000000000001);expect(layout.placed_surd_height==0.08);
            expect(layout.root_offset==0);expect(layout.root_height==2.1567);expect(layout.root_dx==0.0775);
            check_point(layout.surd_origin,0,0.1675000000000001);
            check_point(layout.base_origin,0.24,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0.0496,2.1567);else expect(false);
            check_rect(layout.overbar,0.24,0.37750000000000006,0.48,0.08);
        }
        { // Source-paired radical case 91: sqrt, display, size=2, wrapper=1
            RadicalInput input{make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"),{},RadicalFontParameters{0.15,0.08,0.27},true,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==0.5975);expect(request.stretch_height==0.5275000000000001);expect(request.stretch_depth==0.07);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.24, 0.29, 0.09, 0.88, 1.17, -0.06, 0.11, 0.03, 0.02, -0.01, 0.05,""),2);
            check_box(layout.bbox,make_box(0.72, 0.6075, 0.07, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.27);expect(layout.q==0.14750000000000002);expect(layout.required_surd_height==0.5975);
            expect(layout.measured_height==0.5275000000000001);expect(layout.placed_height==0.4575000000000001);expect(layout.placed_surd_height==0.08);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.1675000000000001);
            check_point(layout.base_origin,0.24,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.24,0.37750000000000006,0.48,0.08);
        }
        { // Source-paired radical case 92: root, display, size=2, wrapper=1
            RadicalInput input{make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"),make_box(0.11, 0.2, 0.28, 1.2, 0.64, -0.07, 0.09, 0.04, -0.01, 0.02, -0.04,"9%"),RadicalFontParameters{0.15,0.08,0.27},true,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.11, 0.2, 0.28, 1.2, 0.64, -0.07, 0.09, 0.04, -0.01, 0.02, -0.04,"9%"));
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==0.5975);expect(request.stretch_height==0.5275000000000001);expect(request.stretch_depth==0.07);expect(request.root_width==0.47040000000000004);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.24, 0.29, 0.09, 0.88, 1.17, -0.06, 0.11, 0.03, 0.02, -0.01, 0.05,""),2);
            check_box(layout.bbox,make_box(0.72, 0.6637000000000001, 0.07, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.27);expect(layout.q==0.14750000000000002);expect(layout.required_surd_height==0.5975);
            expect(layout.measured_height==0.5275000000000001);expect(layout.placed_height==0.4575000000000001);expect(layout.placed_surd_height==0.08);
            expect(layout.root_offset==0);expect(layout.root_height==0.4657000000000001);expect(layout.root_dx==0.11499999999999998);
            check_point(layout.surd_origin,0,0.1675000000000001);
            check_point(layout.base_origin,0.24,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0.07359999999999998,0.4657000000000001);else expect(false);
            check_rect(layout.overbar,0.24,0.37750000000000006,0.48,0.08);
        }
        { // Source-paired radical case 93: sqrt, display, size=-1, wrapper=1.35
            RadicalInput input{make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"),{},RadicalFontParameters{0.15,0.08,0.27},true,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==0.5975);expect(request.stretch_height==0.5275000000000001);expect(request.stretch_depth==0.07);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.24, 0.29, 0.09, 0.88, 1.17, -0.06, 0.11, 0.03, 0.02, -0.01, 0.05,""),-1);
            check_box(layout.bbox,make_box(0.72, 0.6075, 0.07, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.27);expect(layout.q==0.14750000000000002);expect(layout.required_surd_height==0.5975);
            expect(layout.measured_height==0.5275000000000001);expect(layout.placed_height==0.48550000000000004);expect(layout.placed_surd_height==0.10800000000000001);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.19550000000000006);
            check_point(layout.base_origin,0.24,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.24,0.37750000000000006,0.48,0.10800000000000001);
        }
        { // Source-paired radical case 94: root, display, size=-1, wrapper=1.35
            RadicalInput input{make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"),make_box(0.11, 0.2, 0.28, 1.2, 0.64, -0.07, 0.09, 0.04, -0.01, 0.02, -0.04,"9%"),RadicalFontParameters{0.15,0.08,0.27},true,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.11, 0.2, 0.28, 1.2, 0.64, -0.07, 0.09, 0.04, -0.01, 0.02, -0.04,"9%"));
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==0.5975);expect(request.stretch_height==0.5275000000000001);expect(request.stretch_depth==0.07);expect(request.root_width==0.47040000000000004);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.24, 0.29, 0.09, 0.88, 1.17, -0.06, 0.11, 0.03, 0.02, -0.01, 0.05,""),-1);
            check_box(layout.bbox,make_box(0.72, 2.3547000000000002, 0.07, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.27);expect(layout.q==0.14750000000000002);expect(layout.required_surd_height==0.5975);
            expect(layout.measured_height==0.5275000000000001);expect(layout.placed_height==0.48550000000000004);expect(layout.placed_surd_height==0.10800000000000001);
            expect(layout.root_offset==0);expect(layout.root_height==2.1847000000000003);expect(layout.root_dx==0.0775);
            check_point(layout.surd_origin,0,0.19550000000000006);
            check_point(layout.base_origin,0.24,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0.0496,2.1847000000000003);else expect(false);
            check_rect(layout.overbar,0.24,0.37750000000000006,0.48,0.10800000000000001);
        }
        { // Source-paired radical case 95: sqrt, display, size=2, wrapper=1.35
            RadicalInput input{make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"),{},RadicalFontParameters{0.15,0.08,0.27},true,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==0.5975);expect(request.stretch_height==0.5275000000000001);expect(request.stretch_depth==0.07);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.24, 0.29, 0.09, 0.88, 1.17, -0.06, 0.11, 0.03, 0.02, -0.01, 0.05,""),2);
            check_box(layout.bbox,make_box(0.72, 0.6075, 0.07, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.27);expect(layout.q==0.14750000000000002);expect(layout.required_surd_height==0.5975);
            expect(layout.measured_height==0.5275000000000001);expect(layout.placed_height==0.48550000000000004);expect(layout.placed_surd_height==0.10800000000000001);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.19550000000000006);
            check_point(layout.base_origin,0.24,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.24,0.37750000000000006,0.48,0.10800000000000001);
        }
        { // Source-paired radical case 96: root, display, size=2, wrapper=1.35
            RadicalInput input{make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"),make_box(0.11, 0.2, 0.28, 1.2, 0.64, -0.07, 0.09, 0.04, -0.01, 0.02, -0.04,"9%"),RadicalFontParameters{0.15,0.08,0.27},true,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(0.48, 0.23, 0.07, 0.72, 1.4, -0.04, 0.13, -0.02, 0.06, -0.03, 0.08,"40%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.11, 0.2, 0.28, 1.2, 0.64, -0.07, 0.09, 0.04, -0.01, 0.02, -0.04,"9%"));
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==0.5975);expect(request.stretch_height==0.5275000000000001);expect(request.stretch_depth==0.07);expect(request.root_width==0.47040000000000004);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.24, 0.29, 0.09, 0.88, 1.17, -0.06, 0.11, 0.03, 0.02, -0.01, 0.05,""),2);
            check_box(layout.bbox,make_box(0.72, 0.6637000000000001, 0.07, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.27);expect(layout.q==0.14750000000000002);expect(layout.required_surd_height==0.5975);
            expect(layout.measured_height==0.5275000000000001);expect(layout.placed_height==0.48550000000000004);expect(layout.placed_surd_height==0.10800000000000001);
            expect(layout.root_offset==0);expect(layout.root_height==0.4937000000000001);expect(layout.root_dx==0.11499999999999998);
            check_point(layout.surd_origin,0,0.19550000000000006);
            check_point(layout.base_origin,0.24,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0.07359999999999998,0.4937000000000001);else expect(false);
            check_rect(layout.overbar,0.24,0.37750000000000006,0.48,0.10800000000000001);
        }
        { // Source-paired radical case 97: sqrt, inline, size=-1, wrapper=0.75
            RadicalInput input{make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"),{},RadicalFontParameters{0.06,0.12,0.43},false,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==false);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==1.765);expect(request.stretch_height==1.305);expect(request.stretch_depth==0.46);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.71, 1.35, 0.3, 1.2, 0.73, 0.14, -0.05, 0.06, -0.02, 0.04, -0.02,""),-1);
            check_box(layout.bbox,make_box(2.41, 1.4250000000000003, 0.46, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.06);expect(layout.q==0.135);expect(layout.required_surd_height==1.765);
            expect(layout.measured_height==1.3050000000000002);expect(layout.placed_height==1.3350000000000002);expect(layout.placed_surd_height==0.09);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,-0.014999999999999902);
            check_point(layout.base_origin,0.71,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.71,1.245,1.7,0.09);
        }
        { // Source-paired radical case 98: root, inline, size=-1, wrapper=0.75
            RadicalInput input{make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"),make_box(0.9, 0.84, -0.13, 0.6, 1.52, 0.12, -0.09, 0.02, 0.03, -0.02, 0.09,"33%"),RadicalFontParameters{0.06,0.12,0.43},false,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.9, 0.84, -0.13, 0.6, 1.52, 0.12, -0.09, 0.02, 0.03, -0.02, 0.09,"33%"));
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==false);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==1.765);expect(request.stretch_height==1.305);expect(request.stretch_depth==0.46);expect(request.root_width==1.7680000000000002);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.71, 1.35, 0.3, 1.2, 0.73, 0.14, -0.05, 0.06, -0.02, 0.04, -0.02,""),-1);
            check_box(layout.bbox,make_box(3.423, 2.8318, 0.46, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.06);expect(layout.q==0.135);expect(layout.required_surd_height==1.765);
            expect(layout.measured_height==1.3050000000000002);expect(layout.placed_height==1.3350000000000002);expect(layout.placed_surd_height==0.09);
            expect(layout.root_offset==1.0130000000000001);expect(layout.root_height==1.585);expect(layout.root_dx==0);
            check_point(layout.surd_origin,1.0130000000000001,-0.014999999999999902);
            check_point(layout.base_origin,1.723,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0,1.585);else expect(false);
            check_rect(layout.overbar,1.723,1.245,1.7,0.09);
        }
        { // Source-paired radical case 99: sqrt, inline, size=2, wrapper=0.75
            RadicalInput input{make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"),{},RadicalFontParameters{0.06,0.12,0.43},false,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==false);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==1.765);expect(request.stretch_height==1.305);expect(request.stretch_depth==0.46);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.71, 1.35, 0.3, 1.2, 0.73, 0.14, -0.05, 0.06, -0.02, 0.04, -0.02,""),2);
            check_box(layout.bbox,make_box(2.41, 1.4250000000000003, 0.46, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.06);expect(layout.q==0.135);expect(layout.required_surd_height==1.765);
            expect(layout.measured_height==1.3050000000000002);expect(layout.placed_height==1.3350000000000002);expect(layout.placed_surd_height==0.09);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,-0.014999999999999902);
            check_point(layout.base_origin,0.71,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.71,1.245,1.7,0.09);
        }
        { // Source-paired radical case 100: root, inline, size=2, wrapper=0.75
            RadicalInput input{make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"),make_box(0.9, 0.84, -0.13, 0.6, 1.52, 0.12, -0.09, 0.02, 0.03, -0.02, 0.09,"33%"),RadicalFontParameters{0.06,0.12,0.43},false,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.9, 0.84, -0.13, 0.6, 1.52, 0.12, -0.09, 0.02, 0.03, -0.02, 0.09,"33%"));
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==false);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==1.765);expect(request.stretch_height==1.305);expect(request.stretch_depth==0.46);expect(request.root_width==1.7680000000000002);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.71, 1.35, 0.3, 1.2, 0.73, 0.14, -0.05, 0.06, -0.02, 0.04, -0.02,""),2);
            check_box(layout.bbox,make_box(3.3520000000000003, 1.8393000000000002, 0.46, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.06);expect(layout.q==0.135);expect(layout.required_surd_height==1.765);
            expect(layout.measured_height==1.3050000000000002);expect(layout.placed_height==1.3350000000000002);expect(layout.placed_surd_height==0.09);
            expect(layout.root_offset==0.9420000000000002);expect(layout.root_height==0.5925000000000002);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0.9420000000000002,-0.014999999999999902);
            check_point(layout.base_origin,1.6520000000000001,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0,0.5925000000000002);else expect(false);
            check_rect(layout.overbar,1.6520000000000001,1.245,1.7,0.09);
        }
        { // Source-paired radical case 101: sqrt, inline, size=-1, wrapper=1
            RadicalInput input{make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"),{},RadicalFontParameters{0.06,0.12,0.43},false,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==1.765);expect(request.stretch_height==1.305);expect(request.stretch_depth==0.46);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.71, 1.35, 0.3, 1.2, 0.73, 0.14, -0.05, 0.06, -0.02, 0.04, -0.02,""),-1);
            check_box(layout.bbox,make_box(2.41, 1.4250000000000003, 0.46, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.06);expect(layout.q==0.135);expect(layout.required_surd_height==1.765);
            expect(layout.measured_height==1.3050000000000002);expect(layout.placed_height==1.3650000000000002);expect(layout.placed_surd_height==0.12);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.015000000000000124);
            check_point(layout.base_origin,0.71,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.71,1.245,1.7,0.12);
        }
        { // Source-paired radical case 102: root, inline, size=-1, wrapper=1
            RadicalInput input{make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"),make_box(0.9, 0.84, -0.13, 0.6, 1.52, 0.12, -0.09, 0.02, 0.03, -0.02, 0.09,"33%"),RadicalFontParameters{0.06,0.12,0.43},false,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.9, 0.84, -0.13, 0.6, 1.52, 0.12, -0.09, 0.02, 0.03, -0.02, 0.09,"33%"));
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==1.765);expect(request.stretch_height==1.305);expect(request.stretch_depth==0.46);expect(request.root_width==1.7680000000000002);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.71, 1.35, 0.3, 1.2, 0.73, 0.14, -0.05, 0.06, -0.02, 0.04, -0.02,""),-1);
            check_box(layout.bbox,make_box(3.423, 2.8318, 0.46, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.06);expect(layout.q==0.135);expect(layout.required_surd_height==1.765);
            expect(layout.measured_height==1.3050000000000002);expect(layout.placed_height==1.3650000000000002);expect(layout.placed_surd_height==0.12);
            expect(layout.root_offset==1.0130000000000001);expect(layout.root_height==1.615);expect(layout.root_dx==0);
            check_point(layout.surd_origin,1.0130000000000001,0.015000000000000124);
            check_point(layout.base_origin,1.723,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0,1.615);else expect(false);
            check_rect(layout.overbar,1.723,1.245,1.7,0.12);
        }
        { // Source-paired radical case 103: sqrt, inline, size=2, wrapper=1
            RadicalInput input{make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"),{},RadicalFontParameters{0.06,0.12,0.43},false,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==1.765);expect(request.stretch_height==1.305);expect(request.stretch_depth==0.46);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.71, 1.35, 0.3, 1.2, 0.73, 0.14, -0.05, 0.06, -0.02, 0.04, -0.02,""),2);
            check_box(layout.bbox,make_box(2.41, 1.4250000000000003, 0.46, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.06);expect(layout.q==0.135);expect(layout.required_surd_height==1.765);
            expect(layout.measured_height==1.3050000000000002);expect(layout.placed_height==1.3650000000000002);expect(layout.placed_surd_height==0.12);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.015000000000000124);
            check_point(layout.base_origin,0.71,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.71,1.245,1.7,0.12);
        }
        { // Source-paired radical case 104: root, inline, size=2, wrapper=1
            RadicalInput input{make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"),make_box(0.9, 0.84, -0.13, 0.6, 1.52, 0.12, -0.09, 0.02, 0.03, -0.02, 0.09,"33%"),RadicalFontParameters{0.06,0.12,0.43},false,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.9, 0.84, -0.13, 0.6, 1.52, 0.12, -0.09, 0.02, 0.03, -0.02, 0.09,"33%"));
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==1.765);expect(request.stretch_height==1.305);expect(request.stretch_depth==0.46);expect(request.root_width==1.7680000000000002);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.71, 1.35, 0.3, 1.2, 0.73, 0.14, -0.05, 0.06, -0.02, 0.04, -0.02,""),2);
            check_box(layout.bbox,make_box(3.3520000000000003, 1.8393000000000002, 0.46, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.06);expect(layout.q==0.135);expect(layout.required_surd_height==1.765);
            expect(layout.measured_height==1.3050000000000002);expect(layout.placed_height==1.3650000000000002);expect(layout.placed_surd_height==0.12);
            expect(layout.root_offset==0.9420000000000002);expect(layout.root_height==0.6225000000000003);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0.9420000000000002,0.015000000000000124);
            check_point(layout.base_origin,1.6520000000000001,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0,0.6225000000000003);else expect(false);
            check_rect(layout.overbar,1.6520000000000001,1.245,1.7,0.12);
        }
        { // Source-paired radical case 105: sqrt, inline, size=-1, wrapper=1.35
            RadicalInput input{make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"),{},RadicalFontParameters{0.06,0.12,0.43},false,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==1.765);expect(request.stretch_height==1.305);expect(request.stretch_depth==0.46);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.71, 1.35, 0.3, 1.2, 0.73, 0.14, -0.05, 0.06, -0.02, 0.04, -0.02,""),-1);
            check_box(layout.bbox,make_box(2.41, 1.4250000000000003, 0.46, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.06);expect(layout.q==0.135);expect(layout.required_surd_height==1.765);
            expect(layout.measured_height==1.3050000000000002);expect(layout.placed_height==1.407);expect(layout.placed_surd_height==0.162);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.05699999999999994);
            check_point(layout.base_origin,0.71,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.71,1.245,1.7,0.162);
        }
        { // Source-paired radical case 106: root, inline, size=-1, wrapper=1.35
            RadicalInput input{make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"),make_box(0.9, 0.84, -0.13, 0.6, 1.52, 0.12, -0.09, 0.02, 0.03, -0.02, 0.09,"33%"),RadicalFontParameters{0.06,0.12,0.43},false,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.9, 0.84, -0.13, 0.6, 1.52, 0.12, -0.09, 0.02, 0.03, -0.02, 0.09,"33%"));
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==1.765);expect(request.stretch_height==1.305);expect(request.stretch_depth==0.46);expect(request.root_width==1.7680000000000002);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.71, 1.35, 0.3, 1.2, 0.73, 0.14, -0.05, 0.06, -0.02, 0.04, -0.02,""),-1);
            check_box(layout.bbox,make_box(3.423, 2.8318, 0.46, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.06);expect(layout.q==0.135);expect(layout.required_surd_height==1.765);
            expect(layout.measured_height==1.3050000000000002);expect(layout.placed_height==1.407);expect(layout.placed_surd_height==0.162);
            expect(layout.root_offset==1.0130000000000001);expect(layout.root_height==1.6569999999999998);expect(layout.root_dx==0);
            check_point(layout.surd_origin,1.0130000000000001,0.05699999999999994);
            check_point(layout.base_origin,1.723,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0,1.6569999999999998);else expect(false);
            check_rect(layout.overbar,1.723,1.245,1.7,0.162);
        }
        { // Source-paired radical case 107: sqrt, inline, size=2, wrapper=1.35
            RadicalInput input{make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"),{},RadicalFontParameters{0.06,0.12,0.43},false,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==1.765);expect(request.stretch_height==1.305);expect(request.stretch_depth==0.46);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.71, 1.35, 0.3, 1.2, 0.73, 0.14, -0.05, 0.06, -0.02, 0.04, -0.02,""),2);
            check_box(layout.bbox,make_box(2.41, 1.4250000000000003, 0.46, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.06);expect(layout.q==0.135);expect(layout.required_surd_height==1.765);
            expect(layout.measured_height==1.3050000000000002);expect(layout.placed_height==1.407);expect(layout.placed_surd_height==0.162);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.05699999999999994);
            check_point(layout.base_origin,0.71,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.71,1.245,1.7,0.162);
        }
        { // Source-paired radical case 108: root, inline, size=2, wrapper=1.35
            RadicalInput input{make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"),make_box(0.9, 0.84, -0.13, 0.6, 1.52, 0.12, -0.09, 0.02, 0.03, -0.02, 0.09,"33%"),RadicalFontParameters{0.06,0.12,0.43},false,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.9, 0.84, -0.13, 0.6, 1.52, 0.12, -0.09, 0.02, 0.03, -0.02, 0.09,"33%"));
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==1.765);expect(request.stretch_height==1.305);expect(request.stretch_depth==0.46);expect(request.root_width==1.7680000000000002);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.71, 1.35, 0.3, 1.2, 0.73, 0.14, -0.05, 0.06, -0.02, 0.04, -0.02,""),2);
            check_box(layout.bbox,make_box(3.3520000000000003, 1.8393000000000002, 0.46, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.06);expect(layout.q==0.135);expect(layout.required_surd_height==1.765);
            expect(layout.measured_height==1.3050000000000002);expect(layout.placed_height==1.407);expect(layout.placed_surd_height==0.162);
            expect(layout.root_offset==0.9420000000000002);expect(layout.root_height==0.6645000000000001);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0.9420000000000002,0.05699999999999994);
            check_point(layout.base_origin,1.6520000000000001,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0,0.6645000000000001);else expect(false);
            check_rect(layout.overbar,1.6520000000000001,1.245,1.7,0.162);
        }
        { // Source-paired radical case 109: sqrt, display, size=-1, wrapper=0.75
            RadicalInput input{make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"),{},RadicalFontParameters{0.06,0.12,0.43},true,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==true);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==1.8575);expect(request.stretch_height==1.3975);expect(request.stretch_depth==0.46);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.71, 1.35, 0.3, 1.2, 0.73, 0.14, -0.05, 0.06, -0.02, 0.04, -0.02,""),-1);
            check_box(layout.bbox,make_box(2.41, 1.5175, 0.46, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.43);expect(layout.q==0.22749999999999998);expect(layout.required_surd_height==1.8575);
            expect(layout.measured_height==1.3975000000000002);expect(layout.placed_height==1.4275000000000002);expect(layout.placed_surd_height==0.09);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.07750000000000012);
            check_point(layout.base_origin,0.71,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.71,1.3375000000000001,1.7,0.09);
        }
        { // Source-paired radical case 110: root, display, size=-1, wrapper=0.75
            RadicalInput input{make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"),make_box(0.9, 0.84, -0.13, 0.6, 1.52, 0.12, -0.09, 0.02, 0.03, -0.02, 0.09,"33%"),RadicalFontParameters{0.06,0.12,0.43},true,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.9, 0.84, -0.13, 0.6, 1.52, 0.12, -0.09, 0.02, 0.03, -0.02, 0.09,"33%"));
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==true);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==1.8575);expect(request.stretch_height==1.3975);expect(request.stretch_depth==0.46);expect(request.root_width==1.7680000000000002);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.71, 1.35, 0.3, 1.2, 0.73, 0.14, -0.05, 0.06, -0.02, 0.04, -0.02,""),-1);
            check_box(layout.bbox,make_box(3.423, 2.9242999999999997, 0.46, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.43);expect(layout.q==0.22749999999999998);expect(layout.required_surd_height==1.8575);
            expect(layout.measured_height==1.3975000000000002);expect(layout.placed_height==1.4275000000000002);expect(layout.placed_surd_height==0.09);
            expect(layout.root_offset==1.0130000000000001);expect(layout.root_height==1.6775);expect(layout.root_dx==0);
            check_point(layout.surd_origin,1.0130000000000001,0.07750000000000012);
            check_point(layout.base_origin,1.723,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0,1.6775);else expect(false);
            check_rect(layout.overbar,1.723,1.3375000000000001,1.7,0.09);
        }
        { // Source-paired radical case 111: sqrt, display, size=2, wrapper=0.75
            RadicalInput input{make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"),{},RadicalFontParameters{0.06,0.12,0.43},true,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==true);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==1.8575);expect(request.stretch_height==1.3975);expect(request.stretch_depth==0.46);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.71, 1.35, 0.3, 1.2, 0.73, 0.14, -0.05, 0.06, -0.02, 0.04, -0.02,""),2);
            check_box(layout.bbox,make_box(2.41, 1.5175, 0.46, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.43);expect(layout.q==0.22749999999999998);expect(layout.required_surd_height==1.8575);
            expect(layout.measured_height==1.3975000000000002);expect(layout.placed_height==1.4275000000000002);expect(layout.placed_surd_height==0.09);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.07750000000000012);
            check_point(layout.base_origin,0.71,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.71,1.3375000000000001,1.7,0.09);
        }
        { // Source-paired radical case 112: root, display, size=2, wrapper=0.75
            RadicalInput input{make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"),make_box(0.9, 0.84, -0.13, 0.6, 1.52, 0.12, -0.09, 0.02, 0.03, -0.02, 0.09,"33%"),RadicalFontParameters{0.06,0.12,0.43},true,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.9, 0.84, -0.13, 0.6, 1.52, 0.12, -0.09, 0.02, 0.03, -0.02, 0.09,"33%"));
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==true);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==1.8575);expect(request.stretch_height==1.3975);expect(request.stretch_depth==0.46);expect(request.root_width==1.7680000000000002);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.71, 1.35, 0.3, 1.2, 0.73, 0.14, -0.05, 0.06, -0.02, 0.04, -0.02,""),2);
            check_box(layout.bbox,make_box(3.3520000000000003, 1.9318000000000002, 0.46, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.43);expect(layout.q==0.22749999999999998);expect(layout.required_surd_height==1.8575);
            expect(layout.measured_height==1.3975000000000002);expect(layout.placed_height==1.4275000000000002);expect(layout.placed_surd_height==0.09);
            expect(layout.root_offset==0.9420000000000002);expect(layout.root_height==0.6850000000000003);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0.9420000000000002,0.07750000000000012);
            check_point(layout.base_origin,1.6520000000000001,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0,0.6850000000000003);else expect(false);
            check_rect(layout.overbar,1.6520000000000001,1.3375000000000001,1.7,0.09);
        }
        { // Source-paired radical case 113: sqrt, display, size=-1, wrapper=1
            RadicalInput input{make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"),{},RadicalFontParameters{0.06,0.12,0.43},true,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==1.8575);expect(request.stretch_height==1.3975);expect(request.stretch_depth==0.46);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.71, 1.35, 0.3, 1.2, 0.73, 0.14, -0.05, 0.06, -0.02, 0.04, -0.02,""),-1);
            check_box(layout.bbox,make_box(2.41, 1.5175, 0.46, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.43);expect(layout.q==0.22749999999999998);expect(layout.required_surd_height==1.8575);
            expect(layout.measured_height==1.3975000000000002);expect(layout.placed_height==1.4575);expect(layout.placed_surd_height==0.12);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.10749999999999993);
            check_point(layout.base_origin,0.71,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.71,1.3375,1.7,0.12);
        }
        { // Source-paired radical case 114: root, display, size=-1, wrapper=1
            RadicalInput input{make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"),make_box(0.9, 0.84, -0.13, 0.6, 1.52, 0.12, -0.09, 0.02, 0.03, -0.02, 0.09,"33%"),RadicalFontParameters{0.06,0.12,0.43},true,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.9, 0.84, -0.13, 0.6, 1.52, 0.12, -0.09, 0.02, 0.03, -0.02, 0.09,"33%"));
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==1.8575);expect(request.stretch_height==1.3975);expect(request.stretch_depth==0.46);expect(request.root_width==1.7680000000000002);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.71, 1.35, 0.3, 1.2, 0.73, 0.14, -0.05, 0.06, -0.02, 0.04, -0.02,""),-1);
            check_box(layout.bbox,make_box(3.423, 2.9242999999999997, 0.46, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.43);expect(layout.q==0.22749999999999998);expect(layout.required_surd_height==1.8575);
            expect(layout.measured_height==1.3975000000000002);expect(layout.placed_height==1.4575);expect(layout.placed_surd_height==0.12);
            expect(layout.root_offset==1.0130000000000001);expect(layout.root_height==1.7074999999999998);expect(layout.root_dx==0);
            check_point(layout.surd_origin,1.0130000000000001,0.10749999999999993);
            check_point(layout.base_origin,1.723,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0,1.7074999999999998);else expect(false);
            check_rect(layout.overbar,1.723,1.3375,1.7,0.12);
        }
        { // Source-paired radical case 115: sqrt, display, size=2, wrapper=1
            RadicalInput input{make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"),{},RadicalFontParameters{0.06,0.12,0.43},true,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==1.8575);expect(request.stretch_height==1.3975);expect(request.stretch_depth==0.46);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.71, 1.35, 0.3, 1.2, 0.73, 0.14, -0.05, 0.06, -0.02, 0.04, -0.02,""),2);
            check_box(layout.bbox,make_box(2.41, 1.5175, 0.46, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.43);expect(layout.q==0.22749999999999998);expect(layout.required_surd_height==1.8575);
            expect(layout.measured_height==1.3975000000000002);expect(layout.placed_height==1.4575);expect(layout.placed_surd_height==0.12);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.10749999999999993);
            check_point(layout.base_origin,0.71,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.71,1.3375,1.7,0.12);
        }
        { // Source-paired radical case 116: root, display, size=2, wrapper=1
            RadicalInput input{make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"),make_box(0.9, 0.84, -0.13, 0.6, 1.52, 0.12, -0.09, 0.02, 0.03, -0.02, 0.09,"33%"),RadicalFontParameters{0.06,0.12,0.43},true,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.9, 0.84, -0.13, 0.6, 1.52, 0.12, -0.09, 0.02, 0.03, -0.02, 0.09,"33%"));
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==1.8575);expect(request.stretch_height==1.3975);expect(request.stretch_depth==0.46);expect(request.root_width==1.7680000000000002);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.71, 1.35, 0.3, 1.2, 0.73, 0.14, -0.05, 0.06, -0.02, 0.04, -0.02,""),2);
            check_box(layout.bbox,make_box(3.3520000000000003, 1.9318000000000002, 0.46, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.43);expect(layout.q==0.22749999999999998);expect(layout.required_surd_height==1.8575);
            expect(layout.measured_height==1.3975000000000002);expect(layout.placed_height==1.4575);expect(layout.placed_surd_height==0.12);
            expect(layout.root_offset==0.9420000000000002);expect(layout.root_height==0.7150000000000001);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0.9420000000000002,0.10749999999999993);
            check_point(layout.base_origin,1.6520000000000001,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0,0.7150000000000001);else expect(false);
            check_rect(layout.overbar,1.6520000000000001,1.3375,1.7,0.12);
        }
        { // Source-paired radical case 117: sqrt, display, size=-1, wrapper=1.35
            RadicalInput input{make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"),{},RadicalFontParameters{0.06,0.12,0.43},true,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==1.8575);expect(request.stretch_height==1.3975);expect(request.stretch_depth==0.46);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.71, 1.35, 0.3, 1.2, 0.73, 0.14, -0.05, 0.06, -0.02, 0.04, -0.02,""),-1);
            check_box(layout.bbox,make_box(2.41, 1.5175, 0.46, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.43);expect(layout.q==0.22749999999999998);expect(layout.required_surd_height==1.8575);
            expect(layout.measured_height==1.3975000000000002);expect(layout.placed_height==1.4995);expect(layout.placed_surd_height==0.162);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.14949999999999997);
            check_point(layout.base_origin,0.71,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.71,1.3375000000000001,1.7,0.162);
        }
        { // Source-paired radical case 118: root, display, size=-1, wrapper=1.35
            RadicalInput input{make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"),make_box(0.9, 0.84, -0.13, 0.6, 1.52, 0.12, -0.09, 0.02, 0.03, -0.02, 0.09,"33%"),RadicalFontParameters{0.06,0.12,0.43},true,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.9, 0.84, -0.13, 0.6, 1.52, 0.12, -0.09, 0.02, 0.03, -0.02, 0.09,"33%"));
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==1.8575);expect(request.stretch_height==1.3975);expect(request.stretch_depth==0.46);expect(request.root_width==1.7680000000000002);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.71, 1.35, 0.3, 1.2, 0.73, 0.14, -0.05, 0.06, -0.02, 0.04, -0.02,""),-1);
            check_box(layout.bbox,make_box(3.423, 2.9242999999999997, 0.46, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.43);expect(layout.q==0.22749999999999998);expect(layout.required_surd_height==1.8575);
            expect(layout.measured_height==1.3975000000000002);expect(layout.placed_height==1.4995);expect(layout.placed_surd_height==0.162);
            expect(layout.root_offset==1.0130000000000001);expect(layout.root_height==1.7494999999999998);expect(layout.root_dx==0);
            check_point(layout.surd_origin,1.0130000000000001,0.14949999999999997);
            check_point(layout.base_origin,1.723,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0,1.7494999999999998);else expect(false);
            check_rect(layout.overbar,1.723,1.3375000000000001,1.7,0.162);
        }
        { // Source-paired radical case 119: sqrt, display, size=2, wrapper=1.35
            RadicalInput input{make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"),{},RadicalFontParameters{0.06,0.12,0.43},true,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==1.8575);expect(request.stretch_height==1.3975);expect(request.stretch_depth==0.46);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.71, 1.35, 0.3, 1.2, 0.73, 0.14, -0.05, 0.06, -0.02, 0.04, -0.02,""),2);
            check_box(layout.bbox,make_box(2.41, 1.5175, 0.46, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.43);expect(layout.q==0.22749999999999998);expect(layout.required_surd_height==1.8575);
            expect(layout.measured_height==1.3975000000000002);expect(layout.placed_height==1.4995);expect(layout.placed_surd_height==0.162);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.14949999999999997);
            check_point(layout.base_origin,0.71,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.71,1.3375000000000001,1.7,0.162);
        }
        { // Source-paired radical case 120: root, display, size=2, wrapper=1.35
            RadicalInput input{make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"),make_box(0.9, 0.84, -0.13, 0.6, 1.52, 0.12, -0.09, 0.02, 0.03, -0.02, 0.09,"33%"),RadicalFontParameters{0.06,0.12,0.43},true,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.9, 0.84, -0.13, 0.6, 1.52, 0.12, -0.09, 0.02, 0.03, -0.02, 0.09,"33%"));
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==1.8575);expect(request.stretch_height==1.3975);expect(request.stretch_depth==0.46);expect(request.root_width==1.7680000000000002);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.71, 1.35, 0.3, 1.2, 0.73, 0.14, -0.05, 0.06, -0.02, 0.04, -0.02,""),2);
            check_box(layout.bbox,make_box(3.3520000000000003, 1.9318000000000002, 0.46, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.43);expect(layout.q==0.22749999999999998);expect(layout.required_surd_height==1.8575);
            expect(layout.measured_height==1.3975000000000002);expect(layout.placed_height==1.4995);expect(layout.placed_surd_height==0.162);
            expect(layout.root_offset==0.9420000000000002);expect(layout.root_height==0.7570000000000001);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0.9420000000000002,0.14949999999999997);
            check_point(layout.base_origin,1.6520000000000001,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0,0.7570000000000001);else expect(false);
            check_rect(layout.overbar,1.6520000000000001,1.3375000000000001,1.7,0.162);
        }
        { // Source-paired radical case 121: sqrt, inline, size=-1, wrapper=0.75
            RadicalInput input{make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"),{},RadicalFontParameters{0.15,0.08,0.27},false,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==false);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==1.8375000000000001);expect(request.stretch_height==1.3775000000000002);expect(request.stretch_depth==0.46);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.71, 1.35, 0.3, 1.2, 0.73, 0.14, -0.05, 0.06, -0.02, 0.04, -0.02,""),-1);
            check_box(layout.bbox,make_box(2.41, 1.4575, 0.46, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.15);expect(layout.q==0.1175);expect(layout.required_surd_height==1.8375000000000001);
            expect(layout.measured_height==1.3775);expect(layout.placed_height==1.2875);expect(layout.placed_surd_height==0.06);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,-0.0625);
            check_point(layout.base_origin,0.71,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.71,1.2275,1.7,0.06);
        }
        { // Source-paired radical case 122: root, inline, size=-1, wrapper=0.75
            RadicalInput input{make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"),make_box(0.9, 0.84, -0.13, 0.6, 1.52, 0.12, -0.09, 0.02, 0.03, -0.02, 0.09,"33%"),RadicalFontParameters{0.15,0.08,0.27},false,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.9, 0.84, -0.13, 0.6, 1.52, 0.12, -0.09, 0.02, 0.03, -0.02, 0.09,"33%"));
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==false);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==1.8375000000000001);expect(request.stretch_height==1.3775000000000002);expect(request.stretch_depth==0.46);expect(request.root_width==1.7680000000000002);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.71, 1.35, 0.3, 1.2, 0.73, 0.14, -0.05, 0.06, -0.02, 0.04, -0.02,""),-1);
            check_box(layout.bbox,make_box(3.423, 2.9042999999999997, 0.46, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.15);expect(layout.q==0.1175);expect(layout.required_surd_height==1.8375000000000001);
            expect(layout.measured_height==1.3775);expect(layout.placed_height==1.2875);expect(layout.placed_surd_height==0.06);
            expect(layout.root_offset==1.0130000000000001);expect(layout.root_height==1.5374999999999999);expect(layout.root_dx==0);
            check_point(layout.surd_origin,1.0130000000000001,-0.0625);
            check_point(layout.base_origin,1.723,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0,1.5374999999999999);else expect(false);
            check_rect(layout.overbar,1.723,1.2275,1.7,0.06);
        }
        { // Source-paired radical case 123: sqrt, inline, size=2, wrapper=0.75
            RadicalInput input{make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"),{},RadicalFontParameters{0.15,0.08,0.27},false,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==false);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==1.8375000000000001);expect(request.stretch_height==1.3775000000000002);expect(request.stretch_depth==0.46);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.71, 1.35, 0.3, 1.2, 0.73, 0.14, -0.05, 0.06, -0.02, 0.04, -0.02,""),2);
            check_box(layout.bbox,make_box(2.41, 1.4575, 0.46, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.15);expect(layout.q==0.1175);expect(layout.required_surd_height==1.8375000000000001);
            expect(layout.measured_height==1.3775);expect(layout.placed_height==1.2875);expect(layout.placed_surd_height==0.06);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,-0.0625);
            check_point(layout.base_origin,0.71,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.71,1.2275,1.7,0.06);
        }
        { // Source-paired radical case 124: root, inline, size=2, wrapper=0.75
            RadicalInput input{make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"),make_box(0.9, 0.84, -0.13, 0.6, 1.52, 0.12, -0.09, 0.02, 0.03, -0.02, 0.09,"33%"),RadicalFontParameters{0.15,0.08,0.27},false,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.9, 0.84, -0.13, 0.6, 1.52, 0.12, -0.09, 0.02, 0.03, -0.02, 0.09,"33%"));
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==false);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==1.8375000000000001);expect(request.stretch_height==1.3775000000000002);expect(request.stretch_depth==0.46);expect(request.root_width==1.7680000000000002);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.71, 1.35, 0.3, 1.2, 0.73, 0.14, -0.05, 0.06, -0.02, 0.04, -0.02,""),2);
            check_box(layout.bbox,make_box(3.3520000000000003, 1.9118, 0.46, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.15);expect(layout.q==0.1175);expect(layout.required_surd_height==1.8375000000000001);
            expect(layout.measured_height==1.3775);expect(layout.placed_height==1.2875);expect(layout.placed_surd_height==0.06);
            expect(layout.root_offset==0.9420000000000002);expect(layout.root_height==0.5450000000000002);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0.9420000000000002,-0.0625);
            check_point(layout.base_origin,1.6520000000000001,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0,0.5450000000000002);else expect(false);
            check_rect(layout.overbar,1.6520000000000001,1.2275,1.7,0.06);
        }
        { // Source-paired radical case 125: sqrt, inline, size=-1, wrapper=1
            RadicalInput input{make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"),{},RadicalFontParameters{0.15,0.08,0.27},false,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==1.8375000000000001);expect(request.stretch_height==1.3775000000000002);expect(request.stretch_depth==0.46);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.71, 1.35, 0.3, 1.2, 0.73, 0.14, -0.05, 0.06, -0.02, 0.04, -0.02,""),-1);
            check_box(layout.bbox,make_box(2.41, 1.4575, 0.46, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.15);expect(layout.q==0.1175);expect(layout.required_surd_height==1.8375000000000001);
            expect(layout.measured_height==1.3775);expect(layout.placed_height==1.3075);expect(layout.placed_surd_height==0.08);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,-0.04249999999999998);
            check_point(layout.base_origin,0.71,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.71,1.2275,1.7,0.08);
        }
        { // Source-paired radical case 126: root, inline, size=-1, wrapper=1
            RadicalInput input{make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"),make_box(0.9, 0.84, -0.13, 0.6, 1.52, 0.12, -0.09, 0.02, 0.03, -0.02, 0.09,"33%"),RadicalFontParameters{0.15,0.08,0.27},false,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.9, 0.84, -0.13, 0.6, 1.52, 0.12, -0.09, 0.02, 0.03, -0.02, 0.09,"33%"));
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==1.8375000000000001);expect(request.stretch_height==1.3775000000000002);expect(request.stretch_depth==0.46);expect(request.root_width==1.7680000000000002);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.71, 1.35, 0.3, 1.2, 0.73, 0.14, -0.05, 0.06, -0.02, 0.04, -0.02,""),-1);
            check_box(layout.bbox,make_box(3.423, 2.9042999999999997, 0.46, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.15);expect(layout.q==0.1175);expect(layout.required_surd_height==1.8375000000000001);
            expect(layout.measured_height==1.3775);expect(layout.placed_height==1.3075);expect(layout.placed_surd_height==0.08);
            expect(layout.root_offset==1.0130000000000001);expect(layout.root_height==1.5574999999999999);expect(layout.root_dx==0);
            check_point(layout.surd_origin,1.0130000000000001,-0.04249999999999998);
            check_point(layout.base_origin,1.723,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0,1.5574999999999999);else expect(false);
            check_rect(layout.overbar,1.723,1.2275,1.7,0.08);
        }
        { // Source-paired radical case 127: sqrt, inline, size=2, wrapper=1
            RadicalInput input{make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"),{},RadicalFontParameters{0.15,0.08,0.27},false,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==1.8375000000000001);expect(request.stretch_height==1.3775000000000002);expect(request.stretch_depth==0.46);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.71, 1.35, 0.3, 1.2, 0.73, 0.14, -0.05, 0.06, -0.02, 0.04, -0.02,""),2);
            check_box(layout.bbox,make_box(2.41, 1.4575, 0.46, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.15);expect(layout.q==0.1175);expect(layout.required_surd_height==1.8375000000000001);
            expect(layout.measured_height==1.3775);expect(layout.placed_height==1.3075);expect(layout.placed_surd_height==0.08);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,-0.04249999999999998);
            check_point(layout.base_origin,0.71,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.71,1.2275,1.7,0.08);
        }
        { // Source-paired radical case 128: root, inline, size=2, wrapper=1
            RadicalInput input{make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"),make_box(0.9, 0.84, -0.13, 0.6, 1.52, 0.12, -0.09, 0.02, 0.03, -0.02, 0.09,"33%"),RadicalFontParameters{0.15,0.08,0.27},false,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.9, 0.84, -0.13, 0.6, 1.52, 0.12, -0.09, 0.02, 0.03, -0.02, 0.09,"33%"));
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==1.8375000000000001);expect(request.stretch_height==1.3775000000000002);expect(request.stretch_depth==0.46);expect(request.root_width==1.7680000000000002);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.71, 1.35, 0.3, 1.2, 0.73, 0.14, -0.05, 0.06, -0.02, 0.04, -0.02,""),2);
            check_box(layout.bbox,make_box(3.3520000000000003, 1.9118, 0.46, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.15);expect(layout.q==0.1175);expect(layout.required_surd_height==1.8375000000000001);
            expect(layout.measured_height==1.3775);expect(layout.placed_height==1.3075);expect(layout.placed_surd_height==0.08);
            expect(layout.root_offset==0.9420000000000002);expect(layout.root_height==0.5650000000000002);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0.9420000000000002,-0.04249999999999998);
            check_point(layout.base_origin,1.6520000000000001,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0,0.5650000000000002);else expect(false);
            check_rect(layout.overbar,1.6520000000000001,1.2275,1.7,0.08);
        }
        { // Source-paired radical case 129: sqrt, inline, size=-1, wrapper=1.35
            RadicalInput input{make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"),{},RadicalFontParameters{0.15,0.08,0.27},false,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==1.8375000000000001);expect(request.stretch_height==1.3775000000000002);expect(request.stretch_depth==0.46);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.71, 1.35, 0.3, 1.2, 0.73, 0.14, -0.05, 0.06, -0.02, 0.04, -0.02,""),-1);
            check_box(layout.bbox,make_box(2.41, 1.4575, 0.46, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.15);expect(layout.q==0.1175);expect(layout.required_surd_height==1.8375000000000001);
            expect(layout.measured_height==1.3775);expect(layout.placed_height==1.3355000000000001);expect(layout.placed_surd_height==0.10800000000000001);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,-0.014499999999999957);
            check_point(layout.base_origin,0.71,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.71,1.2275,1.7,0.10800000000000001);
        }
        { // Source-paired radical case 130: root, inline, size=-1, wrapper=1.35
            RadicalInput input{make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"),make_box(0.9, 0.84, -0.13, 0.6, 1.52, 0.12, -0.09, 0.02, 0.03, -0.02, 0.09,"33%"),RadicalFontParameters{0.15,0.08,0.27},false,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.9, 0.84, -0.13, 0.6, 1.52, 0.12, -0.09, 0.02, 0.03, -0.02, 0.09,"33%"));
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==1.8375000000000001);expect(request.stretch_height==1.3775000000000002);expect(request.stretch_depth==0.46);expect(request.root_width==1.7680000000000002);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.71, 1.35, 0.3, 1.2, 0.73, 0.14, -0.05, 0.06, -0.02, 0.04, -0.02,""),-1);
            check_box(layout.bbox,make_box(3.423, 2.9042999999999997, 0.46, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.15);expect(layout.q==0.1175);expect(layout.required_surd_height==1.8375000000000001);
            expect(layout.measured_height==1.3775);expect(layout.placed_height==1.3355000000000001);expect(layout.placed_surd_height==0.10800000000000001);
            expect(layout.root_offset==1.0130000000000001);expect(layout.root_height==1.5855);expect(layout.root_dx==0);
            check_point(layout.surd_origin,1.0130000000000001,-0.014499999999999957);
            check_point(layout.base_origin,1.723,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0,1.5855);else expect(false);
            check_rect(layout.overbar,1.723,1.2275,1.7,0.10800000000000001);
        }
        { // Source-paired radical case 131: sqrt, inline, size=2, wrapper=1.35
            RadicalInput input{make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"),{},RadicalFontParameters{0.15,0.08,0.27},false,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==1.8375000000000001);expect(request.stretch_height==1.3775000000000002);expect(request.stretch_depth==0.46);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.71, 1.35, 0.3, 1.2, 0.73, 0.14, -0.05, 0.06, -0.02, 0.04, -0.02,""),2);
            check_box(layout.bbox,make_box(2.41, 1.4575, 0.46, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.15);expect(layout.q==0.1175);expect(layout.required_surd_height==1.8375000000000001);
            expect(layout.measured_height==1.3775);expect(layout.placed_height==1.3355000000000001);expect(layout.placed_surd_height==0.10800000000000001);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,-0.014499999999999957);
            check_point(layout.base_origin,0.71,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.71,1.2275,1.7,0.10800000000000001);
        }
        { // Source-paired radical case 132: root, inline, size=2, wrapper=1.35
            RadicalInput input{make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"),make_box(0.9, 0.84, -0.13, 0.6, 1.52, 0.12, -0.09, 0.02, 0.03, -0.02, 0.09,"33%"),RadicalFontParameters{0.15,0.08,0.27},false,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.9, 0.84, -0.13, 0.6, 1.52, 0.12, -0.09, 0.02, 0.03, -0.02, 0.09,"33%"));
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==1.8375000000000001);expect(request.stretch_height==1.3775000000000002);expect(request.stretch_depth==0.46);expect(request.root_width==1.7680000000000002);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.71, 1.35, 0.3, 1.2, 0.73, 0.14, -0.05, 0.06, -0.02, 0.04, -0.02,""),2);
            check_box(layout.bbox,make_box(3.3520000000000003, 1.9118, 0.46, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.15);expect(layout.q==0.1175);expect(layout.required_surd_height==1.8375000000000001);
            expect(layout.measured_height==1.3775);expect(layout.placed_height==1.3355000000000001);expect(layout.placed_surd_height==0.10800000000000001);
            expect(layout.root_offset==0.9420000000000002);expect(layout.root_height==0.5930000000000002);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0.9420000000000002,-0.014499999999999957);
            check_point(layout.base_origin,1.6520000000000001,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0,0.5930000000000002);else expect(false);
            check_rect(layout.overbar,1.6520000000000001,1.2275,1.7,0.10800000000000001);
        }
        { // Source-paired radical case 133: sqrt, display, size=-1, wrapper=0.75
            RadicalInput input{make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"),{},RadicalFontParameters{0.15,0.08,0.27},true,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==true);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==1.8675000000000002);expect(request.stretch_height==1.4075000000000002);expect(request.stretch_depth==0.46);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.71, 1.35, 0.3, 1.2, 0.73, 0.14, -0.05, 0.06, -0.02, 0.04, -0.02,""),-1);
            check_box(layout.bbox,make_box(2.41, 1.4875, 0.46, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.27);expect(layout.q==0.14750000000000002);expect(layout.required_surd_height==1.8675000000000002);
            expect(layout.measured_height==1.4075);expect(layout.placed_height==1.3175000000000001);expect(layout.placed_surd_height==0.06);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,-0.03249999999999997);
            check_point(layout.base_origin,0.71,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.71,1.2575,1.7,0.06);
        }
        { // Source-paired radical case 134: root, display, size=-1, wrapper=0.75
            RadicalInput input{make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"),make_box(0.9, 0.84, -0.13, 0.6, 1.52, 0.12, -0.09, 0.02, 0.03, -0.02, 0.09,"33%"),RadicalFontParameters{0.15,0.08,0.27},true,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.9, 0.84, -0.13, 0.6, 1.52, 0.12, -0.09, 0.02, 0.03, -0.02, 0.09,"33%"));
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==true);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==1.8675000000000002);expect(request.stretch_height==1.4075000000000002);expect(request.stretch_depth==0.46);expect(request.root_width==1.7680000000000002);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.71, 1.35, 0.3, 1.2, 0.73, 0.14, -0.05, 0.06, -0.02, 0.04, -0.02,""),-1);
            check_box(layout.bbox,make_box(3.423, 2.9342999999999995, 0.46, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.27);expect(layout.q==0.14750000000000002);expect(layout.required_surd_height==1.8675000000000002);
            expect(layout.measured_height==1.4075);expect(layout.placed_height==1.3175000000000001);expect(layout.placed_surd_height==0.06);
            expect(layout.root_offset==1.0130000000000001);expect(layout.root_height==1.5675);expect(layout.root_dx==0);
            check_point(layout.surd_origin,1.0130000000000001,-0.03249999999999997);
            check_point(layout.base_origin,1.723,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0,1.5675);else expect(false);
            check_rect(layout.overbar,1.723,1.2575,1.7,0.06);
        }
        { // Source-paired radical case 135: sqrt, display, size=2, wrapper=0.75
            RadicalInput input{make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"),{},RadicalFontParameters{0.15,0.08,0.27},true,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==true);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==1.8675000000000002);expect(request.stretch_height==1.4075000000000002);expect(request.stretch_depth==0.46);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.71, 1.35, 0.3, 1.2, 0.73, 0.14, -0.05, 0.06, -0.02, 0.04, -0.02,""),2);
            check_box(layout.bbox,make_box(2.41, 1.4875, 0.46, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.27);expect(layout.q==0.14750000000000002);expect(layout.required_surd_height==1.8675000000000002);
            expect(layout.measured_height==1.4075);expect(layout.placed_height==1.3175000000000001);expect(layout.placed_surd_height==0.06);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,-0.03249999999999997);
            check_point(layout.base_origin,0.71,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.71,1.2575,1.7,0.06);
        }
        { // Source-paired radical case 136: root, display, size=2, wrapper=0.75
            RadicalInput input{make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"),make_box(0.9, 0.84, -0.13, 0.6, 1.52, 0.12, -0.09, 0.02, 0.03, -0.02, 0.09,"33%"),RadicalFontParameters{0.15,0.08,0.27},true,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.9, 0.84, -0.13, 0.6, 1.52, 0.12, -0.09, 0.02, 0.03, -0.02, 0.09,"33%"));
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==true);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==1.8675000000000002);expect(request.stretch_height==1.4075000000000002);expect(request.stretch_depth==0.46);expect(request.root_width==1.7680000000000002);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.71, 1.35, 0.3, 1.2, 0.73, 0.14, -0.05, 0.06, -0.02, 0.04, -0.02,""),2);
            check_box(layout.bbox,make_box(3.3520000000000003, 1.9418, 0.46, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.27);expect(layout.q==0.14750000000000002);expect(layout.required_surd_height==1.8675000000000002);
            expect(layout.measured_height==1.4075);expect(layout.placed_height==1.3175000000000001);expect(layout.placed_surd_height==0.06);
            expect(layout.root_offset==0.9420000000000002);expect(layout.root_height==0.5750000000000002);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0.9420000000000002,-0.03249999999999997);
            check_point(layout.base_origin,1.6520000000000001,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0,0.5750000000000002);else expect(false);
            check_rect(layout.overbar,1.6520000000000001,1.2575,1.7,0.06);
        }
        { // Source-paired radical case 137: sqrt, display, size=-1, wrapper=1
            RadicalInput input{make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"),{},RadicalFontParameters{0.15,0.08,0.27},true,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==1.8675000000000002);expect(request.stretch_height==1.4075000000000002);expect(request.stretch_depth==0.46);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.71, 1.35, 0.3, 1.2, 0.73, 0.14, -0.05, 0.06, -0.02, 0.04, -0.02,""),-1);
            check_box(layout.bbox,make_box(2.41, 1.4875, 0.46, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.27);expect(layout.q==0.14750000000000002);expect(layout.required_surd_height==1.8675000000000002);
            expect(layout.measured_height==1.4075);expect(layout.placed_height==1.3375000000000001);expect(layout.placed_surd_height==0.08);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,-0.012499999999999956);
            check_point(layout.base_origin,0.71,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.71,1.2575,1.7,0.08);
        }
        { // Source-paired radical case 138: root, display, size=-1, wrapper=1
            RadicalInput input{make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"),make_box(0.9, 0.84, -0.13, 0.6, 1.52, 0.12, -0.09, 0.02, 0.03, -0.02, 0.09,"33%"),RadicalFontParameters{0.15,0.08,0.27},true,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.9, 0.84, -0.13, 0.6, 1.52, 0.12, -0.09, 0.02, 0.03, -0.02, 0.09,"33%"));
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==1.8675000000000002);expect(request.stretch_height==1.4075000000000002);expect(request.stretch_depth==0.46);expect(request.root_width==1.7680000000000002);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.71, 1.35, 0.3, 1.2, 0.73, 0.14, -0.05, 0.06, -0.02, 0.04, -0.02,""),-1);
            check_box(layout.bbox,make_box(3.423, 2.9342999999999995, 0.46, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.27);expect(layout.q==0.14750000000000002);expect(layout.required_surd_height==1.8675000000000002);
            expect(layout.measured_height==1.4075);expect(layout.placed_height==1.3375000000000001);expect(layout.placed_surd_height==0.08);
            expect(layout.root_offset==1.0130000000000001);expect(layout.root_height==1.5875);expect(layout.root_dx==0);
            check_point(layout.surd_origin,1.0130000000000001,-0.012499999999999956);
            check_point(layout.base_origin,1.723,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0,1.5875);else expect(false);
            check_rect(layout.overbar,1.723,1.2575,1.7,0.08);
        }
        { // Source-paired radical case 139: sqrt, display, size=2, wrapper=1
            RadicalInput input{make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"),{},RadicalFontParameters{0.15,0.08,0.27},true,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==1.8675000000000002);expect(request.stretch_height==1.4075000000000002);expect(request.stretch_depth==0.46);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.71, 1.35, 0.3, 1.2, 0.73, 0.14, -0.05, 0.06, -0.02, 0.04, -0.02,""),2);
            check_box(layout.bbox,make_box(2.41, 1.4875, 0.46, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.27);expect(layout.q==0.14750000000000002);expect(layout.required_surd_height==1.8675000000000002);
            expect(layout.measured_height==1.4075);expect(layout.placed_height==1.3375000000000001);expect(layout.placed_surd_height==0.08);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,-0.012499999999999956);
            check_point(layout.base_origin,0.71,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.71,1.2575,1.7,0.08);
        }
        { // Source-paired radical case 140: root, display, size=2, wrapper=1
            RadicalInput input{make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"),make_box(0.9, 0.84, -0.13, 0.6, 1.52, 0.12, -0.09, 0.02, 0.03, -0.02, 0.09,"33%"),RadicalFontParameters{0.15,0.08,0.27},true,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.9, 0.84, -0.13, 0.6, 1.52, 0.12, -0.09, 0.02, 0.03, -0.02, 0.09,"33%"));
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==1.8675000000000002);expect(request.stretch_height==1.4075000000000002);expect(request.stretch_depth==0.46);expect(request.root_width==1.7680000000000002);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.71, 1.35, 0.3, 1.2, 0.73, 0.14, -0.05, 0.06, -0.02, 0.04, -0.02,""),2);
            check_box(layout.bbox,make_box(3.3520000000000003, 1.9418, 0.46, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.27);expect(layout.q==0.14750000000000002);expect(layout.required_surd_height==1.8675000000000002);
            expect(layout.measured_height==1.4075);expect(layout.placed_height==1.3375000000000001);expect(layout.placed_surd_height==0.08);
            expect(layout.root_offset==0.9420000000000002);expect(layout.root_height==0.5950000000000002);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0.9420000000000002,-0.012499999999999956);
            check_point(layout.base_origin,1.6520000000000001,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0,0.5950000000000002);else expect(false);
            check_rect(layout.overbar,1.6520000000000001,1.2575,1.7,0.08);
        }
        { // Source-paired radical case 141: sqrt, display, size=-1, wrapper=1.35
            RadicalInput input{make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"),{},RadicalFontParameters{0.15,0.08,0.27},true,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==1.8675000000000002);expect(request.stretch_height==1.4075000000000002);expect(request.stretch_depth==0.46);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.71, 1.35, 0.3, 1.2, 0.73, 0.14, -0.05, 0.06, -0.02, 0.04, -0.02,""),-1);
            check_box(layout.bbox,make_box(2.41, 1.4875, 0.46, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.27);expect(layout.q==0.14750000000000002);expect(layout.required_surd_height==1.8675000000000002);
            expect(layout.measured_height==1.4075);expect(layout.placed_height==1.3655000000000002);expect(layout.placed_surd_height==0.10800000000000001);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.01550000000000007);
            check_point(layout.base_origin,0.71,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.71,1.2575,1.7,0.10800000000000001);
        }
        { // Source-paired radical case 142: root, display, size=-1, wrapper=1.35
            RadicalInput input{make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"),make_box(0.9, 0.84, -0.13, 0.6, 1.52, 0.12, -0.09, 0.02, 0.03, -0.02, 0.09,"33%"),RadicalFontParameters{0.15,0.08,0.27},true,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.9, 0.84, -0.13, 0.6, 1.52, 0.12, -0.09, 0.02, 0.03, -0.02, 0.09,"33%"));
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==1.8675000000000002);expect(request.stretch_height==1.4075000000000002);expect(request.stretch_depth==0.46);expect(request.root_width==1.7680000000000002);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.71, 1.35, 0.3, 1.2, 0.73, 0.14, -0.05, 0.06, -0.02, 0.04, -0.02,""),-1);
            check_box(layout.bbox,make_box(3.423, 2.9342999999999995, 0.46, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.27);expect(layout.q==0.14750000000000002);expect(layout.required_surd_height==1.8675000000000002);
            expect(layout.measured_height==1.4075);expect(layout.placed_height==1.3655000000000002);expect(layout.placed_surd_height==0.10800000000000001);
            expect(layout.root_offset==1.0130000000000001);expect(layout.root_height==1.6155);expect(layout.root_dx==0);
            check_point(layout.surd_origin,1.0130000000000001,0.01550000000000007);
            check_point(layout.base_origin,1.723,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0,1.6155);else expect(false);
            check_rect(layout.overbar,1.723,1.2575,1.7,0.10800000000000001);
        }
        { // Source-paired radical case 143: sqrt, display, size=2, wrapper=1.35
            RadicalInput input{make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"),{},RadicalFontParameters{0.15,0.08,0.27},true,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==1.8675000000000002);expect(request.stretch_height==1.4075000000000002);expect(request.stretch_depth==0.46);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.71, 1.35, 0.3, 1.2, 0.73, 0.14, -0.05, 0.06, -0.02, 0.04, -0.02,""),2);
            check_box(layout.bbox,make_box(2.41, 1.4875, 0.46, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.27);expect(layout.q==0.14750000000000002);expect(layout.required_surd_height==1.8675000000000002);
            expect(layout.measured_height==1.4075);expect(layout.placed_height==1.3655000000000002);expect(layout.placed_surd_height==0.10800000000000001);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,0.01550000000000007);
            check_point(layout.base_origin,0.71,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.71,1.2575,1.7,0.10800000000000001);
        }
        { // Source-paired radical case 144: root, display, size=2, wrapper=1.35
            RadicalInput input{make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"),make_box(0.9, 0.84, -0.13, 0.6, 1.52, 0.12, -0.09, 0.02, 0.03, -0.02, 0.09,"33%"),RadicalFontParameters{0.15,0.08,0.27},true,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(1.7, 1.11, 0.46, 1.8, 0.55, 0.21, -0.08, 0.07, -0.03, 0.08, -0.06,"118%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(0.9, 0.84, -0.13, 0.6, 1.52, 0.12, -0.09, 0.02, 0.03, -0.02, 0.09,"33%"));
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==1.8675000000000002);expect(request.stretch_height==1.4075000000000002);expect(request.stretch_depth==0.46);expect(request.root_width==1.7680000000000002);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.71, 1.35, 0.3, 1.2, 0.73, 0.14, -0.05, 0.06, -0.02, 0.04, -0.02,""),2);
            check_box(layout.bbox,make_box(3.3520000000000003, 1.9418, 0.46, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.27);expect(layout.q==0.14750000000000002);expect(layout.required_surd_height==1.8675000000000002);
            expect(layout.measured_height==1.4075);expect(layout.placed_height==1.3655000000000002);expect(layout.placed_surd_height==0.10800000000000001);
            expect(layout.root_offset==0.9420000000000002);expect(layout.root_height==0.6230000000000002);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0.9420000000000002,0.01550000000000007);
            check_point(layout.base_origin,1.6520000000000001,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0,0.6230000000000002);else expect(false);
            check_rect(layout.overbar,1.6520000000000001,1.2575,1.7,0.10800000000000001);
        }
        { // Source-paired radical case 145: sqrt, inline, size=-1, wrapper=0.75
            RadicalInput input{make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"),{},RadicalFontParameters{0.06,0.12,0.43},false,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==false);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==0.375);expect(request.stretch_height==0.025000000000000022);expect(request.stretch_depth==0.35);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.33, 0.41, -0.16, 0.66, 1.32, 0.05, 0.09, -0.04, 0.06, 0.02, 0.07,""),-1);
            check_box(layout.bbox,make_box(0.6204, 0.1562, 0.35, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.06);expect(layout.q==0.135);expect(layout.required_surd_height==0.375);
            expect(layout.measured_height==0.024999999999999994);expect(layout.placed_height==0.05499999999999999);expect(layout.placed_surd_height==0.09);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,-0.355);
            check_point(layout.base_origin,0.33,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.33,-0.035,-0.21,0.09);
        }
        { // Source-paired radical case 146: root, inline, size=-1, wrapper=0.75
            RadicalInput input{make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"),make_box(-0.28, 0.31, 0.22, 0.95, 0.81, 0.06, 0.1, -0.03, 0.05, 0.01, -0.07,"4%"),RadicalFontParameters{0.06,0.12,0.43},false,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(-0.28, 0.31, 0.22, 0.95, 0.81, 0.06, 0.1, -0.03, 0.05, 0.01, -0.07,"4%"));
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==false);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==0.375);expect(request.stretch_height==0.025000000000000022);expect(request.stretch_depth==0.35);expect(request.root_width==0.1732);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.33, 0.41, -0.16, 0.66, 1.32, 0.05, 0.09, -0.04, 0.06, 0.02, 0.07,""),-1);
            check_box(layout.bbox,make_box(0.6204, 2.1043, 0.35, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.06);expect(layout.q==0.135);expect(layout.required_surd_height==0.375);
            expect(layout.measured_height==0.024999999999999994);expect(layout.placed_height==0.05499999999999999);expect(layout.placed_surd_height==0.09);
            expect(layout.root_offset==0);expect(layout.root_height==1.8832);expect(layout.root_dx==0.48370370370370375);
            check_point(layout.surd_origin,0,-0.355);
            check_point(layout.base_origin,0.33,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0.39180000000000004,1.8832);else expect(false);
            check_rect(layout.overbar,0.33,-0.035,-0.21,0.09);
        }
        { // Source-paired radical case 147: sqrt, inline, size=2, wrapper=0.75
            RadicalInput input{make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"),{},RadicalFontParameters{0.06,0.12,0.43},false,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==false);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==0.375);expect(request.stretch_height==0.025000000000000022);expect(request.stretch_depth==0.35);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.33, 0.41, -0.16, 0.66, 1.32, 0.05, 0.09, -0.04, 0.06, 0.02, 0.07,""),2);
            check_box(layout.bbox,make_box(0.6204, 0.1562, 0.35, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.06);expect(layout.q==0.135);expect(layout.required_surd_height==0.375);
            expect(layout.measured_height==0.024999999999999994);expect(layout.placed_height==0.05499999999999999);expect(layout.placed_surd_height==0.09);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,-0.355);
            check_point(layout.base_origin,0.33,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.33,-0.035,-0.21,0.09);
        }
        { // Source-paired radical case 148: root, inline, size=2, wrapper=0.75
            RadicalInput input{make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"),make_box(-0.28, 0.31, 0.22, 0.95, 0.81, 0.06, 0.1, -0.03, 0.05, 0.01, -0.07,"4%"),RadicalFontParameters{0.06,0.12,0.43},false,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(-0.28, 0.31, 0.22, 0.95, 0.81, 0.06, 0.1, -0.03, 0.05, 0.01, -0.07,"4%"));
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==false);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==0.375);expect(request.stretch_height==0.025000000000000022);expect(request.stretch_depth==0.35);expect(request.root_width==0.1732);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.33, 0.41, -0.16, 0.66, 1.32, 0.05, 0.09, -0.04, 0.06, 0.02, 0.07,""),2);
            check_box(layout.bbox,make_box(0.6204, 0.3418, 0.35, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.06);expect(layout.q==0.135);expect(layout.required_surd_height==0.375);
            expect(layout.measured_height==0.024999999999999994);expect(layout.placed_height==0.05499999999999999);expect(layout.placed_surd_height==0.09);
            expect(layout.root_offset==0);expect(layout.root_height==0.12070000000000003);expect(layout.root_dx==0.5244444444444445);
            check_point(layout.surd_origin,0,-0.355);
            check_point(layout.base_origin,0.33,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0.42480000000000007,0.12070000000000003);else expect(false);
            check_rect(layout.overbar,0.33,-0.035,-0.21,0.09);
        }
        { // Source-paired radical case 149: sqrt, inline, size=-1, wrapper=1
            RadicalInput input{make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"),{},RadicalFontParameters{0.06,0.12,0.43},false,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==0.375);expect(request.stretch_height==0.025000000000000022);expect(request.stretch_depth==0.35);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.33, 0.41, -0.16, 0.66, 1.32, 0.05, 0.09, -0.04, 0.06, 0.02, 0.07,""),-1);
            check_box(layout.bbox,make_box(0.6204, 0.1562, 0.35, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.06);expect(layout.q==0.135);expect(layout.required_surd_height==0.375);
            expect(layout.measured_height==0.024999999999999994);expect(layout.placed_height==0.08499999999999999);expect(layout.placed_surd_height==0.12);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,-0.32499999999999996);
            check_point(layout.base_origin,0.33,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.33,-0.035,-0.21,0.12);
        }
        { // Source-paired radical case 150: root, inline, size=-1, wrapper=1
            RadicalInput input{make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"),make_box(-0.28, 0.31, 0.22, 0.95, 0.81, 0.06, 0.1, -0.03, 0.05, 0.01, -0.07,"4%"),RadicalFontParameters{0.06,0.12,0.43},false,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(-0.28, 0.31, 0.22, 0.95, 0.81, 0.06, 0.1, -0.03, 0.05, 0.01, -0.07,"4%"));
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==0.375);expect(request.stretch_height==0.025000000000000022);expect(request.stretch_depth==0.35);expect(request.root_width==0.1732);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.33, 0.41, -0.16, 0.66, 1.32, 0.05, 0.09, -0.04, 0.06, 0.02, 0.07,""),-1);
            check_box(layout.bbox,make_box(0.6204, 2.1043, 0.35, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.06);expect(layout.q==0.135);expect(layout.required_surd_height==0.375);
            expect(layout.measured_height==0.024999999999999994);expect(layout.placed_height==0.08499999999999999);expect(layout.placed_surd_height==0.12);
            expect(layout.root_offset==0);expect(layout.root_height==1.9131999999999998);expect(layout.root_dx==0.48370370370370375);
            check_point(layout.surd_origin,0,-0.32499999999999996);
            check_point(layout.base_origin,0.33,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0.39180000000000004,1.9131999999999998);else expect(false);
            check_rect(layout.overbar,0.33,-0.035,-0.21,0.12);
        }
        { // Source-paired radical case 151: sqrt, inline, size=2, wrapper=1
            RadicalInput input{make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"),{},RadicalFontParameters{0.06,0.12,0.43},false,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==0.375);expect(request.stretch_height==0.025000000000000022);expect(request.stretch_depth==0.35);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.33, 0.41, -0.16, 0.66, 1.32, 0.05, 0.09, -0.04, 0.06, 0.02, 0.07,""),2);
            check_box(layout.bbox,make_box(0.6204, 0.1562, 0.35, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.06);expect(layout.q==0.135);expect(layout.required_surd_height==0.375);
            expect(layout.measured_height==0.024999999999999994);expect(layout.placed_height==0.08499999999999999);expect(layout.placed_surd_height==0.12);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,-0.32499999999999996);
            check_point(layout.base_origin,0.33,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.33,-0.035,-0.21,0.12);
        }
        { // Source-paired radical case 152: root, inline, size=2, wrapper=1
            RadicalInput input{make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"),make_box(-0.28, 0.31, 0.22, 0.95, 0.81, 0.06, 0.1, -0.03, 0.05, 0.01, -0.07,"4%"),RadicalFontParameters{0.06,0.12,0.43},false,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(-0.28, 0.31, 0.22, 0.95, 0.81, 0.06, 0.1, -0.03, 0.05, 0.01, -0.07,"4%"));
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==0.375);expect(request.stretch_height==0.025000000000000022);expect(request.stretch_depth==0.35);expect(request.root_width==0.1732);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.33, 0.41, -0.16, 0.66, 1.32, 0.05, 0.09, -0.04, 0.06, 0.02, 0.07,""),2);
            check_box(layout.bbox,make_box(0.6204, 0.3418, 0.35, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.06);expect(layout.q==0.135);expect(layout.required_surd_height==0.375);
            expect(layout.measured_height==0.024999999999999994);expect(layout.placed_height==0.08499999999999999);expect(layout.placed_surd_height==0.12);
            expect(layout.root_offset==0);expect(layout.root_height==0.15070000000000003);expect(layout.root_dx==0.5244444444444445);
            check_point(layout.surd_origin,0,-0.32499999999999996);
            check_point(layout.base_origin,0.33,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0.42480000000000007,0.15070000000000003);else expect(false);
            check_rect(layout.overbar,0.33,-0.035,-0.21,0.12);
        }
        { // Source-paired radical case 153: sqrt, inline, size=-1, wrapper=1.35
            RadicalInput input{make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"),{},RadicalFontParameters{0.06,0.12,0.43},false,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==0.375);expect(request.stretch_height==0.025000000000000022);expect(request.stretch_depth==0.35);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.33, 0.41, -0.16, 0.66, 1.32, 0.05, 0.09, -0.04, 0.06, 0.02, 0.07,""),-1);
            check_box(layout.bbox,make_box(0.6204, 0.1562, 0.35, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.06);expect(layout.q==0.135);expect(layout.required_surd_height==0.375);
            expect(layout.measured_height==0.024999999999999994);expect(layout.placed_height==0.127);expect(layout.placed_surd_height==0.162);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,-0.283);
            check_point(layout.base_origin,0.33,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.33,-0.035,-0.21,0.162);
        }
        { // Source-paired radical case 154: root, inline, size=-1, wrapper=1.35
            RadicalInput input{make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"),make_box(-0.28, 0.31, 0.22, 0.95, 0.81, 0.06, 0.1, -0.03, 0.05, 0.01, -0.07,"4%"),RadicalFontParameters{0.06,0.12,0.43},false,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(-0.28, 0.31, 0.22, 0.95, 0.81, 0.06, 0.1, -0.03, 0.05, 0.01, -0.07,"4%"));
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==0.375);expect(request.stretch_height==0.025000000000000022);expect(request.stretch_depth==0.35);expect(request.root_width==0.1732);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.33, 0.41, -0.16, 0.66, 1.32, 0.05, 0.09, -0.04, 0.06, 0.02, 0.07,""),-1);
            check_box(layout.bbox,make_box(0.6204, 2.1043, 0.35, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.06);expect(layout.q==0.135);expect(layout.required_surd_height==0.375);
            expect(layout.measured_height==0.024999999999999994);expect(layout.placed_height==0.127);expect(layout.placed_surd_height==0.162);
            expect(layout.root_offset==0);expect(layout.root_height==1.9552);expect(layout.root_dx==0.48370370370370375);
            check_point(layout.surd_origin,0,-0.283);
            check_point(layout.base_origin,0.33,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0.39180000000000004,1.9552);else expect(false);
            check_rect(layout.overbar,0.33,-0.035,-0.21,0.162);
        }
        { // Source-paired radical case 155: sqrt, inline, size=2, wrapper=1.35
            RadicalInput input{make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"),{},RadicalFontParameters{0.06,0.12,0.43},false,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==0.375);expect(request.stretch_height==0.025000000000000022);expect(request.stretch_depth==0.35);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.33, 0.41, -0.16, 0.66, 1.32, 0.05, 0.09, -0.04, 0.06, 0.02, 0.07,""),2);
            check_box(layout.bbox,make_box(0.6204, 0.1562, 0.35, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.06);expect(layout.q==0.135);expect(layout.required_surd_height==0.375);
            expect(layout.measured_height==0.024999999999999994);expect(layout.placed_height==0.127);expect(layout.placed_surd_height==0.162);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,-0.283);
            check_point(layout.base_origin,0.33,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.33,-0.035,-0.21,0.162);
        }
        { // Source-paired radical case 156: root, inline, size=2, wrapper=1.35
            RadicalInput input{make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"),make_box(-0.28, 0.31, 0.22, 0.95, 0.81, 0.06, 0.1, -0.03, 0.05, 0.01, -0.07,"4%"),RadicalFontParameters{0.06,0.12,0.43},false,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(-0.28, 0.31, 0.22, 0.95, 0.81, 0.06, 0.1, -0.03, 0.05, 0.01, -0.07,"4%"));
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==0.375);expect(request.stretch_height==0.025000000000000022);expect(request.stretch_depth==0.35);expect(request.root_width==0.1732);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.33, 0.41, -0.16, 0.66, 1.32, 0.05, 0.09, -0.04, 0.06, 0.02, 0.07,""),2);
            check_box(layout.bbox,make_box(0.6204, 0.3418, 0.35, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.06);expect(layout.q==0.135);expect(layout.required_surd_height==0.375);
            expect(layout.measured_height==0.024999999999999994);expect(layout.placed_height==0.127);expect(layout.placed_surd_height==0.162);
            expect(layout.root_offset==0);expect(layout.root_height==0.19270000000000004);expect(layout.root_dx==0.5244444444444445);
            check_point(layout.surd_origin,0,-0.283);
            check_point(layout.base_origin,0.33,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0.42480000000000007,0.19270000000000004);else expect(false);
            check_rect(layout.overbar,0.33,-0.035,-0.21,0.162);
        }
        { // Source-paired radical case 157: sqrt, display, size=-1, wrapper=0.75
            RadicalInput input{make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"),{},RadicalFontParameters{0.06,0.12,0.43},true,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==true);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==0.46749999999999997);expect(request.stretch_height==0.1175);expect(request.stretch_depth==0.35);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.33, 0.41, -0.16, 0.66, 1.32, 0.05, 0.09, -0.04, 0.06, 0.02, 0.07,""),-1);
            check_box(layout.bbox,make_box(0.6204, 0.24870000000000003, 0.35, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.43);expect(layout.q==0.22749999999999998);expect(layout.required_surd_height==0.46749999999999997);
            expect(layout.measured_height==0.11749999999999997);expect(layout.placed_height==0.14749999999999996);expect(layout.placed_surd_height==0.09);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,-0.2625);
            check_point(layout.base_origin,0.33,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.33,0.05749999999999997,-0.21,0.09);
        }
        { // Source-paired radical case 158: root, display, size=-1, wrapper=0.75
            RadicalInput input{make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"),make_box(-0.28, 0.31, 0.22, 0.95, 0.81, 0.06, 0.1, -0.03, 0.05, 0.01, -0.07,"4%"),RadicalFontParameters{0.06,0.12,0.43},true,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(-0.28, 0.31, 0.22, 0.95, 0.81, 0.06, 0.1, -0.03, 0.05, 0.01, -0.07,"4%"));
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==true);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==0.46749999999999997);expect(request.stretch_height==0.1175);expect(request.stretch_depth==0.35);expect(request.root_width==0.1732);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.33, 0.41, -0.16, 0.66, 1.32, 0.05, 0.09, -0.04, 0.06, 0.02, 0.07,""),-1);
            check_box(layout.bbox,make_box(0.6204, 2.1968, 0.35, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.43);expect(layout.q==0.22749999999999998);expect(layout.required_surd_height==0.46749999999999997);
            expect(layout.measured_height==0.11749999999999997);expect(layout.placed_height==0.14749999999999996);expect(layout.placed_surd_height==0.09);
            expect(layout.root_offset==0);expect(layout.root_height==1.9756999999999998);expect(layout.root_dx==0.48370370370370375);
            check_point(layout.surd_origin,0,-0.2625);
            check_point(layout.base_origin,0.33,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0.39180000000000004,1.9756999999999998);else expect(false);
            check_rect(layout.overbar,0.33,0.05749999999999997,-0.21,0.09);
        }
        { // Source-paired radical case 159: sqrt, display, size=2, wrapper=0.75
            RadicalInput input{make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"),{},RadicalFontParameters{0.06,0.12,0.43},true,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==true);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==0.46749999999999997);expect(request.stretch_height==0.1175);expect(request.stretch_depth==0.35);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.33, 0.41, -0.16, 0.66, 1.32, 0.05, 0.09, -0.04, 0.06, 0.02, 0.07,""),2);
            check_box(layout.bbox,make_box(0.6204, 0.24870000000000003, 0.35, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.43);expect(layout.q==0.22749999999999998);expect(layout.required_surd_height==0.46749999999999997);
            expect(layout.measured_height==0.11749999999999997);expect(layout.placed_height==0.14749999999999996);expect(layout.placed_surd_height==0.09);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,-0.2625);
            check_point(layout.base_origin,0.33,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.33,0.05749999999999997,-0.21,0.09);
        }
        { // Source-paired radical case 160: root, display, size=2, wrapper=0.75
            RadicalInput input{make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"),make_box(-0.28, 0.31, 0.22, 0.95, 0.81, 0.06, 0.1, -0.03, 0.05, 0.01, -0.07,"4%"),RadicalFontParameters{0.06,0.12,0.43},true,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(-0.28, 0.31, 0.22, 0.95, 0.81, 0.06, 0.1, -0.03, 0.05, 0.01, -0.07,"4%"));
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==true);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==0.46749999999999997);expect(request.stretch_height==0.1175);expect(request.stretch_depth==0.35);expect(request.root_width==0.1732);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.33, 0.41, -0.16, 0.66, 1.32, 0.05, 0.09, -0.04, 0.06, 0.02, 0.07,""),2);
            check_box(layout.bbox,make_box(0.6204, 0.4343, 0.35, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.43);expect(layout.q==0.22749999999999998);expect(layout.required_surd_height==0.46749999999999997);
            expect(layout.measured_height==0.11749999999999997);expect(layout.placed_height==0.14749999999999996);expect(layout.placed_surd_height==0.09);
            expect(layout.root_offset==0);expect(layout.root_height==0.2132);expect(layout.root_dx==0.5244444444444445);
            check_point(layout.surd_origin,0,-0.2625);
            check_point(layout.base_origin,0.33,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0.42480000000000007,0.2132);else expect(false);
            check_rect(layout.overbar,0.33,0.05749999999999997,-0.21,0.09);
        }
        { // Source-paired radical case 161: sqrt, display, size=-1, wrapper=1
            RadicalInput input{make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"),{},RadicalFontParameters{0.06,0.12,0.43},true,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==0.46749999999999997);expect(request.stretch_height==0.1175);expect(request.stretch_depth==0.35);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.33, 0.41, -0.16, 0.66, 1.32, 0.05, 0.09, -0.04, 0.06, 0.02, 0.07,""),-1);
            check_box(layout.bbox,make_box(0.6204, 0.24870000000000003, 0.35, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.43);expect(layout.q==0.22749999999999998);expect(layout.required_surd_height==0.46749999999999997);
            expect(layout.measured_height==0.11749999999999997);expect(layout.placed_height==0.17749999999999996);expect(layout.placed_surd_height==0.12);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,-0.2325);
            check_point(layout.base_origin,0.33,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.33,0.05749999999999997,-0.21,0.12);
        }
        { // Source-paired radical case 162: root, display, size=-1, wrapper=1
            RadicalInput input{make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"),make_box(-0.28, 0.31, 0.22, 0.95, 0.81, 0.06, 0.1, -0.03, 0.05, 0.01, -0.07,"4%"),RadicalFontParameters{0.06,0.12,0.43},true,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(-0.28, 0.31, 0.22, 0.95, 0.81, 0.06, 0.1, -0.03, 0.05, 0.01, -0.07,"4%"));
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==0.46749999999999997);expect(request.stretch_height==0.1175);expect(request.stretch_depth==0.35);expect(request.root_width==0.1732);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.33, 0.41, -0.16, 0.66, 1.32, 0.05, 0.09, -0.04, 0.06, 0.02, 0.07,""),-1);
            check_box(layout.bbox,make_box(0.6204, 2.1968, 0.35, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.43);expect(layout.q==0.22749999999999998);expect(layout.required_surd_height==0.46749999999999997);
            expect(layout.measured_height==0.11749999999999997);expect(layout.placed_height==0.17749999999999996);expect(layout.placed_surd_height==0.12);
            expect(layout.root_offset==0);expect(layout.root_height==2.0057);expect(layout.root_dx==0.48370370370370375);
            check_point(layout.surd_origin,0,-0.2325);
            check_point(layout.base_origin,0.33,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0.39180000000000004,2.0057);else expect(false);
            check_rect(layout.overbar,0.33,0.05749999999999997,-0.21,0.12);
        }
        { // Source-paired radical case 163: sqrt, display, size=2, wrapper=1
            RadicalInput input{make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"),{},RadicalFontParameters{0.06,0.12,0.43},true,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==0.46749999999999997);expect(request.stretch_height==0.1175);expect(request.stretch_depth==0.35);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.33, 0.41, -0.16, 0.66, 1.32, 0.05, 0.09, -0.04, 0.06, 0.02, 0.07,""),2);
            check_box(layout.bbox,make_box(0.6204, 0.24870000000000003, 0.35, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.43);expect(layout.q==0.22749999999999998);expect(layout.required_surd_height==0.46749999999999997);
            expect(layout.measured_height==0.11749999999999997);expect(layout.placed_height==0.17749999999999996);expect(layout.placed_surd_height==0.12);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,-0.2325);
            check_point(layout.base_origin,0.33,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.33,0.05749999999999997,-0.21,0.12);
        }
        { // Source-paired radical case 164: root, display, size=2, wrapper=1
            RadicalInput input{make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"),make_box(-0.28, 0.31, 0.22, 0.95, 0.81, 0.06, 0.1, -0.03, 0.05, 0.01, -0.07,"4%"),RadicalFontParameters{0.06,0.12,0.43},true,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(-0.28, 0.31, 0.22, 0.95, 0.81, 0.06, 0.1, -0.03, 0.05, 0.01, -0.07,"4%"));
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==0.46749999999999997);expect(request.stretch_height==0.1175);expect(request.stretch_depth==0.35);expect(request.root_width==0.1732);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.33, 0.41, -0.16, 0.66, 1.32, 0.05, 0.09, -0.04, 0.06, 0.02, 0.07,""),2);
            check_box(layout.bbox,make_box(0.6204, 0.4343, 0.35, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.43);expect(layout.q==0.22749999999999998);expect(layout.required_surd_height==0.46749999999999997);
            expect(layout.measured_height==0.11749999999999997);expect(layout.placed_height==0.17749999999999996);expect(layout.placed_surd_height==0.12);
            expect(layout.root_offset==0);expect(layout.root_height==0.2432);expect(layout.root_dx==0.5244444444444445);
            check_point(layout.surd_origin,0,-0.2325);
            check_point(layout.base_origin,0.33,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0.42480000000000007,0.2432);else expect(false);
            check_rect(layout.overbar,0.33,0.05749999999999997,-0.21,0.12);
        }
        { // Source-paired radical case 165: sqrt, display, size=-1, wrapper=1.35
            RadicalInput input{make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"),{},RadicalFontParameters{0.06,0.12,0.43},true,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==0.46749999999999997);expect(request.stretch_height==0.1175);expect(request.stretch_depth==0.35);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.33, 0.41, -0.16, 0.66, 1.32, 0.05, 0.09, -0.04, 0.06, 0.02, 0.07,""),-1);
            check_box(layout.bbox,make_box(0.6204, 0.24870000000000003, 0.35, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.43);expect(layout.q==0.22749999999999998);expect(layout.required_surd_height==0.46749999999999997);
            expect(layout.measured_height==0.11749999999999997);expect(layout.placed_height==0.21949999999999997);expect(layout.placed_surd_height==0.162);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,-0.1905);
            check_point(layout.base_origin,0.33,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.33,0.05749999999999997,-0.21,0.162);
        }
        { // Source-paired radical case 166: root, display, size=-1, wrapper=1.35
            RadicalInput input{make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"),make_box(-0.28, 0.31, 0.22, 0.95, 0.81, 0.06, 0.1, -0.03, 0.05, 0.01, -0.07,"4%"),RadicalFontParameters{0.06,0.12,0.43},true,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(-0.28, 0.31, 0.22, 0.95, 0.81, 0.06, 0.1, -0.03, 0.05, 0.01, -0.07,"4%"));
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==0.46749999999999997);expect(request.stretch_height==0.1175);expect(request.stretch_depth==0.35);expect(request.root_width==0.1732);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.33, 0.41, -0.16, 0.66, 1.32, 0.05, 0.09, -0.04, 0.06, 0.02, 0.07,""),-1);
            check_box(layout.bbox,make_box(0.6204, 2.1968, 0.35, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.43);expect(layout.q==0.22749999999999998);expect(layout.required_surd_height==0.46749999999999997);
            expect(layout.measured_height==0.11749999999999997);expect(layout.placed_height==0.21949999999999997);expect(layout.placed_surd_height==0.162);
            expect(layout.root_offset==0);expect(layout.root_height==2.0477);expect(layout.root_dx==0.48370370370370375);
            check_point(layout.surd_origin,0,-0.1905);
            check_point(layout.base_origin,0.33,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0.39180000000000004,2.0477);else expect(false);
            check_rect(layout.overbar,0.33,0.05749999999999997,-0.21,0.162);
        }
        { // Source-paired radical case 167: sqrt, display, size=2, wrapper=1.35
            RadicalInput input{make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"),{},RadicalFontParameters{0.06,0.12,0.43},true,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==0.46749999999999997);expect(request.stretch_height==0.1175);expect(request.stretch_depth==0.35);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.33, 0.41, -0.16, 0.66, 1.32, 0.05, 0.09, -0.04, 0.06, 0.02, 0.07,""),2);
            check_box(layout.bbox,make_box(0.6204, 0.24870000000000003, 0.35, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.43);expect(layout.q==0.22749999999999998);expect(layout.required_surd_height==0.46749999999999997);
            expect(layout.measured_height==0.11749999999999997);expect(layout.placed_height==0.21949999999999997);expect(layout.placed_surd_height==0.162);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,-0.1905);
            check_point(layout.base_origin,0.33,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.33,0.05749999999999997,-0.21,0.162);
        }
        { // Source-paired radical case 168: root, display, size=2, wrapper=1.35
            RadicalInput input{make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"),make_box(-0.28, 0.31, 0.22, 0.95, 0.81, 0.06, 0.1, -0.03, 0.05, 0.01, -0.07,"4%"),RadicalFontParameters{0.06,0.12,0.43},true,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(-0.28, 0.31, 0.22, 0.95, 0.81, 0.06, 0.1, -0.03, 0.05, 0.01, -0.07,"4%"));
            check_font(request.font,0.06,0.12,0.43);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==0.46749999999999997);expect(request.stretch_height==0.1175);expect(request.stretch_depth==0.35);expect(request.root_width==0.1732);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.33, 0.41, -0.16, 0.66, 1.32, 0.05, 0.09, -0.04, 0.06, 0.02, 0.07,""),2);
            check_box(layout.bbox,make_box(0.6204, 0.4343, 0.35, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.43);expect(layout.q==0.22749999999999998);expect(layout.required_surd_height==0.46749999999999997);
            expect(layout.measured_height==0.11749999999999997);expect(layout.placed_height==0.21949999999999997);expect(layout.placed_surd_height==0.162);
            expect(layout.root_offset==0);expect(layout.root_height==0.2852);expect(layout.root_dx==0.5244444444444445);
            check_point(layout.surd_origin,0,-0.1905);
            check_point(layout.base_origin,0.33,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0.42480000000000007,0.2852);else expect(false);
            check_rect(layout.overbar,0.33,0.05749999999999997,-0.21,0.162);
        }
        { // Source-paired radical case 169: sqrt, inline, size=-1, wrapper=0.75
            RadicalInput input{make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"),{},RadicalFontParameters{0.15,0.08,0.27},false,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==false);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==0.44749999999999995);expect(request.stretch_height==0.09749999999999998);expect(request.stretch_depth==0.35);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.33, 0.41, -0.16, 0.66, 1.32, 0.05, 0.09, -0.04, 0.06, 0.02, 0.07,""),-1);
            check_box(layout.bbox,make_box(0.6204, 0.22870000000000001, 0.35, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.15);expect(layout.q==0.1175);expect(layout.required_surd_height==0.44749999999999995);
            expect(layout.measured_height==0.09749999999999998);expect(layout.placed_height==0.007499999999999979);expect(layout.placed_surd_height==0.06);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,-0.40249999999999997);
            check_point(layout.base_origin,0.33,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.33,-0.05250000000000002,-0.21,0.06);
        }
        { // Source-paired radical case 170: root, inline, size=-1, wrapper=0.75
            RadicalInput input{make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"),make_box(-0.28, 0.31, 0.22, 0.95, 0.81, 0.06, 0.1, -0.03, 0.05, 0.01, -0.07,"4%"),RadicalFontParameters{0.15,0.08,0.27},false,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(-0.28, 0.31, 0.22, 0.95, 0.81, 0.06, 0.1, -0.03, 0.05, 0.01, -0.07,"4%"));
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==false);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==0.44749999999999995);expect(request.stretch_height==0.09749999999999998);expect(request.stretch_depth==0.35);expect(request.root_width==0.1732);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.33, 0.41, -0.16, 0.66, 1.32, 0.05, 0.09, -0.04, 0.06, 0.02, 0.07,""),-1);
            check_box(layout.bbox,make_box(0.6204, 2.1768, 0.35, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.15);expect(layout.q==0.1175);expect(layout.required_surd_height==0.44749999999999995);
            expect(layout.measured_height==0.09749999999999998);expect(layout.placed_height==0.007499999999999979);expect(layout.placed_surd_height==0.06);
            expect(layout.root_offset==0);expect(layout.root_height==1.8357);expect(layout.root_dx==0.48370370370370375);
            check_point(layout.surd_origin,0,-0.40249999999999997);
            check_point(layout.base_origin,0.33,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0.39180000000000004,1.8357);else expect(false);
            check_rect(layout.overbar,0.33,-0.05250000000000002,-0.21,0.06);
        }
        { // Source-paired radical case 171: sqrt, inline, size=2, wrapper=0.75
            RadicalInput input{make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"),{},RadicalFontParameters{0.15,0.08,0.27},false,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==false);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==0.44749999999999995);expect(request.stretch_height==0.09749999999999998);expect(request.stretch_depth==0.35);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.33, 0.41, -0.16, 0.66, 1.32, 0.05, 0.09, -0.04, 0.06, 0.02, 0.07,""),2);
            check_box(layout.bbox,make_box(0.6204, 0.22870000000000001, 0.35, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.15);expect(layout.q==0.1175);expect(layout.required_surd_height==0.44749999999999995);
            expect(layout.measured_height==0.09749999999999998);expect(layout.placed_height==0.007499999999999979);expect(layout.placed_surd_height==0.06);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,-0.40249999999999997);
            check_point(layout.base_origin,0.33,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.33,-0.05250000000000002,-0.21,0.06);
        }
        { // Source-paired radical case 172: root, inline, size=2, wrapper=0.75
            RadicalInput input{make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"),make_box(-0.28, 0.31, 0.22, 0.95, 0.81, 0.06, 0.1, -0.03, 0.05, 0.01, -0.07,"4%"),RadicalFontParameters{0.15,0.08,0.27},false,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(-0.28, 0.31, 0.22, 0.95, 0.81, 0.06, 0.1, -0.03, 0.05, 0.01, -0.07,"4%"));
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==false);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==0.44749999999999995);expect(request.stretch_height==0.09749999999999998);expect(request.stretch_depth==0.35);expect(request.root_width==0.1732);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.33, 0.41, -0.16, 0.66, 1.32, 0.05, 0.09, -0.04, 0.06, 0.02, 0.07,""),2);
            check_box(layout.bbox,make_box(0.6204, 0.4143, 0.35, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.15);expect(layout.q==0.1175);expect(layout.required_surd_height==0.44749999999999995);
            expect(layout.measured_height==0.09749999999999998);expect(layout.placed_height==0.007499999999999979);expect(layout.placed_surd_height==0.06);
            expect(layout.root_offset==0);expect(layout.root_height==0.07320000000000002);expect(layout.root_dx==0.5244444444444445);
            check_point(layout.surd_origin,0,-0.40249999999999997);
            check_point(layout.base_origin,0.33,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0.42480000000000007,0.07320000000000002);else expect(false);
            check_rect(layout.overbar,0.33,-0.05250000000000002,-0.21,0.06);
        }
        { // Source-paired radical case 173: sqrt, inline, size=-1, wrapper=1
            RadicalInput input{make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"),{},RadicalFontParameters{0.15,0.08,0.27},false,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==0.44749999999999995);expect(request.stretch_height==0.09749999999999998);expect(request.stretch_depth==0.35);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.33, 0.41, -0.16, 0.66, 1.32, 0.05, 0.09, -0.04, 0.06, 0.02, 0.07,""),-1);
            check_box(layout.bbox,make_box(0.6204, 0.22870000000000001, 0.35, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.15);expect(layout.q==0.1175);expect(layout.required_surd_height==0.44749999999999995);
            expect(layout.measured_height==0.09749999999999998);expect(layout.placed_height==0.027499999999999983);expect(layout.placed_surd_height==0.08);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,-0.3825);
            check_point(layout.base_origin,0.33,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.33,-0.05250000000000002,-0.21,0.08);
        }
        { // Source-paired radical case 174: root, inline, size=-1, wrapper=1
            RadicalInput input{make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"),make_box(-0.28, 0.31, 0.22, 0.95, 0.81, 0.06, 0.1, -0.03, 0.05, 0.01, -0.07,"4%"),RadicalFontParameters{0.15,0.08,0.27},false,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(-0.28, 0.31, 0.22, 0.95, 0.81, 0.06, 0.1, -0.03, 0.05, 0.01, -0.07,"4%"));
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==0.44749999999999995);expect(request.stretch_height==0.09749999999999998);expect(request.stretch_depth==0.35);expect(request.root_width==0.1732);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.33, 0.41, -0.16, 0.66, 1.32, 0.05, 0.09, -0.04, 0.06, 0.02, 0.07,""),-1);
            check_box(layout.bbox,make_box(0.6204, 2.1768, 0.35, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.15);expect(layout.q==0.1175);expect(layout.required_surd_height==0.44749999999999995);
            expect(layout.measured_height==0.09749999999999998);expect(layout.placed_height==0.027499999999999983);expect(layout.placed_surd_height==0.08);
            expect(layout.root_offset==0);expect(layout.root_height==1.8557000000000001);expect(layout.root_dx==0.48370370370370375);
            check_point(layout.surd_origin,0,-0.3825);
            check_point(layout.base_origin,0.33,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0.39180000000000004,1.8557000000000001);else expect(false);
            check_rect(layout.overbar,0.33,-0.05250000000000002,-0.21,0.08);
        }
        { // Source-paired radical case 175: sqrt, inline, size=2, wrapper=1
            RadicalInput input{make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"),{},RadicalFontParameters{0.15,0.08,0.27},false,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==0.44749999999999995);expect(request.stretch_height==0.09749999999999998);expect(request.stretch_depth==0.35);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.33, 0.41, -0.16, 0.66, 1.32, 0.05, 0.09, -0.04, 0.06, 0.02, 0.07,""),2);
            check_box(layout.bbox,make_box(0.6204, 0.22870000000000001, 0.35, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.15);expect(layout.q==0.1175);expect(layout.required_surd_height==0.44749999999999995);
            expect(layout.measured_height==0.09749999999999998);expect(layout.placed_height==0.027499999999999983);expect(layout.placed_surd_height==0.08);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,-0.3825);
            check_point(layout.base_origin,0.33,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.33,-0.05250000000000002,-0.21,0.08);
        }
        { // Source-paired radical case 176: root, inline, size=2, wrapper=1
            RadicalInput input{make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"),make_box(-0.28, 0.31, 0.22, 0.95, 0.81, 0.06, 0.1, -0.03, 0.05, 0.01, -0.07,"4%"),RadicalFontParameters{0.15,0.08,0.27},false,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(-0.28, 0.31, 0.22, 0.95, 0.81, 0.06, 0.1, -0.03, 0.05, 0.01, -0.07,"4%"));
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==0.44749999999999995);expect(request.stretch_height==0.09749999999999998);expect(request.stretch_depth==0.35);expect(request.root_width==0.1732);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.33, 0.41, -0.16, 0.66, 1.32, 0.05, 0.09, -0.04, 0.06, 0.02, 0.07,""),2);
            check_box(layout.bbox,make_box(0.6204, 0.4143, 0.35, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.15);expect(layout.q==0.1175);expect(layout.required_surd_height==0.44749999999999995);
            expect(layout.measured_height==0.09749999999999998);expect(layout.placed_height==0.027499999999999983);expect(layout.placed_surd_height==0.08);
            expect(layout.root_offset==0);expect(layout.root_height==0.09320000000000003);expect(layout.root_dx==0.5244444444444445);
            check_point(layout.surd_origin,0,-0.3825);
            check_point(layout.base_origin,0.33,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0.42480000000000007,0.09320000000000003);else expect(false);
            check_rect(layout.overbar,0.33,-0.05250000000000002,-0.21,0.08);
        }
        { // Source-paired radical case 177: sqrt, inline, size=-1, wrapper=1.35
            RadicalInput input{make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"),{},RadicalFontParameters{0.15,0.08,0.27},false,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==0.44749999999999995);expect(request.stretch_height==0.09749999999999998);expect(request.stretch_depth==0.35);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.33, 0.41, -0.16, 0.66, 1.32, 0.05, 0.09, -0.04, 0.06, 0.02, 0.07,""),-1);
            check_box(layout.bbox,make_box(0.6204, 0.22870000000000001, 0.35, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.15);expect(layout.q==0.1175);expect(layout.required_surd_height==0.44749999999999995);
            expect(layout.measured_height==0.09749999999999998);expect(layout.placed_height==0.055499999999999994);expect(layout.placed_surd_height==0.10800000000000001);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,-0.3545);
            check_point(layout.base_origin,0.33,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.33,-0.05250000000000002,-0.21,0.10800000000000001);
        }
        { // Source-paired radical case 178: root, inline, size=-1, wrapper=1.35
            RadicalInput input{make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"),make_box(-0.28, 0.31, 0.22, 0.95, 0.81, 0.06, 0.1, -0.03, 0.05, 0.01, -0.07,"4%"),RadicalFontParameters{0.15,0.08,0.27},false,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(-0.28, 0.31, 0.22, 0.95, 0.81, 0.06, 0.1, -0.03, 0.05, 0.01, -0.07,"4%"));
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==0.44749999999999995);expect(request.stretch_height==0.09749999999999998);expect(request.stretch_depth==0.35);expect(request.root_width==0.1732);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.33, 0.41, -0.16, 0.66, 1.32, 0.05, 0.09, -0.04, 0.06, 0.02, 0.07,""),-1);
            check_box(layout.bbox,make_box(0.6204, 2.1768, 0.35, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.15);expect(layout.q==0.1175);expect(layout.required_surd_height==0.44749999999999995);
            expect(layout.measured_height==0.09749999999999998);expect(layout.placed_height==0.055499999999999994);expect(layout.placed_surd_height==0.10800000000000001);
            expect(layout.root_offset==0);expect(layout.root_height==1.8837000000000002);expect(layout.root_dx==0.48370370370370375);
            check_point(layout.surd_origin,0,-0.3545);
            check_point(layout.base_origin,0.33,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0.39180000000000004,1.8837000000000002);else expect(false);
            check_rect(layout.overbar,0.33,-0.05250000000000002,-0.21,0.10800000000000001);
        }
        { // Source-paired radical case 179: sqrt, inline, size=2, wrapper=1.35
            RadicalInput input{make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"),{},RadicalFontParameters{0.15,0.08,0.27},false,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==0.44749999999999995);expect(request.stretch_height==0.09749999999999998);expect(request.stretch_depth==0.35);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.33, 0.41, -0.16, 0.66, 1.32, 0.05, 0.09, -0.04, 0.06, 0.02, 0.07,""),2);
            check_box(layout.bbox,make_box(0.6204, 0.22870000000000001, 0.35, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.15);expect(layout.q==0.1175);expect(layout.required_surd_height==0.44749999999999995);
            expect(layout.measured_height==0.09749999999999998);expect(layout.placed_height==0.055499999999999994);expect(layout.placed_surd_height==0.10800000000000001);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,-0.3545);
            check_point(layout.base_origin,0.33,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.33,-0.05250000000000002,-0.21,0.10800000000000001);
        }
        { // Source-paired radical case 180: root, inline, size=2, wrapper=1.35
            RadicalInput input{make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"),make_box(-0.28, 0.31, 0.22, 0.95, 0.81, 0.06, 0.1, -0.03, 0.05, 0.01, -0.07,"4%"),RadicalFontParameters{0.15,0.08,0.27},false,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(-0.28, 0.31, 0.22, 0.95, 0.81, 0.06, 0.1, -0.03, 0.05, 0.01, -0.07,"4%"));
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==false);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==0.44749999999999995);expect(request.stretch_height==0.09749999999999998);expect(request.stretch_depth==0.35);expect(request.root_width==0.1732);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.33, 0.41, -0.16, 0.66, 1.32, 0.05, 0.09, -0.04, 0.06, 0.02, 0.07,""),2);
            check_box(layout.bbox,make_box(0.6204, 0.4143, 0.35, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.15);expect(layout.q==0.1175);expect(layout.required_surd_height==0.44749999999999995);
            expect(layout.measured_height==0.09749999999999998);expect(layout.placed_height==0.055499999999999994);expect(layout.placed_surd_height==0.10800000000000001);
            expect(layout.root_offset==0);expect(layout.root_height==0.12120000000000003);expect(layout.root_dx==0.5244444444444445);
            check_point(layout.surd_origin,0,-0.3545);
            check_point(layout.base_origin,0.33,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0.42480000000000007,0.12120000000000003);else expect(false);
            check_rect(layout.overbar,0.33,-0.05250000000000002,-0.21,0.10800000000000001);
        }
        { // Source-paired radical case 181: sqrt, display, size=-1, wrapper=0.75
            RadicalInput input{make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"),{},RadicalFontParameters{0.15,0.08,0.27},true,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==true);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==0.4775);expect(request.stretch_height==0.1275);expect(request.stretch_depth==0.35);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.33, 0.41, -0.16, 0.66, 1.32, 0.05, 0.09, -0.04, 0.06, 0.02, 0.07,""),-1);
            check_box(layout.bbox,make_box(0.6204, 0.25870000000000004, 0.35, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.27);expect(layout.q==0.14750000000000002);expect(layout.required_surd_height==0.4775);
            expect(layout.measured_height==0.1275);expect(layout.placed_height==0.037500000000000006);expect(layout.placed_surd_height==0.06);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,-0.37249999999999994);
            check_point(layout.base_origin,0.33,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.33,-0.022499999999999992,-0.21,0.06);
        }
        { // Source-paired radical case 182: root, display, size=-1, wrapper=0.75
            RadicalInput input{make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"),make_box(-0.28, 0.31, 0.22, 0.95, 0.81, 0.06, 0.1, -0.03, 0.05, 0.01, -0.07,"4%"),RadicalFontParameters{0.15,0.08,0.27},true,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(-0.28, 0.31, 0.22, 0.95, 0.81, 0.06, 0.1, -0.03, 0.05, 0.01, -0.07,"4%"));
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==true);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==0.4775);expect(request.stretch_height==0.1275);expect(request.stretch_depth==0.35);expect(request.root_width==0.1732);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.33, 0.41, -0.16, 0.66, 1.32, 0.05, 0.09, -0.04, 0.06, 0.02, 0.07,""),-1);
            check_box(layout.bbox,make_box(0.6204, 2.2068, 0.35, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.27);expect(layout.q==0.14750000000000002);expect(layout.required_surd_height==0.4775);
            expect(layout.measured_height==0.1275);expect(layout.placed_height==0.037500000000000006);expect(layout.placed_surd_height==0.06);
            expect(layout.root_offset==0);expect(layout.root_height==1.8657);expect(layout.root_dx==0.48370370370370375);
            check_point(layout.surd_origin,0,-0.37249999999999994);
            check_point(layout.base_origin,0.33,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0.39180000000000004,1.8657);else expect(false);
            check_rect(layout.overbar,0.33,-0.022499999999999992,-0.21,0.06);
        }
        { // Source-paired radical case 183: sqrt, display, size=2, wrapper=0.75
            RadicalInput input{make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"),{},RadicalFontParameters{0.15,0.08,0.27},true,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==true);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==0.4775);expect(request.stretch_height==0.1275);expect(request.stretch_depth==0.35);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.33, 0.41, -0.16, 0.66, 1.32, 0.05, 0.09, -0.04, 0.06, 0.02, 0.07,""),2);
            check_box(layout.bbox,make_box(0.6204, 0.25870000000000004, 0.35, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.27);expect(layout.q==0.14750000000000002);expect(layout.required_surd_height==0.4775);
            expect(layout.measured_height==0.1275);expect(layout.placed_height==0.037500000000000006);expect(layout.placed_surd_height==0.06);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,-0.37249999999999994);
            check_point(layout.base_origin,0.33,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.33,-0.022499999999999992,-0.21,0.06);
        }
        { // Source-paired radical case 184: root, display, size=2, wrapper=0.75
            RadicalInput input{make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"),make_box(-0.28, 0.31, 0.22, 0.95, 0.81, 0.06, 0.1, -0.03, 0.05, 0.01, -0.07,"4%"),RadicalFontParameters{0.15,0.08,0.27},true,0.75};
            auto request=request_radical(input);
            check_box(request.base,make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(-0.28, 0.31, 0.22, 0.95, 0.81, 0.06, 0.1, -0.03, 0.05, 0.01, -0.07,"4%"));
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==true);expect(request.wrapper_bbox_scale==0.75);
            expect(request.required_surd_height==0.4775);expect(request.stretch_height==0.1275);expect(request.stretch_depth==0.35);expect(request.root_width==0.1732);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.33, 0.41, -0.16, 0.66, 1.32, 0.05, 0.09, -0.04, 0.06, 0.02, 0.07,""),2);
            check_box(layout.bbox,make_box(0.6204, 0.44430000000000003, 0.35, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.27);expect(layout.q==0.14750000000000002);expect(layout.required_surd_height==0.4775);
            expect(layout.measured_height==0.1275);expect(layout.placed_height==0.037500000000000006);expect(layout.placed_surd_height==0.06);
            expect(layout.root_offset==0);expect(layout.root_height==0.10320000000000004);expect(layout.root_dx==0.5244444444444445);
            check_point(layout.surd_origin,0,-0.37249999999999994);
            check_point(layout.base_origin,0.33,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0.42480000000000007,0.10320000000000004);else expect(false);
            check_rect(layout.overbar,0.33,-0.022499999999999992,-0.21,0.06);
        }
        { // Source-paired radical case 185: sqrt, display, size=-1, wrapper=1
            RadicalInput input{make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"),{},RadicalFontParameters{0.15,0.08,0.27},true,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==0.4775);expect(request.stretch_height==0.1275);expect(request.stretch_depth==0.35);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.33, 0.41, -0.16, 0.66, 1.32, 0.05, 0.09, -0.04, 0.06, 0.02, 0.07,""),-1);
            check_box(layout.bbox,make_box(0.6204, 0.25870000000000004, 0.35, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.27);expect(layout.q==0.14750000000000002);expect(layout.required_surd_height==0.4775);
            expect(layout.measured_height==0.1275);expect(layout.placed_height==0.05750000000000001);expect(layout.placed_surd_height==0.08);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,-0.3525);
            check_point(layout.base_origin,0.33,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.33,-0.022499999999999992,-0.21,0.08);
        }
        { // Source-paired radical case 186: root, display, size=-1, wrapper=1
            RadicalInput input{make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"),make_box(-0.28, 0.31, 0.22, 0.95, 0.81, 0.06, 0.1, -0.03, 0.05, 0.01, -0.07,"4%"),RadicalFontParameters{0.15,0.08,0.27},true,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(-0.28, 0.31, 0.22, 0.95, 0.81, 0.06, 0.1, -0.03, 0.05, 0.01, -0.07,"4%"));
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==0.4775);expect(request.stretch_height==0.1275);expect(request.stretch_depth==0.35);expect(request.root_width==0.1732);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.33, 0.41, -0.16, 0.66, 1.32, 0.05, 0.09, -0.04, 0.06, 0.02, 0.07,""),-1);
            check_box(layout.bbox,make_box(0.6204, 2.2068, 0.35, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.27);expect(layout.q==0.14750000000000002);expect(layout.required_surd_height==0.4775);
            expect(layout.measured_height==0.1275);expect(layout.placed_height==0.05750000000000001);expect(layout.placed_surd_height==0.08);
            expect(layout.root_offset==0);expect(layout.root_height==1.8857);expect(layout.root_dx==0.48370370370370375);
            check_point(layout.surd_origin,0,-0.3525);
            check_point(layout.base_origin,0.33,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0.39180000000000004,1.8857);else expect(false);
            check_rect(layout.overbar,0.33,-0.022499999999999992,-0.21,0.08);
        }
        { // Source-paired radical case 187: sqrt, display, size=2, wrapper=1
            RadicalInput input{make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"),{},RadicalFontParameters{0.15,0.08,0.27},true,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==0.4775);expect(request.stretch_height==0.1275);expect(request.stretch_depth==0.35);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.33, 0.41, -0.16, 0.66, 1.32, 0.05, 0.09, -0.04, 0.06, 0.02, 0.07,""),2);
            check_box(layout.bbox,make_box(0.6204, 0.25870000000000004, 0.35, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.27);expect(layout.q==0.14750000000000002);expect(layout.required_surd_height==0.4775);
            expect(layout.measured_height==0.1275);expect(layout.placed_height==0.05750000000000001);expect(layout.placed_surd_height==0.08);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,-0.3525);
            check_point(layout.base_origin,0.33,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.33,-0.022499999999999992,-0.21,0.08);
        }
        { // Source-paired radical case 188: root, display, size=2, wrapper=1
            RadicalInput input{make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"),make_box(-0.28, 0.31, 0.22, 0.95, 0.81, 0.06, 0.1, -0.03, 0.05, 0.01, -0.07,"4%"),RadicalFontParameters{0.15,0.08,0.27},true,1};
            auto request=request_radical(input);
            check_box(request.base,make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(-0.28, 0.31, 0.22, 0.95, 0.81, 0.06, 0.1, -0.03, 0.05, 0.01, -0.07,"4%"));
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1);
            expect(request.required_surd_height==0.4775);expect(request.stretch_height==0.1275);expect(request.stretch_depth==0.35);expect(request.root_width==0.1732);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.33, 0.41, -0.16, 0.66, 1.32, 0.05, 0.09, -0.04, 0.06, 0.02, 0.07,""),2);
            check_box(layout.bbox,make_box(0.6204, 0.44430000000000003, 0.35, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.27);expect(layout.q==0.14750000000000002);expect(layout.required_surd_height==0.4775);
            expect(layout.measured_height==0.1275);expect(layout.placed_height==0.05750000000000001);expect(layout.placed_surd_height==0.08);
            expect(layout.root_offset==0);expect(layout.root_height==0.12320000000000006);expect(layout.root_dx==0.5244444444444445);
            check_point(layout.surd_origin,0,-0.3525);
            check_point(layout.base_origin,0.33,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0.42480000000000007,0.12320000000000006);else expect(false);
            check_rect(layout.overbar,0.33,-0.022499999999999992,-0.21,0.08);
        }
        { // Source-paired radical case 189: sqrt, display, size=-1, wrapper=1.35
            RadicalInput input{make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"),{},RadicalFontParameters{0.15,0.08,0.27},true,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==0.4775);expect(request.stretch_height==0.1275);expect(request.stretch_depth==0.35);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.33, 0.41, -0.16, 0.66, 1.32, 0.05, 0.09, -0.04, 0.06, 0.02, 0.07,""),-1);
            check_box(layout.bbox,make_box(0.6204, 0.25870000000000004, 0.35, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.27);expect(layout.q==0.14750000000000002);expect(layout.required_surd_height==0.4775);
            expect(layout.measured_height==0.1275);expect(layout.placed_height==0.08550000000000002);expect(layout.placed_surd_height==0.10800000000000001);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,-0.32449999999999996);
            check_point(layout.base_origin,0.33,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.33,-0.022499999999999992,-0.21,0.10800000000000001);
        }
        { // Source-paired radical case 190: root, display, size=-1, wrapper=1.35
            RadicalInput input{make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"),make_box(-0.28, 0.31, 0.22, 0.95, 0.81, 0.06, 0.1, -0.03, 0.05, 0.01, -0.07,"4%"),RadicalFontParameters{0.15,0.08,0.27},true,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(-0.28, 0.31, 0.22, 0.95, 0.81, 0.06, 0.1, -0.03, 0.05, 0.01, -0.07,"4%"));
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==0.4775);expect(request.stretch_height==0.1275);expect(request.stretch_depth==0.35);expect(request.root_width==0.1732);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.33, 0.41, -0.16, 0.66, 1.32, 0.05, 0.09, -0.04, 0.06, 0.02, 0.07,""),-1);
            check_box(layout.bbox,make_box(0.6204, 2.2068, 0.35, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.27);expect(layout.q==0.14750000000000002);expect(layout.required_surd_height==0.4775);
            expect(layout.measured_height==0.1275);expect(layout.placed_height==0.08550000000000002);expect(layout.placed_surd_height==0.10800000000000001);
            expect(layout.root_offset==0);expect(layout.root_height==1.9137);expect(layout.root_dx==0.48370370370370375);
            check_point(layout.surd_origin,0,-0.32449999999999996);
            check_point(layout.base_origin,0.33,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0.39180000000000004,1.9137);else expect(false);
            check_rect(layout.overbar,0.33,-0.022499999999999992,-0.21,0.10800000000000001);
        }
        { // Source-paired radical case 191: sqrt, display, size=2, wrapper=1.35
            RadicalInput input{make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"),{},RadicalFontParameters{0.15,0.08,0.27},true,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"));
            expect(!request.degree.has_value());
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==0.4775);expect(request.stretch_height==0.1275);expect(request.stretch_depth==0.35);expect(request.root_width==1.25);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            auto layout=finish_radical(request,make_box(0.33, 0.41, -0.16, 0.66, 1.32, 0.05, 0.09, -0.04, 0.06, 0.02, 0.07,""),2);
            check_box(layout.bbox,make_box(0.6204, 0.25870000000000004, 0.35, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.27);expect(layout.q==0.14750000000000002);expect(layout.required_surd_height==0.4775);
            expect(layout.measured_height==0.1275);expect(layout.placed_height==0.08550000000000002);expect(layout.placed_surd_height==0.10800000000000001);
            expect(layout.root_offset==0);expect(layout.root_height==0);expect(layout.root_dx==0);
            check_point(layout.surd_origin,0,-0.32449999999999996);
            check_point(layout.base_origin,0.33,0);
            expect(!layout.degree_origin.has_value());
            check_rect(layout.overbar,0.33,-0.022499999999999992,-0.21,0.10800000000000001);
        }
        { // Source-paired radical case 192: root, display, size=2, wrapper=1.35
            RadicalInput input{make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"),make_box(-0.28, 0.31, 0.22, 0.95, 0.81, 0.06, 0.1, -0.03, 0.05, 0.01, -0.07,"4%"),RadicalFontParameters{0.15,0.08,0.27},true,1.35};
            auto request=request_radical(input);
            check_box(request.base,make_box(-0.21, -0.17, 0.35, 1.3, 0.69, 0.16, 0.05, -0.08, 0.04, 0.07, 0.02,"17%"));
            expect(request.degree.has_value());check_box(*request.degree,make_box(-0.28, 0.31, 0.22, 0.95, 0.81, 0.06, 0.1, -0.03, 0.05, 0.01, -0.07,"4%"));
            check_font(request.font,0.15,0.08,0.27);
            expect(request.display==true);expect(request.wrapper_bbox_scale==1.35);
            expect(request.required_surd_height==0.4775);expect(request.stretch_height==0.1275);expect(request.stretch_depth==0.35);expect(request.root_width==0.1732);
            expect(request.stretch_exact==true);
            input.base.w=999;input.font.rule_thickness=999;input.wrapper_bbox_scale=999;
            input.degree->w=999;
            auto layout=finish_radical(request,make_box(0.33, 0.41, -0.16, 0.66, 1.32, 0.05, 0.09, -0.04, 0.06, 0.02, 0.07,""),2);
            check_box(layout.bbox,make_box(0.6204, 0.44430000000000003, 0.35, 1, 1, 0, 0, 0, 0, 0, 0,""));
            expect(layout.p==0.27);expect(layout.q==0.14750000000000002);expect(layout.required_surd_height==0.4775);
            expect(layout.measured_height==0.1275);expect(layout.placed_height==0.08550000000000002);expect(layout.placed_surd_height==0.10800000000000001);
            expect(layout.root_offset==0);expect(layout.root_height==0.15120000000000006);expect(layout.root_dx==0.5244444444444445);
            check_point(layout.surd_origin,0,-0.32449999999999996);
            check_point(layout.base_origin,0.33,0);
            if(layout.degree_origin.has_value())check_point(*layout.degree_origin,0.42480000000000007,0.15120000000000006);else expect(false);
            check_rect(layout.overbar,0.33,-0.022499999999999992,-0.21,0.10800000000000001);
        }
    };
};
