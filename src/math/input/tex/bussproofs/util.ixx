// MathJax 4.1.3 input/tex/bussproofs/BussproofsUtil.ts.
export module folia.math.input.tex.bussproofs.util;
export import folia.math.input.tex.parser;
export import folia.math.util.runtime_modules;
import std;
export namespace folia::math::tex::bussproofs_util {
struct FilterData { core::MathItemReference math=Missing{};core::MathDocumentReference document=Missing{};ParseOptions* data=nullptr; };
struct GeometryState;
class BussproofsUtilModule {
    std::shared_ptr<GeometryState> state_;
    options::Value setProperty_,getProperty_,removeProperty_;
    options::Value saveDocument_,clearDocument_,makeAttributes_,balanceRules_;
public:
    static std::shared_ptr<BussproofsUtilModule> initializeModule(ParserRuntime&,util::RuntimeModules&);
    options::Context& context()const;
    options::Value function(std::u16string_view name)const;
};
void setProperty(mml::NodeValue,std::string_view,Property);
Property getProperty(mml::NodeValue,std::string_view);
void removeProperty(mml::NodeValue,std::string_view);
void saveDocument(GeometryState&,const FilterData&);
void clearDocument(GeometryState&,const FilterData&);
void makeBsprAttributes(GeometryState&,const FilterData&);
void balanceRules(GeometryState&,const FilterData&);
}
