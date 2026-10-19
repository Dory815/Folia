/*************************************************************
 * MathJax 4.1.3 output/common/Wrappers/mtable.ts.
 *************************************************************/
module folia.math.output.common;
import :mtable;
import :wrapper;
import :mtr;
import folia.math.core.mml_tree.mml_nodes.mtable;
import folia.math.util.property_host;
import folia.math.util.string;
import folia.math.util.js_numeric_input;
import std;
namespace folia::math::output::common {
namespace {
std::vector<std::string> words(std::u16string_view s){
    std::vector<std::string> out;
    for(const auto& word:string_util::split(s))out.push_back(property_host::debugText(word));
    return out;
}
bool isPercentText(std::string_view s){return !s.empty()&&s.back()=='%';}
double sum(std::span<const double> a){double n=0;for(auto x:a)n+=x;return n;}
double widthValue(const ColumnWidth& v,double autoWidth=0){if(auto n=std::get_if<double>(&v))return *n;if(std::holds_alternative<PropertyNull>(v))return autoWidth;return autoWidth;}
}

std::vector<CommonMtr*> CommonMtable::tableRows()const{
    if(!childNodes)throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'map')");
    if(!*childNodes)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'map')");
    std::vector<CommonMtr*> rows;rows.reserve((*childNodes)->size());
    for(std::size_t i=0;i<(*childNodes)->size();++i){
        auto* row=dynamic_cast<CommonMtr*>((*childNodes)->get(i).requireNode("numCells"));
        if(!row)throw options::Exception("TypeError",u"Table child is not an mtr wrapper");rows.push_back(row);
    }return rows;
}
CommonWrapper* CommonMtable::findContainer(){
    auto* current=static_cast<CommonWrapper*>(this);auto* outer=parent;
    while(outer&&(property_host::truthy(outer->node->notParent())||outer->node->kind()=="mrow")){current=outer;outer=outer->parent;}
    container=outer;
    auto position=current->node->childPosition();
    if(!position)throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'childPosition')");
    containerI=*position;return container;
}
double CommonMtable::getPercentageWidth(){
    if(hasLabels){bbox->pwidth=BBox::fullWidth;return 1;}
    const auto width=property_host::string(node->attributes()->get("width"));
    if(isPercentText(property_host::debugText(width))){bbox->pwidth=property_host::debugText(width);return 1;}return 0;
}
std::vector<std::string> CommonMtable::getAttributeArray(std::string_view name)const{
    const auto value=node->attributes()->get(name);
    if(!property_host::truthy(value))return {property_host::debugText(property_host::string(node->attributes()->getDefault(name)))};
    return words(property_host::string(value));
}
std::optional<std::vector<std::string>> CommonMtable::getColumnAttributes(std::string_view name,std::size_t first)const{
    const auto n=static_cast<std::ptrdiff_t>(numCols)-static_cast<std::ptrdiff_t>(first);auto values=getAttributeArray(name);
    if(values.empty())return std::nullopt;
    while(n>0&&values.size()<static_cast<std::size_t>(n))values.push_back(values.back());
    if(static_cast<std::ptrdiff_t>(values.size())>n){
        const auto start=n<0?std::max<std::ptrdiff_t>(0,static_cast<std::ptrdiff_t>(values.size())+n):n;
        values.resize(static_cast<std::size_t>(start));
    }
    return values;
}
std::optional<std::vector<std::string>> CommonMtable::getRowAttributes(std::string_view name,std::size_t first)const{
    const auto n=static_cast<std::ptrdiff_t>(numRows)-static_cast<std::ptrdiff_t>(first);auto values=getAttributeArray(name);
    if(values.empty())return std::nullopt;
    while(n>0&&values.size()<static_cast<std::size_t>(n))values.push_back(values.back());
    if(static_cast<std::ptrdiff_t>(values.size())>n){
        const auto start=n<0?std::max<std::ptrdiff_t>(0,static_cast<std::ptrdiff_t>(values.size())+n):n;
        values.resize(static_cast<std::size_t>(start));
    }
    return values;
}
std::optional<std::vector<double>> CommonMtable::convertLengths(const std::vector<std::string>& values)const{
    std::vector<double> out;out.reserve(values.size());for(const auto& s:values)out.push_back(length2em(Property(s)));return out;
}
std::vector<std::string> CommonMtable::getEmHalfSpacing(const std::array<double,4>& frame,
    const std::vector<double>& spaces,double scale)const{
    std::vector<std::string> out;out.reserve(spaces.size()+2);
    for(const auto value:spaces)out.push_back(em(value*scale/2));
    out.insert(out.begin(),em(frame[0]*scale));
    out.push_back(em(frame[1]*scale));
    return out;
}
std::optional<std::vector<std::string>> CommonMtable::addEm(const std::vector<double>& values,double n)const{
    std::vector<std::string> out;out.reserve(values.size());for(double v:values)out.push_back(em(v/n));return out;
}
std::array<double,4> CommonMtable::getFrameSpacing(){
    std::array<double,4> result{0,0,0,0};
    if(fframe){const auto vals=convertLengths(getAttributeArray("framespacing"));if(vals){if(vals->size()>0)result[0]=(*vals)[0];if(vals->size()>1)result[1]=(*vals)[1];}}
    result[2]=result[0];
    const auto rawPadding=node->attributes()->get("data-array-padding");
    if(property_host::truthy(rawPadding)){const auto padding=property_host::string(rawPadding);const auto vals=convertLengths(words(padding));if(vals){if(vals->size()>0)result[0]=(*vals)[0];if(vals->size()>1)result[2]=(*vals)[1];}}
    return result;
}

CommonMtable::CommonMtable(CommonWrapperFactory& factory,mml::MmlNode& source,CommonWrapper* parentValue)
    :CommonWrapper(factory,source,parentValue){}
void CommonMtable::initializeSource(){
    CommonWrapper::initializeSource();
    const auto rows=tableRows();numRows=rows.size();
    for(auto* row:rows){numCols=std::max(numCols,row->numCells());hasLabels=hasLabels||row->labeled();}
    findContainer();isTop=!container||(container->node->kind()=="math"&&!container->parent);
    if(isTop)jax().table=this;
    getPercentageWidth();const auto attrs=node->attributes();
    frame=property_host::string(attrs->get("frame"))!=u"none";fframe=frame||!attrs->get("data-frame-styles").missing();fLine=frame?.07:0;
    fSpace=getFrameSpacing();
    if(auto x=getColumnAttributes("columnspacing"))if(auto v=convertLengths(*x))cSpace=*v;
    if(auto x=getRowAttributes("rowspacing"))if(auto v=convertLengths(*x))rSpace=*v;
    if(auto x=getColumnAttributes("columnlines"))for(auto& s:*x)cLines.push_back(s=="none"?0:.07);
    if(auto x=getRowAttributes("rowlines"))for(auto& s:*x)rLines.push_back(s=="none"?0:.07);
    // Source computes table data lazily from getColumnWidths/adjustWideTable;
    // fixed-width tables do not measure cells here.
    cWidths=getColumnWidths();
    adjustWideTable();stretchColumns();stretchRows();
}
TableData& CommonMtable::getTableData(){
    if(data)return *data;
    TableData result;result.H.assign(numRows,0);result.D.assign(numRows,0);result.W.assign(numCols,0);
    result.NH.resize(numRows);result.ND.resize(numRows);std::vector<double> labelWidth(1,0);
    const auto rows=tableRows();
    for(std::size_t j=0;j<rows.size();++j){auto* row=rows[j];const auto align=property_host::debugText(property_host::string(row->node->attributes()->get("rowalign")));
        for(std::size_t i=0;i<row->numCells();++i){
            const auto cellValue=row->tableCells()->get(i);
            // Source immediately reads cell.getBBox(); preserve the source
            // slot's null-versus-hole distinction at that member access.
            auto* cell=cellValue.requireNode("getBBox");
            updateHDW(*cell,i,j,align,result,&result.W);recordPWidthCell(*cell,i);
        }
        result.NH[j]=result.H[j];result.ND[j]=result.D[j];
        if(row->labeled()){
            // tableCells() is the Source Mtr accessor for stretchable table
            // cells; Mlabeledtr overrides it with slice(1). The label itself
            // remains childNodes[0] and supplies the separate label width.
            if(!row->childNodes)throw options::Exception("TypeError",u"Cannot read properties of undefined (reading '0')");
            if(!*row->childNodes)throw options::Exception("TypeError",u"Cannot read properties of null (reading '0')");
            auto* label=(*row->childNodes)->get(0).requireNode("getBBox");
            updateHDW(*label,0,j,align,result,&labelWidth);
        }
        if(!row->bbox)throw options::Exception("TypeError",u"Cannot set properties of undefined (setting 'h')");
        row->bbox->h=result.H[j];row->bbox->d=result.D[j];
    }
    result.L=labelWidth[0];data=std::move(result);return *data;
}
void CommonMtable::updateHDW(CommonWrapper& cell,std::size_t i,std::size_t j,std::string_view rowAlign,TableData& table,std::vector<double>* widths){
    const auto box=cell.getBBox();double h=box->h,d=box->d,w=box->w;
    if(!cell.parent)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'bbox')");
    if(!cell.parent->bbox)throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'rscale')");
    const double scale=cell.parent->bbox->rscale;
    if(scale!=1){h*=scale;d*=scale;w*=scale;}
    if(property_host::truthy(node->getProperty("useHeight"))){h=std::max(h,.75);d=std::max(d,.25);}
    const auto cellAlign=cell.node->attributes()->get("rowalign");
    std::string align=property_host::truthy(cellAlign)
        ?property_host::debugText(property_host::string(cellAlign)):std::string(rowAlign);
    if(align=="top"){
        if(h>table.H[j]){table.D[j]-=h-table.H[j];table.H[j]=h;}if(h+d>table.H[j]+table.D[j])table.D[j]=h+d-table.H[j];
    }else if(align=="bottom"){
        if(d>table.D[j]){table.H[j]-=d-table.D[j];table.D[j]=d;}if(h+d>table.H[j]+table.D[j])table.H[j]=h+d-table.D[j];
    }else if(align=="center"){
        if(h+d>table.H[j]+table.D[j])table.H[j]=table.D[j]=(h+d)/2;
    }else {table.H[j]=std::max(table.H[j],h);table.D[j]=std::max(table.D[j],d);}
    if(widths&&i<widths->size()&&w>(*widths)[i])(*widths)[i]=w;
}
void CommonMtable::recordPWidthCell(CommonWrapper& cell,std::size_t i){
    if(!cell.childNodes||!(*cell.childNodes)||(*cell.childNodes)->size()==0)return;
    auto* child=dynamic_cast<CommonWrapper*>((*cell.childNodes)->get(0).nodeOrNull());if(child&&!child->getBBox()->pwidth.empty())pwidthCells.emplace_back(&cell,i);
}
void CommonMtable::setColumnPWidths(){
    for(auto [cell,i]:pwidthCells){if(!cell||i>=cWidths.size())continue;const double width=widthValue(cWidths[i]);if(cell->setChildPWidths(false,width)){cell->invalidateBBox();cell->getBBox();}}
}
std::array<double,2> CommonMtable::getBBoxHD(double height)const{
    const auto [align,row]=getAlignmentRow();if(!row){const double axis=font->params().axis_height,h=height/2;
        if(align=="top")return {0,height};if(align=="bottom")return {height,0};if(align=="axis")return {h+axis,h-axis};return {h,h};}
    auto* self=const_cast<CommonMtable*>(this);const double y=self->getVerticalPosition(*row,align);return {y,height-y};
}
std::pair<std::string,std::optional<double>> CommonMtable::getAlignmentRow()const{
    auto wordsValue=words(property_host::string(node->attributes()->get("align")));
    const std::string align=wordsValue.empty()?"axis":wordsValue[0];if(wordsValue.size()<2)return {align,{}};
    const double parsed=js_input::parseInt(property_host::string(Property(wordsValue[1])));
    const double row=parsed<0?parsed+static_cast<double>(numRows)+1:parsed;
    // In Source, NaN survives both range comparisons and is returned as the
    // row index. Do not collapse it into the null/no-row branch.
    if(row<1||row>static_cast<double>(numRows))return {align,{}};
    return {align,row-1};
}
double CommonMtable::getVerticalPosition(double i,std::string_view align){
    auto t=getTableData();double y=fLine;const auto space=getRowHalfSpacing();const bool equal=property_host::truthy(node->attributes()->get("equalrows"));const double hd=equal?getEqualRowHeight():0;
    for(std::size_t j=0;static_cast<double>(j)<i;++j)y+=space[j]+(equal?hd:t.H[j]+t.D[j])+space[j+1]+(j<rLines.size()?rLines[j]:0);
    if(!std::isfinite(i)||i<0||std::floor(i)!=i||i>=static_cast<double>(t.H.size()))
        return std::numeric_limits<double>::quiet_NaN();
    const auto index=static_cast<std::size_t>(i);
    const double h=equal?(hd+t.H[index]-t.D[index])/2:t.H[index],d=equal?(hd-t.H[index]+t.D[index])/2:t.D[index];
    if(align=="center")return y+space[index]+(h+d)/2;if(align=="bottom")return y+space[index]+h+d+space[index+1];if(align=="baseline")return y+space[index]+h;if(align=="axis")return y+space[index]+h-.25;return y;
}
std::vector<double> CommonMtable::getRowHalfSpacing()const{auto out=rSpace;for(auto&x:out)x/=2;out.insert(out.begin(),fSpace[1]);out.push_back(fSpace[1]);return out;}
std::vector<double> CommonMtable::getColumnHalfSpacing()const{auto out=cSpace;for(auto&x:out)x/=2;out.insert(out.begin(),fSpace[0]);out.push_back(fSpace[2]);return out;}
double CommonMtable::getEqualRowHeight(){auto t=getTableData();double h=0;for(std::size_t i=0;i<t.H.size();++i)h=std::max(h,t.H[i]+t.D[i]);return h;}
std::vector<double> CommonMtable::getComputedWidths(){auto t=getTableData();for(std::size_t i=0;i<t.W.size();++i)if(i<cWidths.size())t.W[i]=widthValue(cWidths[i],t.W[i]);if(property_host::truthy(node->attributes()->get("equalcolumns"))){double m=0;for(auto w:t.W)m=std::max(m,w);std::fill(t.W.begin(),t.W.end(),m);}return t.W;}
double CommonMtable::naturalWidth(){
    // Source sums one concatenated array, preserving its exact floating-point
    // addition order: computed widths, column lines, then column spacing.
    auto widths=getComputedWidths();
    widths.insert(widths.end(),cLines.begin(),cLines.end());
    widths.insert(widths.end(),cSpace.begin(),cSpace.end());
    return sum(widths)+2*fLine+fSpace[0]+fSpace[2];
}
std::array<double,2> CommonMtable::getBBoxLR(){
    if(hasLabels){const auto attrs=node->attributes();const auto side=property_host::debugText(property_host::string(attrs->get("side")));auto [pad,align,shift]=getPadAlignShift(side);(void)shift;
        const bool labels=property_host::truthy(attrs->get("data-width-includes-label"));if(labels&&frame&&fSpace[0])pad-=fSpace[0];
        return align=="center"&&!labels?std::array<double,2>{pad,pad}:side=="left"?std::array<double,2>{pad,0}:std::array<double,2>{0,pad};
    }return {bbox?bbox->L:0,0};
}
std::tuple<double,std::string,double> CommonMtable::getPadAlignShift(std::string_view side){
    const auto table=getTableData();double pad=table.L+length2em(node->attributes()->get("minlabelspacing"));
    if(styles){const auto left=styles->get(u"padding-left"),right=styles->get(u"padding-right");if(!left.empty()||!right.empty())pad=std::max({pad,length2em(Property(property_host::debugText(left.empty()?std::u16string_view(u"0"):std::u16string_view(left)))),length2em(Property(property_host::debugText(right.empty()?std::u16string_view(u"0"):std::u16string_view(right))))});}
    auto [align,shift]=getAlignShift();
    if(align==side){if(side=="left")shift=std::max(pad,shift)-pad;else shift=std::min(-pad,shift)+pad;}
    return {pad,align,shift};
}
void CommonMtable::computeBBox(BBox& box,bool){auto t=getTableData();const bool equal=property_host::truthy(node->attributes()->get("equalrows"));double height;if(equal)height=sum(rLines)+sum(rSpace)+getEqualRowHeight()*numRows;else height=sum(t.H)+sum(t.D)+sum(rLines)+sum(rSpace);height+=2*(fLine+fSpace[1]);double width=naturalWidth();const auto w=property_host::string(node->attributes()->get("width"));if(w!=u"auto")width=std::max(length2em(node->attributes()->get("width"),0)+2*fLine,width);const auto hd=getBBoxHD(height),lr=getBBoxLR();box.h=hd[0];box.d=hd[1];box.w=width;box.L=lr[0];box.R=lr[1];if(!isPercentText(property_host::debugText(w)))setColumnPWidths();}
void CommonMtable::getStyles(){
    CommonWrapper::getStyles();
    const auto value=node->attributes()->get("data-frame-styles");
    if(!property_host::truthy(value))return;
    if(!styles)styles=std::make_unique<folia::math::styles::Styles>(u"");
    const auto frame=property_host::string(value);
    std::vector<std::u16string_view> tokens;
    std::size_t start=0;
    for(std::size_t i=0;i<=frame.size();++i){
        if(i==frame.size()||frame[i]==u' '){tokens.emplace_back(frame.data()+start,i-start);start=i+1;}
    }
    static constexpr std::array<std::u16string_view,4> names={u"top",u"right",u"bottom",u"left"};
    for(std::size_t i=0;i<std::min(tokens.size(),names.size());++i){
        if(tokens[i]!=u"none")styles->set(std::u16string(u"border-")+std::u16string(names[i]),
            std::u16string(u".07em ")+std::u16string(tokens[i]));
    }
}
std::string CommonMtable::getChildAlign(std::size_t i)const{auto* row=container;return row?row->getChildAlign(containerI):"center";}
double CommonMtable::getWrapWidth(std::size_t){return getWidth();}
double CommonMtable::getWidth(){return pWidth?pWidth:getBBox()->w;}
bool CommonMtable::setChildPWidths(bool,std::optional<double> cwidth,bool){
    const auto width=property_host::string(node->attributes()->get("width"));if(!isPercentText(property_host::debugText(width)))return false;
    if(!hasLabels){
        bbox->pwidth="";
        if(!container)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'bbox')");
        if(!container->bbox)throw options::Exception("TypeError",u"Cannot set properties of null (setting 'pwidth')");
        container->bbox->pwidth="";
    }
    const auto w=bbox->w;const auto label=property_host::truthy(node->attributes()->get("data-width-includes-label"));
    const double W=std::max(w,length2em(node->attributes()->get("width"),std::max(cwidth.value_or(containerWidth()),bbox->L+w+bbox->R)))-(label?bbox->L+bbox->R:0);
    std::vector<std::string> cols;
    if(property_host::truthy(node->attributes()->get("equalcolumns")))cols.assign(numCols,em(1.0/std::max<std::size_t>(1,numCols)));
    else cols=getColumnAttributes("columnwidth",0).value_or(std::vector<std::string>{});
    cWidths=getColumnWidthsFixed(cols,W);pWidth=naturalWidth();if(isTop)bbox->w=pWidth;setColumnPWidths();
    if(pWidth!=w&&parent)parent->invalidateBBox();return pWidth!=w;
}
std::pair<std::string,double> CommonMtable::getAlignShift(){return isTop?CommonWrapper::getAlignShift():std::pair{container?container->getChildAlign(containerI):"center",0.0};}
ColumnWidths CommonMtable::getColumnWidths(){
    const auto width=property_host::debugText(property_host::string(node->attributes()->get("width")));
    if(property_host::truthy(node->attributes()->get("equalcolumns")))return getEqualColumns(width);
    auto attrs=getColumnAttributes("columnwidth",0).value_or(std::vector<std::string>{});
    if(width=="auto")return getColumnWidthsAuto(attrs);
    if(isPercentText(width))return getColumnWidthsPercent(attrs);
    return getColumnWidthsFixed(attrs,length2em(Property(width)));
}
ColumnWidths CommonMtable::getEqualColumns(std::string_view width){
    const auto count=std::max<std::size_t>(1,numCols);ColumnWidth value=0.0;
    if(width=="auto"){for(double w:getTableData().W)if(w>std::get<double>(value))value=w;}
    else if(isPercentText(width))value=CommonWrapper::percent(1.0/count);
    else value=std::max(0.0,length2em(Property(std::string(width)))-(sum(cLines)+sum(cSpace)+fSpace[0]+fSpace[2]))/count;
    return ColumnWidths(numCols,std::move(value));
}
ColumnWidths CommonMtable::getColumnWidthsAuto(const std::vector<std::string>& widths){ColumnWidths out;for(const auto& value:widths){if(value=="auto"||value=="fit")out.emplace_back(PropertyNull{});else if(isPercentText(value))out.emplace_back(value);else out.emplace_back(length2em(Property(value)));}return out;}
ColumnWidths CommonMtable::getColumnWidthsPercent(const std::vector<std::string>& widths){const bool hasFit=std::find(widths.begin(),widths.end(),"fit")!=widths.end();const auto data=hasFit?std::optional<TableData>(getTableData()):std::nullopt;ColumnWidths out;for(std::size_t i=0;i<widths.size();++i){const auto& value=widths[i];if(value=="fit")out.emplace_back(PropertyNull{});else if(value=="auto")out.emplace_back(data&&i<data->W.size()?ColumnWidth(data->W[i]):ColumnWidth(PropertyNull{}));else if(isPercentText(value))out.emplace_back(value);else out.emplace_back(length2em(Property(value)));}return out;}
ColumnWidths CommonMtable::getColumnWidthsFixed(const std::vector<std::string>& widths,double width){const auto indices=widths.size();std::vector<std::size_t> fit,automatic;for(std::size_t i=0;i<indices;++i){if(widths[i]=="fit")fit.push_back(i);else if(widths[i]=="auto")automatic.push_back(i);}const auto n=fit.empty()?automatic.size():fit.size();const auto data=n?std::optional<TableData>(getTableData()):std::nullopt;const double cwidth=width-sum(cLines)-sum(cSpace)-fSpace[0]-fSpace[2];double dw=cwidth;for(std::size_t i=0;i<indices;++i){const auto& v=widths[i];dw-=v=="fit"||v=="auto"?(data&&i<data->W.size()?data->W[i]:0):length2em(Property(v),cwidth);}const double fw=n&&dw>0?dw/n:0;ColumnWidths out;for(std::size_t i=0;i<indices;++i){const auto& v=widths[i];if(v=="fit")out.emplace_back((data&&i<data->W.size()?data->W[i]:0)+fw);else if(v=="auto")out.emplace_back((data&&i<data->W.size()?data->W[i]:0)+(fit.empty()?fw:0));else out.emplace_back(length2em(Property(v),cwidth));}return out;}
void CommonMtable::stretchRows(){
    const bool equal=property_host::truthy(node->attributes()->get("equalrows"));
    const double common=equal?getEqualRowHeight():0;auto data=equal?getTableData():TableData{};const auto rows=tableRows();
    for(std::size_t i=0;i<rows.size();++i){
        if(equal){const auto h=(common+data.H[i]-data.D[i])/2,d=(common-data.H[i]+data.D[i])/2;rows[i]->stretchChildren(std::array<double,2>{h,d});}
        else rows[i]->stretchChildren();
    }
}
void CommonMtable::stretchColumns(){
    const auto widths=getColumnAttributes("columnwidth",0).value_or(std::vector<std::string>{});
    for(std::size_t i=0;i<numCols;++i){
        std::optional<double> width;if(i<cWidths.size())if(auto* v=std::get_if<double>(&cWidths[i]))width=*v;
        stretchColumn(i,width);if(width)breakColumn(i,*width,i<widths.size()?widths[i]:"");
    }
}
void CommonMtable::stretchColumn(std::size_t i,std::optional<double> width){
    std::vector<CommonWrapper*> stretchy;const auto rows=tableRows();
    auto first=[&](CommonWrapper& cell)->CommonWrapper&{
        if(!cell.childNodes)throw options::Exception("TypeError",u"Cannot read properties of undefined (reading '0')");
        if(!*cell.childNodes||(*cell.childNodes)->size()==0)throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'stretch')");
        auto* c=dynamic_cast<CommonWrapper*>((*cell.childNodes)->get(0).requireNode("stretch"));if(!c)throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'stretch')");return *c;
    };
    for(auto* row:rows)if(auto* cell=row->getChild(i)){auto& c=first(*cell);if(c.stretch&&c.stretch->dir==NONE&&c.canStretch("h"))stretchy.push_back(&c);}
    if(stretchy.empty())return;
    if(!width){width=0;const bool all=stretchy.size()==rows.size();for(auto* row:rows)if(auto* cell=row->getChild(i)){auto& c=first(*cell);const bool noStretch=c.stretch&&c.stretch->dir==NONE;if(all||noStretch)width=std::max(*width,c.getBBox(noStretch)->w);}}
    auto& widths=getTableData().W;
    for(auto* c:stretchy){const double before=c->getBBox()->w;auto* mo=c->coreMO();if(!mo)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'getStretchedVariant')");mo->getStretchedVariant(std::array<double,1>{std::max(*width,before)/c->coreRScale()});widths[i]=std::max(widths[i],c->getBBox()->w);}
}
void CommonMtable::breakColumn(std::size_t i,double width,std::string_view type){
    auto* root=jax().math?jax().math->root.nodeOrNull():nullptr;
    if(!root||property_host::string(root->attributes()->get("overflow"))!=u"linebreak"||!property_host::truthy(jax().math->display))return;
    auto& table=getTableData();std::size_t j=0;double maxWidth=0;
    for(auto* row:tableRows()){
        auto* cell=row->getChild(i);
        if(cell){const double scale=row->getBBox()->rscale;auto box=cell->getBBox();if(box->w*scale>width){if(!cell->childNodes||!*cell->childNodes||(*cell->childNodes)->size()==0)throw options::Exception("TypeError",u"Cannot read properties of undefined (reading 'breakToWidth')");auto* child=dynamic_cast<CommonWrapper*>((*cell->childNodes)->get(0).requireNode("breakToWidth"));child->breakToWidth(width);updateHDW(*cell,i,j,property_host::debugText(property_host::string(row->node->attributes()->get("rowalign"))),table);}
            maxWidth=std::max(maxWidth,box->w*scale);
        }
        row->getBBox()->h=table.H[j];row->getBBox()->d=table.D[j];++j;
    }
    if(i>=cWidths.size())return;
    if(type=="fit"||type=="auto"||isPercentText(type)||maxWidth>widthValue(cWidths[i]))cWidths[i]=maxWidth;
}
void CommonMtable::adjustWideTable(){
    if(property_host::string(node->attributes()->get("width"))!=u"auto")return;
    const auto [pad,align,shift]=getPadAlignShift(property_host::debugText(property_host::string(node->attributes()->get("side"))));(void)shift;
    const double available=std::max(containerWidth()/10,containerWidth()-pad-(align=="center"?pad:0));
    if(naturalWidth()>available)adjustColumnWidths(available);
}
void CommonMtable::adjustColumnWidths(double width){
    const auto data=getTableData();auto attrs=getColumnAttributes("columnwidth",0).value_or(std::vector<std::string>{});
    std::vector<std::size_t> order;auto add=[&](auto pred,bool largest){std::vector<std::size_t> part;for(std::size_t i=0;i<attrs.size();++i)if(pred(attrs[i]))part.push_back(i);std::sort(part.begin(),part.end(),[&](auto a,auto b){return largest?data.W[a]>data.W[b]:data.W[a]<data.W[b];});order.insert(order.end(),part.begin(),part.end());};
    add([](auto& x){return x=="fit";},true);add([](auto& x){return x=="auto";},true);add([](auto& x){return isPercentText(x);},true);add([](auto& x){return x!="fit"&&x!="auto"&&!isPercentText(x);},true);if(order.empty())return;
    for(std::size_t i=0;i<cWidths.size();++i)cWidths[i]=widthValue(cWidths[i],data.W[i]);
    const double cwidth=width-sum(cLines)-sum(cSpace)-fSpace[0]-fSpace[2];double dw=0;for(const auto& x:cWidths)dw+=widthValue(x);dw-=cwidth;
    double total=0;std::size_t n=0;while(n<order.size()){total+=data.W[order[n++]];if(total&&dw/total<.333)break;}if(!total)return;const double scale=1-dw/total;for(std::size_t j=0;j<n;++j){const auto i=order[j];cWidths[i]=widthValue(cWidths[i])*scale;}
}
}
