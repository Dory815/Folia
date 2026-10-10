/*************************************************************
 *
 * Copyright (c) 2017-2026 The MathJax Consortium
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Native port of MathJax 4.1.3 ts/util/BBox.ts.
 * Source SHA256: a4232fac0fce35a15ad05a4dbb4af0999bf04f90ec3f0fe0c594b22c5da70b5b
 *************************************************************/
export module folia.math.util.bbox;
import std;
import folia.math.util.constants;

export namespace folia::math {
    struct BBoxData {
        std::optional<double> w;
        std::optional<double> h;
        std::optional<double> d;
    };

    struct BBoxSide {
        std::string_view name;
        int index;
        std::string_view dimension;
    };

    class BBox {
    public:
        inline static constexpr std::string_view fullWidth = "100%";
        inline static constexpr std::array<BBoxSide, 4> boxSides{{
            {"Top", 0, "h"},
            {"Right", 1, "w"},
            {"Bottom", 2, "d"},
            {"Left", 3, "w"},
        }};

        double w;
        double h;
        double d;
        double scale;
        double rscale; // scale relative to the parent's scale
        double L;      // extra space on the left
        double R;      // extra space on the right
        std::string pwidth; // percentage width (for tables)
        double ic;     // italic correction
        double oc;     // alternate italic correction for -tex-mit variant
        double sk;     // skew
        double dx;     // offset for combining characters as accents

        BBox() : BBox(BBoxData{}) {}

        explicit BBox(BBoxData def)
            : w(def.w && !std::isnan(*def.w) && *def.w != 0.0 ? *def.w : 0.0),
              h(def.h ? *def.h : -BIGDIMEN),
              d(def.d ? *def.d : -BIGDIMEN),
              scale(1), rscale(1), L(0), R(0), pwidth(), ic(0), oc(0), sk(0), dx(0) {}

        static BBox zero() {
            return BBox(BBoxData{.w = 0.0, .h = 0.0, .d = 0.0});
        }

        // C++ cannot overload the static and instance empty() signatures.
        static BBox makeEmpty() {
            return BBox();
        }

        BBox& empty() {
            w = 0;
            h = d = -BIGDIMEN;
            return *this;
        }

        void clean() {
            if (w == -BIGDIMEN) w = 0;
            if (h == -BIGDIMEN) h = 0;
            if (d == -BIGDIMEN) d = 0;
        }

        void rescale(double factor) {
            w *= factor;
            h *= factor;
            d *= factor;
        }

        void combine(BBox const& cbox, double x = 0, double y = 0) {
            double const child_rscale = cbox.rscale;
            double const child_w = x + child_rscale * (cbox.w + cbox.L + cbox.R);
            double const child_h = y + child_rscale * cbox.h;
            double const child_d = child_rscale * cbox.d - y;
            if (child_w > w) w = child_w;
            if (child_h > h) h = child_h;
            if (child_d > d) d = child_d;
        }

        void append(BBox const& cbox) {
            double const child_scale = cbox.rscale;
            w += child_scale * (cbox.w + cbox.L + cbox.R);
            double const child_h = child_scale * cbox.h;
            if (child_h > h) h = child_h;
            double const child_d = child_scale * cbox.d;
            if (child_d > d) d = child_d;
        }

        void updateFrom(BBox const& cbox) {
            h = cbox.h;
            d = cbox.d;
            w = cbox.w;
            if (!cbox.pwidth.empty()) pwidth = cbox.pwidth;
        }

        BBox copy() const {
            return *this;
        }
    };
}
