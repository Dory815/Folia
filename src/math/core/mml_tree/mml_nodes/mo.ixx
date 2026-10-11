// MathJax 4.1.3 MmlNodes/mo.ts; root-frozen declarations. Apache-2.0.
export module folia.math.core.mml_tree.mml_nodes.mo;
export import folia.math.core.mml_tree.mml_node;
export import folia.math.core.mml_tree.mo_types;
import std;
export namespace folia::math::mml {
class MmlMo:public AbstractMmlTokenNode {
protected:
    const MoClassServices& constructorOperators()const;
    virtual Property getOperatorDef(std::u16string_view);
    virtual void checkOperatorTable(std::u16string_view);
    virtual PropertyArray handleExplicitForm(PropertyArray);
    virtual void checkPseudoScripts(std::u16string_view);
    virtual void checkPrimes(std::u16string_view);
    virtual void checkMathAccent(std::u16string_view);
public:
    using AbstractMmlTokenNode::AbstractMmlTokenNode;
    // Source class fields are undefined during superclass initialization.
    // This is the source's own _texClass field; the inherited texclass source
    // field has a different identity and is never a fallback for Mo's getter.
    Property _texClass=Missing{},lspace=Missing{},rspace=Missing{};
    static PropertyList& defaults();
    static Property& operatorTable();
    static Property& mathSpacing();
    static Property& remapPrimes();
    static MoPatternHandle& pseudoScripts();
    static MoPatternHandle& primes();
    static MoPatternHandle& opPattern();
    static MoPatternHandle& mathaccents();
    static MoPatternHandle& mathaccentsWithWidth();
    template<class T>static std::shared_ptr<MoClassServices> staticBinding(){
        auto s=std::make_shared<MoClassServices>();
        s->table=[] {return T::operatorTable();};s->spacing=[] {return T::mathSpacing();};s->remap=[] {return T::remapPrimes();};
        s->pseudoScripts=[] {return T::pseudoScripts();};s->primes=[] {return T::primes();};s->opPattern=[] {return T::opPattern();};
        s->mathaccents=[] {return T::mathaccents();};s->mathaccentsWithWidth=[] {return T::mathaccentsWithWidth();};return s;
    }
    void initializeClassFields()override;
    std::string kind()const override;
    bool isEmbellished()const override;
    Property texClass()override;
    void setTexClass(Property)override;
    virtual MmlNode* coreParent();
    virtual std::u16string coreText(MmlNode*);
    bool hasSpacingAttributes()override;
    virtual Property isAccent();
    MmlNode* setTeXclass(MmlNode*)override;
    virtual MmlNode* adjustTeXclass(MmlNode*);
    protected: void inheritAttributes(AttributeList,Property display,Property level,Property prime)override; public:
    virtual PropertyArray getForms();
    virtual bool isMathAccent(std::optional<std::u16string_view> text={});
    virtual bool isMathAccentWithWidth(std::optional<std::u16string_view> text={});
};
}
