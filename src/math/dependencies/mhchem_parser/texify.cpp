// Native port of mhchemparser 4.2.1 src/mhchemParser.ts: _mhchemTexify.
// Copyright (c) 2015-2023 Martin Hensel. Apache-2.0.
module folia.math.dependencies.mhchem_parser;
import std;

namespace folia::math::mhchem {
namespace {

[[noreturn]] void texifyBug() {
    throw Error(u"MhchemBugT", u"mhchem bug T. Please report.");
}

std::u16string arrowName(std::u16string_view kind) {
    if (kind == u"->" || kind == u"\u2192" || kind == u"\u27F6") return u"rightarrow";
    if (kind == u"<-") return u"leftarrow";
    if (kind == u"<->") return u"leftrightarrow";
    if (kind == u"<-->") return u"leftrightarrows";
    if (kind == u"<=>" || kind == u"\u21CC") return u"rightleftharpoons";
    if (kind == u"<=>>") return u"Rightleftharpoons";
    if (kind == u"<<=>") return u"Leftrightharpoons";
    texifyBug();
}

std::u16string bondText(std::u16string_view kind) {
    if (kind == u"-" || kind == u"1") return u"{-}";
    if (kind == u"=" || kind == u"2") return u"{=}";
    if (kind == u"#" || kind == u"3") return u"{\\equiv}";
    if (kind == u"~") return u"{\\tripledash}";
    if (kind == u"~-") return u"{\\rlap{\\lower.1em{-}}\\raise.1em{\\tripledash}}";
    if (kind == u"~=" || kind == u"~--") {
        return u"{\\rlap{\\lower.2em{-}}\\rlap{\\raise.2em{\\tripledash}}-}";
    }
    if (kind == u"-~-") return u"{\\rlap{\\lower.2em{-}}\\rlap{\\raise.2em{-}}\\tripledash}";
    if (kind == u"...") return u"{{\\cdot}{\\cdot}{\\cdot}}";
    if (kind == u"....") return u"{{\\cdot}{\\cdot}{\\cdot}{\\cdot}}";
    if (kind == u"->") return u"{\\rightarrow}";
    if (kind == u"<-") return u"{\\leftarrow}";
    if (kind == u"<") return u"{<}";
    if (kind == u">") return u"{>}";
    texifyBug();
}

std::u16string operatorText(std::u16string_view kind) {
    if (kind == u"+") return u" {}+{} ";
    if (kind == u"-") return u" {}-{} ";
    if (kind == u"=") return u" {}={} ";
    if (kind == u"<") return u" {}<{} ";
    if (kind == u">") return u" {}>{} ";
    if (kind == u"<<") return u" {}\\ll{} ";
    if (kind == u">>") return u" {}\\gg{} ";
    if (kind == u"\\pm") return u" {}\\pm{} ";
    if (kind == u"\\approx" || kind == u"$\\approx$") return u" {}\\approx{} ";
    if (kind == u"v" || kind == u"(v)") return u" \\downarrow{} ";
    if (kind == u"^" || kind == u"(^)") return u" \\uparrow{} ";
    texifyBug();
}

std::u16string texifyNode(Parsed& buf) {
    const auto& type = buf.type;
    if (type == u"chemfive") {
        std::u16string res;
        auto a = texify(buf.a, false);
        const auto b = texify(buf.b, false);
        const auto p = texify(buf.p, false);
        auto o = texify(buf.o, false);
        const auto q = texify(buf.q, false);
        const auto d = texify(buf.d, false);
        if (!a.empty()) {
            if (a.front() == u'+' || a.front() == u'-') a = u"{" + a + u"}";
            res += a + u"\\,";
        }
        if (!b.empty() || !p.empty()) {
            res += u"{\\vphantom{A}}";
            res += u"^{\\hphantom{" + b + u"}}_{\\hphantom{" + p + u"}}";
            res += u"\\mkern-1.5mu";
            res += u"{\\vphantom{A}}";
            res += u"^{\\smash[t]{\\vphantom{2}}\\llap{" + b + u"}}";
            res += u"_{\\vphantom{2}\\llap{\\smash[t]{" + p + u"}}}";
        }
        if (!o.empty()) {
            if (o.front() == u'+' || o.front() == u'-') o = u"{" + o + u"}";
            res += o;
        }
        if (buf.dType == u"kv") {
            if (!d.empty() || !q.empty()) res += u"{\\vphantom{A}}";
            if (!d.empty()) res += u"^{" + d + u"}";
            if (!q.empty()) res += u"_{\\smash[t]{" + q + u"}}";
        } else if (buf.dType == u"oxidation") {
            if (!d.empty()) {
                res += u"{\\vphantom{A}}";
                res += u"^{" + d + u"}";
            }
            if (!q.empty()) {
                res += u"{\\vphantom{A}}";
                res += u"_{\\smash[t]{" + q + u"}}";
            }
        } else {
            if (!q.empty()) {
                res += u"{\\vphantom{A}}";
                res += u"_{\\smash[t]{" + q + u"}}";
            }
            if (!d.empty()) {
                res += u"{\\vphantom{A}}";
                res += u"^{" + d + u"}";
            }
        }
        return res;
    }
    if (type == u"rm") return u"\\mathrm{" + buf.p1 + u"}";
    if (type == u"text") {
        if (buf.p1.find_first_of(u"^_") != std::u16string::npos) {
            auto& text = buf.p1;
            if (const auto space = text.find(u' '); space != std::u16string::npos) {
                text.replace(space, 1, u"~");
            }
            if (const auto hyphen = text.find(u'-'); hyphen != std::u16string::npos) {
                text.replace(hyphen, 1, u"\\text{-}");
            }
            return u"\\mathrm{" + text + u"}";
        }
        return u"\\text{" + buf.p1 + u"}";
    }
    if (type == u"roman numeral") return u"\\mathrm{" + buf.p1 + u"}";
    if (type == u"state of aggregation") return u"\\mskip2mu " + texify(buf.p1Nodes, false);
    if (type == u"state of aggregation subscript") return u"\\mskip1mu " + texify(buf.p1Nodes, false);
    if (type == u"bond") {
        auto res = bondText(buf.kind);
        if (res.empty()) {
            throw Error(u"MhchemErrorBond", u"mhchem Error. Unknown bond type (" + buf.kind + u")");
        }
        return res;
    }
    if (type == u"frac") {
        const auto fraction = u"\\frac{" + buf.p1 + u"}{" + buf.p2 + u"}";
        return u"\\mathchoice{\\textstyle" + fraction + u"}{" + fraction + u"}{" + fraction + u"}{" + fraction + u"}";
    }
    if (type == u"pu-frac") {
        const auto fraction = u"\\frac{" + texify(buf.p1Nodes, false) + u"}{" + texify(buf.p2Nodes, false) + u"}";
        return u"\\mathchoice{\\textstyle" + fraction + u"}{" + fraction + u"}{" + fraction + u"}{" + fraction + u"}";
    }
    if (type == u"tex-math") return buf.p1 + u" ";
    if (type == u"frac-ce") return u"\\frac{" + texify(buf.p1Nodes, false) + u"}{" + texify(buf.p2Nodes, false) + u"}";
    if (type == u"overset") return u"\\overset{" + texify(buf.p1Nodes, false) + u"}{" + texify(buf.p2Nodes, false) + u"}";
    if (type == u"underset") return u"\\underset{" + texify(buf.p1Nodes, false) + u"}{" + texify(buf.p2Nodes, false) + u"}";
    if (type == u"underbrace") return u"\\underbrace{" + texify(buf.p1Nodes, false) + u"}_{" + texify(buf.p2Nodes, false) + u"}";
    if (type == u"color") return u"{\\color{" + buf.color1 + u"}{" + texify(buf.color2, false) + u"}}";
    if (type == u"color0") return u"\\color{" + buf.color + u"}";
    if (type == u"arrow") {
        const auto rd = texify(buf.rd, false);
        const auto rq = texify(buf.rq, false);
        auto arrow = arrowName(buf.r);
        if (!rd.empty() || !rq.empty()) {
            if (buf.r == u"<=>" || buf.r == u"<=>>" || buf.r == u"<<=>" || buf.r == u"<-->") {
                arrow = u"\\long" + arrow;
                if (!rd.empty()) arrow = u"\\overset{" + rd + u"}{" + arrow + u"}";
                if (!rq.empty()) {
                    if (buf.r == u"<-->") {
                        arrow = u"\\underset{\\lower2mu{" + rq + u"}}{" + arrow + u"}";
                    } else {
                        arrow = u"\\underset{\\lower6mu{" + rq + u"}}{" + arrow + u"}";
                    }
                }
                arrow = u" {}\\mathrel{" + arrow + u"}{} ";
            } else {
                if (!rq.empty()) arrow += u"[{" + rq + u"}]";
                arrow += u"{" + rd + u"}";
                arrow = u" {}\\mathrel{\\x" + arrow + u"}{} ";
            }
        } else {
            arrow = u" {}\\mathrel{\\long" + arrow + u"}{} ";
        }
        return arrow;
    }
    if (type == u"operator") return operatorText(buf.kind);
    if (type == u"1st-level escape") return buf.p1 + u" ";
    if (type == u"space") return u" ";
    if (type == u"tinySkip") return u"\\mkern2mu";
    if (type == u"entitySkip") return u"~";
    if (type == u"pu-space-1") return u"~";
    if (type == u"pu-space-2") return u"\\mkern3mu ";
    if (type == u"1000 separator") return u"\\mkern2mu ";
    if (type == u"commaDecimal") return u"{,}";
    if (type == u"comma enumeration L") return u"{" + buf.p1 + u"}\\mkern6mu ";
    if (type == u"comma enumeration M") return u"{" + buf.p1 + u"}\\mkern3mu ";
    if (type == u"comma enumeration S") return u"{" + buf.p1 + u"}\\mkern1mu ";
    if (type == u"hyphen") return u"\\text{-}";
    if (type == u"addition compound") return u"\\,{\\cdot}\\,";
    if (type == u"electron dot") return u"\\mkern1mu \\bullet\\mkern1mu ";
    if (type == u"KV x") return u"{\\times}";
    if (type == u"prime") return u"\\prime ";
    if (type == u"cdot") return u"\\cdot ";
    if (type == u"tight cdot") return u"\\mkern1mu{\\cdot}\\mkern1mu ";
    if (type == u"times") return u"\\times ";
    if (type == u"circa") return u"{\\sim}";
    if (type == u"^") return u"uparrow";
    if (type == u"v") return u"downarrow";
    if (type == u"ellipsis") return u"\\ldots ";
    if (type == u"/") return u"/";
    if (type == u" / ") return u"\\,/\\,";
    texifyBug();
}

} // namespace

std::u16string texify(Output& input, bool addOuterBraces) {
    std::u16string res;
    bool cee = false;
    for (auto& item : input) {
        if (item.raw) {
            res += item.text;
        } else {
            res += texifyNode(item);
            if (item.type == u"1st-level escape") cee = true;
        }
    }
    if (addOuterBraces && !cee && !res.empty()) res = u"{" + res + u"}";
    return res;
}

} // namespace folia::math::mhchem
