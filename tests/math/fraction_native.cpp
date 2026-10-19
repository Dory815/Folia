#include "../support/folia_test.hpp"
import folia.math.util.bbox;
import folia.math.output.common.placement.fraction;

using namespace boost::ut;
using folia::math::BBox;
using namespace folia::math::output::common::placement;
namespace {
BBox make_box(double w,double h,double d,double scale,double rscale,double L,double R) { BBox b; b.w=w;b.h=h;b.d=d;b.scale=scale;b.rscale=rscale;b.L=L;b.R=R;return b; }
void check_box(BBox const& a,BBox const& e) { expect(a.w==e.w);expect(a.h==e.h);expect(a.d==e.d);expect(a.scale==e.scale);expect(a.rscale==e.rscale);expect(a.L==e.L);expect(a.R==e.R);expect(a.pwidth==e.pwidth);expect(a.ic==e.ic);expect(a.oc==e.oc);expect(a.sk==e.sk);expect(a.dx==e.dx); }
void check_point(FractionPoint const& a,double x,double y) { expect(a.x==x);expect(a.y==y); }
void check_font(FractionFontParameters const& a,double axis,double n1,double n2,double n3,double d1,double d2,double rule,double nullspace) { expect(a.axis_height==axis);expect(a.num1==n1);expect(a.num2==n2);expect(a.num3==n3);expect(a.denom1==d1);expect(a.denom2==d2);expect(a.rule_thickness==rule);expect(a.nulldelimiterspace==nullspace); }
}
suite native_fraction_source_pairs = [] {
    "all_generated_source_fraction_pairs"_test = [] {
        { // Paired Source observation 1: ruled, inline, left/right, delims=false
            FractionInput in{make_box(0.46, 0.31, 0.08, 1, 0.83, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1, 1.14, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0.06,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(1.8194000000000001, 0.6637, 0.692, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.2532,0.4064);
            check_point(actual.denominator_origin,0.31120000000000014,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0.12);expect(actual.rule->y==0.22);expect(actual.rule->width==1.5794000000000001);expect(actual.rule->height==0.06);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==1.3794000000000002);else expect(false);
            expect(actual.numerator_wrap_width==1.3794000000000002);expect(actual.denominator_wrap_width==1.3794000000000002);
        }
        { // Paired Source observation 2: atop, inline, left/right, delims=false
            FractionInput in{make_box(0.46, 0.31, 0.08, 1, 0.83, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1, 1.14, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(1.6194000000000002, 0.7773, 0.692, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.1532,0.52);
            check_point(actual.denominator_origin,0.21120000000000017,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==1.3794000000000002);else expect(false);
            expect(actual.numerator_wrap_width==1.3794000000000002);expect(actual.denominator_wrap_width==1.3794000000000002);
        }
        { // Paired Source observation 3: bevelled, inline, left/right, delims=false
            FractionInput in{make_box(0.46, 0.31, 0.08, 1, 0.83, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1, 1.14, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0.06,FractionAlignment::left,FractionAlignment::right};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==0.7);expect(request.delta==0.15);
            expect(request.numerator_y==0.28500000000000003);expect(request.denominator_y==0.19999999999999998);expect(request.display==false);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(0.46, 0.31, 0.08, 1, 0.83, 0.04, 0.02));check_box(request.denominator,make_box(1.08, 0.1, 0.3, 1, 1.14, 0.08, 0.05));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(2.0796, 0.6916, 0.142, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.0332,0.28500000000000003);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.3566,0);else expect(false);
            check_point(actual.denominator_origin,0.7927999999999998,0.19999999999999998);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.7001999999999999,0.19999999999999998);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==0.46);expect(actual.denominator_wrap_width==1.08);
        }
        { // Paired Source observation 4: ruled, inline, left/right, delims=false
            FractionInput in{make_box(1.35, 1.05, 0.24, 1, 0.83, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1, 1.14, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0.17,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(1.6268999999999998, 1.5756999999999999, 1.6381999999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.1287,0.7041999999999999);
            check_point(actual.denominator_origin,0.8482999999999998,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0.12);expect(actual.rule->y==0.16499999999999998);expect(actual.rule->width==1.3868999999999998);expect(actual.rule->height==0.17);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==1.1868999999999998);else expect(false);
            expect(actual.numerator_wrap_width==1.1868999999999998);expect(actual.denominator_wrap_width==1.1868999999999998);
        }
        { // Paired Source observation 5: atop, inline, left/right, delims=false
            FractionInput in{make_box(1.35, 1.05, 0.24, 1, 0.83, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1, 1.14, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(1.4268999999999998, 1.3915, 1.6381999999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.028700000000000003,0.52);
            check_point(actual.denominator_origin,0.7482999999999999,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==1.1868999999999998);else expect(false);
            expect(actual.numerator_wrap_width==1.1868999999999998);expect(actual.denominator_wrap_width==1.1868999999999998);
        }
        { // Paired Source observation 6: bevelled, inline, left/right, delims=false
            FractionInput in{make_box(1.35, 1.05, 0.24, 1, 0.83, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1, 1.14, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0.06,FractionAlignment::left,FractionAlignment::right};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==1.66);expect(request.delta==0.15);
            expect(request.numerator_y==-0.005000000000000032);expect(request.denominator_y==0.5499999999999999);expect(request.display==false);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(1.35, 1.05, 0.24, 1, 0.83, -0.11, 0.19));check_box(request.denominator,make_box(0.51, 0.23, 1.13, 1, 1.14, 0.03, -0.02));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(2.0483000000000002, 0.8664999999999999, 0.7381999999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.09129999999999999,-0.005000000000000032);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,1.1118999999999999,0);else expect(false);
            check_point(actual.denominator_origin,1.4910999999999999,0.5499999999999999);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,1.4555,0.5499999999999999);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==1.35);expect(actual.denominator_wrap_width==0.51);
        }
        { // Paired Source observation 7: ruled, inline, left/right, delims=false
            FractionInput in{make_box(0.22, 0.16, -0.39, 1, 0.83, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1, 1.14, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0.025,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(0.8161999999999999, 0.5228, 0.4982, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.3611,0.39);
            check_point(actual.denominator_origin,0.15159999999999993,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0.12);expect(actual.rule->y==0.2375);expect(actual.rule->width==0.5761999999999999);expect(actual.rule->height==0.025);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0.3761999999999999);else expect(false);
            expect(actual.numerator_wrap_width==0.37619999999999987);expect(actual.denominator_wrap_width==0.37619999999999987);
        }
        { // Paired Source observation 8: atop, inline, left/right, delims=false
            FractionInput in{make_box(0.22, 0.16, -0.39, 1, 0.83, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1, 1.14, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0.6161999999999999, 0.6528, 0.4982, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.2611,0.52);
            check_point(actual.denominator_origin,0.051599999999999924,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0.37619999999999987);else expect(false);
            expect(actual.numerator_wrap_width==0.37619999999999987);expect(actual.denominator_wrap_width==0.37619999999999987);
        }
        { // Paired Source observation 9: bevelled, inline, left/right, delims=false
            FractionInput in{make_box(0.22, 0.16, -0.39, 1, 0.83, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1, 1.14, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0.06,FractionAlignment::left,FractionAlignment::right};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==0.06999999999999998);expect(request.delta==0.15);
            expect(request.numerator_y==0.12499999999999997);expect(request.denominator_y==0.365);expect(request.display==false);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(0.22, 0.16, -0.39, 1, 0.83, 0.17, -0.03));check_box(request.denominator,make_box(0.18, -0.4, 0.13, 1, 1.14, -0.06, 0.21));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(0.9435999999999998, 0.6916, 0.0728, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.1411,0.12499999999999997);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.22379999999999994,0);else expect(false);
            check_point(actual.denominator_origin,0.5003999999999998,0.365);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.5673999999999999,0.365);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==0.22);expect(actual.denominator_wrap_width==0.18);
        }
        { // Paired Source observation 10: ruled, inline, left/right, delims=false
            FractionInput in{make_box(-0.31, 0.27, 0.12, 1, 0.83, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1, 1.14, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,-0.03,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(0.44, 0.6141, 0.9314, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.15360000000000001,0.39);
            check_point(actual.denominator_origin,0.6219,-0.35000000000000003);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0.12);expect(actual.rule->y==0.265);expect(actual.rule->width==0.009100000000000025);expect(actual.rule->height==-0.03);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0);else expect(false);
            expect(actual.numerator_wrap_width==0);expect(actual.denominator_wrap_width==0);
        }
        { // Paired Source observation 11: atop, inline, left/right, delims=false
            FractionInput in{make_box(-0.31, 0.27, 0.12, 1, 0.83, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1, 1.14, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0.24, 0.7441, 0.9313999999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.053599999999999995,0.52);
            check_point(actual.denominator_origin,0.5219,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0);else expect(false);
            expect(actual.numerator_wrap_width==0);expect(actual.denominator_wrap_width==0);
        }
        { // Paired Source observation 12: bevelled, inline, left/right, delims=false
            FractionInput in{make_box(-0.31, 0.27, 0.12, 1, 0.83, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1, 1.14, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0.06,FractionAlignment::left,FractionAlignment::right};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==0.69);expect(request.delta==0.15);
            expect(request.numerator_y==0.32499999999999996);expect(request.denominator_y==0.46499999999999997);expect(request.display==false);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(-0.31, 0.27, 0.12, 1, 0.83, -0.08, 0.16));check_box(request.denominator,make_box(-0.48, -0.22, 0.51, 1, 1.14, -0.15, -0.04));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(0.34359999999999996, 0.6916, 0.11639999999999995, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.0664,0.32499999999999996);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,-0.26589999999999997,0);else expect(false);
            check_point(actual.denominator_origin,-0.09189999999999998,0.46499999999999997);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.26859999999999995,0.46499999999999997);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==-0.31);expect(actual.denominator_wrap_width==-0.48);
        }
        { // Paired Source observation 13: ruled, inline, left/right, delims=true
            FractionInput in{make_box(0.46, 0.31, 0.08, 1, 0.83, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1, 1.14, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0.06,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(1.5794000000000001, 0.6637, 0.692, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.1332,0.4064);
            check_point(actual.denominator_origin,0.19120000000000018,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0);expect(actual.rule->y==0.22);expect(actual.rule->width==1.5794000000000001);expect(actual.rule->height==0.06);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==1.3794000000000002);else expect(false);
            expect(actual.numerator_wrap_width==1.3794000000000002);expect(actual.denominator_wrap_width==1.3794000000000002);
        }
        { // Paired Source observation 14: atop, inline, left/right, delims=true
            FractionInput in{make_box(0.46, 0.31, 0.08, 1, 0.83, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1, 1.14, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(1.3794000000000002, 0.7773, 0.692, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.0332,0.52);
            check_point(actual.denominator_origin,0.09120000000000017,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==1.3794000000000002);else expect(false);
            expect(actual.numerator_wrap_width==1.3794000000000002);expect(actual.denominator_wrap_width==1.3794000000000002);
        }
        { // Paired Source observation 15: bevelled, inline, left/right, delims=true
            FractionInput in{make_box(0.46, 0.31, 0.08, 1, 0.83, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1, 1.14, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0.06,FractionAlignment::left,FractionAlignment::right};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==0.7);expect(request.delta==0.15);
            expect(request.numerator_y==0.28500000000000003);expect(request.denominator_y==0.19999999999999998);expect(request.display==false);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(0.46, 0.31, 0.08, 1, 0.83, 0.04, 0.02));check_box(request.denominator,make_box(1.08, 0.1, 0.3, 1, 1.14, 0.08, 0.05));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(2.0796, 0.6916, 0.142, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.0332,0.28500000000000003);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.3566,0);else expect(false);
            check_point(actual.denominator_origin,0.7927999999999998,0.19999999999999998);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.7001999999999999,0.19999999999999998);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==0.46);expect(actual.denominator_wrap_width==1.08);
        }
        { // Paired Source observation 16: ruled, inline, left/right, delims=true
            FractionInput in{make_box(1.35, 1.05, 0.24, 1, 0.83, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1, 1.14, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0.17,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(1.3868999999999998, 1.5756999999999999, 1.6381999999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.008700000000000013,0.7041999999999999);
            check_point(actual.denominator_origin,0.7282999999999998,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0);expect(actual.rule->y==0.16499999999999998);expect(actual.rule->width==1.3868999999999998);expect(actual.rule->height==0.17);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==1.1868999999999998);else expect(false);
            expect(actual.numerator_wrap_width==1.1868999999999998);expect(actual.denominator_wrap_width==1.1868999999999998);
        }
        { // Paired Source observation 17: atop, inline, left/right, delims=true
            FractionInput in{make_box(1.35, 1.05, 0.24, 1, 0.83, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1, 1.14, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(1.1868999999999998, 1.3915, 1.6381999999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.09129999999999999,0.52);
            check_point(actual.denominator_origin,0.6282999999999999,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==1.1868999999999998);else expect(false);
            expect(actual.numerator_wrap_width==1.1868999999999998);expect(actual.denominator_wrap_width==1.1868999999999998);
        }
        { // Paired Source observation 18: bevelled, inline, left/right, delims=true
            FractionInput in{make_box(1.35, 1.05, 0.24, 1, 0.83, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1, 1.14, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0.06,FractionAlignment::left,FractionAlignment::right};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==1.66);expect(request.delta==0.15);
            expect(request.numerator_y==-0.005000000000000032);expect(request.denominator_y==0.5499999999999999);expect(request.display==false);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(1.35, 1.05, 0.24, 1, 0.83, -0.11, 0.19));check_box(request.denominator,make_box(0.51, 0.23, 1.13, 1, 1.14, 0.03, -0.02));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(2.0483000000000002, 0.8664999999999999, 0.7381999999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.09129999999999999,-0.005000000000000032);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,1.1118999999999999,0);else expect(false);
            check_point(actual.denominator_origin,1.4910999999999999,0.5499999999999999);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,1.4555,0.5499999999999999);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==1.35);expect(actual.denominator_wrap_width==0.51);
        }
        { // Paired Source observation 19: ruled, inline, left/right, delims=true
            FractionInput in{make_box(0.22, 0.16, -0.39, 1, 0.83, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1, 1.14, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0.025,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(0.5761999999999999, 0.5228, 0.4982, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.2411,0.39);
            check_point(actual.denominator_origin,0.031599999999999934,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0);expect(actual.rule->y==0.2375);expect(actual.rule->width==0.5761999999999999);expect(actual.rule->height==0.025);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0.3761999999999999);else expect(false);
            expect(actual.numerator_wrap_width==0.3761999999999999);expect(actual.denominator_wrap_width==0.3761999999999999);
        }
        { // Paired Source observation 20: atop, inline, left/right, delims=true
            FractionInput in{make_box(0.22, 0.16, -0.39, 1, 0.83, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1, 1.14, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0.3761999999999999, 0.6528, 0.4982, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.1411,0.52);
            check_point(actual.denominator_origin,-0.06840000000000007,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0.3761999999999999);else expect(false);
            expect(actual.numerator_wrap_width==0.3761999999999999);expect(actual.denominator_wrap_width==0.3761999999999999);
        }
        { // Paired Source observation 21: bevelled, inline, left/right, delims=true
            FractionInput in{make_box(0.22, 0.16, -0.39, 1, 0.83, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1, 1.14, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0.06,FractionAlignment::left,FractionAlignment::right};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==0.06999999999999998);expect(request.delta==0.15);
            expect(request.numerator_y==0.12499999999999997);expect(request.denominator_y==0.365);expect(request.display==false);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(0.22, 0.16, -0.39, 1, 0.83, 0.17, -0.03));check_box(request.denominator,make_box(0.18, -0.4, 0.13, 1, 1.14, -0.06, 0.21));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(0.9435999999999998, 0.6916, 0.0728, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.1411,0.12499999999999997);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.22379999999999994,0);else expect(false);
            check_point(actual.denominator_origin,0.5003999999999998,0.365);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.5673999999999999,0.365);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==0.22);expect(actual.denominator_wrap_width==0.18);
        }
        { // Paired Source observation 22: ruled, inline, left/right, delims=true
            FractionInput in{make_box(-0.31, 0.27, 0.12, 1, 0.83, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1, 1.14, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,-0.03,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(0.2, 0.6141, 0.9314, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.033600000000000005,0.39);
            check_point(actual.denominator_origin,0.5019,-0.35000000000000003);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0);expect(actual.rule->y==0.265);expect(actual.rule->width==0.009100000000000025);expect(actual.rule->height==-0.03);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0);else expect(false);
            expect(actual.numerator_wrap_width==0);expect(actual.denominator_wrap_width==0);
        }
        { // Paired Source observation 23: atop, inline, left/right, delims=true
            FractionInput in{make_box(-0.31, 0.27, 0.12, 1, 0.83, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1, 1.14, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0, 0.7441, 0.9313999999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.0664,0.52);
            check_point(actual.denominator_origin,0.40190000000000003,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0);else expect(false);
            expect(actual.numerator_wrap_width==0);expect(actual.denominator_wrap_width==0);
        }
        { // Paired Source observation 24: bevelled, inline, left/right, delims=true
            FractionInput in{make_box(-0.31, 0.27, 0.12, 1, 0.83, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1, 1.14, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0.06,FractionAlignment::left,FractionAlignment::right};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==0.69);expect(request.delta==0.15);
            expect(request.numerator_y==0.32499999999999996);expect(request.denominator_y==0.46499999999999997);expect(request.display==false);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(-0.31, 0.27, 0.12, 1, 0.83, -0.08, 0.16));check_box(request.denominator,make_box(-0.48, -0.22, 0.51, 1, 1.14, -0.15, -0.04));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(0.34359999999999996, 0.6916, 0.11639999999999995, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.0664,0.32499999999999996);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,-0.26589999999999997,0);else expect(false);
            check_point(actual.denominator_origin,-0.09189999999999998,0.46499999999999997);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.26859999999999995,0.46499999999999997);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==-0.31);expect(actual.denominator_wrap_width==-0.48);
        }
        { // Paired Source observation 25: ruled, inline, center/left, delims=false
            FractionInput in{make_box(0.46, 0.31, 0.08, 1, 0.83, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1, 1.14, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0.06,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(1.8194000000000001, 0.6637, 0.692, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.7188000000000001,0.4064);
            check_point(actual.denominator_origin,0.3112,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0.12);expect(actual.rule->y==0.22);expect(actual.rule->width==1.5794000000000001);expect(actual.rule->height==0.06);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==1.3794000000000002);else expect(false);
            expect(actual.numerator_wrap_width==1.3794000000000002);expect(actual.denominator_wrap_width==1.3794000000000002);
        }
        { // Paired Source observation 26: atop, inline, center/left, delims=false
            FractionInput in{make_box(0.46, 0.31, 0.08, 1, 0.83, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1, 1.14, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(1.6194000000000002, 0.7773, 0.692, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.6188000000000001,0.52);
            check_point(actual.denominator_origin,0.2112,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==1.3794000000000002);else expect(false);
            expect(actual.numerator_wrap_width==1.3794000000000002);expect(actual.denominator_wrap_width==1.3794000000000002);
        }
        { // Paired Source observation 27: bevelled, inline, center/left, delims=false
            FractionInput in{make_box(0.46, 0.31, 0.08, 1, 0.83, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1, 1.14, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0.06,FractionAlignment::center,FractionAlignment::left};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==0.7);expect(request.delta==0.15);
            expect(request.numerator_y==0.28500000000000003);expect(request.denominator_y==0.19999999999999998);expect(request.display==false);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(0.46, 0.31, 0.08, 1, 0.83, 0.04, 0.02));check_box(request.denominator,make_box(1.08, 0.1, 0.3, 1, 1.14, 0.08, 0.05));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(2.0796, 0.6916, 0.142, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.0332,0.28500000000000003);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.3566,0);else expect(false);
            check_point(actual.denominator_origin,0.7927999999999998,0.19999999999999998);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.7001999999999999,0.19999999999999998);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==0.46);expect(actual.denominator_wrap_width==1.08);
        }
        { // Paired Source observation 28: ruled, inline, center/left, delims=false
            FractionInput in{make_box(1.35, 1.05, 0.24, 1, 0.83, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1, 1.14, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0.17,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(1.6268999999999998, 1.5756999999999999, 1.6381999999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.25319999999999987,0.7041999999999999);
            check_point(actual.denominator_origin,0.2542,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0.12);expect(actual.rule->y==0.16499999999999998);expect(actual.rule->width==1.3868999999999998);expect(actual.rule->height==0.17);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==1.1868999999999998);else expect(false);
            expect(actual.numerator_wrap_width==1.1868999999999998);expect(actual.denominator_wrap_width==1.1868999999999998);
        }
        { // Paired Source observation 29: atop, inline, center/left, delims=false
            FractionInput in{make_box(1.35, 1.05, 0.24, 1, 0.83, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1, 1.14, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(1.4268999999999998, 1.3915, 1.6381999999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.1531999999999999,0.52);
            check_point(actual.denominator_origin,0.1542,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==1.1868999999999998);else expect(false);
            expect(actual.numerator_wrap_width==1.1868999999999998);expect(actual.denominator_wrap_width==1.1868999999999998);
        }
        { // Paired Source observation 30: bevelled, inline, center/left, delims=false
            FractionInput in{make_box(1.35, 1.05, 0.24, 1, 0.83, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1, 1.14, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0.06,FractionAlignment::center,FractionAlignment::left};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==1.66);expect(request.delta==0.15);
            expect(request.numerator_y==-0.005000000000000032);expect(request.denominator_y==0.5499999999999999);expect(request.display==false);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(1.35, 1.05, 0.24, 1, 0.83, -0.11, 0.19));check_box(request.denominator,make_box(0.51, 0.23, 1.13, 1, 1.14, 0.03, -0.02));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(2.0483000000000002, 0.8664999999999999, 0.7381999999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.09129999999999999,-0.005000000000000032);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,1.1118999999999999,0);else expect(false);
            check_point(actual.denominator_origin,1.4910999999999999,0.5499999999999999);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,1.4555,0.5499999999999999);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==1.35);expect(actual.denominator_wrap_width==0.51);
        }
        { // Paired Source observation 31: ruled, inline, center/left, delims=false
            FractionInput in{make_box(0.22, 0.16, -0.39, 1, 0.83, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1, 1.14, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0.025,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(0.8161999999999999, 0.5228, 0.4982, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.31679999999999997,0.39);
            check_point(actual.denominator_origin,0.1516,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0.12);expect(actual.rule->y==0.2375);expect(actual.rule->width==0.5761999999999999);expect(actual.rule->height==0.025);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0.3761999999999999);else expect(false);
            expect(actual.numerator_wrap_width==0.37619999999999987);expect(actual.denominator_wrap_width==0.37619999999999987);
        }
        { // Paired Source observation 32: atop, inline, center/left, delims=false
            FractionInput in{make_box(0.22, 0.16, -0.39, 1, 0.83, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1, 1.14, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0.6161999999999999, 0.6528, 0.4982, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.21679999999999996,0.52);
            check_point(actual.denominator_origin,0.05160000000000001,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0.37619999999999987);else expect(false);
            expect(actual.numerator_wrap_width==0.37619999999999987);expect(actual.denominator_wrap_width==0.37619999999999987);
        }
        { // Paired Source observation 33: bevelled, inline, center/left, delims=false
            FractionInput in{make_box(0.22, 0.16, -0.39, 1, 0.83, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1, 1.14, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0.06,FractionAlignment::center,FractionAlignment::left};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==0.06999999999999998);expect(request.delta==0.15);
            expect(request.numerator_y==0.12499999999999997);expect(request.denominator_y==0.365);expect(request.display==false);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(0.22, 0.16, -0.39, 1, 0.83, 0.17, -0.03));check_box(request.denominator,make_box(0.18, -0.4, 0.13, 1, 1.14, -0.06, 0.21));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(0.9435999999999998, 0.6916, 0.0728, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.1411,0.12499999999999997);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.22379999999999994,0);else expect(false);
            check_point(actual.denominator_origin,0.5003999999999998,0.365);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.5673999999999999,0.365);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==0.22);expect(actual.denominator_wrap_width==0.18);
        }
        { // Paired Source observation 34: ruled, inline, center/left, delims=false
            FractionInput in{make_box(-0.31, 0.27, 0.12, 1, 0.83, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1, 1.14, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,-0.03,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(0.44, 0.6141, 0.9314, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.2532,0.39);
            check_point(actual.denominator_origin,0.049000000000000016,-0.35000000000000003);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0.12);expect(actual.rule->y==0.265);expect(actual.rule->width==0.009100000000000025);expect(actual.rule->height==-0.03);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0);else expect(false);
            expect(actual.numerator_wrap_width==0);expect(actual.denominator_wrap_width==0);
        }
        { // Paired Source observation 35: atop, inline, center/left, delims=false
            FractionInput in{make_box(-0.31, 0.27, 0.12, 1, 0.83, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1, 1.14, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0.24, 0.7441, 0.9313999999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.1532,0.52);
            check_point(actual.denominator_origin,-0.05099999999999999,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0);else expect(false);
            expect(actual.numerator_wrap_width==0);expect(actual.denominator_wrap_width==0);
        }
        { // Paired Source observation 36: bevelled, inline, center/left, delims=false
            FractionInput in{make_box(-0.31, 0.27, 0.12, 1, 0.83, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1, 1.14, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0.06,FractionAlignment::center,FractionAlignment::left};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==0.69);expect(request.delta==0.15);
            expect(request.numerator_y==0.32499999999999996);expect(request.denominator_y==0.46499999999999997);expect(request.display==false);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(-0.31, 0.27, 0.12, 1, 0.83, -0.08, 0.16));check_box(request.denominator,make_box(-0.48, -0.22, 0.51, 1, 1.14, -0.15, -0.04));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(0.34359999999999996, 0.6916, 0.11639999999999995, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.0664,0.32499999999999996);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,-0.26589999999999997,0);else expect(false);
            check_point(actual.denominator_origin,-0.09189999999999998,0.46499999999999997);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.26859999999999995,0.46499999999999997);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==-0.31);expect(actual.denominator_wrap_width==-0.48);
        }
        { // Paired Source observation 37: ruled, inline, center/left, delims=true
            FractionInput in{make_box(0.46, 0.31, 0.08, 1, 0.83, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1, 1.14, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0.06,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(1.5794000000000001, 0.6637, 0.692, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.5988000000000001,0.4064);
            check_point(actual.denominator_origin,0.19119999999999998,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0);expect(actual.rule->y==0.22);expect(actual.rule->width==1.5794000000000001);expect(actual.rule->height==0.06);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==1.3794000000000002);else expect(false);
            expect(actual.numerator_wrap_width==1.3794000000000002);expect(actual.denominator_wrap_width==1.3794000000000002);
        }
        { // Paired Source observation 38: atop, inline, center/left, delims=true
            FractionInput in{make_box(0.46, 0.31, 0.08, 1, 0.83, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1, 1.14, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(1.3794000000000002, 0.7773, 0.692, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.49880000000000013,0.52);
            check_point(actual.denominator_origin,0.09119999999999999,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==1.3794000000000002);else expect(false);
            expect(actual.numerator_wrap_width==1.3794000000000002);expect(actual.denominator_wrap_width==1.3794000000000002);
        }
        { // Paired Source observation 39: bevelled, inline, center/left, delims=true
            FractionInput in{make_box(0.46, 0.31, 0.08, 1, 0.83, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1, 1.14, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0.06,FractionAlignment::center,FractionAlignment::left};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==0.7);expect(request.delta==0.15);
            expect(request.numerator_y==0.28500000000000003);expect(request.denominator_y==0.19999999999999998);expect(request.display==false);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(0.46, 0.31, 0.08, 1, 0.83, 0.04, 0.02));check_box(request.denominator,make_box(1.08, 0.1, 0.3, 1, 1.14, 0.08, 0.05));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(2.0796, 0.6916, 0.142, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.0332,0.28500000000000003);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.3566,0);else expect(false);
            check_point(actual.denominator_origin,0.7927999999999998,0.19999999999999998);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.7001999999999999,0.19999999999999998);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==0.46);expect(actual.denominator_wrap_width==1.08);
        }
        { // Paired Source observation 40: ruled, inline, center/left, delims=true
            FractionInput in{make_box(1.35, 1.05, 0.24, 1, 0.83, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1, 1.14, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0.17,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(1.3868999999999998, 1.5756999999999999, 1.6381999999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.1331999999999999,0.7041999999999999);
            check_point(actual.denominator_origin,0.13419999999999999,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0);expect(actual.rule->y==0.16499999999999998);expect(actual.rule->width==1.3868999999999998);expect(actual.rule->height==0.17);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==1.1868999999999998);else expect(false);
            expect(actual.numerator_wrap_width==1.1868999999999998);expect(actual.denominator_wrap_width==1.1868999999999998);
        }
        { // Paired Source observation 41: atop, inline, center/left, delims=true
            FractionInput in{make_box(1.35, 1.05, 0.24, 1, 0.83, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1, 1.14, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(1.1868999999999998, 1.3915, 1.6381999999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.033199999999999896,0.52);
            check_point(actual.denominator_origin,0.034199999999999994,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==1.1868999999999998);else expect(false);
            expect(actual.numerator_wrap_width==1.1868999999999998);expect(actual.denominator_wrap_width==1.1868999999999998);
        }
        { // Paired Source observation 42: bevelled, inline, center/left, delims=true
            FractionInput in{make_box(1.35, 1.05, 0.24, 1, 0.83, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1, 1.14, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0.06,FractionAlignment::center,FractionAlignment::left};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==1.66);expect(request.delta==0.15);
            expect(request.numerator_y==-0.005000000000000032);expect(request.denominator_y==0.5499999999999999);expect(request.display==false);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(1.35, 1.05, 0.24, 1, 0.83, -0.11, 0.19));check_box(request.denominator,make_box(0.51, 0.23, 1.13, 1, 1.14, 0.03, -0.02));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(2.0483000000000002, 0.8664999999999999, 0.7381999999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.09129999999999999,-0.005000000000000032);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,1.1118999999999999,0);else expect(false);
            check_point(actual.denominator_origin,1.4910999999999999,0.5499999999999999);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,1.4555,0.5499999999999999);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==1.35);expect(actual.denominator_wrap_width==0.51);
        }
        { // Paired Source observation 43: ruled, inline, center/left, delims=true
            FractionInput in{make_box(0.22, 0.16, -0.39, 1, 0.83, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1, 1.14, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0.025,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(0.5761999999999999, 0.5228, 0.4982, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.19679999999999997,0.39);
            check_point(actual.denominator_origin,0.03160000000000002,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0);expect(actual.rule->y==0.2375);expect(actual.rule->width==0.5761999999999999);expect(actual.rule->height==0.025);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0.3761999999999999);else expect(false);
            expect(actual.numerator_wrap_width==0.3761999999999999);expect(actual.denominator_wrap_width==0.3761999999999999);
        }
        { // Paired Source observation 44: atop, inline, center/left, delims=true
            FractionInput in{make_box(0.22, 0.16, -0.39, 1, 0.83, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1, 1.14, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0.3761999999999999, 0.6528, 0.4982, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.09679999999999997,0.52);
            check_point(actual.denominator_origin,-0.06839999999999999,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0.3761999999999999);else expect(false);
            expect(actual.numerator_wrap_width==0.3761999999999999);expect(actual.denominator_wrap_width==0.3761999999999999);
        }
        { // Paired Source observation 45: bevelled, inline, center/left, delims=true
            FractionInput in{make_box(0.22, 0.16, -0.39, 1, 0.83, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1, 1.14, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0.06,FractionAlignment::center,FractionAlignment::left};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==0.06999999999999998);expect(request.delta==0.15);
            expect(request.numerator_y==0.12499999999999997);expect(request.denominator_y==0.365);expect(request.display==false);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(0.22, 0.16, -0.39, 1, 0.83, 0.17, -0.03));check_box(request.denominator,make_box(0.18, -0.4, 0.13, 1, 1.14, -0.06, 0.21));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(0.9435999999999998, 0.6916, 0.0728, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.1411,0.12499999999999997);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.22379999999999994,0);else expect(false);
            check_point(actual.denominator_origin,0.5003999999999998,0.365);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.5673999999999999,0.365);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==0.22);expect(actual.denominator_wrap_width==0.18);
        }
        { // Paired Source observation 46: ruled, inline, center/left, delims=true
            FractionInput in{make_box(-0.31, 0.27, 0.12, 1, 0.83, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1, 1.14, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,-0.03,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(0.2, 0.6141, 0.9314, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.13319999999999999,0.39);
            check_point(actual.denominator_origin,-0.07099999999999998,-0.35000000000000003);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0);expect(actual.rule->y==0.265);expect(actual.rule->width==0.009100000000000025);expect(actual.rule->height==-0.03);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0);else expect(false);
            expect(actual.numerator_wrap_width==0);expect(actual.denominator_wrap_width==0);
        }
        { // Paired Source observation 47: atop, inline, center/left, delims=true
            FractionInput in{make_box(-0.31, 0.27, 0.12, 1, 0.83, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1, 1.14, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0, 0.7441, 0.9313999999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.03319999999999999,0.52);
            check_point(actual.denominator_origin,-0.17099999999999999,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0);else expect(false);
            expect(actual.numerator_wrap_width==0);expect(actual.denominator_wrap_width==0);
        }
        { // Paired Source observation 48: bevelled, inline, center/left, delims=true
            FractionInput in{make_box(-0.31, 0.27, 0.12, 1, 0.83, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1, 1.14, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0.06,FractionAlignment::center,FractionAlignment::left};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==0.69);expect(request.delta==0.15);
            expect(request.numerator_y==0.32499999999999996);expect(request.denominator_y==0.46499999999999997);expect(request.display==false);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(-0.31, 0.27, 0.12, 1, 0.83, -0.08, 0.16));check_box(request.denominator,make_box(-0.48, -0.22, 0.51, 1, 1.14, -0.15, -0.04));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(0.34359999999999996, 0.6916, 0.11639999999999995, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.0664,0.32499999999999996);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,-0.26589999999999997,0);else expect(false);
            check_point(actual.denominator_origin,-0.09189999999999998,0.46499999999999997);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.26859999999999995,0.46499999999999997);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==-0.31);expect(actual.denominator_wrap_width==-0.48);
        }
        { // Paired Source observation 49: ruled, inline, right/center, delims=false
            FractionInput in{make_box(0.46, 0.31, 0.08, 1, 0.83, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1, 1.14, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0.06,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(1.8194000000000001, 0.6637, 0.692, 1, 1, 0, 0));
            check_point(actual.numerator_origin,1.201,0.4064);
            check_point(actual.denominator_origin,0.29410000000000003,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0.12);expect(actual.rule->y==0.22);expect(actual.rule->width==1.5794000000000001);expect(actual.rule->height==0.06);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==1.3794000000000002);else expect(false);
            expect(actual.numerator_wrap_width==1.3794000000000002);expect(actual.denominator_wrap_width==1.3794000000000002);
        }
        { // Paired Source observation 50: atop, inline, right/center, delims=false
            FractionInput in{make_box(0.46, 0.31, 0.08, 1, 0.83, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1, 1.14, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(1.6194000000000002, 0.7773, 0.692, 1, 1, 0, 0));
            check_point(actual.numerator_origin,1.101,0.52);
            check_point(actual.denominator_origin,0.19410000000000005,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==1.3794000000000002);else expect(false);
            expect(actual.numerator_wrap_width==1.3794000000000002);expect(actual.denominator_wrap_width==1.3794000000000002);
        }
        { // Paired Source observation 51: bevelled, inline, right/center, delims=false
            FractionInput in{make_box(0.46, 0.31, 0.08, 1, 0.83, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1, 1.14, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0.06,FractionAlignment::right,FractionAlignment::center};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==0.7);expect(request.delta==0.15);
            expect(request.numerator_y==0.28500000000000003);expect(request.denominator_y==0.19999999999999998);expect(request.display==false);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(0.46, 0.31, 0.08, 1, 0.83, 0.04, 0.02));check_box(request.denominator,make_box(1.08, 0.1, 0.3, 1, 1.14, 0.08, 0.05));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(2.0796, 0.6916, 0.142, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.0332,0.28500000000000003);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.3566,0);else expect(false);
            check_point(actual.denominator_origin,0.7927999999999998,0.19999999999999998);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.7001999999999999,0.19999999999999998);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==0.46);expect(actual.denominator_wrap_width==1.08);
        }
        { // Paired Source observation 52: ruled, inline, right/center, delims=false
            FractionInput in{make_box(1.35, 1.05, 0.24, 1, 0.83, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1, 1.14, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0.17,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(1.6268999999999998, 1.5756999999999999, 1.6381999999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.12869999999999984,0.7041999999999999);
            check_point(actual.denominator_origin,0.5227499999999999,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0.12);expect(actual.rule->y==0.16499999999999998);expect(actual.rule->width==1.3868999999999998);expect(actual.rule->height==0.17);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==1.1868999999999998);else expect(false);
            expect(actual.numerator_wrap_width==1.1868999999999998);expect(actual.denominator_wrap_width==1.1868999999999998);
        }
        { // Paired Source observation 53: atop, inline, right/center, delims=false
            FractionInput in{make_box(1.35, 1.05, 0.24, 1, 0.83, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1, 1.14, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(1.4268999999999998, 1.3915, 1.6381999999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.028699999999999837,0.52);
            check_point(actual.denominator_origin,0.42274999999999996,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==1.1868999999999998);else expect(false);
            expect(actual.numerator_wrap_width==1.1868999999999998);expect(actual.denominator_wrap_width==1.1868999999999998);
        }
        { // Paired Source observation 54: bevelled, inline, right/center, delims=false
            FractionInput in{make_box(1.35, 1.05, 0.24, 1, 0.83, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1, 1.14, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0.06,FractionAlignment::right,FractionAlignment::center};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==1.66);expect(request.delta==0.15);
            expect(request.numerator_y==-0.005000000000000032);expect(request.denominator_y==0.5499999999999999);expect(request.display==false);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(1.35, 1.05, 0.24, 1, 0.83, -0.11, 0.19));check_box(request.denominator,make_box(0.51, 0.23, 1.13, 1, 1.14, 0.03, -0.02));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(2.0483000000000002, 0.8664999999999999, 0.7381999999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.09129999999999999,-0.005000000000000032);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,1.1118999999999999,0);else expect(false);
            check_point(actual.denominator_origin,1.4910999999999999,0.5499999999999999);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,1.4555,0.5499999999999999);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==1.35);expect(actual.denominator_wrap_width==0.51);
        }
        { // Paired Source observation 55: ruled, inline, right/center, delims=false
            FractionInput in{make_box(0.22, 0.16, -0.39, 1, 0.83, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1, 1.14, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0.025,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(0.8161999999999999, 0.5228, 0.4982, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.4384999999999999,0.39);
            check_point(actual.denominator_origin,0.3055,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0.12);expect(actual.rule->y==0.2375);expect(actual.rule->width==0.5761999999999999);expect(actual.rule->height==0.025);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0.3761999999999999);else expect(false);
            expect(actual.numerator_wrap_width==0.37619999999999987);expect(actual.denominator_wrap_width==0.37619999999999987);
        }
        { // Paired Source observation 56: atop, inline, right/center, delims=false
            FractionInput in{make_box(0.22, 0.16, -0.39, 1, 0.83, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1, 1.14, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0.6161999999999999, 0.6528, 0.4982, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.3384999999999999,0.52);
            check_point(actual.denominator_origin,0.20549999999999996,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0.37619999999999987);else expect(false);
            expect(actual.numerator_wrap_width==0.37619999999999987);expect(actual.denominator_wrap_width==0.37619999999999987);
        }
        { // Paired Source observation 57: bevelled, inline, right/center, delims=false
            FractionInput in{make_box(0.22, 0.16, -0.39, 1, 0.83, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1, 1.14, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0.06,FractionAlignment::right,FractionAlignment::center};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==0.06999999999999998);expect(request.delta==0.15);
            expect(request.numerator_y==0.12499999999999997);expect(request.denominator_y==0.365);expect(request.display==false);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(0.22, 0.16, -0.39, 1, 0.83, 0.17, -0.03));check_box(request.denominator,make_box(0.18, -0.4, 0.13, 1, 1.14, -0.06, 0.21));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(0.9435999999999998, 0.6916, 0.0728, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.1411,0.12499999999999997);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.22379999999999994,0);else expect(false);
            check_point(actual.denominator_origin,0.5003999999999998,0.365);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.5673999999999999,0.365);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==0.22);expect(actual.denominator_wrap_width==0.18);
        }
        { // Paired Source observation 58: ruled, inline, right/center, delims=false
            FractionInput in{make_box(-0.31, 0.27, 0.12, 1, 0.83, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1, 1.14, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,-0.03,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(0.44, 0.6141, 0.9314, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.15360000000000001,0.39);
            check_point(actual.denominator_origin,0.39815,-0.35000000000000003);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0.12);expect(actual.rule->y==0.265);expect(actual.rule->width==0.009100000000000025);expect(actual.rule->height==-0.03);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0);else expect(false);
            expect(actual.numerator_wrap_width==0);expect(actual.denominator_wrap_width==0);
        }
        { // Paired Source observation 59: atop, inline, right/center, delims=false
            FractionInput in{make_box(-0.31, 0.27, 0.12, 1, 0.83, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1, 1.14, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0.24, 0.7441, 0.9313999999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.053599999999999995,0.52);
            check_point(actual.denominator_origin,0.29814999999999997,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0);else expect(false);
            expect(actual.numerator_wrap_width==0);expect(actual.denominator_wrap_width==0);
        }
        { // Paired Source observation 60: bevelled, inline, right/center, delims=false
            FractionInput in{make_box(-0.31, 0.27, 0.12, 1, 0.83, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1, 1.14, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0.06,FractionAlignment::right,FractionAlignment::center};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==0.69);expect(request.delta==0.15);
            expect(request.numerator_y==0.32499999999999996);expect(request.denominator_y==0.46499999999999997);expect(request.display==false);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(-0.31, 0.27, 0.12, 1, 0.83, -0.08, 0.16));check_box(request.denominator,make_box(-0.48, -0.22, 0.51, 1, 1.14, -0.15, -0.04));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(0.34359999999999996, 0.6916, 0.11639999999999995, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.0664,0.32499999999999996);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,-0.26589999999999997,0);else expect(false);
            check_point(actual.denominator_origin,-0.09189999999999998,0.46499999999999997);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.26859999999999995,0.46499999999999997);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==-0.31);expect(actual.denominator_wrap_width==-0.48);
        }
        { // Paired Source observation 61: ruled, inline, right/center, delims=true
            FractionInput in{make_box(0.46, 0.31, 0.08, 1, 0.83, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1, 1.14, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0.06,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(1.5794000000000001, 0.6637, 0.692, 1, 1, 0, 0));
            check_point(actual.numerator_origin,1.0810000000000002,0.4064);
            check_point(actual.denominator_origin,0.17410000000000006,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0);expect(actual.rule->y==0.22);expect(actual.rule->width==1.5794000000000001);expect(actual.rule->height==0.06);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==1.3794000000000002);else expect(false);
            expect(actual.numerator_wrap_width==1.3794000000000002);expect(actual.denominator_wrap_width==1.3794000000000002);
        }
        { // Paired Source observation 62: atop, inline, right/center, delims=true
            FractionInput in{make_box(0.46, 0.31, 0.08, 1, 0.83, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1, 1.14, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(1.3794000000000002, 0.7773, 0.692, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.9810000000000001,0.52);
            check_point(actual.denominator_origin,0.07410000000000005,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==1.3794000000000002);else expect(false);
            expect(actual.numerator_wrap_width==1.3794000000000002);expect(actual.denominator_wrap_width==1.3794000000000002);
        }
        { // Paired Source observation 63: bevelled, inline, right/center, delims=true
            FractionInput in{make_box(0.46, 0.31, 0.08, 1, 0.83, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1, 1.14, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0.06,FractionAlignment::right,FractionAlignment::center};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==0.7);expect(request.delta==0.15);
            expect(request.numerator_y==0.28500000000000003);expect(request.denominator_y==0.19999999999999998);expect(request.display==false);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(0.46, 0.31, 0.08, 1, 0.83, 0.04, 0.02));check_box(request.denominator,make_box(1.08, 0.1, 0.3, 1, 1.14, 0.08, 0.05));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(2.0796, 0.6916, 0.142, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.0332,0.28500000000000003);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.3566,0);else expect(false);
            check_point(actual.denominator_origin,0.7927999999999998,0.19999999999999998);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.7001999999999999,0.19999999999999998);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==0.46);expect(actual.denominator_wrap_width==1.08);
        }
        { // Paired Source observation 64: ruled, inline, right/center, delims=true
            FractionInput in{make_box(1.35, 1.05, 0.24, 1, 0.83, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1, 1.14, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0.17,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(1.3868999999999998, 1.5756999999999999, 1.6381999999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.008699999999999847,0.7041999999999999);
            check_point(actual.denominator_origin,0.40274999999999994,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0);expect(actual.rule->y==0.16499999999999998);expect(actual.rule->width==1.3868999999999998);expect(actual.rule->height==0.17);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==1.1868999999999998);else expect(false);
            expect(actual.numerator_wrap_width==1.1868999999999998);expect(actual.denominator_wrap_width==1.1868999999999998);
        }
        { // Paired Source observation 65: atop, inline, right/center, delims=true
            FractionInput in{make_box(1.35, 1.05, 0.24, 1, 0.83, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1, 1.14, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(1.1868999999999998, 1.3915, 1.6381999999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.09130000000000016,0.52);
            check_point(actual.denominator_origin,0.30274999999999996,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==1.1868999999999998);else expect(false);
            expect(actual.numerator_wrap_width==1.1868999999999998);expect(actual.denominator_wrap_width==1.1868999999999998);
        }
        { // Paired Source observation 66: bevelled, inline, right/center, delims=true
            FractionInput in{make_box(1.35, 1.05, 0.24, 1, 0.83, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1, 1.14, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0.06,FractionAlignment::right,FractionAlignment::center};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==1.66);expect(request.delta==0.15);
            expect(request.numerator_y==-0.005000000000000032);expect(request.denominator_y==0.5499999999999999);expect(request.display==false);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(1.35, 1.05, 0.24, 1, 0.83, -0.11, 0.19));check_box(request.denominator,make_box(0.51, 0.23, 1.13, 1, 1.14, 0.03, -0.02));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(2.0483000000000002, 0.8664999999999999, 0.7381999999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.09129999999999999,-0.005000000000000032);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,1.1118999999999999,0);else expect(false);
            check_point(actual.denominator_origin,1.4910999999999999,0.5499999999999999);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,1.4555,0.5499999999999999);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==1.35);expect(actual.denominator_wrap_width==0.51);
        }
        { // Paired Source observation 67: ruled, inline, right/center, delims=true
            FractionInput in{make_box(0.22, 0.16, -0.39, 1, 0.83, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1, 1.14, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0.025,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(0.5761999999999999, 0.5228, 0.4982, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.3184999999999999,0.39);
            check_point(actual.denominator_origin,0.1855,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0);expect(actual.rule->y==0.2375);expect(actual.rule->width==0.5761999999999999);expect(actual.rule->height==0.025);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0.3761999999999999);else expect(false);
            expect(actual.numerator_wrap_width==0.3761999999999999);expect(actual.denominator_wrap_width==0.3761999999999999);
        }
        { // Paired Source observation 68: atop, inline, right/center, delims=true
            FractionInput in{make_box(0.22, 0.16, -0.39, 1, 0.83, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1, 1.14, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0.3761999999999999, 0.6528, 0.4982, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.21849999999999992,0.52);
            check_point(actual.denominator_origin,0.08549999999999998,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0.3761999999999999);else expect(false);
            expect(actual.numerator_wrap_width==0.3761999999999999);expect(actual.denominator_wrap_width==0.3761999999999999);
        }
        { // Paired Source observation 69: bevelled, inline, right/center, delims=true
            FractionInput in{make_box(0.22, 0.16, -0.39, 1, 0.83, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1, 1.14, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0.06,FractionAlignment::right,FractionAlignment::center};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==0.06999999999999998);expect(request.delta==0.15);
            expect(request.numerator_y==0.12499999999999997);expect(request.denominator_y==0.365);expect(request.display==false);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(0.22, 0.16, -0.39, 1, 0.83, 0.17, -0.03));check_box(request.denominator,make_box(0.18, -0.4, 0.13, 1, 1.14, -0.06, 0.21));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(0.9435999999999998, 0.6916, 0.0728, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.1411,0.12499999999999997);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.22379999999999994,0);else expect(false);
            check_point(actual.denominator_origin,0.5003999999999998,0.365);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.5673999999999999,0.365);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==0.22);expect(actual.denominator_wrap_width==0.18);
        }
        { // Paired Source observation 70: ruled, inline, right/center, delims=true
            FractionInput in{make_box(-0.31, 0.27, 0.12, 1, 0.83, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1, 1.14, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,-0.03,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(0.2, 0.6141, 0.9314, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.033600000000000005,0.39);
            check_point(actual.denominator_origin,0.27815,-0.35000000000000003);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0);expect(actual.rule->y==0.265);expect(actual.rule->width==0.009100000000000025);expect(actual.rule->height==-0.03);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0);else expect(false);
            expect(actual.numerator_wrap_width==0);expect(actual.denominator_wrap_width==0);
        }
        { // Paired Source observation 71: atop, inline, right/center, delims=true
            FractionInput in{make_box(-0.31, 0.27, 0.12, 1, 0.83, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1, 1.14, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0, 0.7441, 0.9313999999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.0664,0.52);
            check_point(actual.denominator_origin,0.17814999999999998,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0);else expect(false);
            expect(actual.numerator_wrap_width==0);expect(actual.denominator_wrap_width==0);
        }
        { // Paired Source observation 72: bevelled, inline, right/center, delims=true
            FractionInput in{make_box(-0.31, 0.27, 0.12, 1, 0.83, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1, 1.14, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0.06,FractionAlignment::right,FractionAlignment::center};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==0.69);expect(request.delta==0.15);
            expect(request.numerator_y==0.32499999999999996);expect(request.denominator_y==0.46499999999999997);expect(request.display==false);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(-0.31, 0.27, 0.12, 1, 0.83, -0.08, 0.16));check_box(request.denominator,make_box(-0.48, -0.22, 0.51, 1, 1.14, -0.15, -0.04));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(0.34359999999999996, 0.6916, 0.11639999999999995, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.0664,0.32499999999999996);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,-0.26589999999999997,0);else expect(false);
            check_point(actual.denominator_origin,-0.09189999999999998,0.46499999999999997);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.26859999999999995,0.46499999999999997);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==-0.31);expect(actual.denominator_wrap_width==-0.48);
        }
        { // Paired Source observation 73: ruled, inline, left/right, delims=false
            FractionInput in{make_box(0.46, 0.31, 0.08, 0.72, 0.5975999999999999, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1.23, 1.4022, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0.06,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(2.1366620000000003, 0.575256, 0.7706599999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.243904,0.39);
            check_point(actual.denominator_origin,0.332176,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0.12);expect(actual.rule->y==0.22);expect(actual.rule->width==1.896662);expect(actual.rule->height==0.06);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==1.6966620000000003);else expect(false);
            expect(actual.numerator_wrap_width==1.6966620000000003);expect(actual.denominator_wrap_width==1.6966620000000003);
        }
        { // Paired Source observation 74: atop, inline, left/right, delims=false
            FractionInput in{make_box(0.46, 0.31, 0.08, 0.72, 0.5975999999999999, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1.23, 1.4022, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(1.936662, 0.705256, 0.7706599999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.143904,0.52);
            check_point(actual.denominator_origin,0.23217600000000005,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==1.6966620000000001);else expect(false);
            expect(actual.numerator_wrap_width==1.6966620000000001);expect(actual.denominator_wrap_width==1.6966620000000001);
        }
        { // Paired Source observation 75: bevelled, inline, left/right, delims=false
            FractionInput in{make_box(0.46, 0.31, 0.08, 0.72, 0.5975999999999999, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1.23, 1.4022, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0.06,FractionAlignment::left,FractionAlignment::right};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==0.792);expect(request.delta==0.15);
            expect(request.numerator_y==0.31720000000000004);expect(request.denominator_y==0.223);expect(request.display==false);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(0.46, 0.31, 0.08, 0.72, 0.5975999999999999, 0.04, 0.02));check_box(request.denominator,make_box(1.08, 0.1, 0.3, 1.23, 1.4022, 0.08, 0.05));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(2.276014, 0.6916, 0.19765999999999997, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.023903999999999998,0.31720000000000004);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.23575199999999996,0);else expect(false);
            check_point(actual.denominator_origin,0.6929279999999999,0.223);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.579352,0.223);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==0.46);expect(actual.denominator_wrap_width==1.08);
        }
        { // Paired Source observation 76: ruled, inline, left/right, delims=false
            FractionInput in{make_box(1.35, 1.05, 0.24, 0.72, 0.5975999999999999, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1.23, 1.4022, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0.17,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(1.294568, 1.275904, 1.9344859999999997, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.154264,0.648424);
            check_point(actual.denominator_origin,0.3874899999999999,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0.12);expect(actual.rule->y==0.16499999999999998);expect(actual.rule->width==1.054568);expect(actual.rule->height==0.17);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0.854568);else expect(false);
            expect(actual.numerator_wrap_width==0.854568);expect(actual.denominator_wrap_width==0.854568);
        }
        { // Paired Source observation 77: atop, inline, left/right, delims=false
            FractionInput in{make_box(1.35, 1.05, 0.24, 0.72, 0.5975999999999999, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1.23, 1.4022, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(1.0945679999999998, 1.1474799999999998, 1.9344859999999997, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.05426400000000001,0.52);
            check_point(actual.denominator_origin,0.2874899999999999,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0.8545679999999998);else expect(false);
            expect(actual.numerator_wrap_width==0.8545679999999998);expect(actual.denominator_wrap_width==0.8545679999999998);
        }
        { // Paired Source observation 78: bevelled, inline, left/right, delims=false
            FractionInput in{make_box(1.35, 1.05, 0.24, 0.72, 0.5975999999999999, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1.23, 1.4022, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0.06,FractionAlignment::left,FractionAlignment::right};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==1.9727999999999999);expect(request.delta==0.15);
            expect(request.numerator_y==0.10839999999999997);expect(request.denominator_y==0.6535);expect(request.display==false);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(1.35, 1.05, 0.24, 0.72, 0.5975999999999999, -0.11, 0.19));check_box(request.denominator,make_box(0.51, 0.23, 1.13, 1.23, 1.4022, 0.03, -0.02));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(1.852312, 0.9760059999999999, 0.9309859999999996, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.06573599999999999,0.10839999999999997);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.7795679999999999,0);else expect(false);
            check_point(actual.denominator_origin,1.166634,0.6535);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,1.123168,0.6535);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==1.35);expect(actual.denominator_wrap_width==0.51);
        }
        { // Paired Source observation 79: ruled, inline, left/right, delims=false
            FractionInput in{make_box(0.22, 0.16, -0.39, 0.72, 0.5975999999999999, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1.23, 1.4022, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0.025,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(0.9027259999999999, 0.485616, 0.532286, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.321592,0.39);
            check_point(actual.denominator_origin,0.13586799999999996,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0.12);expect(actual.rule->y==0.2375);expect(actual.rule->width==0.6627259999999999);expect(actual.rule->height==0.025);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0.4627259999999999);else expect(false);
            expect(actual.numerator_wrap_width==0.46272599999999997);expect(actual.denominator_wrap_width==0.46272599999999997);
        }
        { // Paired Source observation 80: atop, inline, left/right, delims=false
            FractionInput in{make_box(0.22, 0.16, -0.39, 0.72, 0.5975999999999999, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1.23, 1.4022, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0.702726, 0.615616, 0.532286, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.22159199999999998,0.52);
            check_point(actual.denominator_origin,0.035867999999999955,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0.46272599999999997);else expect(false);
            expect(actual.numerator_wrap_width==0.46272599999999997);expect(actual.denominator_wrap_width==0.46272599999999997);
        }
        { // Paired Source observation 81: bevelled, inline, left/right, delims=false
            FractionInput in{make_box(0.22, 0.16, -0.39, 0.72, 0.5975999999999999, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1.23, 1.4022, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0.06,FractionAlignment::left,FractionAlignment::right};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==0.1344);expect(request.delta==0.15);
            expect(request.numerator_y==0.20199999999999999);expect(request.denominator_y==0.42594999999999994);expect(request.display==false);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(0.22, 0.16, -0.39, 0.72, 0.5975999999999999, 0.17, -0.03));check_box(request.denominator,make_box(0.18, -0.4, 0.13, 1.23, 1.4022, -0.06, 0.21));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(0.9464619999999998, 0.6916, 0.0728, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.10159199999999999,0.20199999999999999);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.14013599999999998,0);else expect(false);
            check_point(actual.denominator_origin,0.4010039999999999,0.42594999999999994);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.4837359999999999,0.42594999999999994);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==0.22);expect(actual.denominator_wrap_width==0.18);
        }
        { // Paired Source observation 82: ruled, inline, left/right, delims=false
            FractionInput in{make_box(-0.31, 0.27, 0.12, 0.72, 0.5975999999999999, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1.23, 1.4022, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,-0.03,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(0.44, 0.5513520000000001, 1.065122, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.172192,0.39);
            check_point(actual.denominator_origin,0.811696,-0.35000000000000003);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0.12);expect(actual.rule->y==0.265);expect(actual.rule->width==0.06255200000000002);expect(actual.rule->height==-0.03);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0);else expect(false);
            expect(actual.numerator_wrap_width==0);expect(actual.denominator_wrap_width==0);
        }
        { // Paired Source observation 83: atop, inline, left/right, delims=false
            FractionInput in{make_box(-0.31, 0.27, 0.12, 0.72, 0.5975999999999999, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1.23, 1.4022, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0.24, 0.681352, 1.065122, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.072192,0.52);
            check_point(actual.denominator_origin,0.711696,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0);else expect(false);
            expect(actual.numerator_wrap_width==0);expect(actual.denominator_wrap_width==0);
        }
        { // Paired Source observation 84: bevelled, inline, left/right, delims=false
            FractionInput in{make_box(-0.31, 0.27, 0.12, 0.72, 0.5975999999999999, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1.23, 1.4022, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0.06,FractionAlignment::left,FractionAlignment::right};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==0.6567000000000001);expect(request.delta==0.15);
            expect(request.numerator_y==0.346);expect(request.denominator_y==0.5489499999999999);expect(request.display==false);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(-0.31, 0.27, 0.12, 0.72, 0.5975999999999999, -0.08, 0.16));check_box(request.denominator,make_box(-0.48, -0.22, 0.51, 1.23, 1.4022, -0.15, -0.04));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(0.34359999999999996, 0.6916, 0.166172, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.047807999999999996,0.346);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,-0.21244799999999997,0);else expect(false);
            check_point(actual.denominator_origin,-0.07777799999999996,0.5489499999999999);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.26859999999999995,0.5489499999999999);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==-0.31);expect(actual.denominator_wrap_width==-0.48);
        }
        { // Paired Source observation 85: ruled, inline, left/right, delims=true
            FractionInput in{make_box(0.46, 0.31, 0.08, 0.72, 0.5975999999999999, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1.23, 1.4022, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0.06,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(1.896662, 0.575256, 0.7706599999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.123904,0.39);
            check_point(actual.denominator_origin,0.21217600000000006,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0);expect(actual.rule->y==0.22);expect(actual.rule->width==1.896662);expect(actual.rule->height==0.06);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==1.6966620000000001);else expect(false);
            expect(actual.numerator_wrap_width==1.6966620000000001);expect(actual.denominator_wrap_width==1.6966620000000001);
        }
        { // Paired Source observation 86: atop, inline, left/right, delims=true
            FractionInput in{make_box(0.46, 0.31, 0.08, 0.72, 0.5975999999999999, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1.23, 1.4022, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(1.6966620000000001, 0.705256, 0.7706599999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.023903999999999998,0.52);
            check_point(actual.denominator_origin,0.11217600000000005,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==1.6966620000000001);else expect(false);
            expect(actual.numerator_wrap_width==1.6966620000000001);expect(actual.denominator_wrap_width==1.6966620000000001);
        }
        { // Paired Source observation 87: bevelled, inline, left/right, delims=true
            FractionInput in{make_box(0.46, 0.31, 0.08, 0.72, 0.5975999999999999, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1.23, 1.4022, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0.06,FractionAlignment::left,FractionAlignment::right};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==0.792);expect(request.delta==0.15);
            expect(request.numerator_y==0.31720000000000004);expect(request.denominator_y==0.223);expect(request.display==false);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(0.46, 0.31, 0.08, 0.72, 0.5975999999999999, 0.04, 0.02));check_box(request.denominator,make_box(1.08, 0.1, 0.3, 1.23, 1.4022, 0.08, 0.05));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(2.276014, 0.6916, 0.19765999999999997, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.023903999999999998,0.31720000000000004);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.23575199999999996,0);else expect(false);
            check_point(actual.denominator_origin,0.6929279999999999,0.223);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.579352,0.223);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==0.46);expect(actual.denominator_wrap_width==1.08);
        }
        { // Paired Source observation 88: ruled, inline, left/right, delims=true
            FractionInput in{make_box(1.35, 1.05, 0.24, 0.72, 0.5975999999999999, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1.23, 1.4022, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0.17,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(1.054568, 1.275904, 1.9344859999999997, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.03426400000000002,0.648424);
            check_point(actual.denominator_origin,0.2674899999999999,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0);expect(actual.rule->y==0.16499999999999998);expect(actual.rule->width==1.054568);expect(actual.rule->height==0.17);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0.854568);else expect(false);
            expect(actual.numerator_wrap_width==0.854568);expect(actual.denominator_wrap_width==0.854568);
        }
        { // Paired Source observation 89: atop, inline, left/right, delims=true
            FractionInput in{make_box(1.35, 1.05, 0.24, 0.72, 0.5975999999999999, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1.23, 1.4022, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0.8545679999999999, 1.1474799999999998, 1.9344859999999997, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.06573599999999999,0.52);
            check_point(actual.denominator_origin,0.16748999999999992,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0.8545679999999999);else expect(false);
            expect(actual.numerator_wrap_width==0.8545679999999999);expect(actual.denominator_wrap_width==0.8545679999999999);
        }
        { // Paired Source observation 90: bevelled, inline, left/right, delims=true
            FractionInput in{make_box(1.35, 1.05, 0.24, 0.72, 0.5975999999999999, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1.23, 1.4022, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0.06,FractionAlignment::left,FractionAlignment::right};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==1.9727999999999999);expect(request.delta==0.15);
            expect(request.numerator_y==0.10839999999999997);expect(request.denominator_y==0.6535);expect(request.display==false);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(1.35, 1.05, 0.24, 0.72, 0.5975999999999999, -0.11, 0.19));check_box(request.denominator,make_box(0.51, 0.23, 1.13, 1.23, 1.4022, 0.03, -0.02));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(1.852312, 0.9760059999999999, 0.9309859999999996, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.06573599999999999,0.10839999999999997);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.7795679999999999,0);else expect(false);
            check_point(actual.denominator_origin,1.166634,0.6535);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,1.123168,0.6535);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==1.35);expect(actual.denominator_wrap_width==0.51);
        }
        { // Paired Source observation 91: ruled, inline, left/right, delims=true
            FractionInput in{make_box(0.22, 0.16, -0.39, 0.72, 0.5975999999999999, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1.23, 1.4022, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0.025,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(0.6627259999999999, 0.485616, 0.532286, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.201592,0.39);
            check_point(actual.denominator_origin,0.015867999999999965,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0);expect(actual.rule->y==0.2375);expect(actual.rule->width==0.6627259999999999);expect(actual.rule->height==0.025);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0.4627259999999999);else expect(false);
            expect(actual.numerator_wrap_width==0.4627259999999999);expect(actual.denominator_wrap_width==0.4627259999999999);
        }
        { // Paired Source observation 92: atop, inline, left/right, delims=true
            FractionInput in{make_box(0.22, 0.16, -0.39, 0.72, 0.5975999999999999, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1.23, 1.4022, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0.4627259999999999, 0.615616, 0.532286, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.10159199999999999,0.52);
            check_point(actual.denominator_origin,-0.08413200000000004,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0.4627259999999999);else expect(false);
            expect(actual.numerator_wrap_width==0.4627259999999999);expect(actual.denominator_wrap_width==0.4627259999999999);
        }
        { // Paired Source observation 93: bevelled, inline, left/right, delims=true
            FractionInput in{make_box(0.22, 0.16, -0.39, 0.72, 0.5975999999999999, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1.23, 1.4022, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0.06,FractionAlignment::left,FractionAlignment::right};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==0.1344);expect(request.delta==0.15);
            expect(request.numerator_y==0.20199999999999999);expect(request.denominator_y==0.42594999999999994);expect(request.display==false);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(0.22, 0.16, -0.39, 0.72, 0.5975999999999999, 0.17, -0.03));check_box(request.denominator,make_box(0.18, -0.4, 0.13, 1.23, 1.4022, -0.06, 0.21));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(0.9464619999999998, 0.6916, 0.0728, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.10159199999999999,0.20199999999999999);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.14013599999999998,0);else expect(false);
            check_point(actual.denominator_origin,0.4010039999999999,0.42594999999999994);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.4837359999999999,0.42594999999999994);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==0.22);expect(actual.denominator_wrap_width==0.18);
        }
        { // Paired Source observation 94: ruled, inline, left/right, delims=true
            FractionInput in{make_box(-0.31, 0.27, 0.12, 0.72, 0.5975999999999999, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1.23, 1.4022, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,-0.03,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(0.2, 0.5513520000000001, 1.065122, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.05219200000000001,0.39);
            check_point(actual.denominator_origin,0.691696,-0.35000000000000003);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0);expect(actual.rule->y==0.265);expect(actual.rule->width==0.06255200000000002);expect(actual.rule->height==-0.03);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0);else expect(false);
            expect(actual.numerator_wrap_width==0);expect(actual.denominator_wrap_width==0);
        }
        { // Paired Source observation 95: atop, inline, left/right, delims=true
            FractionInput in{make_box(-0.31, 0.27, 0.12, 0.72, 0.5975999999999999, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1.23, 1.4022, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0, 0.681352, 1.065122, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.047807999999999996,0.52);
            check_point(actual.denominator_origin,0.591696,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0);else expect(false);
            expect(actual.numerator_wrap_width==0);expect(actual.denominator_wrap_width==0);
        }
        { // Paired Source observation 96: bevelled, inline, left/right, delims=true
            FractionInput in{make_box(-0.31, 0.27, 0.12, 0.72, 0.5975999999999999, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1.23, 1.4022, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0.06,FractionAlignment::left,FractionAlignment::right};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==0.6567000000000001);expect(request.delta==0.15);
            expect(request.numerator_y==0.346);expect(request.denominator_y==0.5489499999999999);expect(request.display==false);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(-0.31, 0.27, 0.12, 0.72, 0.5975999999999999, -0.08, 0.16));check_box(request.denominator,make_box(-0.48, -0.22, 0.51, 1.23, 1.4022, -0.15, -0.04));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(0.34359999999999996, 0.6916, 0.166172, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.047807999999999996,0.346);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,-0.21244799999999997,0);else expect(false);
            check_point(actual.denominator_origin,-0.07777799999999996,0.5489499999999999);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.26859999999999995,0.5489499999999999);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==-0.31);expect(actual.denominator_wrap_width==-0.48);
        }
        { // Paired Source observation 97: ruled, inline, center/left, delims=false
            FractionInput in{make_box(0.46, 0.31, 0.08, 0.72, 0.5975999999999999, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1.23, 1.4022, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0.06,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(2.1366620000000003, 0.575256, 0.7706599999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.930883,0.39);
            check_point(actual.denominator_origin,0.332176,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0.12);expect(actual.rule->y==0.22);expect(actual.rule->width==1.896662);expect(actual.rule->height==0.06);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==1.6966620000000003);else expect(false);
            expect(actual.numerator_wrap_width==1.6966620000000003);expect(actual.denominator_wrap_width==1.6966620000000003);
        }
        { // Paired Source observation 98: atop, inline, center/left, delims=false
            FractionInput in{make_box(0.46, 0.31, 0.08, 0.72, 0.5975999999999999, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1.23, 1.4022, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(1.936662, 0.705256, 0.7706599999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.830883,0.52);
            check_point(actual.denominator_origin,0.232176,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==1.6966620000000001);else expect(false);
            expect(actual.numerator_wrap_width==1.6966620000000001);expect(actual.denominator_wrap_width==1.6966620000000001);
        }
        { // Paired Source observation 99: bevelled, inline, center/left, delims=false
            FractionInput in{make_box(0.46, 0.31, 0.08, 0.72, 0.5975999999999999, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1.23, 1.4022, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0.06,FractionAlignment::center,FractionAlignment::left};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==0.792);expect(request.delta==0.15);
            expect(request.numerator_y==0.31720000000000004);expect(request.denominator_y==0.223);expect(request.display==false);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(0.46, 0.31, 0.08, 0.72, 0.5975999999999999, 0.04, 0.02));check_box(request.denominator,make_box(1.08, 0.1, 0.3, 1.23, 1.4022, 0.08, 0.05));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(2.276014, 0.6916, 0.19765999999999997, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.023903999999999998,0.31720000000000004);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.23575199999999996,0);else expect(false);
            check_point(actual.denominator_origin,0.6929279999999999,0.223);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.579352,0.223);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==0.46);expect(actual.denominator_wrap_width==1.08);
        }
        { // Paired Source observation 100: ruled, inline, center/left, delims=false
            FractionInput in{make_box(1.35, 1.05, 0.24, 0.72, 0.5975999999999999, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1.23, 1.4022, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0.17,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(1.294568, 1.275904, 1.9344859999999997, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.24390399999999998,0.648424);
            check_point(actual.denominator_origin,0.262066,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0.12);expect(actual.rule->y==0.16499999999999998);expect(actual.rule->width==1.054568);expect(actual.rule->height==0.17);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0.854568);else expect(false);
            expect(actual.numerator_wrap_width==0.854568);expect(actual.denominator_wrap_width==0.854568);
        }
        { // Paired Source observation 101: atop, inline, center/left, delims=false
            FractionInput in{make_box(1.35, 1.05, 0.24, 0.72, 0.5975999999999999, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1.23, 1.4022, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(1.0945679999999998, 1.1474799999999998, 1.9344859999999997, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.14390399999999998,0.52);
            check_point(actual.denominator_origin,0.162066,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0.8545679999999998);else expect(false);
            expect(actual.numerator_wrap_width==0.8545679999999998);expect(actual.denominator_wrap_width==0.8545679999999998);
        }
        { // Paired Source observation 102: bevelled, inline, center/left, delims=false
            FractionInput in{make_box(1.35, 1.05, 0.24, 0.72, 0.5975999999999999, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1.23, 1.4022, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0.06,FractionAlignment::center,FractionAlignment::left};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==1.9727999999999999);expect(request.delta==0.15);
            expect(request.numerator_y==0.10839999999999997);expect(request.denominator_y==0.6535);expect(request.display==false);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(1.35, 1.05, 0.24, 0.72, 0.5975999999999999, -0.11, 0.19));check_box(request.denominator,make_box(0.51, 0.23, 1.13, 1.23, 1.4022, 0.03, -0.02));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(1.852312, 0.9760059999999999, 0.9309859999999996, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.06573599999999999,0.10839999999999997);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.7795679999999999,0);else expect(false);
            check_point(actual.denominator_origin,1.166634,0.6535);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,1.123168,0.6535);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==1.35);expect(actual.denominator_wrap_width==0.51);
        }
        { // Paired Source observation 103: ruled, inline, center/left, delims=false
            FractionInput in{make_box(0.22, 0.16, -0.39, 0.72, 0.5975999999999999, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1.23, 1.4022, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0.025,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(0.9027259999999999, 0.485616, 0.532286, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.38562699999999994,0.39);
            check_point(actual.denominator_origin,0.13586800000000002,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0.12);expect(actual.rule->y==0.2375);expect(actual.rule->width==0.6627259999999999);expect(actual.rule->height==0.025);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0.4627259999999999);else expect(false);
            expect(actual.numerator_wrap_width==0.46272599999999997);expect(actual.denominator_wrap_width==0.46272599999999997);
        }
        { // Paired Source observation 104: atop, inline, center/left, delims=false
            FractionInput in{make_box(0.22, 0.16, -0.39, 0.72, 0.5975999999999999, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1.23, 1.4022, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0.702726, 0.615616, 0.532286, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.28562699999999996,0.52);
            check_point(actual.denominator_origin,0.03586800000000001,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0.46272599999999997);else expect(false);
            expect(actual.numerator_wrap_width==0.46272599999999997);expect(actual.denominator_wrap_width==0.46272599999999997);
        }
        { // Paired Source observation 105: bevelled, inline, center/left, delims=false
            FractionInput in{make_box(0.22, 0.16, -0.39, 0.72, 0.5975999999999999, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1.23, 1.4022, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0.06,FractionAlignment::center,FractionAlignment::left};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==0.1344);expect(request.delta==0.15);
            expect(request.numerator_y==0.20199999999999999);expect(request.denominator_y==0.42594999999999994);expect(request.display==false);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(0.22, 0.16, -0.39, 0.72, 0.5975999999999999, 0.17, -0.03));check_box(request.denominator,make_box(0.18, -0.4, 0.13, 1.23, 1.4022, -0.06, 0.21));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(0.9464619999999998, 0.6916, 0.0728, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.10159199999999999,0.20199999999999999);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.14013599999999998,0);else expect(false);
            check_point(actual.denominator_origin,0.4010039999999999,0.42594999999999994);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.4837359999999999,0.42594999999999994);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==0.22);expect(actual.denominator_wrap_width==0.18);
        }
        { // Paired Source observation 106: ruled, inline, center/left, delims=false
            FractionInput in{make_box(-0.31, 0.27, 0.12, 0.72, 0.5975999999999999, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1.23, 1.4022, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,-0.03,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(0.44, 0.5513520000000001, 1.065122, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.243904,0.39);
            check_point(actual.denominator_origin,0.009670000000000012,-0.35000000000000003);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0.12);expect(actual.rule->y==0.265);expect(actual.rule->width==0.06255200000000002);expect(actual.rule->height==-0.03);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0);else expect(false);
            expect(actual.numerator_wrap_width==0);expect(actual.denominator_wrap_width==0);
        }
        { // Paired Source observation 107: atop, inline, center/left, delims=false
            FractionInput in{make_box(-0.31, 0.27, 0.12, 0.72, 0.5975999999999999, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1.23, 1.4022, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0.24, 0.681352, 1.065122, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.14390399999999998,0.52);
            check_point(actual.denominator_origin,-0.09033,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0);else expect(false);
            expect(actual.numerator_wrap_width==0);expect(actual.denominator_wrap_width==0);
        }
        { // Paired Source observation 108: bevelled, inline, center/left, delims=false
            FractionInput in{make_box(-0.31, 0.27, 0.12, 0.72, 0.5975999999999999, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1.23, 1.4022, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0.06,FractionAlignment::center,FractionAlignment::left};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==0.6567000000000001);expect(request.delta==0.15);
            expect(request.numerator_y==0.346);expect(request.denominator_y==0.5489499999999999);expect(request.display==false);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(-0.31, 0.27, 0.12, 0.72, 0.5975999999999999, -0.08, 0.16));check_box(request.denominator,make_box(-0.48, -0.22, 0.51, 1.23, 1.4022, -0.15, -0.04));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(0.34359999999999996, 0.6916, 0.166172, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.047807999999999996,0.346);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,-0.21244799999999997,0);else expect(false);
            check_point(actual.denominator_origin,-0.07777799999999996,0.5489499999999999);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.26859999999999995,0.5489499999999999);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==-0.31);expect(actual.denominator_wrap_width==-0.48);
        }
        { // Paired Source observation 109: ruled, inline, center/left, delims=true
            FractionInput in{make_box(0.46, 0.31, 0.08, 0.72, 0.5975999999999999, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1.23, 1.4022, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0.06,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(1.896662, 0.575256, 0.7706599999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.810883,0.39);
            check_point(actual.denominator_origin,0.212176,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0);expect(actual.rule->y==0.22);expect(actual.rule->width==1.896662);expect(actual.rule->height==0.06);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==1.6966620000000001);else expect(false);
            expect(actual.numerator_wrap_width==1.6966620000000001);expect(actual.denominator_wrap_width==1.6966620000000001);
        }
        { // Paired Source observation 110: atop, inline, center/left, delims=true
            FractionInput in{make_box(0.46, 0.31, 0.08, 0.72, 0.5975999999999999, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1.23, 1.4022, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(1.6966620000000001, 0.705256, 0.7706599999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.710883,0.52);
            check_point(actual.denominator_origin,0.112176,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==1.6966620000000001);else expect(false);
            expect(actual.numerator_wrap_width==1.6966620000000001);expect(actual.denominator_wrap_width==1.6966620000000001);
        }
        { // Paired Source observation 111: bevelled, inline, center/left, delims=true
            FractionInput in{make_box(0.46, 0.31, 0.08, 0.72, 0.5975999999999999, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1.23, 1.4022, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0.06,FractionAlignment::center,FractionAlignment::left};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==0.792);expect(request.delta==0.15);
            expect(request.numerator_y==0.31720000000000004);expect(request.denominator_y==0.223);expect(request.display==false);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(0.46, 0.31, 0.08, 0.72, 0.5975999999999999, 0.04, 0.02));check_box(request.denominator,make_box(1.08, 0.1, 0.3, 1.23, 1.4022, 0.08, 0.05));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(2.276014, 0.6916, 0.19765999999999997, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.023903999999999998,0.31720000000000004);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.23575199999999996,0);else expect(false);
            check_point(actual.denominator_origin,0.6929279999999999,0.223);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.579352,0.223);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==0.46);expect(actual.denominator_wrap_width==1.08);
        }
        { // Paired Source observation 112: ruled, inline, center/left, delims=true
            FractionInput in{make_box(1.35, 1.05, 0.24, 0.72, 0.5975999999999999, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1.23, 1.4022, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0.17,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(1.054568, 1.275904, 1.9344859999999997, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.12390399999999999,0.648424);
            check_point(actual.denominator_origin,0.142066,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0);expect(actual.rule->y==0.16499999999999998);expect(actual.rule->width==1.054568);expect(actual.rule->height==0.17);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0.854568);else expect(false);
            expect(actual.numerator_wrap_width==0.854568);expect(actual.denominator_wrap_width==0.854568);
        }
        { // Paired Source observation 113: atop, inline, center/left, delims=true
            FractionInput in{make_box(1.35, 1.05, 0.24, 0.72, 0.5975999999999999, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1.23, 1.4022, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0.8545679999999999, 1.1474799999999998, 1.9344859999999997, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.02390399999999998,0.52);
            check_point(actual.denominator_origin,0.04206599999999999,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0.8545679999999999);else expect(false);
            expect(actual.numerator_wrap_width==0.8545679999999999);expect(actual.denominator_wrap_width==0.8545679999999999);
        }
        { // Paired Source observation 114: bevelled, inline, center/left, delims=true
            FractionInput in{make_box(1.35, 1.05, 0.24, 0.72, 0.5975999999999999, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1.23, 1.4022, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0.06,FractionAlignment::center,FractionAlignment::left};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==1.9727999999999999);expect(request.delta==0.15);
            expect(request.numerator_y==0.10839999999999997);expect(request.denominator_y==0.6535);expect(request.display==false);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(1.35, 1.05, 0.24, 0.72, 0.5975999999999999, -0.11, 0.19));check_box(request.denominator,make_box(0.51, 0.23, 1.13, 1.23, 1.4022, 0.03, -0.02));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(1.852312, 0.9760059999999999, 0.9309859999999996, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.06573599999999999,0.10839999999999997);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.7795679999999999,0);else expect(false);
            check_point(actual.denominator_origin,1.166634,0.6535);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,1.123168,0.6535);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==1.35);expect(actual.denominator_wrap_width==0.51);
        }
        { // Paired Source observation 115: ruled, inline, center/left, delims=true
            FractionInput in{make_box(0.22, 0.16, -0.39, 0.72, 0.5975999999999999, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1.23, 1.4022, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0.025,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(0.6627259999999999, 0.485616, 0.532286, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.26562699999999995,0.39);
            check_point(actual.denominator_origin,0.01586800000000002,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0);expect(actual.rule->y==0.2375);expect(actual.rule->width==0.6627259999999999);expect(actual.rule->height==0.025);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0.4627259999999999);else expect(false);
            expect(actual.numerator_wrap_width==0.4627259999999999);expect(actual.denominator_wrap_width==0.4627259999999999);
        }
        { // Paired Source observation 116: atop, inline, center/left, delims=true
            FractionInput in{make_box(0.22, 0.16, -0.39, 0.72, 0.5975999999999999, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1.23, 1.4022, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0.4627259999999999, 0.615616, 0.532286, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.16562699999999997,0.52);
            check_point(actual.denominator_origin,-0.08413199999999998,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0.4627259999999999);else expect(false);
            expect(actual.numerator_wrap_width==0.4627259999999999);expect(actual.denominator_wrap_width==0.4627259999999999);
        }
        { // Paired Source observation 117: bevelled, inline, center/left, delims=true
            FractionInput in{make_box(0.22, 0.16, -0.39, 0.72, 0.5975999999999999, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1.23, 1.4022, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0.06,FractionAlignment::center,FractionAlignment::left};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==0.1344);expect(request.delta==0.15);
            expect(request.numerator_y==0.20199999999999999);expect(request.denominator_y==0.42594999999999994);expect(request.display==false);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(0.22, 0.16, -0.39, 0.72, 0.5975999999999999, 0.17, -0.03));check_box(request.denominator,make_box(0.18, -0.4, 0.13, 1.23, 1.4022, -0.06, 0.21));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(0.9464619999999998, 0.6916, 0.0728, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.10159199999999999,0.20199999999999999);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.14013599999999998,0);else expect(false);
            check_point(actual.denominator_origin,0.4010039999999999,0.42594999999999994);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.4837359999999999,0.42594999999999994);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==0.22);expect(actual.denominator_wrap_width==0.18);
        }
        { // Paired Source observation 118: ruled, inline, center/left, delims=true
            FractionInput in{make_box(-0.31, 0.27, 0.12, 0.72, 0.5975999999999999, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1.23, 1.4022, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,-0.03,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(0.2, 0.5513520000000001, 1.065122, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.123904,0.39);
            check_point(actual.denominator_origin,-0.11032999999999998,-0.35000000000000003);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0);expect(actual.rule->y==0.265);expect(actual.rule->width==0.06255200000000002);expect(actual.rule->height==-0.03);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0);else expect(false);
            expect(actual.numerator_wrap_width==0);expect(actual.denominator_wrap_width==0);
        }
        { // Paired Source observation 119: atop, inline, center/left, delims=true
            FractionInput in{make_box(-0.31, 0.27, 0.12, 0.72, 0.5975999999999999, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1.23, 1.4022, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0, 0.681352, 1.065122, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.023903999999999995,0.52);
            check_point(actual.denominator_origin,-0.21033,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0);else expect(false);
            expect(actual.numerator_wrap_width==0);expect(actual.denominator_wrap_width==0);
        }
        { // Paired Source observation 120: bevelled, inline, center/left, delims=true
            FractionInput in{make_box(-0.31, 0.27, 0.12, 0.72, 0.5975999999999999, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1.23, 1.4022, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0.06,FractionAlignment::center,FractionAlignment::left};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==0.6567000000000001);expect(request.delta==0.15);
            expect(request.numerator_y==0.346);expect(request.denominator_y==0.5489499999999999);expect(request.display==false);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(-0.31, 0.27, 0.12, 0.72, 0.5975999999999999, -0.08, 0.16));check_box(request.denominator,make_box(-0.48, -0.22, 0.51, 1.23, 1.4022, -0.15, -0.04));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(0.34359999999999996, 0.6916, 0.166172, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.047807999999999996,0.346);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,-0.21244799999999997,0);else expect(false);
            check_point(actual.denominator_origin,-0.07777799999999996,0.5489499999999999);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.26859999999999995,0.5489499999999999);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==-0.31);expect(actual.denominator_wrap_width==-0.48);
        }
        { // Paired Source observation 121: ruled, inline, right/center, delims=false
            FractionInput in{make_box(0.46, 0.31, 0.08, 0.72, 0.5975999999999999, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1.23, 1.4022, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0.06,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(2.1366620000000003, 0.575256, 0.7706599999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,1.629814,0.39);
            check_point(actual.denominator_origin,0.31114300000000006,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0.12);expect(actual.rule->y==0.22);expect(actual.rule->width==1.896662);expect(actual.rule->height==0.06);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==1.6966620000000003);else expect(false);
            expect(actual.numerator_wrap_width==1.6966620000000003);expect(actual.denominator_wrap_width==1.6966620000000003);
        }
        { // Paired Source observation 122: atop, inline, right/center, delims=false
            FractionInput in{make_box(0.46, 0.31, 0.08, 0.72, 0.5975999999999999, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1.23, 1.4022, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(1.936662, 0.705256, 0.7706599999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,1.529814,0.52);
            check_point(actual.denominator_origin,0.21114300000000008,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==1.6966620000000001);else expect(false);
            expect(actual.numerator_wrap_width==1.6966620000000001);expect(actual.denominator_wrap_width==1.6966620000000001);
        }
        { // Paired Source observation 123: bevelled, inline, right/center, delims=false
            FractionInput in{make_box(0.46, 0.31, 0.08, 0.72, 0.5975999999999999, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1.23, 1.4022, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0.06,FractionAlignment::right,FractionAlignment::center};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==0.792);expect(request.delta==0.15);
            expect(request.numerator_y==0.31720000000000004);expect(request.denominator_y==0.223);expect(request.display==false);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(0.46, 0.31, 0.08, 0.72, 0.5975999999999999, 0.04, 0.02));check_box(request.denominator,make_box(1.08, 0.1, 0.3, 1.23, 1.4022, 0.08, 0.05));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(2.276014, 0.6916, 0.19765999999999997, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.023903999999999998,0.31720000000000004);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.23575199999999996,0);else expect(false);
            check_point(actual.denominator_origin,0.6929279999999999,0.223);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.579352,0.223);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==0.46);expect(actual.denominator_wrap_width==1.08);
        }
        { // Paired Source observation 124: ruled, inline, right/center, delims=false
            FractionInput in{make_box(1.35, 1.05, 0.24, 0.72, 0.5975999999999999, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1.23, 1.4022, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0.17,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(1.294568, 1.275904, 1.9344859999999997, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.15426399999999998,0.648424);
            check_point(actual.denominator_origin,0.28972299999999995,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0.12);expect(actual.rule->y==0.16499999999999998);expect(actual.rule->width==1.054568);expect(actual.rule->height==0.17);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0.854568);else expect(false);
            expect(actual.numerator_wrap_width==0.854568);expect(actual.denominator_wrap_width==0.854568);
        }
        { // Paired Source observation 125: atop, inline, right/center, delims=false
            FractionInput in{make_box(1.35, 1.05, 0.24, 0.72, 0.5975999999999999, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1.23, 1.4022, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(1.0945679999999998, 1.1474799999999998, 1.9344859999999997, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.05426399999999998,0.52);
            check_point(actual.denominator_origin,0.18972299999999997,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0.8545679999999998);else expect(false);
            expect(actual.numerator_wrap_width==0.8545679999999998);expect(actual.denominator_wrap_width==0.8545679999999998);
        }
        { // Paired Source observation 126: bevelled, inline, right/center, delims=false
            FractionInput in{make_box(1.35, 1.05, 0.24, 0.72, 0.5975999999999999, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1.23, 1.4022, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0.06,FractionAlignment::right,FractionAlignment::center};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==1.9727999999999999);expect(request.delta==0.15);
            expect(request.numerator_y==0.10839999999999997);expect(request.denominator_y==0.6535);expect(request.display==false);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(1.35, 1.05, 0.24, 0.72, 0.5975999999999999, -0.11, 0.19));check_box(request.denominator,make_box(0.51, 0.23, 1.13, 1.23, 1.4022, 0.03, -0.02));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(1.852312, 0.9760059999999999, 0.9309859999999996, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.06573599999999999,0.10839999999999997);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.7795679999999999,0);else expect(false);
            check_point(actual.denominator_origin,1.166634,0.6535);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,1.123168,0.6535);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==1.35);expect(actual.denominator_wrap_width==0.51);
        }
        { // Paired Source observation 127: ruled, inline, right/center, delims=false
            FractionInput in{make_box(0.22, 0.16, -0.39, 0.72, 0.5975999999999999, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1.23, 1.4022, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0.025,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(0.9027259999999999, 0.485616, 0.532286, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.569182,0.39);
            check_point(actual.denominator_origin,0.325165,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0.12);expect(actual.rule->y==0.2375);expect(actual.rule->width==0.6627259999999999);expect(actual.rule->height==0.025);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0.4627259999999999);else expect(false);
            expect(actual.numerator_wrap_width==0.46272599999999997);expect(actual.denominator_wrap_width==0.46272599999999997);
        }
        { // Paired Source observation 128: atop, inline, right/center, delims=false
            FractionInput in{make_box(0.22, 0.16, -0.39, 0.72, 0.5975999999999999, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1.23, 1.4022, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0.702726, 0.615616, 0.532286, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.46918199999999993,0.52);
            check_point(actual.denominator_origin,0.22516499999999998,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0.46272599999999997);else expect(false);
            expect(actual.numerator_wrap_width==0.46272599999999997);expect(actual.denominator_wrap_width==0.46272599999999997);
        }
        { // Paired Source observation 129: bevelled, inline, right/center, delims=false
            FractionInput in{make_box(0.22, 0.16, -0.39, 0.72, 0.5975999999999999, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1.23, 1.4022, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0.06,FractionAlignment::right,FractionAlignment::center};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==0.1344);expect(request.delta==0.15);
            expect(request.numerator_y==0.20199999999999999);expect(request.denominator_y==0.42594999999999994);expect(request.display==false);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(0.22, 0.16, -0.39, 0.72, 0.5975999999999999, 0.17, -0.03));check_box(request.denominator,make_box(0.18, -0.4, 0.13, 1.23, 1.4022, -0.06, 0.21));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(0.9464619999999998, 0.6916, 0.0728, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.10159199999999999,0.20199999999999999);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.14013599999999998,0);else expect(false);
            check_point(actual.denominator_origin,0.4010039999999999,0.42594999999999994);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.4837359999999999,0.42594999999999994);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==0.22);expect(actual.denominator_wrap_width==0.18);
        }
        { // Paired Source observation 130: ruled, inline, right/center, delims=false
            FractionInput in{make_box(-0.31, 0.27, 0.12, 0.72, 0.5975999999999999, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1.23, 1.4022, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,-0.03,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(0.44, 0.5513520000000001, 1.065122, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.172192,0.39);
            check_point(actual.denominator_origin,0.4878039999999999,-0.35000000000000003);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0.12);expect(actual.rule->y==0.265);expect(actual.rule->width==0.06255200000000002);expect(actual.rule->height==-0.03);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0);else expect(false);
            expect(actual.numerator_wrap_width==0);expect(actual.denominator_wrap_width==0);
        }
        { // Paired Source observation 131: atop, inline, right/center, delims=false
            FractionInput in{make_box(-0.31, 0.27, 0.12, 0.72, 0.5975999999999999, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1.23, 1.4022, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0.24, 0.681352, 1.065122, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.07219199999999999,0.52);
            check_point(actual.denominator_origin,0.3878039999999999,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0);else expect(false);
            expect(actual.numerator_wrap_width==0);expect(actual.denominator_wrap_width==0);
        }
        { // Paired Source observation 132: bevelled, inline, right/center, delims=false
            FractionInput in{make_box(-0.31, 0.27, 0.12, 0.72, 0.5975999999999999, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1.23, 1.4022, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,false,0.06,FractionAlignment::right,FractionAlignment::center};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==0.6567000000000001);expect(request.delta==0.15);
            expect(request.numerator_y==0.346);expect(request.denominator_y==0.5489499999999999);expect(request.display==false);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(-0.31, 0.27, 0.12, 0.72, 0.5975999999999999, -0.08, 0.16));check_box(request.denominator,make_box(-0.48, -0.22, 0.51, 1.23, 1.4022, -0.15, -0.04));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(0.34359999999999996, 0.6916, 0.166172, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.047807999999999996,0.346);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,-0.21244799999999997,0);else expect(false);
            check_point(actual.denominator_origin,-0.07777799999999996,0.5489499999999999);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.26859999999999995,0.5489499999999999);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==-0.31);expect(actual.denominator_wrap_width==-0.48);
        }
        { // Paired Source observation 133: ruled, inline, right/center, delims=true
            FractionInput in{make_box(0.46, 0.31, 0.08, 0.72, 0.5975999999999999, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1.23, 1.4022, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0.06,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(1.896662, 0.575256, 0.7706599999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,1.5098140000000002,0.39);
            check_point(actual.denominator_origin,0.1911430000000001,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0);expect(actual.rule->y==0.22);expect(actual.rule->width==1.896662);expect(actual.rule->height==0.06);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==1.6966620000000001);else expect(false);
            expect(actual.numerator_wrap_width==1.6966620000000001);expect(actual.denominator_wrap_width==1.6966620000000001);
        }
        { // Paired Source observation 134: atop, inline, right/center, delims=true
            FractionInput in{make_box(0.46, 0.31, 0.08, 0.72, 0.5975999999999999, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1.23, 1.4022, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(1.6966620000000001, 0.705256, 0.7706599999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,1.4098140000000001,0.52);
            check_point(actual.denominator_origin,0.09114300000000009,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==1.6966620000000001);else expect(false);
            expect(actual.numerator_wrap_width==1.6966620000000001);expect(actual.denominator_wrap_width==1.6966620000000001);
        }
        { // Paired Source observation 135: bevelled, inline, right/center, delims=true
            FractionInput in{make_box(0.46, 0.31, 0.08, 0.72, 0.5975999999999999, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1.23, 1.4022, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0.06,FractionAlignment::right,FractionAlignment::center};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==0.792);expect(request.delta==0.15);
            expect(request.numerator_y==0.31720000000000004);expect(request.denominator_y==0.223);expect(request.display==false);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(0.46, 0.31, 0.08, 0.72, 0.5975999999999999, 0.04, 0.02));check_box(request.denominator,make_box(1.08, 0.1, 0.3, 1.23, 1.4022, 0.08, 0.05));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(2.276014, 0.6916, 0.19765999999999997, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.023903999999999998,0.31720000000000004);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.23575199999999996,0);else expect(false);
            check_point(actual.denominator_origin,0.6929279999999999,0.223);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.579352,0.223);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==0.46);expect(actual.denominator_wrap_width==1.08);
        }
        { // Paired Source observation 136: ruled, inline, right/center, delims=true
            FractionInput in{make_box(1.35, 1.05, 0.24, 0.72, 0.5975999999999999, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1.23, 1.4022, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0.17,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(1.054568, 1.275904, 1.9344859999999997, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.03426399999999999,0.648424);
            check_point(actual.denominator_origin,0.16972299999999998,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0);expect(actual.rule->y==0.16499999999999998);expect(actual.rule->width==1.054568);expect(actual.rule->height==0.17);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0.854568);else expect(false);
            expect(actual.numerator_wrap_width==0.854568);expect(actual.denominator_wrap_width==0.854568);
        }
        { // Paired Source observation 137: atop, inline, right/center, delims=true
            FractionInput in{make_box(1.35, 1.05, 0.24, 0.72, 0.5975999999999999, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1.23, 1.4022, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0.8545679999999999, 1.1474799999999998, 1.9344859999999997, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.06573600000000002,0.52);
            check_point(actual.denominator_origin,0.06972299999999998,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0.8545679999999999);else expect(false);
            expect(actual.numerator_wrap_width==0.8545679999999999);expect(actual.denominator_wrap_width==0.8545679999999999);
        }
        { // Paired Source observation 138: bevelled, inline, right/center, delims=true
            FractionInput in{make_box(1.35, 1.05, 0.24, 0.72, 0.5975999999999999, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1.23, 1.4022, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0.06,FractionAlignment::right,FractionAlignment::center};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==1.9727999999999999);expect(request.delta==0.15);
            expect(request.numerator_y==0.10839999999999997);expect(request.denominator_y==0.6535);expect(request.display==false);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(1.35, 1.05, 0.24, 0.72, 0.5975999999999999, -0.11, 0.19));check_box(request.denominator,make_box(0.51, 0.23, 1.13, 1.23, 1.4022, 0.03, -0.02));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(1.852312, 0.9760059999999999, 0.9309859999999996, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.06573599999999999,0.10839999999999997);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.7795679999999999,0);else expect(false);
            check_point(actual.denominator_origin,1.166634,0.6535);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,1.123168,0.6535);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==1.35);expect(actual.denominator_wrap_width==0.51);
        }
        { // Paired Source observation 139: ruled, inline, right/center, delims=true
            FractionInput in{make_box(0.22, 0.16, -0.39, 0.72, 0.5975999999999999, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1.23, 1.4022, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0.025,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(0.6627259999999999, 0.485616, 0.532286, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.44918199999999997,0.39);
            check_point(actual.denominator_origin,0.205165,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0);expect(actual.rule->y==0.2375);expect(actual.rule->width==0.6627259999999999);expect(actual.rule->height==0.025);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0.4627259999999999);else expect(false);
            expect(actual.numerator_wrap_width==0.4627259999999999);expect(actual.denominator_wrap_width==0.4627259999999999);
        }
        { // Paired Source observation 140: atop, inline, right/center, delims=true
            FractionInput in{make_box(0.22, 0.16, -0.39, 0.72, 0.5975999999999999, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1.23, 1.4022, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0.4627259999999999, 0.615616, 0.532286, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.34918199999999994,0.52);
            check_point(actual.denominator_origin,0.10516499999999998,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0.4627259999999999);else expect(false);
            expect(actual.numerator_wrap_width==0.4627259999999999);expect(actual.denominator_wrap_width==0.4627259999999999);
        }
        { // Paired Source observation 141: bevelled, inline, right/center, delims=true
            FractionInput in{make_box(0.22, 0.16, -0.39, 0.72, 0.5975999999999999, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1.23, 1.4022, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0.06,FractionAlignment::right,FractionAlignment::center};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==0.1344);expect(request.delta==0.15);
            expect(request.numerator_y==0.20199999999999999);expect(request.denominator_y==0.42594999999999994);expect(request.display==false);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(0.22, 0.16, -0.39, 0.72, 0.5975999999999999, 0.17, -0.03));check_box(request.denominator,make_box(0.18, -0.4, 0.13, 1.23, 1.4022, -0.06, 0.21));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(0.9464619999999998, 0.6916, 0.0728, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.10159199999999999,0.20199999999999999);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.14013599999999998,0);else expect(false);
            check_point(actual.denominator_origin,0.4010039999999999,0.42594999999999994);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.4837359999999999,0.42594999999999994);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==0.22);expect(actual.denominator_wrap_width==0.18);
        }
        { // Paired Source observation 142: ruled, inline, right/center, delims=true
            FractionInput in{make_box(-0.31, 0.27, 0.12, 0.72, 0.5975999999999999, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1.23, 1.4022, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,-0.03,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(0.2, 0.5513520000000001, 1.065122, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.052192,0.39);
            check_point(actual.denominator_origin,0.3678039999999999,-0.35000000000000003);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0);expect(actual.rule->y==0.265);expect(actual.rule->width==0.06255200000000002);expect(actual.rule->height==-0.03);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0);else expect(false);
            expect(actual.numerator_wrap_width==0);expect(actual.denominator_wrap_width==0);
        }
        { // Paired Source observation 143: atop, inline, right/center, delims=true
            FractionInput in{make_box(-0.31, 0.27, 0.12, 0.72, 0.5975999999999999, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1.23, 1.4022, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0, 0.681352, 1.065122, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.047808,0.52);
            check_point(actual.denominator_origin,0.26780399999999993,-0.35);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0);else expect(false);
            expect(actual.numerator_wrap_width==0);expect(actual.denominator_wrap_width==0);
        }
        { // Paired Source observation 144: bevelled, inline, right/center, delims=true
            FractionInput in{make_box(-0.31, 0.27, 0.12, 0.72, 0.5975999999999999, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1.23, 1.4022, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},false,true,0.06,FractionAlignment::right,FractionAlignment::center};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==0.6567000000000001);expect(request.delta==0.15);
            expect(request.numerator_y==0.346);expect(request.denominator_y==0.5489499999999999);expect(request.display==false);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(-0.31, 0.27, 0.12, 0.72, 0.5975999999999999, -0.08, 0.16));check_box(request.denominator,make_box(-0.48, -0.22, 0.51, 1.23, 1.4022, -0.15, -0.04));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(0.34359999999999996, 0.6916, 0.166172, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.047807999999999996,0.346);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,-0.21244799999999997,0);else expect(false);
            check_point(actual.denominator_origin,-0.07777799999999996,0.5489499999999999);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.26859999999999995,0.5489499999999999);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==-0.31);expect(actual.denominator_wrap_width==-0.48);
        }
        { // Paired Source observation 145: ruled, display, left/right, delims=false
            FractionInput in{make_box(0.46, 0.31, 0.08, 1, 0.83, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1, 1.14, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0.06,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(1.8194000000000001, 0.9373, 1.032, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.2532,0.68);
            check_point(actual.denominator_origin,0.31120000000000014,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0.12);expect(actual.rule->y==0.22);expect(actual.rule->width==1.5794000000000001);expect(actual.rule->height==0.06);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==1.3794000000000002);else expect(false);
            expect(actual.numerator_wrap_width==1.3794000000000002);expect(actual.denominator_wrap_width==1.3794000000000002);
        }
        { // Paired Source observation 146: atop, display, left/right, delims=false
            FractionInput in{make_box(0.46, 0.31, 0.08, 1, 0.83, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1, 1.14, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(1.6194000000000002, 0.9373, 1.032, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.1532,0.68);
            check_point(actual.denominator_origin,0.21120000000000017,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==1.3794000000000002);else expect(false);
            expect(actual.numerator_wrap_width==1.3794000000000002);expect(actual.denominator_wrap_width==1.3794000000000002);
        }
        { // Paired Source observation 147: bevelled, display, left/right, delims=false
            FractionInput in{make_box(0.46, 0.31, 0.08, 1, 0.83, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1, 1.14, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0.06,FractionAlignment::left,FractionAlignment::right};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==1.2000000000000002);expect(request.delta==0.4);
            expect(request.numerator_y==0.535);expect(request.denominator_y==-0.050000000000000044);expect(request.display==true);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(0.46, 0.31, 0.08, 1, 0.83, 0.04, 0.02));check_box(request.denominator,make_box(1.08, 0.1, 0.3, 1, 1.14, 0.08, 0.05));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(1.8296000000000001, 0.7923, 0.392, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.0332,0.535);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.23159999999999997,0);else expect(false);
            check_point(actual.denominator_origin,0.5427999999999998,-0.050000000000000044);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.4501999999999999,-0.050000000000000044);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==0.46);expect(actual.denominator_wrap_width==1.08);
        }
        { // Paired Source observation 148: ruled, display, left/right, delims=false
            FractionInput in{make_box(1.35, 1.05, 0.24, 1, 0.83, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1, 1.14, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0.17,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(1.6268999999999998, 1.9157, 1.9781999999999997, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.1287,1.0442);
            check_point(actual.denominator_origin,0.8482999999999998,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0.12);expect(actual.rule->y==0.16499999999999998);expect(actual.rule->width==1.3868999999999998);expect(actual.rule->height==0.17);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==1.1868999999999998);else expect(false);
            expect(actual.numerator_wrap_width==1.1868999999999998);expect(actual.denominator_wrap_width==1.1868999999999998);
        }
        { // Paired Source observation 149: atop, display, left/right, delims=false
            FractionInput in{make_box(1.35, 1.05, 0.24, 1, 0.83, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1, 1.14, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(1.4268999999999998, 1.5514999999999999, 1.9781999999999997, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.028700000000000003,0.68);
            check_point(actual.denominator_origin,0.7482999999999999,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==1.1868999999999998);else expect(false);
            expect(actual.numerator_wrap_width==1.1868999999999998);expect(actual.denominator_wrap_width==1.1868999999999998);
        }
        { // Paired Source observation 150: bevelled, display, left/right, delims=false
            FractionInput in{make_box(1.35, 1.05, 0.24, 1, 0.83, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1, 1.14, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0.06,FractionAlignment::left,FractionAlignment::right};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==2.16);expect(request.delta==0.4);
            expect(request.numerator_y==0.245);expect(request.denominator_y==0.29999999999999993);expect(request.display==true);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(1.35, 1.05, 0.24, 1, 0.83, -0.11, 0.19));check_box(request.denominator,make_box(0.51, 0.23, 1.13, 1, 1.14, 0.03, -0.02));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(1.7983, 1.1164999999999998, 0.9881999999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.09129999999999999,0.245);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.9868999999999999,0);else expect(false);
            check_point(actual.denominator_origin,1.2410999999999999,0.29999999999999993);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,1.2055,0.29999999999999993);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==1.35);expect(actual.denominator_wrap_width==0.51);
        }
        { // Paired Source observation 151: ruled, display, left/right, delims=false
            FractionInput in{make_box(0.22, 0.16, -0.39, 1, 0.83, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1, 1.14, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0.025,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(0.8161999999999999, 0.8128000000000001, 0.8382, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.3611,0.68);
            check_point(actual.denominator_origin,0.15159999999999993,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0.12);expect(actual.rule->y==0.2375);expect(actual.rule->width==0.5761999999999999);expect(actual.rule->height==0.025);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0.3761999999999999);else expect(false);
            expect(actual.numerator_wrap_width==0.37619999999999987);expect(actual.denominator_wrap_width==0.37619999999999987);
        }
        { // Paired Source observation 152: atop, display, left/right, delims=false
            FractionInput in{make_box(0.22, 0.16, -0.39, 1, 0.83, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1, 1.14, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0.6161999999999999, 0.8128000000000001, 0.8382, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.2611,0.68);
            check_point(actual.denominator_origin,0.051599999999999924,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0.37619999999999987);else expect(false);
            expect(actual.numerator_wrap_width==0.37619999999999987);expect(actual.denominator_wrap_width==0.37619999999999987);
        }
        { // Paired Source observation 153: bevelled, display, left/right, delims=false
            FractionInput in{make_box(0.22, 0.16, -0.39, 1, 0.83, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1, 1.14, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0.06,FractionAlignment::left,FractionAlignment::right};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==0.5700000000000001);expect(request.delta==0.4);
            expect(request.numerator_y==0.375);expect(request.denominator_y==0.11499999999999999);expect(request.display==true);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(0.22, 0.16, -0.39, 1, 0.83, 0.17, -0.03));check_box(request.denominator,make_box(0.18, -0.4, 0.13, 1, 1.14, -0.06, 0.21));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(0.6935999999999998, 0.6916, 0.0728, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.1411,0.375);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.09879999999999994,0);else expect(false);
            check_point(actual.denominator_origin,0.25039999999999984,0.11499999999999999);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.31739999999999985,0.11499999999999999);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==0.22);expect(actual.denominator_wrap_width==0.18);
        }
        { // Paired Source observation 154: ruled, display, left/right, delims=false
            FractionInput in{make_box(-0.31, 0.27, 0.12, 1, 0.83, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1, 1.14, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,-0.03,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(0.44, 0.9041, 1.2713999999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.15360000000000001,0.68);
            check_point(actual.denominator_origin,0.6219,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0.12);expect(actual.rule->y==0.265);expect(actual.rule->width==0.009100000000000025);expect(actual.rule->height==-0.03);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0);else expect(false);
            expect(actual.numerator_wrap_width==0);expect(actual.denominator_wrap_width==0);
        }
        { // Paired Source observation 155: atop, display, left/right, delims=false
            FractionInput in{make_box(-0.31, 0.27, 0.12, 1, 0.83, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1, 1.14, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0.24, 0.9041, 1.2713999999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.053599999999999995,0.68);
            check_point(actual.denominator_origin,0.5219,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0);else expect(false);
            expect(actual.numerator_wrap_width==0);expect(actual.denominator_wrap_width==0);
        }
        { // Paired Source observation 156: bevelled, display, left/right, delims=false
            FractionInput in{make_box(-0.31, 0.27, 0.12, 1, 0.83, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1, 1.14, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0.06,FractionAlignment::left,FractionAlignment::right};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==1.19);expect(request.delta==0.4);
            expect(request.numerator_y==0.575);expect(request.denominator_y==0.21499999999999997);expect(request.display==true);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(-0.31, 0.27, 0.12, 1, 0.83, -0.08, 0.16));check_box(request.denominator,make_box(-0.48, -0.22, 0.51, 1, 1.14, -0.15, -0.04));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(0.21859999999999996, 0.7990999999999999, 0.36639999999999995, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.0664,0.575);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,-0.3909,0);else expect(false);
            check_point(actual.denominator_origin,-0.3419,0.21499999999999997);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.01859999999999995,0.21499999999999997);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==-0.31);expect(actual.denominator_wrap_width==-0.48);
        }
        { // Paired Source observation 157: ruled, display, left/right, delims=true
            FractionInput in{make_box(0.46, 0.31, 0.08, 1, 0.83, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1, 1.14, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0.06,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(1.5794000000000001, 0.9373, 1.032, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.1332,0.68);
            check_point(actual.denominator_origin,0.19120000000000018,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0);expect(actual.rule->y==0.22);expect(actual.rule->width==1.5794000000000001);expect(actual.rule->height==0.06);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==1.3794000000000002);else expect(false);
            expect(actual.numerator_wrap_width==1.3794000000000002);expect(actual.denominator_wrap_width==1.3794000000000002);
        }
        { // Paired Source observation 158: atop, display, left/right, delims=true
            FractionInput in{make_box(0.46, 0.31, 0.08, 1, 0.83, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1, 1.14, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(1.3794000000000002, 0.9373, 1.032, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.0332,0.68);
            check_point(actual.denominator_origin,0.09120000000000017,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==1.3794000000000002);else expect(false);
            expect(actual.numerator_wrap_width==1.3794000000000002);expect(actual.denominator_wrap_width==1.3794000000000002);
        }
        { // Paired Source observation 159: bevelled, display, left/right, delims=true
            FractionInput in{make_box(0.46, 0.31, 0.08, 1, 0.83, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1, 1.14, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0.06,FractionAlignment::left,FractionAlignment::right};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==1.2000000000000002);expect(request.delta==0.4);
            expect(request.numerator_y==0.535);expect(request.denominator_y==-0.050000000000000044);expect(request.display==true);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(0.46, 0.31, 0.08, 1, 0.83, 0.04, 0.02));check_box(request.denominator,make_box(1.08, 0.1, 0.3, 1, 1.14, 0.08, 0.05));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(1.8296000000000001, 0.7923, 0.392, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.0332,0.535);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.23159999999999997,0);else expect(false);
            check_point(actual.denominator_origin,0.5427999999999998,-0.050000000000000044);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.4501999999999999,-0.050000000000000044);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==0.46);expect(actual.denominator_wrap_width==1.08);
        }
        { // Paired Source observation 160: ruled, display, left/right, delims=true
            FractionInput in{make_box(1.35, 1.05, 0.24, 1, 0.83, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1, 1.14, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0.17,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(1.3868999999999998, 1.9157, 1.9781999999999997, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.008700000000000013,1.0442);
            check_point(actual.denominator_origin,0.7282999999999998,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0);expect(actual.rule->y==0.16499999999999998);expect(actual.rule->width==1.3868999999999998);expect(actual.rule->height==0.17);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==1.1868999999999998);else expect(false);
            expect(actual.numerator_wrap_width==1.1868999999999998);expect(actual.denominator_wrap_width==1.1868999999999998);
        }
        { // Paired Source observation 161: atop, display, left/right, delims=true
            FractionInput in{make_box(1.35, 1.05, 0.24, 1, 0.83, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1, 1.14, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(1.1868999999999998, 1.5514999999999999, 1.9781999999999997, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.09129999999999999,0.68);
            check_point(actual.denominator_origin,0.6282999999999999,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==1.1868999999999998);else expect(false);
            expect(actual.numerator_wrap_width==1.1868999999999998);expect(actual.denominator_wrap_width==1.1868999999999998);
        }
        { // Paired Source observation 162: bevelled, display, left/right, delims=true
            FractionInput in{make_box(1.35, 1.05, 0.24, 1, 0.83, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1, 1.14, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0.06,FractionAlignment::left,FractionAlignment::right};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==2.16);expect(request.delta==0.4);
            expect(request.numerator_y==0.245);expect(request.denominator_y==0.29999999999999993);expect(request.display==true);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(1.35, 1.05, 0.24, 1, 0.83, -0.11, 0.19));check_box(request.denominator,make_box(0.51, 0.23, 1.13, 1, 1.14, 0.03, -0.02));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(1.7983, 1.1164999999999998, 0.9881999999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.09129999999999999,0.245);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.9868999999999999,0);else expect(false);
            check_point(actual.denominator_origin,1.2410999999999999,0.29999999999999993);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,1.2055,0.29999999999999993);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==1.35);expect(actual.denominator_wrap_width==0.51);
        }
        { // Paired Source observation 163: ruled, display, left/right, delims=true
            FractionInput in{make_box(0.22, 0.16, -0.39, 1, 0.83, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1, 1.14, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0.025,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(0.5761999999999999, 0.8128000000000001, 0.8382, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.2411,0.68);
            check_point(actual.denominator_origin,0.031599999999999934,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0);expect(actual.rule->y==0.2375);expect(actual.rule->width==0.5761999999999999);expect(actual.rule->height==0.025);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0.3761999999999999);else expect(false);
            expect(actual.numerator_wrap_width==0.3761999999999999);expect(actual.denominator_wrap_width==0.3761999999999999);
        }
        { // Paired Source observation 164: atop, display, left/right, delims=true
            FractionInput in{make_box(0.22, 0.16, -0.39, 1, 0.83, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1, 1.14, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0.3761999999999999, 0.8128000000000001, 0.8382, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.1411,0.68);
            check_point(actual.denominator_origin,-0.06840000000000007,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0.3761999999999999);else expect(false);
            expect(actual.numerator_wrap_width==0.3761999999999999);expect(actual.denominator_wrap_width==0.3761999999999999);
        }
        { // Paired Source observation 165: bevelled, display, left/right, delims=true
            FractionInput in{make_box(0.22, 0.16, -0.39, 1, 0.83, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1, 1.14, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0.06,FractionAlignment::left,FractionAlignment::right};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==0.5700000000000001);expect(request.delta==0.4);
            expect(request.numerator_y==0.375);expect(request.denominator_y==0.11499999999999999);expect(request.display==true);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(0.22, 0.16, -0.39, 1, 0.83, 0.17, -0.03));check_box(request.denominator,make_box(0.18, -0.4, 0.13, 1, 1.14, -0.06, 0.21));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(0.6935999999999998, 0.6916, 0.0728, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.1411,0.375);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.09879999999999994,0);else expect(false);
            check_point(actual.denominator_origin,0.25039999999999984,0.11499999999999999);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.31739999999999985,0.11499999999999999);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==0.22);expect(actual.denominator_wrap_width==0.18);
        }
        { // Paired Source observation 166: ruled, display, left/right, delims=true
            FractionInput in{make_box(-0.31, 0.27, 0.12, 1, 0.83, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1, 1.14, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,-0.03,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(0.2, 0.9041, 1.2713999999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.033600000000000005,0.68);
            check_point(actual.denominator_origin,0.5019,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0);expect(actual.rule->y==0.265);expect(actual.rule->width==0.009100000000000025);expect(actual.rule->height==-0.03);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0);else expect(false);
            expect(actual.numerator_wrap_width==0);expect(actual.denominator_wrap_width==0);
        }
        { // Paired Source observation 167: atop, display, left/right, delims=true
            FractionInput in{make_box(-0.31, 0.27, 0.12, 1, 0.83, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1, 1.14, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0, 0.9041, 1.2713999999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.0664,0.68);
            check_point(actual.denominator_origin,0.40190000000000003,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0);else expect(false);
            expect(actual.numerator_wrap_width==0);expect(actual.denominator_wrap_width==0);
        }
        { // Paired Source observation 168: bevelled, display, left/right, delims=true
            FractionInput in{make_box(-0.31, 0.27, 0.12, 1, 0.83, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1, 1.14, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0.06,FractionAlignment::left,FractionAlignment::right};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==1.19);expect(request.delta==0.4);
            expect(request.numerator_y==0.575);expect(request.denominator_y==0.21499999999999997);expect(request.display==true);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(-0.31, 0.27, 0.12, 1, 0.83, -0.08, 0.16));check_box(request.denominator,make_box(-0.48, -0.22, 0.51, 1, 1.14, -0.15, -0.04));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(0.21859999999999996, 0.7990999999999999, 0.36639999999999995, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.0664,0.575);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,-0.3909,0);else expect(false);
            check_point(actual.denominator_origin,-0.3419,0.21499999999999997);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.01859999999999995,0.21499999999999997);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==-0.31);expect(actual.denominator_wrap_width==-0.48);
        }
        { // Paired Source observation 169: ruled, display, center/left, delims=false
            FractionInput in{make_box(0.46, 0.31, 0.08, 1, 0.83, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1, 1.14, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0.06,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(1.8194000000000001, 0.9373, 1.032, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.7188000000000001,0.68);
            check_point(actual.denominator_origin,0.3112,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0.12);expect(actual.rule->y==0.22);expect(actual.rule->width==1.5794000000000001);expect(actual.rule->height==0.06);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==1.3794000000000002);else expect(false);
            expect(actual.numerator_wrap_width==1.3794000000000002);expect(actual.denominator_wrap_width==1.3794000000000002);
        }
        { // Paired Source observation 170: atop, display, center/left, delims=false
            FractionInput in{make_box(0.46, 0.31, 0.08, 1, 0.83, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1, 1.14, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(1.6194000000000002, 0.9373, 1.032, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.6188000000000001,0.68);
            check_point(actual.denominator_origin,0.2112,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==1.3794000000000002);else expect(false);
            expect(actual.numerator_wrap_width==1.3794000000000002);expect(actual.denominator_wrap_width==1.3794000000000002);
        }
        { // Paired Source observation 171: bevelled, display, center/left, delims=false
            FractionInput in{make_box(0.46, 0.31, 0.08, 1, 0.83, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1, 1.14, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0.06,FractionAlignment::center,FractionAlignment::left};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==1.2000000000000002);expect(request.delta==0.4);
            expect(request.numerator_y==0.535);expect(request.denominator_y==-0.050000000000000044);expect(request.display==true);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(0.46, 0.31, 0.08, 1, 0.83, 0.04, 0.02));check_box(request.denominator,make_box(1.08, 0.1, 0.3, 1, 1.14, 0.08, 0.05));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(1.8296000000000001, 0.7923, 0.392, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.0332,0.535);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.23159999999999997,0);else expect(false);
            check_point(actual.denominator_origin,0.5427999999999998,-0.050000000000000044);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.4501999999999999,-0.050000000000000044);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==0.46);expect(actual.denominator_wrap_width==1.08);
        }
        { // Paired Source observation 172: ruled, display, center/left, delims=false
            FractionInput in{make_box(1.35, 1.05, 0.24, 1, 0.83, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1, 1.14, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0.17,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(1.6268999999999998, 1.9157, 1.9781999999999997, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.25319999999999987,1.0442);
            check_point(actual.denominator_origin,0.2542,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0.12);expect(actual.rule->y==0.16499999999999998);expect(actual.rule->width==1.3868999999999998);expect(actual.rule->height==0.17);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==1.1868999999999998);else expect(false);
            expect(actual.numerator_wrap_width==1.1868999999999998);expect(actual.denominator_wrap_width==1.1868999999999998);
        }
        { // Paired Source observation 173: atop, display, center/left, delims=false
            FractionInput in{make_box(1.35, 1.05, 0.24, 1, 0.83, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1, 1.14, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(1.4268999999999998, 1.5514999999999999, 1.9781999999999997, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.1531999999999999,0.68);
            check_point(actual.denominator_origin,0.1542,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==1.1868999999999998);else expect(false);
            expect(actual.numerator_wrap_width==1.1868999999999998);expect(actual.denominator_wrap_width==1.1868999999999998);
        }
        { // Paired Source observation 174: bevelled, display, center/left, delims=false
            FractionInput in{make_box(1.35, 1.05, 0.24, 1, 0.83, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1, 1.14, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0.06,FractionAlignment::center,FractionAlignment::left};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==2.16);expect(request.delta==0.4);
            expect(request.numerator_y==0.245);expect(request.denominator_y==0.29999999999999993);expect(request.display==true);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(1.35, 1.05, 0.24, 1, 0.83, -0.11, 0.19));check_box(request.denominator,make_box(0.51, 0.23, 1.13, 1, 1.14, 0.03, -0.02));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(1.7983, 1.1164999999999998, 0.9881999999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.09129999999999999,0.245);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.9868999999999999,0);else expect(false);
            check_point(actual.denominator_origin,1.2410999999999999,0.29999999999999993);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,1.2055,0.29999999999999993);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==1.35);expect(actual.denominator_wrap_width==0.51);
        }
        { // Paired Source observation 175: ruled, display, center/left, delims=false
            FractionInput in{make_box(0.22, 0.16, -0.39, 1, 0.83, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1, 1.14, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0.025,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(0.8161999999999999, 0.8128000000000001, 0.8382, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.31679999999999997,0.68);
            check_point(actual.denominator_origin,0.1516,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0.12);expect(actual.rule->y==0.2375);expect(actual.rule->width==0.5761999999999999);expect(actual.rule->height==0.025);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0.3761999999999999);else expect(false);
            expect(actual.numerator_wrap_width==0.37619999999999987);expect(actual.denominator_wrap_width==0.37619999999999987);
        }
        { // Paired Source observation 176: atop, display, center/left, delims=false
            FractionInput in{make_box(0.22, 0.16, -0.39, 1, 0.83, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1, 1.14, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0.6161999999999999, 0.8128000000000001, 0.8382, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.21679999999999996,0.68);
            check_point(actual.denominator_origin,0.05160000000000001,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0.37619999999999987);else expect(false);
            expect(actual.numerator_wrap_width==0.37619999999999987);expect(actual.denominator_wrap_width==0.37619999999999987);
        }
        { // Paired Source observation 177: bevelled, display, center/left, delims=false
            FractionInput in{make_box(0.22, 0.16, -0.39, 1, 0.83, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1, 1.14, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0.06,FractionAlignment::center,FractionAlignment::left};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==0.5700000000000001);expect(request.delta==0.4);
            expect(request.numerator_y==0.375);expect(request.denominator_y==0.11499999999999999);expect(request.display==true);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(0.22, 0.16, -0.39, 1, 0.83, 0.17, -0.03));check_box(request.denominator,make_box(0.18, -0.4, 0.13, 1, 1.14, -0.06, 0.21));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(0.6935999999999998, 0.6916, 0.0728, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.1411,0.375);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.09879999999999994,0);else expect(false);
            check_point(actual.denominator_origin,0.25039999999999984,0.11499999999999999);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.31739999999999985,0.11499999999999999);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==0.22);expect(actual.denominator_wrap_width==0.18);
        }
        { // Paired Source observation 178: ruled, display, center/left, delims=false
            FractionInput in{make_box(-0.31, 0.27, 0.12, 1, 0.83, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1, 1.14, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,-0.03,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(0.44, 0.9041, 1.2713999999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.2532,0.68);
            check_point(actual.denominator_origin,0.049000000000000016,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0.12);expect(actual.rule->y==0.265);expect(actual.rule->width==0.009100000000000025);expect(actual.rule->height==-0.03);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0);else expect(false);
            expect(actual.numerator_wrap_width==0);expect(actual.denominator_wrap_width==0);
        }
        { // Paired Source observation 179: atop, display, center/left, delims=false
            FractionInput in{make_box(-0.31, 0.27, 0.12, 1, 0.83, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1, 1.14, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0.24, 0.9041, 1.2713999999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.1532,0.68);
            check_point(actual.denominator_origin,-0.05099999999999999,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0);else expect(false);
            expect(actual.numerator_wrap_width==0);expect(actual.denominator_wrap_width==0);
        }
        { // Paired Source observation 180: bevelled, display, center/left, delims=false
            FractionInput in{make_box(-0.31, 0.27, 0.12, 1, 0.83, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1, 1.14, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0.06,FractionAlignment::center,FractionAlignment::left};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==1.19);expect(request.delta==0.4);
            expect(request.numerator_y==0.575);expect(request.denominator_y==0.21499999999999997);expect(request.display==true);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(-0.31, 0.27, 0.12, 1, 0.83, -0.08, 0.16));check_box(request.denominator,make_box(-0.48, -0.22, 0.51, 1, 1.14, -0.15, -0.04));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(0.21859999999999996, 0.7990999999999999, 0.36639999999999995, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.0664,0.575);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,-0.3909,0);else expect(false);
            check_point(actual.denominator_origin,-0.3419,0.21499999999999997);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.01859999999999995,0.21499999999999997);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==-0.31);expect(actual.denominator_wrap_width==-0.48);
        }
        { // Paired Source observation 181: ruled, display, center/left, delims=true
            FractionInput in{make_box(0.46, 0.31, 0.08, 1, 0.83, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1, 1.14, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0.06,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(1.5794000000000001, 0.9373, 1.032, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.5988000000000001,0.68);
            check_point(actual.denominator_origin,0.19119999999999998,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0);expect(actual.rule->y==0.22);expect(actual.rule->width==1.5794000000000001);expect(actual.rule->height==0.06);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==1.3794000000000002);else expect(false);
            expect(actual.numerator_wrap_width==1.3794000000000002);expect(actual.denominator_wrap_width==1.3794000000000002);
        }
        { // Paired Source observation 182: atop, display, center/left, delims=true
            FractionInput in{make_box(0.46, 0.31, 0.08, 1, 0.83, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1, 1.14, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(1.3794000000000002, 0.9373, 1.032, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.49880000000000013,0.68);
            check_point(actual.denominator_origin,0.09119999999999999,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==1.3794000000000002);else expect(false);
            expect(actual.numerator_wrap_width==1.3794000000000002);expect(actual.denominator_wrap_width==1.3794000000000002);
        }
        { // Paired Source observation 183: bevelled, display, center/left, delims=true
            FractionInput in{make_box(0.46, 0.31, 0.08, 1, 0.83, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1, 1.14, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0.06,FractionAlignment::center,FractionAlignment::left};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==1.2000000000000002);expect(request.delta==0.4);
            expect(request.numerator_y==0.535);expect(request.denominator_y==-0.050000000000000044);expect(request.display==true);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(0.46, 0.31, 0.08, 1, 0.83, 0.04, 0.02));check_box(request.denominator,make_box(1.08, 0.1, 0.3, 1, 1.14, 0.08, 0.05));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(1.8296000000000001, 0.7923, 0.392, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.0332,0.535);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.23159999999999997,0);else expect(false);
            check_point(actual.denominator_origin,0.5427999999999998,-0.050000000000000044);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.4501999999999999,-0.050000000000000044);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==0.46);expect(actual.denominator_wrap_width==1.08);
        }
        { // Paired Source observation 184: ruled, display, center/left, delims=true
            FractionInput in{make_box(1.35, 1.05, 0.24, 1, 0.83, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1, 1.14, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0.17,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(1.3868999999999998, 1.9157, 1.9781999999999997, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.1331999999999999,1.0442);
            check_point(actual.denominator_origin,0.13419999999999999,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0);expect(actual.rule->y==0.16499999999999998);expect(actual.rule->width==1.3868999999999998);expect(actual.rule->height==0.17);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==1.1868999999999998);else expect(false);
            expect(actual.numerator_wrap_width==1.1868999999999998);expect(actual.denominator_wrap_width==1.1868999999999998);
        }
        { // Paired Source observation 185: atop, display, center/left, delims=true
            FractionInput in{make_box(1.35, 1.05, 0.24, 1, 0.83, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1, 1.14, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(1.1868999999999998, 1.5514999999999999, 1.9781999999999997, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.033199999999999896,0.68);
            check_point(actual.denominator_origin,0.034199999999999994,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==1.1868999999999998);else expect(false);
            expect(actual.numerator_wrap_width==1.1868999999999998);expect(actual.denominator_wrap_width==1.1868999999999998);
        }
        { // Paired Source observation 186: bevelled, display, center/left, delims=true
            FractionInput in{make_box(1.35, 1.05, 0.24, 1, 0.83, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1, 1.14, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0.06,FractionAlignment::center,FractionAlignment::left};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==2.16);expect(request.delta==0.4);
            expect(request.numerator_y==0.245);expect(request.denominator_y==0.29999999999999993);expect(request.display==true);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(1.35, 1.05, 0.24, 1, 0.83, -0.11, 0.19));check_box(request.denominator,make_box(0.51, 0.23, 1.13, 1, 1.14, 0.03, -0.02));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(1.7983, 1.1164999999999998, 0.9881999999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.09129999999999999,0.245);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.9868999999999999,0);else expect(false);
            check_point(actual.denominator_origin,1.2410999999999999,0.29999999999999993);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,1.2055,0.29999999999999993);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==1.35);expect(actual.denominator_wrap_width==0.51);
        }
        { // Paired Source observation 187: ruled, display, center/left, delims=true
            FractionInput in{make_box(0.22, 0.16, -0.39, 1, 0.83, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1, 1.14, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0.025,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(0.5761999999999999, 0.8128000000000001, 0.8382, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.19679999999999997,0.68);
            check_point(actual.denominator_origin,0.03160000000000002,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0);expect(actual.rule->y==0.2375);expect(actual.rule->width==0.5761999999999999);expect(actual.rule->height==0.025);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0.3761999999999999);else expect(false);
            expect(actual.numerator_wrap_width==0.3761999999999999);expect(actual.denominator_wrap_width==0.3761999999999999);
        }
        { // Paired Source observation 188: atop, display, center/left, delims=true
            FractionInput in{make_box(0.22, 0.16, -0.39, 1, 0.83, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1, 1.14, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0.3761999999999999, 0.8128000000000001, 0.8382, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.09679999999999997,0.68);
            check_point(actual.denominator_origin,-0.06839999999999999,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0.3761999999999999);else expect(false);
            expect(actual.numerator_wrap_width==0.3761999999999999);expect(actual.denominator_wrap_width==0.3761999999999999);
        }
        { // Paired Source observation 189: bevelled, display, center/left, delims=true
            FractionInput in{make_box(0.22, 0.16, -0.39, 1, 0.83, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1, 1.14, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0.06,FractionAlignment::center,FractionAlignment::left};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==0.5700000000000001);expect(request.delta==0.4);
            expect(request.numerator_y==0.375);expect(request.denominator_y==0.11499999999999999);expect(request.display==true);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(0.22, 0.16, -0.39, 1, 0.83, 0.17, -0.03));check_box(request.denominator,make_box(0.18, -0.4, 0.13, 1, 1.14, -0.06, 0.21));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(0.6935999999999998, 0.6916, 0.0728, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.1411,0.375);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.09879999999999994,0);else expect(false);
            check_point(actual.denominator_origin,0.25039999999999984,0.11499999999999999);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.31739999999999985,0.11499999999999999);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==0.22);expect(actual.denominator_wrap_width==0.18);
        }
        { // Paired Source observation 190: ruled, display, center/left, delims=true
            FractionInput in{make_box(-0.31, 0.27, 0.12, 1, 0.83, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1, 1.14, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,-0.03,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(0.2, 0.9041, 1.2713999999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.13319999999999999,0.68);
            check_point(actual.denominator_origin,-0.07099999999999998,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0);expect(actual.rule->y==0.265);expect(actual.rule->width==0.009100000000000025);expect(actual.rule->height==-0.03);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0);else expect(false);
            expect(actual.numerator_wrap_width==0);expect(actual.denominator_wrap_width==0);
        }
        { // Paired Source observation 191: atop, display, center/left, delims=true
            FractionInput in{make_box(-0.31, 0.27, 0.12, 1, 0.83, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1, 1.14, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0, 0.9041, 1.2713999999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.03319999999999999,0.68);
            check_point(actual.denominator_origin,-0.17099999999999999,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0);else expect(false);
            expect(actual.numerator_wrap_width==0);expect(actual.denominator_wrap_width==0);
        }
        { // Paired Source observation 192: bevelled, display, center/left, delims=true
            FractionInput in{make_box(-0.31, 0.27, 0.12, 1, 0.83, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1, 1.14, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0.06,FractionAlignment::center,FractionAlignment::left};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==1.19);expect(request.delta==0.4);
            expect(request.numerator_y==0.575);expect(request.denominator_y==0.21499999999999997);expect(request.display==true);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(-0.31, 0.27, 0.12, 1, 0.83, -0.08, 0.16));check_box(request.denominator,make_box(-0.48, -0.22, 0.51, 1, 1.14, -0.15, -0.04));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(0.21859999999999996, 0.7990999999999999, 0.36639999999999995, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.0664,0.575);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,-0.3909,0);else expect(false);
            check_point(actual.denominator_origin,-0.3419,0.21499999999999997);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.01859999999999995,0.21499999999999997);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==-0.31);expect(actual.denominator_wrap_width==-0.48);
        }
        { // Paired Source observation 193: ruled, display, right/center, delims=false
            FractionInput in{make_box(0.46, 0.31, 0.08, 1, 0.83, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1, 1.14, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0.06,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(1.8194000000000001, 0.9373, 1.032, 1, 1, 0, 0));
            check_point(actual.numerator_origin,1.201,0.68);
            check_point(actual.denominator_origin,0.29410000000000003,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0.12);expect(actual.rule->y==0.22);expect(actual.rule->width==1.5794000000000001);expect(actual.rule->height==0.06);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==1.3794000000000002);else expect(false);
            expect(actual.numerator_wrap_width==1.3794000000000002);expect(actual.denominator_wrap_width==1.3794000000000002);
        }
        { // Paired Source observation 194: atop, display, right/center, delims=false
            FractionInput in{make_box(0.46, 0.31, 0.08, 1, 0.83, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1, 1.14, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(1.6194000000000002, 0.9373, 1.032, 1, 1, 0, 0));
            check_point(actual.numerator_origin,1.101,0.68);
            check_point(actual.denominator_origin,0.19410000000000005,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==1.3794000000000002);else expect(false);
            expect(actual.numerator_wrap_width==1.3794000000000002);expect(actual.denominator_wrap_width==1.3794000000000002);
        }
        { // Paired Source observation 195: bevelled, display, right/center, delims=false
            FractionInput in{make_box(0.46, 0.31, 0.08, 1, 0.83, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1, 1.14, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0.06,FractionAlignment::right,FractionAlignment::center};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==1.2000000000000002);expect(request.delta==0.4);
            expect(request.numerator_y==0.535);expect(request.denominator_y==-0.050000000000000044);expect(request.display==true);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(0.46, 0.31, 0.08, 1, 0.83, 0.04, 0.02));check_box(request.denominator,make_box(1.08, 0.1, 0.3, 1, 1.14, 0.08, 0.05));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(1.8296000000000001, 0.7923, 0.392, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.0332,0.535);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.23159999999999997,0);else expect(false);
            check_point(actual.denominator_origin,0.5427999999999998,-0.050000000000000044);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.4501999999999999,-0.050000000000000044);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==0.46);expect(actual.denominator_wrap_width==1.08);
        }
        { // Paired Source observation 196: ruled, display, right/center, delims=false
            FractionInput in{make_box(1.35, 1.05, 0.24, 1, 0.83, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1, 1.14, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0.17,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(1.6268999999999998, 1.9157, 1.9781999999999997, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.12869999999999984,1.0442);
            check_point(actual.denominator_origin,0.5227499999999999,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0.12);expect(actual.rule->y==0.16499999999999998);expect(actual.rule->width==1.3868999999999998);expect(actual.rule->height==0.17);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==1.1868999999999998);else expect(false);
            expect(actual.numerator_wrap_width==1.1868999999999998);expect(actual.denominator_wrap_width==1.1868999999999998);
        }
        { // Paired Source observation 197: atop, display, right/center, delims=false
            FractionInput in{make_box(1.35, 1.05, 0.24, 1, 0.83, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1, 1.14, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(1.4268999999999998, 1.5514999999999999, 1.9781999999999997, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.028699999999999837,0.68);
            check_point(actual.denominator_origin,0.42274999999999996,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==1.1868999999999998);else expect(false);
            expect(actual.numerator_wrap_width==1.1868999999999998);expect(actual.denominator_wrap_width==1.1868999999999998);
        }
        { // Paired Source observation 198: bevelled, display, right/center, delims=false
            FractionInput in{make_box(1.35, 1.05, 0.24, 1, 0.83, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1, 1.14, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0.06,FractionAlignment::right,FractionAlignment::center};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==2.16);expect(request.delta==0.4);
            expect(request.numerator_y==0.245);expect(request.denominator_y==0.29999999999999993);expect(request.display==true);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(1.35, 1.05, 0.24, 1, 0.83, -0.11, 0.19));check_box(request.denominator,make_box(0.51, 0.23, 1.13, 1, 1.14, 0.03, -0.02));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(1.7983, 1.1164999999999998, 0.9881999999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.09129999999999999,0.245);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.9868999999999999,0);else expect(false);
            check_point(actual.denominator_origin,1.2410999999999999,0.29999999999999993);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,1.2055,0.29999999999999993);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==1.35);expect(actual.denominator_wrap_width==0.51);
        }
        { // Paired Source observation 199: ruled, display, right/center, delims=false
            FractionInput in{make_box(0.22, 0.16, -0.39, 1, 0.83, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1, 1.14, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0.025,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(0.8161999999999999, 0.8128000000000001, 0.8382, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.4384999999999999,0.68);
            check_point(actual.denominator_origin,0.3055,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0.12);expect(actual.rule->y==0.2375);expect(actual.rule->width==0.5761999999999999);expect(actual.rule->height==0.025);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0.3761999999999999);else expect(false);
            expect(actual.numerator_wrap_width==0.37619999999999987);expect(actual.denominator_wrap_width==0.37619999999999987);
        }
        { // Paired Source observation 200: atop, display, right/center, delims=false
            FractionInput in{make_box(0.22, 0.16, -0.39, 1, 0.83, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1, 1.14, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0.6161999999999999, 0.8128000000000001, 0.8382, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.3384999999999999,0.68);
            check_point(actual.denominator_origin,0.20549999999999996,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0.37619999999999987);else expect(false);
            expect(actual.numerator_wrap_width==0.37619999999999987);expect(actual.denominator_wrap_width==0.37619999999999987);
        }
        { // Paired Source observation 201: bevelled, display, right/center, delims=false
            FractionInput in{make_box(0.22, 0.16, -0.39, 1, 0.83, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1, 1.14, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0.06,FractionAlignment::right,FractionAlignment::center};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==0.5700000000000001);expect(request.delta==0.4);
            expect(request.numerator_y==0.375);expect(request.denominator_y==0.11499999999999999);expect(request.display==true);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(0.22, 0.16, -0.39, 1, 0.83, 0.17, -0.03));check_box(request.denominator,make_box(0.18, -0.4, 0.13, 1, 1.14, -0.06, 0.21));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(0.6935999999999998, 0.6916, 0.0728, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.1411,0.375);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.09879999999999994,0);else expect(false);
            check_point(actual.denominator_origin,0.25039999999999984,0.11499999999999999);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.31739999999999985,0.11499999999999999);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==0.22);expect(actual.denominator_wrap_width==0.18);
        }
        { // Paired Source observation 202: ruled, display, right/center, delims=false
            FractionInput in{make_box(-0.31, 0.27, 0.12, 1, 0.83, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1, 1.14, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,-0.03,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(0.44, 0.9041, 1.2713999999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.15360000000000001,0.68);
            check_point(actual.denominator_origin,0.39815,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0.12);expect(actual.rule->y==0.265);expect(actual.rule->width==0.009100000000000025);expect(actual.rule->height==-0.03);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0);else expect(false);
            expect(actual.numerator_wrap_width==0);expect(actual.denominator_wrap_width==0);
        }
        { // Paired Source observation 203: atop, display, right/center, delims=false
            FractionInput in{make_box(-0.31, 0.27, 0.12, 1, 0.83, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1, 1.14, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0.24, 0.9041, 1.2713999999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.053599999999999995,0.68);
            check_point(actual.denominator_origin,0.29814999999999997,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0);else expect(false);
            expect(actual.numerator_wrap_width==0);expect(actual.denominator_wrap_width==0);
        }
        { // Paired Source observation 204: bevelled, display, right/center, delims=false
            FractionInput in{make_box(-0.31, 0.27, 0.12, 1, 0.83, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1, 1.14, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0.06,FractionAlignment::right,FractionAlignment::center};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==1.19);expect(request.delta==0.4);
            expect(request.numerator_y==0.575);expect(request.denominator_y==0.21499999999999997);expect(request.display==true);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(-0.31, 0.27, 0.12, 1, 0.83, -0.08, 0.16));check_box(request.denominator,make_box(-0.48, -0.22, 0.51, 1, 1.14, -0.15, -0.04));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(0.21859999999999996, 0.7990999999999999, 0.36639999999999995, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.0664,0.575);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,-0.3909,0);else expect(false);
            check_point(actual.denominator_origin,-0.3419,0.21499999999999997);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.01859999999999995,0.21499999999999997);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==-0.31);expect(actual.denominator_wrap_width==-0.48);
        }
        { // Paired Source observation 205: ruled, display, right/center, delims=true
            FractionInput in{make_box(0.46, 0.31, 0.08, 1, 0.83, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1, 1.14, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0.06,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(1.5794000000000001, 0.9373, 1.032, 1, 1, 0, 0));
            check_point(actual.numerator_origin,1.0810000000000002,0.68);
            check_point(actual.denominator_origin,0.17410000000000006,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0);expect(actual.rule->y==0.22);expect(actual.rule->width==1.5794000000000001);expect(actual.rule->height==0.06);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==1.3794000000000002);else expect(false);
            expect(actual.numerator_wrap_width==1.3794000000000002);expect(actual.denominator_wrap_width==1.3794000000000002);
        }
        { // Paired Source observation 206: atop, display, right/center, delims=true
            FractionInput in{make_box(0.46, 0.31, 0.08, 1, 0.83, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1, 1.14, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(1.3794000000000002, 0.9373, 1.032, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.9810000000000001,0.68);
            check_point(actual.denominator_origin,0.07410000000000005,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==1.3794000000000002);else expect(false);
            expect(actual.numerator_wrap_width==1.3794000000000002);expect(actual.denominator_wrap_width==1.3794000000000002);
        }
        { // Paired Source observation 207: bevelled, display, right/center, delims=true
            FractionInput in{make_box(0.46, 0.31, 0.08, 1, 0.83, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1, 1.14, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0.06,FractionAlignment::right,FractionAlignment::center};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==1.2000000000000002);expect(request.delta==0.4);
            expect(request.numerator_y==0.535);expect(request.denominator_y==-0.050000000000000044);expect(request.display==true);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(0.46, 0.31, 0.08, 1, 0.83, 0.04, 0.02));check_box(request.denominator,make_box(1.08, 0.1, 0.3, 1, 1.14, 0.08, 0.05));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(1.8296000000000001, 0.7923, 0.392, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.0332,0.535);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.23159999999999997,0);else expect(false);
            check_point(actual.denominator_origin,0.5427999999999998,-0.050000000000000044);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.4501999999999999,-0.050000000000000044);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==0.46);expect(actual.denominator_wrap_width==1.08);
        }
        { // Paired Source observation 208: ruled, display, right/center, delims=true
            FractionInput in{make_box(1.35, 1.05, 0.24, 1, 0.83, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1, 1.14, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0.17,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(1.3868999999999998, 1.9157, 1.9781999999999997, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.008699999999999847,1.0442);
            check_point(actual.denominator_origin,0.40274999999999994,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0);expect(actual.rule->y==0.16499999999999998);expect(actual.rule->width==1.3868999999999998);expect(actual.rule->height==0.17);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==1.1868999999999998);else expect(false);
            expect(actual.numerator_wrap_width==1.1868999999999998);expect(actual.denominator_wrap_width==1.1868999999999998);
        }
        { // Paired Source observation 209: atop, display, right/center, delims=true
            FractionInput in{make_box(1.35, 1.05, 0.24, 1, 0.83, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1, 1.14, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(1.1868999999999998, 1.5514999999999999, 1.9781999999999997, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.09130000000000016,0.68);
            check_point(actual.denominator_origin,0.30274999999999996,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==1.1868999999999998);else expect(false);
            expect(actual.numerator_wrap_width==1.1868999999999998);expect(actual.denominator_wrap_width==1.1868999999999998);
        }
        { // Paired Source observation 210: bevelled, display, right/center, delims=true
            FractionInput in{make_box(1.35, 1.05, 0.24, 1, 0.83, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1, 1.14, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0.06,FractionAlignment::right,FractionAlignment::center};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==2.16);expect(request.delta==0.4);
            expect(request.numerator_y==0.245);expect(request.denominator_y==0.29999999999999993);expect(request.display==true);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(1.35, 1.05, 0.24, 1, 0.83, -0.11, 0.19));check_box(request.denominator,make_box(0.51, 0.23, 1.13, 1, 1.14, 0.03, -0.02));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(1.7983, 1.1164999999999998, 0.9881999999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.09129999999999999,0.245);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.9868999999999999,0);else expect(false);
            check_point(actual.denominator_origin,1.2410999999999999,0.29999999999999993);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,1.2055,0.29999999999999993);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==1.35);expect(actual.denominator_wrap_width==0.51);
        }
        { // Paired Source observation 211: ruled, display, right/center, delims=true
            FractionInput in{make_box(0.22, 0.16, -0.39, 1, 0.83, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1, 1.14, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0.025,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(0.5761999999999999, 0.8128000000000001, 0.8382, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.3184999999999999,0.68);
            check_point(actual.denominator_origin,0.1855,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0);expect(actual.rule->y==0.2375);expect(actual.rule->width==0.5761999999999999);expect(actual.rule->height==0.025);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0.3761999999999999);else expect(false);
            expect(actual.numerator_wrap_width==0.3761999999999999);expect(actual.denominator_wrap_width==0.3761999999999999);
        }
        { // Paired Source observation 212: atop, display, right/center, delims=true
            FractionInput in{make_box(0.22, 0.16, -0.39, 1, 0.83, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1, 1.14, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0.3761999999999999, 0.8128000000000001, 0.8382, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.21849999999999992,0.68);
            check_point(actual.denominator_origin,0.08549999999999998,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0.3761999999999999);else expect(false);
            expect(actual.numerator_wrap_width==0.3761999999999999);expect(actual.denominator_wrap_width==0.3761999999999999);
        }
        { // Paired Source observation 213: bevelled, display, right/center, delims=true
            FractionInput in{make_box(0.22, 0.16, -0.39, 1, 0.83, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1, 1.14, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0.06,FractionAlignment::right,FractionAlignment::center};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==0.5700000000000001);expect(request.delta==0.4);
            expect(request.numerator_y==0.375);expect(request.denominator_y==0.11499999999999999);expect(request.display==true);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(0.22, 0.16, -0.39, 1, 0.83, 0.17, -0.03));check_box(request.denominator,make_box(0.18, -0.4, 0.13, 1, 1.14, -0.06, 0.21));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(0.6935999999999998, 0.6916, 0.0728, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.1411,0.375);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.09879999999999994,0);else expect(false);
            check_point(actual.denominator_origin,0.25039999999999984,0.11499999999999999);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.31739999999999985,0.11499999999999999);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==0.22);expect(actual.denominator_wrap_width==0.18);
        }
        { // Paired Source observation 214: ruled, display, right/center, delims=true
            FractionInput in{make_box(-0.31, 0.27, 0.12, 1, 0.83, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1, 1.14, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,-0.03,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(0.2, 0.9041, 1.2713999999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.033600000000000005,0.68);
            check_point(actual.denominator_origin,0.27815,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0);expect(actual.rule->y==0.265);expect(actual.rule->width==0.009100000000000025);expect(actual.rule->height==-0.03);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0);else expect(false);
            expect(actual.numerator_wrap_width==0);expect(actual.denominator_wrap_width==0);
        }
        { // Paired Source observation 215: atop, display, right/center, delims=true
            FractionInput in{make_box(-0.31, 0.27, 0.12, 1, 0.83, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1, 1.14, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0, 0.9041, 1.2713999999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.0664,0.68);
            check_point(actual.denominator_origin,0.17814999999999998,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0);else expect(false);
            expect(actual.numerator_wrap_width==0);expect(actual.denominator_wrap_width==0);
        }
        { // Paired Source observation 216: bevelled, display, right/center, delims=true
            FractionInput in{make_box(-0.31, 0.27, 0.12, 1, 0.83, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1, 1.14, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0.06,FractionAlignment::right,FractionAlignment::center};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==1.19);expect(request.delta==0.4);
            expect(request.numerator_y==0.575);expect(request.denominator_y==0.21499999999999997);expect(request.display==true);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(-0.31, 0.27, 0.12, 1, 0.83, -0.08, 0.16));check_box(request.denominator,make_box(-0.48, -0.22, 0.51, 1, 1.14, -0.15, -0.04));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(0.21859999999999996, 0.7990999999999999, 0.36639999999999995, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.0664,0.575);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,-0.3909,0);else expect(false);
            check_point(actual.denominator_origin,-0.3419,0.21499999999999997);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.01859999999999995,0.21499999999999997);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==-0.31);expect(actual.denominator_wrap_width==-0.48);
        }
        { // Paired Source observation 217: ruled, display, left/right, delims=false
            FractionInput in{make_box(0.46, 0.31, 0.08, 0.72, 0.5975999999999999, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1.23, 1.4022, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0.06,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(2.1366620000000003, 0.865256, 1.11066, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.243904,0.68);
            check_point(actual.denominator_origin,0.332176,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0.12);expect(actual.rule->y==0.22);expect(actual.rule->width==1.896662);expect(actual.rule->height==0.06);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==1.6966620000000003);else expect(false);
            expect(actual.numerator_wrap_width==1.6966620000000003);expect(actual.denominator_wrap_width==1.6966620000000003);
        }
        { // Paired Source observation 218: atop, display, left/right, delims=false
            FractionInput in{make_box(0.46, 0.31, 0.08, 0.72, 0.5975999999999999, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1.23, 1.4022, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(1.936662, 0.865256, 1.11066, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.143904,0.68);
            check_point(actual.denominator_origin,0.23217600000000005,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==1.6966620000000001);else expect(false);
            expect(actual.numerator_wrap_width==1.6966620000000001);expect(actual.denominator_wrap_width==1.6966620000000001);
        }
        { // Paired Source observation 219: bevelled, display, left/right, delims=false
            FractionInput in{make_box(0.46, 0.31, 0.08, 0.72, 0.5975999999999999, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1.23, 1.4022, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0.06,FractionAlignment::left,FractionAlignment::right};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==1.292);expect(request.delta==0.4);
            expect(request.numerator_y==0.5672);expect(request.denominator_y==-0.027000000000000024);expect(request.display==true);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(0.46, 0.31, 0.08, 0.72, 0.5975999999999999, 0.04, 0.02));check_box(request.denominator,make_box(1.08, 0.1, 0.3, 1.23, 1.4022, 0.08, 0.05));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(2.026014, 0.752456, 0.44766, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.023903999999999998,0.5672);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.11075199999999996,0);else expect(false);
            check_point(actual.denominator_origin,0.4429279999999999,-0.027000000000000024);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.3293519999999999,-0.027000000000000024);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==0.46);expect(actual.denominator_wrap_width==1.08);
        }
        { // Paired Source observation 220: ruled, display, left/right, delims=false
            FractionInput in{make_box(1.35, 1.05, 0.24, 0.72, 0.5975999999999999, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1.23, 1.4022, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0.17,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(1.294568, 1.615904, 2.2744859999999996, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.154264,0.9884240000000001);
            check_point(actual.denominator_origin,0.3874899999999999,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0.12);expect(actual.rule->y==0.16499999999999998);expect(actual.rule->width==1.054568);expect(actual.rule->height==0.17);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0.854568);else expect(false);
            expect(actual.numerator_wrap_width==0.854568);expect(actual.denominator_wrap_width==0.854568);
        }
        { // Paired Source observation 221: atop, display, left/right, delims=false
            FractionInput in{make_box(1.35, 1.05, 0.24, 0.72, 0.5975999999999999, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1.23, 1.4022, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(1.0945679999999998, 1.30748, 2.2744859999999996, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.05426400000000001,0.68);
            check_point(actual.denominator_origin,0.2874899999999999,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0.8545679999999998);else expect(false);
            expect(actual.numerator_wrap_width==0.8545679999999998);expect(actual.denominator_wrap_width==0.8545679999999998);
        }
        { // Paired Source observation 222: bevelled, display, left/right, delims=false
            FractionInput in{make_box(1.35, 1.05, 0.24, 0.72, 0.5975999999999999, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1.23, 1.4022, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0.06,FractionAlignment::left,FractionAlignment::right};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==2.4728);expect(request.delta==0.4);
            expect(request.numerator_y==0.3584);expect(request.denominator_y==0.40349999999999997);expect(request.display==true);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(1.35, 1.05, 0.24, 0.72, 0.5975999999999999, -0.11, 0.19));check_box(request.denominator,make_box(0.51, 0.23, 1.13, 1.23, 1.4022, 0.03, -0.02));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(1.602312, 0.9858799999999999, 1.1809859999999996, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.06573599999999999,0.3584);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.6545679999999998,0);else expect(false);
            check_point(actual.denominator_origin,0.9166339999999998,0.40349999999999997);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.8731679999999999,0.40349999999999997);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==1.35);expect(actual.denominator_wrap_width==0.51);
        }
        { // Paired Source observation 223: ruled, display, left/right, delims=false
            FractionInput in{make_box(0.22, 0.16, -0.39, 0.72, 0.5975999999999999, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1.23, 1.4022, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0.025,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(0.9027259999999999, 0.7756160000000001, 0.8722859999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.321592,0.68);
            check_point(actual.denominator_origin,0.13586799999999996,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0.12);expect(actual.rule->y==0.2375);expect(actual.rule->width==0.6627259999999999);expect(actual.rule->height==0.025);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0.4627259999999999);else expect(false);
            expect(actual.numerator_wrap_width==0.46272599999999997);expect(actual.denominator_wrap_width==0.46272599999999997);
        }
        { // Paired Source observation 224: atop, display, left/right, delims=false
            FractionInput in{make_box(0.22, 0.16, -0.39, 0.72, 0.5975999999999999, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1.23, 1.4022, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0.702726, 0.7756160000000001, 0.8722859999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.22159199999999998,0.68);
            check_point(actual.denominator_origin,0.035867999999999955,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0.46272599999999997);else expect(false);
            expect(actual.numerator_wrap_width==0.46272599999999997);expect(actual.denominator_wrap_width==0.46272599999999997);
        }
        { // Paired Source observation 225: bevelled, display, left/right, delims=false
            FractionInput in{make_box(0.22, 0.16, -0.39, 0.72, 0.5975999999999999, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1.23, 1.4022, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0.06,FractionAlignment::left,FractionAlignment::right};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==0.6344000000000001);expect(request.delta==0.4);
            expect(request.numerator_y==0.452);expect(request.denominator_y==0.17594999999999994);expect(request.display==true);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(0.22, 0.16, -0.39, 0.72, 0.5975999999999999, 0.17, -0.03));check_box(request.denominator,make_box(0.18, -0.4, 0.13, 1.23, 1.4022, -0.06, 0.21));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(0.6964619999999998, 0.6916, 0.0728, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.10159199999999999,0.452);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.015135999999999955,0);else expect(false);
            check_point(actual.denominator_origin,0.15100399999999992,0.17594999999999994);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.2337359999999999,0.17594999999999994);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==0.22);expect(actual.denominator_wrap_width==0.18);
        }
        { // Paired Source observation 226: ruled, display, left/right, delims=false
            FractionInput in{make_box(-0.31, 0.27, 0.12, 0.72, 0.5975999999999999, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1.23, 1.4022, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,-0.03,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(0.44, 0.8413520000000001, 1.405122, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.172192,0.68);
            check_point(actual.denominator_origin,0.811696,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0.12);expect(actual.rule->y==0.265);expect(actual.rule->width==0.06255200000000002);expect(actual.rule->height==-0.03);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0);else expect(false);
            expect(actual.numerator_wrap_width==0);expect(actual.denominator_wrap_width==0);
        }
        { // Paired Source observation 227: atop, display, left/right, delims=false
            FractionInput in{make_box(-0.31, 0.27, 0.12, 0.72, 0.5975999999999999, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1.23, 1.4022, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0.24, 0.8413520000000001, 1.405122, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.072192,0.68);
            check_point(actual.denominator_origin,0.711696,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0);else expect(false);
            expect(actual.numerator_wrap_width==0);expect(actual.denominator_wrap_width==0);
        }
        { // Paired Source observation 228: bevelled, display, left/right, delims=false
            FractionInput in{make_box(-0.31, 0.27, 0.12, 0.72, 0.5975999999999999, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1.23, 1.4022, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0.06,FractionAlignment::left,FractionAlignment::right};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==1.1567);expect(request.delta==0.4);
            expect(request.numerator_y==0.5960000000000001);expect(request.denominator_y==0.29894999999999994);expect(request.display==true);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(-0.31, 0.27, 0.12, 0.72, 0.5975999999999999, -0.08, 0.16));check_box(request.denominator,make_box(-0.48, -0.22, 0.51, 1.23, 1.4022, -0.15, -0.04));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(0.21859999999999996, 0.757352, 0.416172, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.047807999999999996,0.5960000000000001);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,-0.33744799999999997,0);else expect(false);
            check_point(actual.denominator_origin,-0.327778,0.29894999999999994);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.01859999999999995,0.29894999999999994);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==-0.31);expect(actual.denominator_wrap_width==-0.48);
        }
        { // Paired Source observation 229: ruled, display, left/right, delims=true
            FractionInput in{make_box(0.46, 0.31, 0.08, 0.72, 0.5975999999999999, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1.23, 1.4022, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0.06,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(1.896662, 0.865256, 1.11066, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.123904,0.68);
            check_point(actual.denominator_origin,0.21217600000000006,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0);expect(actual.rule->y==0.22);expect(actual.rule->width==1.896662);expect(actual.rule->height==0.06);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==1.6966620000000001);else expect(false);
            expect(actual.numerator_wrap_width==1.6966620000000001);expect(actual.denominator_wrap_width==1.6966620000000001);
        }
        { // Paired Source observation 230: atop, display, left/right, delims=true
            FractionInput in{make_box(0.46, 0.31, 0.08, 0.72, 0.5975999999999999, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1.23, 1.4022, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(1.6966620000000001, 0.865256, 1.11066, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.023903999999999998,0.68);
            check_point(actual.denominator_origin,0.11217600000000005,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==1.6966620000000001);else expect(false);
            expect(actual.numerator_wrap_width==1.6966620000000001);expect(actual.denominator_wrap_width==1.6966620000000001);
        }
        { // Paired Source observation 231: bevelled, display, left/right, delims=true
            FractionInput in{make_box(0.46, 0.31, 0.08, 0.72, 0.5975999999999999, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1.23, 1.4022, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0.06,FractionAlignment::left,FractionAlignment::right};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==1.292);expect(request.delta==0.4);
            expect(request.numerator_y==0.5672);expect(request.denominator_y==-0.027000000000000024);expect(request.display==true);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(0.46, 0.31, 0.08, 0.72, 0.5975999999999999, 0.04, 0.02));check_box(request.denominator,make_box(1.08, 0.1, 0.3, 1.23, 1.4022, 0.08, 0.05));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(2.026014, 0.752456, 0.44766, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.023903999999999998,0.5672);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.11075199999999996,0);else expect(false);
            check_point(actual.denominator_origin,0.4429279999999999,-0.027000000000000024);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.3293519999999999,-0.027000000000000024);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==0.46);expect(actual.denominator_wrap_width==1.08);
        }
        { // Paired Source observation 232: ruled, display, left/right, delims=true
            FractionInput in{make_box(1.35, 1.05, 0.24, 0.72, 0.5975999999999999, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1.23, 1.4022, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0.17,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(1.054568, 1.615904, 2.2744859999999996, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.03426400000000002,0.9884240000000001);
            check_point(actual.denominator_origin,0.2674899999999999,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0);expect(actual.rule->y==0.16499999999999998);expect(actual.rule->width==1.054568);expect(actual.rule->height==0.17);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0.854568);else expect(false);
            expect(actual.numerator_wrap_width==0.854568);expect(actual.denominator_wrap_width==0.854568);
        }
        { // Paired Source observation 233: atop, display, left/right, delims=true
            FractionInput in{make_box(1.35, 1.05, 0.24, 0.72, 0.5975999999999999, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1.23, 1.4022, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0.8545679999999999, 1.30748, 2.2744859999999996, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.06573599999999999,0.68);
            check_point(actual.denominator_origin,0.16748999999999992,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0.8545679999999999);else expect(false);
            expect(actual.numerator_wrap_width==0.8545679999999999);expect(actual.denominator_wrap_width==0.8545679999999999);
        }
        { // Paired Source observation 234: bevelled, display, left/right, delims=true
            FractionInput in{make_box(1.35, 1.05, 0.24, 0.72, 0.5975999999999999, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1.23, 1.4022, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0.06,FractionAlignment::left,FractionAlignment::right};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==2.4728);expect(request.delta==0.4);
            expect(request.numerator_y==0.3584);expect(request.denominator_y==0.40349999999999997);expect(request.display==true);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(1.35, 1.05, 0.24, 0.72, 0.5975999999999999, -0.11, 0.19));check_box(request.denominator,make_box(0.51, 0.23, 1.13, 1.23, 1.4022, 0.03, -0.02));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(1.602312, 0.9858799999999999, 1.1809859999999996, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.06573599999999999,0.3584);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.6545679999999998,0);else expect(false);
            check_point(actual.denominator_origin,0.9166339999999998,0.40349999999999997);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.8731679999999999,0.40349999999999997);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==1.35);expect(actual.denominator_wrap_width==0.51);
        }
        { // Paired Source observation 235: ruled, display, left/right, delims=true
            FractionInput in{make_box(0.22, 0.16, -0.39, 0.72, 0.5975999999999999, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1.23, 1.4022, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0.025,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(0.6627259999999999, 0.7756160000000001, 0.8722859999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.201592,0.68);
            check_point(actual.denominator_origin,0.015867999999999965,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0);expect(actual.rule->y==0.2375);expect(actual.rule->width==0.6627259999999999);expect(actual.rule->height==0.025);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0.4627259999999999);else expect(false);
            expect(actual.numerator_wrap_width==0.4627259999999999);expect(actual.denominator_wrap_width==0.4627259999999999);
        }
        { // Paired Source observation 236: atop, display, left/right, delims=true
            FractionInput in{make_box(0.22, 0.16, -0.39, 0.72, 0.5975999999999999, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1.23, 1.4022, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0.4627259999999999, 0.7756160000000001, 0.8722859999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.10159199999999999,0.68);
            check_point(actual.denominator_origin,-0.08413200000000004,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0.4627259999999999);else expect(false);
            expect(actual.numerator_wrap_width==0.4627259999999999);expect(actual.denominator_wrap_width==0.4627259999999999);
        }
        { // Paired Source observation 237: bevelled, display, left/right, delims=true
            FractionInput in{make_box(0.22, 0.16, -0.39, 0.72, 0.5975999999999999, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1.23, 1.4022, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0.06,FractionAlignment::left,FractionAlignment::right};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==0.6344000000000001);expect(request.delta==0.4);
            expect(request.numerator_y==0.452);expect(request.denominator_y==0.17594999999999994);expect(request.display==true);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(0.22, 0.16, -0.39, 0.72, 0.5975999999999999, 0.17, -0.03));check_box(request.denominator,make_box(0.18, -0.4, 0.13, 1.23, 1.4022, -0.06, 0.21));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(0.6964619999999998, 0.6916, 0.0728, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.10159199999999999,0.452);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.015135999999999955,0);else expect(false);
            check_point(actual.denominator_origin,0.15100399999999992,0.17594999999999994);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.2337359999999999,0.17594999999999994);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==0.22);expect(actual.denominator_wrap_width==0.18);
        }
        { // Paired Source observation 238: ruled, display, left/right, delims=true
            FractionInput in{make_box(-0.31, 0.27, 0.12, 0.72, 0.5975999999999999, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1.23, 1.4022, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,-0.03,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(0.2, 0.8413520000000001, 1.405122, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.05219200000000001,0.68);
            check_point(actual.denominator_origin,0.691696,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0);expect(actual.rule->y==0.265);expect(actual.rule->width==0.06255200000000002);expect(actual.rule->height==-0.03);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0);else expect(false);
            expect(actual.numerator_wrap_width==0);expect(actual.denominator_wrap_width==0);
        }
        { // Paired Source observation 239: atop, display, left/right, delims=true
            FractionInput in{make_box(-0.31, 0.27, 0.12, 0.72, 0.5975999999999999, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1.23, 1.4022, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0,FractionAlignment::left,FractionAlignment::right};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0, 0.8413520000000001, 1.405122, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.047807999999999996,0.68);
            check_point(actual.denominator_origin,0.591696,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0);else expect(false);
            expect(actual.numerator_wrap_width==0);expect(actual.denominator_wrap_width==0);
        }
        { // Paired Source observation 240: bevelled, display, left/right, delims=true
            FractionInput in{make_box(-0.31, 0.27, 0.12, 0.72, 0.5975999999999999, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1.23, 1.4022, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0.06,FractionAlignment::left,FractionAlignment::right};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==1.1567);expect(request.delta==0.4);
            expect(request.numerator_y==0.5960000000000001);expect(request.denominator_y==0.29894999999999994);expect(request.display==true);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(-0.31, 0.27, 0.12, 0.72, 0.5975999999999999, -0.08, 0.16));check_box(request.denominator,make_box(-0.48, -0.22, 0.51, 1.23, 1.4022, -0.15, -0.04));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(0.21859999999999996, 0.757352, 0.416172, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.047807999999999996,0.5960000000000001);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,-0.33744799999999997,0);else expect(false);
            check_point(actual.denominator_origin,-0.327778,0.29894999999999994);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.01859999999999995,0.29894999999999994);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==-0.31);expect(actual.denominator_wrap_width==-0.48);
        }
        { // Paired Source observation 241: ruled, display, center/left, delims=false
            FractionInput in{make_box(0.46, 0.31, 0.08, 0.72, 0.5975999999999999, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1.23, 1.4022, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0.06,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(2.1366620000000003, 0.865256, 1.11066, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.930883,0.68);
            check_point(actual.denominator_origin,0.332176,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0.12);expect(actual.rule->y==0.22);expect(actual.rule->width==1.896662);expect(actual.rule->height==0.06);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==1.6966620000000003);else expect(false);
            expect(actual.numerator_wrap_width==1.6966620000000003);expect(actual.denominator_wrap_width==1.6966620000000003);
        }
        { // Paired Source observation 242: atop, display, center/left, delims=false
            FractionInput in{make_box(0.46, 0.31, 0.08, 0.72, 0.5975999999999999, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1.23, 1.4022, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(1.936662, 0.865256, 1.11066, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.830883,0.68);
            check_point(actual.denominator_origin,0.232176,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==1.6966620000000001);else expect(false);
            expect(actual.numerator_wrap_width==1.6966620000000001);expect(actual.denominator_wrap_width==1.6966620000000001);
        }
        { // Paired Source observation 243: bevelled, display, center/left, delims=false
            FractionInput in{make_box(0.46, 0.31, 0.08, 0.72, 0.5975999999999999, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1.23, 1.4022, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0.06,FractionAlignment::center,FractionAlignment::left};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==1.292);expect(request.delta==0.4);
            expect(request.numerator_y==0.5672);expect(request.denominator_y==-0.027000000000000024);expect(request.display==true);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(0.46, 0.31, 0.08, 0.72, 0.5975999999999999, 0.04, 0.02));check_box(request.denominator,make_box(1.08, 0.1, 0.3, 1.23, 1.4022, 0.08, 0.05));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(2.026014, 0.752456, 0.44766, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.023903999999999998,0.5672);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.11075199999999996,0);else expect(false);
            check_point(actual.denominator_origin,0.4429279999999999,-0.027000000000000024);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.3293519999999999,-0.027000000000000024);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==0.46);expect(actual.denominator_wrap_width==1.08);
        }
        { // Paired Source observation 244: ruled, display, center/left, delims=false
            FractionInput in{make_box(1.35, 1.05, 0.24, 0.72, 0.5975999999999999, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1.23, 1.4022, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0.17,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(1.294568, 1.615904, 2.2744859999999996, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.24390399999999998,0.9884240000000001);
            check_point(actual.denominator_origin,0.262066,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0.12);expect(actual.rule->y==0.16499999999999998);expect(actual.rule->width==1.054568);expect(actual.rule->height==0.17);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0.854568);else expect(false);
            expect(actual.numerator_wrap_width==0.854568);expect(actual.denominator_wrap_width==0.854568);
        }
        { // Paired Source observation 245: atop, display, center/left, delims=false
            FractionInput in{make_box(1.35, 1.05, 0.24, 0.72, 0.5975999999999999, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1.23, 1.4022, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(1.0945679999999998, 1.30748, 2.2744859999999996, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.14390399999999998,0.68);
            check_point(actual.denominator_origin,0.162066,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0.8545679999999998);else expect(false);
            expect(actual.numerator_wrap_width==0.8545679999999998);expect(actual.denominator_wrap_width==0.8545679999999998);
        }
        { // Paired Source observation 246: bevelled, display, center/left, delims=false
            FractionInput in{make_box(1.35, 1.05, 0.24, 0.72, 0.5975999999999999, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1.23, 1.4022, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0.06,FractionAlignment::center,FractionAlignment::left};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==2.4728);expect(request.delta==0.4);
            expect(request.numerator_y==0.3584);expect(request.denominator_y==0.40349999999999997);expect(request.display==true);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(1.35, 1.05, 0.24, 0.72, 0.5975999999999999, -0.11, 0.19));check_box(request.denominator,make_box(0.51, 0.23, 1.13, 1.23, 1.4022, 0.03, -0.02));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(1.602312, 0.9858799999999999, 1.1809859999999996, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.06573599999999999,0.3584);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.6545679999999998,0);else expect(false);
            check_point(actual.denominator_origin,0.9166339999999998,0.40349999999999997);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.8731679999999999,0.40349999999999997);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==1.35);expect(actual.denominator_wrap_width==0.51);
        }
        { // Paired Source observation 247: ruled, display, center/left, delims=false
            FractionInput in{make_box(0.22, 0.16, -0.39, 0.72, 0.5975999999999999, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1.23, 1.4022, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0.025,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(0.9027259999999999, 0.7756160000000001, 0.8722859999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.38562699999999994,0.68);
            check_point(actual.denominator_origin,0.13586800000000002,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0.12);expect(actual.rule->y==0.2375);expect(actual.rule->width==0.6627259999999999);expect(actual.rule->height==0.025);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0.4627259999999999);else expect(false);
            expect(actual.numerator_wrap_width==0.46272599999999997);expect(actual.denominator_wrap_width==0.46272599999999997);
        }
        { // Paired Source observation 248: atop, display, center/left, delims=false
            FractionInput in{make_box(0.22, 0.16, -0.39, 0.72, 0.5975999999999999, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1.23, 1.4022, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0.702726, 0.7756160000000001, 0.8722859999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.28562699999999996,0.68);
            check_point(actual.denominator_origin,0.03586800000000001,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0.46272599999999997);else expect(false);
            expect(actual.numerator_wrap_width==0.46272599999999997);expect(actual.denominator_wrap_width==0.46272599999999997);
        }
        { // Paired Source observation 249: bevelled, display, center/left, delims=false
            FractionInput in{make_box(0.22, 0.16, -0.39, 0.72, 0.5975999999999999, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1.23, 1.4022, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0.06,FractionAlignment::center,FractionAlignment::left};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==0.6344000000000001);expect(request.delta==0.4);
            expect(request.numerator_y==0.452);expect(request.denominator_y==0.17594999999999994);expect(request.display==true);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(0.22, 0.16, -0.39, 0.72, 0.5975999999999999, 0.17, -0.03));check_box(request.denominator,make_box(0.18, -0.4, 0.13, 1.23, 1.4022, -0.06, 0.21));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(0.6964619999999998, 0.6916, 0.0728, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.10159199999999999,0.452);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.015135999999999955,0);else expect(false);
            check_point(actual.denominator_origin,0.15100399999999992,0.17594999999999994);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.2337359999999999,0.17594999999999994);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==0.22);expect(actual.denominator_wrap_width==0.18);
        }
        { // Paired Source observation 250: ruled, display, center/left, delims=false
            FractionInput in{make_box(-0.31, 0.27, 0.12, 0.72, 0.5975999999999999, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1.23, 1.4022, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,-0.03,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(0.44, 0.8413520000000001, 1.405122, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.243904,0.68);
            check_point(actual.denominator_origin,0.009670000000000012,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0.12);expect(actual.rule->y==0.265);expect(actual.rule->width==0.06255200000000002);expect(actual.rule->height==-0.03);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0);else expect(false);
            expect(actual.numerator_wrap_width==0);expect(actual.denominator_wrap_width==0);
        }
        { // Paired Source observation 251: atop, display, center/left, delims=false
            FractionInput in{make_box(-0.31, 0.27, 0.12, 0.72, 0.5975999999999999, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1.23, 1.4022, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0.24, 0.8413520000000001, 1.405122, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.14390399999999998,0.68);
            check_point(actual.denominator_origin,-0.09033,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0);else expect(false);
            expect(actual.numerator_wrap_width==0);expect(actual.denominator_wrap_width==0);
        }
        { // Paired Source observation 252: bevelled, display, center/left, delims=false
            FractionInput in{make_box(-0.31, 0.27, 0.12, 0.72, 0.5975999999999999, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1.23, 1.4022, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0.06,FractionAlignment::center,FractionAlignment::left};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==1.1567);expect(request.delta==0.4);
            expect(request.numerator_y==0.5960000000000001);expect(request.denominator_y==0.29894999999999994);expect(request.display==true);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(-0.31, 0.27, 0.12, 0.72, 0.5975999999999999, -0.08, 0.16));check_box(request.denominator,make_box(-0.48, -0.22, 0.51, 1.23, 1.4022, -0.15, -0.04));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(0.21859999999999996, 0.757352, 0.416172, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.047807999999999996,0.5960000000000001);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,-0.33744799999999997,0);else expect(false);
            check_point(actual.denominator_origin,-0.327778,0.29894999999999994);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.01859999999999995,0.29894999999999994);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==-0.31);expect(actual.denominator_wrap_width==-0.48);
        }
        { // Paired Source observation 253: ruled, display, center/left, delims=true
            FractionInput in{make_box(0.46, 0.31, 0.08, 0.72, 0.5975999999999999, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1.23, 1.4022, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0.06,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(1.896662, 0.865256, 1.11066, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.810883,0.68);
            check_point(actual.denominator_origin,0.212176,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0);expect(actual.rule->y==0.22);expect(actual.rule->width==1.896662);expect(actual.rule->height==0.06);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==1.6966620000000001);else expect(false);
            expect(actual.numerator_wrap_width==1.6966620000000001);expect(actual.denominator_wrap_width==1.6966620000000001);
        }
        { // Paired Source observation 254: atop, display, center/left, delims=true
            FractionInput in{make_box(0.46, 0.31, 0.08, 0.72, 0.5975999999999999, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1.23, 1.4022, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(1.6966620000000001, 0.865256, 1.11066, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.710883,0.68);
            check_point(actual.denominator_origin,0.112176,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==1.6966620000000001);else expect(false);
            expect(actual.numerator_wrap_width==1.6966620000000001);expect(actual.denominator_wrap_width==1.6966620000000001);
        }
        { // Paired Source observation 255: bevelled, display, center/left, delims=true
            FractionInput in{make_box(0.46, 0.31, 0.08, 0.72, 0.5975999999999999, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1.23, 1.4022, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0.06,FractionAlignment::center,FractionAlignment::left};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==1.292);expect(request.delta==0.4);
            expect(request.numerator_y==0.5672);expect(request.denominator_y==-0.027000000000000024);expect(request.display==true);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(0.46, 0.31, 0.08, 0.72, 0.5975999999999999, 0.04, 0.02));check_box(request.denominator,make_box(1.08, 0.1, 0.3, 1.23, 1.4022, 0.08, 0.05));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(2.026014, 0.752456, 0.44766, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.023903999999999998,0.5672);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.11075199999999996,0);else expect(false);
            check_point(actual.denominator_origin,0.4429279999999999,-0.027000000000000024);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.3293519999999999,-0.027000000000000024);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==0.46);expect(actual.denominator_wrap_width==1.08);
        }
        { // Paired Source observation 256: ruled, display, center/left, delims=true
            FractionInput in{make_box(1.35, 1.05, 0.24, 0.72, 0.5975999999999999, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1.23, 1.4022, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0.17,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(1.054568, 1.615904, 2.2744859999999996, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.12390399999999999,0.9884240000000001);
            check_point(actual.denominator_origin,0.142066,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0);expect(actual.rule->y==0.16499999999999998);expect(actual.rule->width==1.054568);expect(actual.rule->height==0.17);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0.854568);else expect(false);
            expect(actual.numerator_wrap_width==0.854568);expect(actual.denominator_wrap_width==0.854568);
        }
        { // Paired Source observation 257: atop, display, center/left, delims=true
            FractionInput in{make_box(1.35, 1.05, 0.24, 0.72, 0.5975999999999999, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1.23, 1.4022, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0.8545679999999999, 1.30748, 2.2744859999999996, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.02390399999999998,0.68);
            check_point(actual.denominator_origin,0.04206599999999999,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0.8545679999999999);else expect(false);
            expect(actual.numerator_wrap_width==0.8545679999999999);expect(actual.denominator_wrap_width==0.8545679999999999);
        }
        { // Paired Source observation 258: bevelled, display, center/left, delims=true
            FractionInput in{make_box(1.35, 1.05, 0.24, 0.72, 0.5975999999999999, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1.23, 1.4022, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0.06,FractionAlignment::center,FractionAlignment::left};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==2.4728);expect(request.delta==0.4);
            expect(request.numerator_y==0.3584);expect(request.denominator_y==0.40349999999999997);expect(request.display==true);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(1.35, 1.05, 0.24, 0.72, 0.5975999999999999, -0.11, 0.19));check_box(request.denominator,make_box(0.51, 0.23, 1.13, 1.23, 1.4022, 0.03, -0.02));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(1.602312, 0.9858799999999999, 1.1809859999999996, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.06573599999999999,0.3584);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.6545679999999998,0);else expect(false);
            check_point(actual.denominator_origin,0.9166339999999998,0.40349999999999997);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.8731679999999999,0.40349999999999997);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==1.35);expect(actual.denominator_wrap_width==0.51);
        }
        { // Paired Source observation 259: ruled, display, center/left, delims=true
            FractionInput in{make_box(0.22, 0.16, -0.39, 0.72, 0.5975999999999999, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1.23, 1.4022, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0.025,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(0.6627259999999999, 0.7756160000000001, 0.8722859999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.26562699999999995,0.68);
            check_point(actual.denominator_origin,0.01586800000000002,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0);expect(actual.rule->y==0.2375);expect(actual.rule->width==0.6627259999999999);expect(actual.rule->height==0.025);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0.4627259999999999);else expect(false);
            expect(actual.numerator_wrap_width==0.4627259999999999);expect(actual.denominator_wrap_width==0.4627259999999999);
        }
        { // Paired Source observation 260: atop, display, center/left, delims=true
            FractionInput in{make_box(0.22, 0.16, -0.39, 0.72, 0.5975999999999999, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1.23, 1.4022, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0.4627259999999999, 0.7756160000000001, 0.8722859999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.16562699999999997,0.68);
            check_point(actual.denominator_origin,-0.08413199999999998,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0.4627259999999999);else expect(false);
            expect(actual.numerator_wrap_width==0.4627259999999999);expect(actual.denominator_wrap_width==0.4627259999999999);
        }
        { // Paired Source observation 261: bevelled, display, center/left, delims=true
            FractionInput in{make_box(0.22, 0.16, -0.39, 0.72, 0.5975999999999999, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1.23, 1.4022, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0.06,FractionAlignment::center,FractionAlignment::left};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==0.6344000000000001);expect(request.delta==0.4);
            expect(request.numerator_y==0.452);expect(request.denominator_y==0.17594999999999994);expect(request.display==true);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(0.22, 0.16, -0.39, 0.72, 0.5975999999999999, 0.17, -0.03));check_box(request.denominator,make_box(0.18, -0.4, 0.13, 1.23, 1.4022, -0.06, 0.21));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(0.6964619999999998, 0.6916, 0.0728, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.10159199999999999,0.452);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.015135999999999955,0);else expect(false);
            check_point(actual.denominator_origin,0.15100399999999992,0.17594999999999994);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.2337359999999999,0.17594999999999994);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==0.22);expect(actual.denominator_wrap_width==0.18);
        }
        { // Paired Source observation 262: ruled, display, center/left, delims=true
            FractionInput in{make_box(-0.31, 0.27, 0.12, 0.72, 0.5975999999999999, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1.23, 1.4022, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,-0.03,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(0.2, 0.8413520000000001, 1.405122, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.123904,0.68);
            check_point(actual.denominator_origin,-0.11032999999999998,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0);expect(actual.rule->y==0.265);expect(actual.rule->width==0.06255200000000002);expect(actual.rule->height==-0.03);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0);else expect(false);
            expect(actual.numerator_wrap_width==0);expect(actual.denominator_wrap_width==0);
        }
        { // Paired Source observation 263: atop, display, center/left, delims=true
            FractionInput in{make_box(-0.31, 0.27, 0.12, 0.72, 0.5975999999999999, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1.23, 1.4022, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0,FractionAlignment::center,FractionAlignment::left};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0, 0.8413520000000001, 1.405122, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.023903999999999995,0.68);
            check_point(actual.denominator_origin,-0.21033,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0);else expect(false);
            expect(actual.numerator_wrap_width==0);expect(actual.denominator_wrap_width==0);
        }
        { // Paired Source observation 264: bevelled, display, center/left, delims=true
            FractionInput in{make_box(-0.31, 0.27, 0.12, 0.72, 0.5975999999999999, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1.23, 1.4022, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0.06,FractionAlignment::center,FractionAlignment::left};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==1.1567);expect(request.delta==0.4);
            expect(request.numerator_y==0.5960000000000001);expect(request.denominator_y==0.29894999999999994);expect(request.display==true);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(-0.31, 0.27, 0.12, 0.72, 0.5975999999999999, -0.08, 0.16));check_box(request.denominator,make_box(-0.48, -0.22, 0.51, 1.23, 1.4022, -0.15, -0.04));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(0.21859999999999996, 0.757352, 0.416172, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.047807999999999996,0.5960000000000001);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,-0.33744799999999997,0);else expect(false);
            check_point(actual.denominator_origin,-0.327778,0.29894999999999994);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.01859999999999995,0.29894999999999994);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==-0.31);expect(actual.denominator_wrap_width==-0.48);
        }
        { // Paired Source observation 265: ruled, display, right/center, delims=false
            FractionInput in{make_box(0.46, 0.31, 0.08, 0.72, 0.5975999999999999, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1.23, 1.4022, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0.06,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(2.1366620000000003, 0.865256, 1.11066, 1, 1, 0, 0));
            check_point(actual.numerator_origin,1.629814,0.68);
            check_point(actual.denominator_origin,0.31114300000000006,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0.12);expect(actual.rule->y==0.22);expect(actual.rule->width==1.896662);expect(actual.rule->height==0.06);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==1.6966620000000003);else expect(false);
            expect(actual.numerator_wrap_width==1.6966620000000003);expect(actual.denominator_wrap_width==1.6966620000000003);
        }
        { // Paired Source observation 266: atop, display, right/center, delims=false
            FractionInput in{make_box(0.46, 0.31, 0.08, 0.72, 0.5975999999999999, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1.23, 1.4022, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(1.936662, 0.865256, 1.11066, 1, 1, 0, 0));
            check_point(actual.numerator_origin,1.529814,0.68);
            check_point(actual.denominator_origin,0.21114300000000008,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==1.6966620000000001);else expect(false);
            expect(actual.numerator_wrap_width==1.6966620000000001);expect(actual.denominator_wrap_width==1.6966620000000001);
        }
        { // Paired Source observation 267: bevelled, display, right/center, delims=false
            FractionInput in{make_box(0.46, 0.31, 0.08, 0.72, 0.5975999999999999, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1.23, 1.4022, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0.06,FractionAlignment::right,FractionAlignment::center};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==1.292);expect(request.delta==0.4);
            expect(request.numerator_y==0.5672);expect(request.denominator_y==-0.027000000000000024);expect(request.display==true);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(0.46, 0.31, 0.08, 0.72, 0.5975999999999999, 0.04, 0.02));check_box(request.denominator,make_box(1.08, 0.1, 0.3, 1.23, 1.4022, 0.08, 0.05));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(2.026014, 0.752456, 0.44766, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.023903999999999998,0.5672);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.11075199999999996,0);else expect(false);
            check_point(actual.denominator_origin,0.4429279999999999,-0.027000000000000024);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.3293519999999999,-0.027000000000000024);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==0.46);expect(actual.denominator_wrap_width==1.08);
        }
        { // Paired Source observation 268: ruled, display, right/center, delims=false
            FractionInput in{make_box(1.35, 1.05, 0.24, 0.72, 0.5975999999999999, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1.23, 1.4022, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0.17,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(1.294568, 1.615904, 2.2744859999999996, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.15426399999999998,0.9884240000000001);
            check_point(actual.denominator_origin,0.28972299999999995,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0.12);expect(actual.rule->y==0.16499999999999998);expect(actual.rule->width==1.054568);expect(actual.rule->height==0.17);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0.854568);else expect(false);
            expect(actual.numerator_wrap_width==0.854568);expect(actual.denominator_wrap_width==0.854568);
        }
        { // Paired Source observation 269: atop, display, right/center, delims=false
            FractionInput in{make_box(1.35, 1.05, 0.24, 0.72, 0.5975999999999999, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1.23, 1.4022, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(1.0945679999999998, 1.30748, 2.2744859999999996, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.05426399999999998,0.68);
            check_point(actual.denominator_origin,0.18972299999999997,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0.8545679999999998);else expect(false);
            expect(actual.numerator_wrap_width==0.8545679999999998);expect(actual.denominator_wrap_width==0.8545679999999998);
        }
        { // Paired Source observation 270: bevelled, display, right/center, delims=false
            FractionInput in{make_box(1.35, 1.05, 0.24, 0.72, 0.5975999999999999, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1.23, 1.4022, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0.06,FractionAlignment::right,FractionAlignment::center};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==2.4728);expect(request.delta==0.4);
            expect(request.numerator_y==0.3584);expect(request.denominator_y==0.40349999999999997);expect(request.display==true);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(1.35, 1.05, 0.24, 0.72, 0.5975999999999999, -0.11, 0.19));check_box(request.denominator,make_box(0.51, 0.23, 1.13, 1.23, 1.4022, 0.03, -0.02));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(1.602312, 0.9858799999999999, 1.1809859999999996, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.06573599999999999,0.3584);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.6545679999999998,0);else expect(false);
            check_point(actual.denominator_origin,0.9166339999999998,0.40349999999999997);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.8731679999999999,0.40349999999999997);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==1.35);expect(actual.denominator_wrap_width==0.51);
        }
        { // Paired Source observation 271: ruled, display, right/center, delims=false
            FractionInput in{make_box(0.22, 0.16, -0.39, 0.72, 0.5975999999999999, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1.23, 1.4022, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0.025,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(0.9027259999999999, 0.7756160000000001, 0.8722859999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.569182,0.68);
            check_point(actual.denominator_origin,0.325165,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0.12);expect(actual.rule->y==0.2375);expect(actual.rule->width==0.6627259999999999);expect(actual.rule->height==0.025);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0.4627259999999999);else expect(false);
            expect(actual.numerator_wrap_width==0.46272599999999997);expect(actual.denominator_wrap_width==0.46272599999999997);
        }
        { // Paired Source observation 272: atop, display, right/center, delims=false
            FractionInput in{make_box(0.22, 0.16, -0.39, 0.72, 0.5975999999999999, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1.23, 1.4022, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0.702726, 0.7756160000000001, 0.8722859999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.46918199999999993,0.68);
            check_point(actual.denominator_origin,0.22516499999999998,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0.46272599999999997);else expect(false);
            expect(actual.numerator_wrap_width==0.46272599999999997);expect(actual.denominator_wrap_width==0.46272599999999997);
        }
        { // Paired Source observation 273: bevelled, display, right/center, delims=false
            FractionInput in{make_box(0.22, 0.16, -0.39, 0.72, 0.5975999999999999, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1.23, 1.4022, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0.06,FractionAlignment::right,FractionAlignment::center};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==0.6344000000000001);expect(request.delta==0.4);
            expect(request.numerator_y==0.452);expect(request.denominator_y==0.17594999999999994);expect(request.display==true);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(0.22, 0.16, -0.39, 0.72, 0.5975999999999999, 0.17, -0.03));check_box(request.denominator,make_box(0.18, -0.4, 0.13, 1.23, 1.4022, -0.06, 0.21));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(0.6964619999999998, 0.6916, 0.0728, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.10159199999999999,0.452);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.015135999999999955,0);else expect(false);
            check_point(actual.denominator_origin,0.15100399999999992,0.17594999999999994);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.2337359999999999,0.17594999999999994);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==0.22);expect(actual.denominator_wrap_width==0.18);
        }
        { // Paired Source observation 274: ruled, display, right/center, delims=false
            FractionInput in{make_box(-0.31, 0.27, 0.12, 0.72, 0.5975999999999999, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1.23, 1.4022, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,-0.03,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(0.44, 0.8413520000000001, 1.405122, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.172192,0.68);
            check_point(actual.denominator_origin,0.4878039999999999,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0.12);expect(actual.rule->y==0.265);expect(actual.rule->width==0.06255200000000002);expect(actual.rule->height==-0.03);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0);else expect(false);
            expect(actual.numerator_wrap_width==0);expect(actual.denominator_wrap_width==0);
        }
        { // Paired Source observation 275: atop, display, right/center, delims=false
            FractionInput in{make_box(-0.31, 0.27, 0.12, 0.72, 0.5975999999999999, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1.23, 1.4022, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0.24, 0.8413520000000001, 1.405122, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.07219199999999999,0.68);
            check_point(actual.denominator_origin,0.3878039999999999,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0);else expect(false);
            expect(actual.numerator_wrap_width==0);expect(actual.denominator_wrap_width==0);
        }
        { // Paired Source observation 276: bevelled, display, right/center, delims=false
            FractionInput in{make_box(-0.31, 0.27, 0.12, 0.72, 0.5975999999999999, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1.23, 1.4022, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,false,0.06,FractionAlignment::right,FractionAlignment::center};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==1.1567);expect(request.delta==0.4);
            expect(request.numerator_y==0.5960000000000001);expect(request.denominator_y==0.29894999999999994);expect(request.display==true);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(-0.31, 0.27, 0.12, 0.72, 0.5975999999999999, -0.08, 0.16));check_box(request.denominator,make_box(-0.48, -0.22, 0.51, 1.23, 1.4022, -0.15, -0.04));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(0.21859999999999996, 0.757352, 0.416172, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.047807999999999996,0.5960000000000001);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,-0.33744799999999997,0);else expect(false);
            check_point(actual.denominator_origin,-0.327778,0.29894999999999994);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.01859999999999995,0.29894999999999994);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==-0.31);expect(actual.denominator_wrap_width==-0.48);
        }
        { // Paired Source observation 277: ruled, display, right/center, delims=true
            FractionInput in{make_box(0.46, 0.31, 0.08, 0.72, 0.5975999999999999, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1.23, 1.4022, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0.06,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(1.896662, 0.865256, 1.11066, 1, 1, 0, 0));
            check_point(actual.numerator_origin,1.5098140000000002,0.68);
            check_point(actual.denominator_origin,0.1911430000000001,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0);expect(actual.rule->y==0.22);expect(actual.rule->width==1.896662);expect(actual.rule->height==0.06);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==1.6966620000000001);else expect(false);
            expect(actual.numerator_wrap_width==1.6966620000000001);expect(actual.denominator_wrap_width==1.6966620000000001);
        }
        { // Paired Source observation 278: atop, display, right/center, delims=true
            FractionInput in{make_box(0.46, 0.31, 0.08, 0.72, 0.5975999999999999, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1.23, 1.4022, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(1.6966620000000001, 0.865256, 1.11066, 1, 1, 0, 0));
            check_point(actual.numerator_origin,1.4098140000000001,0.68);
            check_point(actual.denominator_origin,0.09114300000000009,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==1.6966620000000001);else expect(false);
            expect(actual.numerator_wrap_width==1.6966620000000001);expect(actual.denominator_wrap_width==1.6966620000000001);
        }
        { // Paired Source observation 279: bevelled, display, right/center, delims=true
            FractionInput in{make_box(0.46, 0.31, 0.08, 0.72, 0.5975999999999999, 0.04, 0.02),make_box(1.08, 0.1, 0.3, 1.23, 1.4022, 0.08, 0.05),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0.06,FractionAlignment::right,FractionAlignment::center};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==1.292);expect(request.delta==0.4);
            expect(request.numerator_y==0.5672);expect(request.denominator_y==-0.027000000000000024);expect(request.display==true);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(0.46, 0.31, 0.08, 0.72, 0.5975999999999999, 0.04, 0.02));check_box(request.denominator,make_box(1.08, 0.1, 0.3, 1.23, 1.4022, 0.08, 0.05));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(2.026014, 0.752456, 0.44766, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.023903999999999998,0.5672);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.11075199999999996,0);else expect(false);
            check_point(actual.denominator_origin,0.4429279999999999,-0.027000000000000024);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.3293519999999999,-0.027000000000000024);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==0.46);expect(actual.denominator_wrap_width==1.08);
        }
        { // Paired Source observation 280: ruled, display, right/center, delims=true
            FractionInput in{make_box(1.35, 1.05, 0.24, 0.72, 0.5975999999999999, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1.23, 1.4022, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0.17,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(1.054568, 1.615904, 2.2744859999999996, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.03426399999999999,0.9884240000000001);
            check_point(actual.denominator_origin,0.16972299999999998,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0);expect(actual.rule->y==0.16499999999999998);expect(actual.rule->width==1.054568);expect(actual.rule->height==0.17);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0.854568);else expect(false);
            expect(actual.numerator_wrap_width==0.854568);expect(actual.denominator_wrap_width==0.854568);
        }
        { // Paired Source observation 281: atop, display, right/center, delims=true
            FractionInput in{make_box(1.35, 1.05, 0.24, 0.72, 0.5975999999999999, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1.23, 1.4022, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0.8545679999999999, 1.30748, 2.2744859999999996, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.06573600000000002,0.68);
            check_point(actual.denominator_origin,0.06972299999999998,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0.8545679999999999);else expect(false);
            expect(actual.numerator_wrap_width==0.8545679999999999);expect(actual.denominator_wrap_width==0.8545679999999999);
        }
        { // Paired Source observation 282: bevelled, display, right/center, delims=true
            FractionInput in{make_box(1.35, 1.05, 0.24, 0.72, 0.5975999999999999, -0.11, 0.19),make_box(0.51, 0.23, 1.13, 1.23, 1.4022, 0.03, -0.02),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0.06,FractionAlignment::right,FractionAlignment::center};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==2.4728);expect(request.delta==0.4);
            expect(request.numerator_y==0.3584);expect(request.denominator_y==0.40349999999999997);expect(request.display==true);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(1.35, 1.05, 0.24, 0.72, 0.5975999999999999, -0.11, 0.19));check_box(request.denominator,make_box(0.51, 0.23, 1.13, 1.23, 1.4022, 0.03, -0.02));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(1.602312, 0.9858799999999999, 1.1809859999999996, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.06573599999999999,0.3584);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.6545679999999998,0);else expect(false);
            check_point(actual.denominator_origin,0.9166339999999998,0.40349999999999997);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.8731679999999999,0.40349999999999997);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==1.35);expect(actual.denominator_wrap_width==0.51);
        }
        { // Paired Source observation 283: ruled, display, right/center, delims=true
            FractionInput in{make_box(0.22, 0.16, -0.39, 0.72, 0.5975999999999999, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1.23, 1.4022, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0.025,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(0.6627259999999999, 0.7756160000000001, 0.8722859999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.44918199999999997,0.68);
            check_point(actual.denominator_origin,0.205165,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0);expect(actual.rule->y==0.2375);expect(actual.rule->width==0.6627259999999999);expect(actual.rule->height==0.025);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0.4627259999999999);else expect(false);
            expect(actual.numerator_wrap_width==0.4627259999999999);expect(actual.denominator_wrap_width==0.4627259999999999);
        }
        { // Paired Source observation 284: atop, display, right/center, delims=true
            FractionInput in{make_box(0.22, 0.16, -0.39, 0.72, 0.5975999999999999, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1.23, 1.4022, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0.4627259999999999, 0.7756160000000001, 0.8722859999999999, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.34918199999999994,0.68);
            check_point(actual.denominator_origin,0.10516499999999998,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0.4627259999999999);else expect(false);
            expect(actual.numerator_wrap_width==0.4627259999999999);expect(actual.denominator_wrap_width==0.4627259999999999);
        }
        { // Paired Source observation 285: bevelled, display, right/center, delims=true
            FractionInput in{make_box(0.22, 0.16, -0.39, 0.72, 0.5975999999999999, 0.17, -0.03),make_box(0.18, -0.4, 0.13, 1.23, 1.4022, -0.06, 0.21),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0.06,FractionAlignment::right,FractionAlignment::center};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==0.6344000000000001);expect(request.delta==0.4);
            expect(request.numerator_y==0.452);expect(request.denominator_y==0.17594999999999994);expect(request.display==true);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(0.22, 0.16, -0.39, 0.72, 0.5975999999999999, 0.17, -0.03));check_box(request.denominator,make_box(0.18, -0.4, 0.13, 1.23, 1.4022, -0.06, 0.21));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(0.6964619999999998, 0.6916, 0.0728, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.10159199999999999,0.452);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,0.015135999999999955,0);else expect(false);
            check_point(actual.denominator_origin,0.15100399999999992,0.17594999999999994);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.2337359999999999,0.17594999999999994);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==0.22);expect(actual.denominator_wrap_width==0.18);
        }
        { // Paired Source observation 286: ruled, display, right/center, delims=true
            FractionInput in{make_box(-0.31, 0.27, 0.12, 0.72, 0.5975999999999999, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1.23, 1.4022, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,-0.03,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::ruled);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::ruled);
            check_box(actual.bbox,make_box(0.2, 0.8413520000000001, 1.405122, 1, 1, 0, 0));
            check_point(actual.numerator_origin,0.052192,0.68);
            check_point(actual.denominator_origin,0.3678039999999999,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            if(actual.rule.has_value()){
                expect(actual.rule->x==0);expect(actual.rule->y==0.265);expect(actual.rule->width==0.06255200000000002);expect(actual.rule->height==-0.03);
            }else expect(false);
            if(actual.content_width.has_value())expect(*actual.content_width==0);else expect(false);
            expect(actual.numerator_wrap_width==0);expect(actual.denominator_wrap_width==0);
        }
        { // Paired Source observation 287: atop, display, right/center, delims=true
            FractionInput in{make_box(-0.31, 0.27, 0.12, 0.72, 0.5975999999999999, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1.23, 1.4022, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0,FractionAlignment::right,FractionAlignment::center};
            auto measured=measure_fraction(in,FractionKind::atop);expect(std::holds_alternative<FractionLayout>(measured));
            const auto& actual=std::get<FractionLayout>(measured);expect(actual.kind==FractionKind::atop);
            check_box(actual.bbox,make_box(0, 0.8413520000000001, 1.405122, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.047808,0.68);
            check_point(actual.denominator_origin,0.26780399999999993,-0.69);
            expect(!actual.slash_origin.has_value());expect(!actual.bbox_denominator_origin.has_value());
            expect(!actual.rule.has_value());
            if(actual.content_width.has_value())expect(*actual.content_width==0);else expect(false);
            expect(actual.numerator_wrap_width==0);expect(actual.denominator_wrap_width==0);
        }
        { // Paired Source observation 288: bevelled, display, right/center, delims=true
            FractionInput in{make_box(-0.31, 0.27, 0.12, 0.72, 0.5975999999999999, -0.08, 0.16),make_box(-0.48, -0.22, 0.51, 1.23, 1.4022, -0.15, -0.04),FractionFontParameters{0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12},true,true,0.06,FractionAlignment::right,FractionAlignment::center};
            auto pending=measure_fraction(in,FractionKind::bevelled);
            expect(std::holds_alternative<BevelRequest>(pending));
            const auto& request=std::get<BevelRequest>(pending);
            expect(request.required_slash_height==1.1567);expect(request.delta==0.4);
            expect(request.numerator_y==0.5960000000000001);expect(request.denominator_y==0.29894999999999994);expect(request.display==true);
            check_font(request.font,0.25,0.68,0.39,0.52,0.69,0.35,0.06,0.12);check_box(request.numerator,make_box(-0.31, 0.27, 0.12, 0.72, 0.5975999999999999, -0.08, 0.16));check_box(request.denominator,make_box(-0.48, -0.22, 0.51, 1.23, 1.4022, -0.15, -0.04));
            auto actual=finish_bevelled_fraction(request,make_box(0.42, 0.76, 0.08, 1, 0.91, 0.03, 0.01));expect(actual.kind==FractionKind::bevelled);
            check_box(actual.bbox,make_box(0.21859999999999996, 0.757352, 0.416172, 1, 1, 0, 0));
            check_point(actual.numerator_origin,-0.047807999999999996,0.5960000000000001);
            if(actual.slash_origin.has_value())check_point(*actual.slash_origin,-0.33744799999999997,0);else expect(false);
            check_point(actual.denominator_origin,-0.327778,0.29894999999999994);
            if(actual.bbox_denominator_origin.has_value())check_point(*actual.bbox_denominator_origin,0.01859999999999995,0.29894999999999994);else expect(false);
            expect(!actual.rule.has_value());
            expect(!actual.content_width.has_value());
            expect(actual.numerator_wrap_width==-0.31);expect(actual.denominator_wrap_width==-0.48);
        }
    };
};
