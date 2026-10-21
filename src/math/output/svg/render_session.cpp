module folia.math.output.svg.render_session;
import folia.math.input.tex.base_configuration;
import folia.math.input.tex.action.configuration;
import folia.math.input.tex.ams.configuration;
import folia.math.input.tex.amscd.configuration;
import folia.math.input.tex.empheq.configuration;
import folia.math.input.tex.bbox.configuration;
import folia.math.input.tex.begingroup.configuration;
import folia.math.input.tex.boldsymbol.configuration;
import folia.math.input.tex.bussproofs.configuration;
import folia.math.input.tex.braket.configuration;
import folia.math.input.tex.bbm.configuration;
import folia.math.input.tex.bboldx.configuration;
import folia.math.input.tex.cancel.configuration;
import folia.math.input.tex.cases.configuration;
import folia.math.input.tex.centernot.configuration;
import folia.math.input.tex.color.configuration;
import folia.math.input.tex.colortbl.configuration;
import folia.math.input.tex.configmacros.configuration;
import folia.math.input.tex.enclose.configuration;
import folia.math.input.tex.newcommand.configuration;
import folia.math.input.tex.mhchem.configuration;
import folia.math.input.tex.mathtools.configuration;
import folia.math.input.tex.physics.configuration;
import folia.math.input.tex.tagformat.tag_format_configuration;
import folia.math.input.tex.setoptions.set_options_configuration;
import folia.math.input.tex.noundefined.configuration;
import folia.math.input.tex.textmacros.configuration;
import folia.math.input.tex.textcomp.configuration;
import folia.math.input.tex.unicode.configuration;
import folia.math.input.tex.units.configuration;
import folia.math.input.tex.upgreek.configuration;
import folia.math.input.tex.verb.configuration;
import folia.math.input.tex.fontsizev3.configuration;
import folia.math.input.tex.gensymb.configuration;
import folia.math.input.tex.extpfeil.configuration;
import folia.math.input.tex.dsfont.configuration;
import folia.math.input.tex.html.configuration;
import folia.math.input.tex.texhtml.tex_html_configuration;
import folia.math.core.find_math_host;
import folia.math.core.input_jax_host;
import folia.math.core.math_item;
import folia.math.output.common.font_data_surface;
import folia.math.output.svg.font_data_surface;
import folia.math.output.svg.menclose_surface;
import folia.math.output.svg.wrapper_constructors;
import folia.math.fonts.newcm_font_data;
import folia.math.fonts.svg_extensions;
import folia.math.util.option_types;
import std;

namespace folia::math::output::svg {
using namespace options;

namespace {
struct ConstructorServiceLease {
    std::shared_ptr<SvgRenderServiceGraph> services;
};
struct SessionJaxLease {
    std::shared_ptr<Context> context;
    std::shared_ptr<SvgRenderServiceGraph> services;
    std::shared_ptr<SvgOutputJax> jax;
};
struct SessionFactoryLease {
    std::shared_ptr<Context> context;
    std::shared_ptr<SvgRenderServiceGraph> services;
    std::shared_ptr<mml::MmlFactory> factory;
};
struct SessionAdaptorLease {
    std::shared_ptr<Context> context;
    std::shared_ptr<SvgRenderServiceGraph> services;
    std::shared_ptr<adaptors::lite::LiteAdaptor> adaptor;
};
struct LiteTextMeasurementProbe {
    adaptors::lite::LiteNodeRef element;
    adaptors::lite::LiteNodeRef text;
};

void requireHost(const common::FontLoadHost& host){
    if(!host.asyncIsSynchronous||!host.warn)
        throw Exception("TypeError",u"NativeRenderSession requires the Source async mode and warning callbacks");
}

common::FontDataContextBindings::Allocator baseFontAllocator(){
    return [](common::FontDataClass& cls,common::FontLoadHost& host,
        Context& context,Value supplied)->std::shared_ptr<common::FontData>{
        return std::make_shared<common::FontData>(cls,host,context,std::move(supplied));
    };
}
common::FontDataContextBindings::Allocator svgFontAllocator(){
    return [](common::FontDataClass& raw,common::FontLoadHost& host,
        Context& context,Value supplied)->std::shared_ptr<common::FontData>{
        // This allocator is installed only on the canonical SvgFontData
        // constructor whose owner is SvgFontDataClass; the base owner is not
        // accepted by that constructor registration path.
        auto& cls=static_cast<SvgFontDataClass&>(raw);
        return std::make_shared<SvgFontData>(cls,host,context,std::move(supplied));
    };
}
common::FontDataContextBindings::Allocator newcmFontAllocator(){
    return [](common::FontDataClass& raw,common::FontLoadHost& host,
        Context& context,Value supplied)->std::shared_ptr<common::FontData>{
        // As above, this closure is bound exclusively to the actual NewCM
        // constructor owner, so a checked RTTI conversion is neither available
        // nor needed for this non-polymorphic Source class hierarchy.
        auto& cls=static_cast<fonts::NewcmFontDataClass&>(raw);
        return std::make_shared<fonts::NewcmFontData>(cls,host,context,std::move(supplied));
    };
}
}

SvgRenderServiceGraph::SvgRenderServiceGraph(Context& context,common::FontLoadHost host,
    NewcmFontProfile profile)
    :context_(context),modules_(std::make_shared<util::RuntimeModules>(context)),
     functions_(std::make_shared<OptionFunctionBindings>(context)),
     regexp_(std::make_shared<regexp_host::RegExpBindings>(context)),
     unicode_(std::make_shared<Unicode16Normalization>()),
     inputJax_(std::make_shared<core::InputJaxModule>(context)),
     findMath_(std::make_shared<core::FindMathModule>(context)),
     findTeX_(std::make_shared<tex::FindTeXModule>(context,*findMath_,*regexp_)),
     parser_(std::make_shared<tex::ParserRuntime>(context,*functions_,*regexp_,*unicode_,
                                                   *inputJax_,*findTeX_,modules_.get())),
     outputJax_(std::make_shared<core::OutputJaxModule>(context,*functions_)),
     mmlFactory_(std::make_shared<mml::MmlFactory>()),
     maps_(modules_->getOrCreate<MapContextBindings>(u"ECMAScript/Map",
         [&]{return std::make_shared<MapContextBindings>(context);})),
     fontHost_(std::make_shared<common::FontLoadHost>(std::move(host))),
     fonts_(std::make_shared<common::FontDataContextBindings>(context)),
     fontProfile_(std::move(profile)),
     commonFactories_(std::make_shared<common::CommonFactoryContextBindings>(context)){
    requireHost(*fontHost_);
    parser_->initializeParserRuntime();
    fontHost_->queue=parser_->queue();
    (void)tex::BasePackage::initializeModule(*parser_);
    (void)tex::BussproofsConfiguration::initializeModule(*parser_,*modules_);
    (void)tex::ActionConfiguration::initializeModule(*parser_);
    (void)tex::AmsConfiguration::initializeModule(*parser_);
    (void)tex::AmsCdConfiguration::initializeModule(*parser_);
    (void)tex::EmpheqConfiguration::initializeModule(*parser_);
    (void)tex::BboxConfiguration::initializeModule(*parser_);
    tex::BegingroupConfiguration::initializeModule(*parser_);
    (void)tex::BoldsymbolConfiguration::initializeModule(*parser_);
    (void)tex::BraketConfiguration::initializeModule(*parser_);
    (void)tex::BbmConfiguration::initializeModule(*parser_);
    (void)tex::BboldxConfiguration::initializeModule(*parser_);
    (void)tex::CancelConfiguration::initializeModule(*parser_);
    (void)tex::CasesConfiguration::initializeModule(*parser_);
    (void)tex::CenternotConfiguration::initializeModule(*parser_);
    (void)tex::ColorConfiguration::initializeModule(*parser_,maps_);
    (void)tex::ColortblConfiguration::initializeModule(*parser_);
    (void)tex::ConfigMacrosConfiguration::initializeModule(*parser_);
    (void)tex::EncloseConfiguration::initializeModule(*parser_);
    tex::NewcommandConfiguration::initializeModule(*parser_);
    (void)tex::MhchemConfiguration::initializeModule(*parser_,*modules_);
    (void)tex::MathtoolsConfiguration::initializeModule(*parser_);
    (void)tex::PhysicsConfiguration::initializeModule(*parser_);
    (void)tex::TagFormatConfiguration::initializeModule(*parser_);
    (void)tex::SetOptionsConfiguration::initializeModule(*parser_);
    (void)tex::ExtpfeilConfiguration::initializeModule(*parser_);
    (void)tex::DsfontConfiguration::initializeModule(*parser_);
    (void)tex::FontSizeV3Configuration::initializeModule(*parser_);
    (void)tex::GensymbConfiguration::initializeModule(*parser_);
    (void)tex::NoUndefinedConfiguration::initializeModule(*parser_);
    tex::TextMacrosConfiguration::initializeModule(*parser_);
    (void)tex::TextcompConfiguration::initializeModule(*parser_);
    (void)tex::UnicodeConfiguration::initializeModule(*parser_);
    (void)tex::UnitsConfiguration::initializeModule(*parser_);
    (void)tex::UpgreekConfiguration::initializeModule(*parser_);
    (void)tex::VerbConfiguration::initializeModule(*parser_);
    (void)parser_->bindMmlFactory(mmlFactory_);
}

void SvgRenderServiceGraph::initialize(){
    auto& context=context_;
    auto properties=std::shared_ptr<ContextPropertyBindings>(parser_,&parser_->properties);
    const auto parserOwner=parser_;
    commonFactories_->bindFactoryMethods(*modules_,*properties,*maps_,
        [parserOwner](ObjectRef identity)->mml::MmlNode&{
            return parserOwner->resolveMmlNode(identity);
        },
        [parserOwner](mml::MmlNode& node)->ObjectRef{
            const auto identity=parserOwner->mmlNodeIdentities.find(&node);
            if(!identity)throw Exception("TypeError",u"MML node has no canonical Context identity");
            return *identity;
        },parserOwner->executionLifetime());
    fontHost_->regexp=regexp_.get();
    fontSurface_=std::make_shared<common::FontDataSourceSurface>(context,*fonts_,*properties,*maps_,regexp_.get());
    // Constructor callbacks own this graph, which in turn owns the borrowed
    // service providers while never owning the Context that stores callbacks.
    const auto hostAnchor=std::static_pointer_cast<void>(shared_from_this());

    baseFontClass_=std::make_shared<common::FontDataClass>(context);
    fontConstructor_=fontSurface_->registerBase(baseFontClass_,*fontHost_,hostAnchor,
                                                 baseFontAllocator());
    svgFontClass_=std::make_shared<SvgFontDataClass>(context);
    svg::SvgFontDataSourceSurface svgSurface(context,*fonts_,*fontSurface_);
    svgFontConstructor_=svgSurface.registerSvg(svgFontClass_,*fontHost_,hostAnchor,
                                               svgFontAllocator(),fontConstructor_);
    newcmFontClass_=std::make_shared<fonts::NewcmFontDataClass>(context);
    fonts::NewcmFontDataSourceSurface newcmSurface(context,*fonts_,*fontSurface_);
    newcmFontConstructor_=newcmSurface.registerNewcm(newcmFontClass_,*fontHost_,hostAnchor,
        newcmFontAllocator(),svgFontConstructor_);
    newcmModules_=std::make_shared<fonts::NewcmFontModuleProvider>(
        newcmFontClass_,parser_->queue());
    fontHost_->asyncLoad=[provider=newcmModules_](
        std::shared_ptr<common::FontData> font,common::DynamicFileRef dynamic,
        std::string request){
            return provider->loadAsync(std::move(font),std::move(dynamic),std::move(request));
        };
    fontHost_->syncLoad=[provider=newcmModules_](common::FontData& font,
        const common::DynamicFileRef& dynamic,std::string request){
            provider->loadSync(font,dynamic,std::move(request));
        };

    commonRuntime_=std::make_shared<common::CommonRuntimeBindings>(
        *modules_,*fonts_,*commonFactories_,*properties);
    commonClasses_=std::make_shared<common::CommonJaxClassBindings>(
        *functions_,outputJax_->filterBindings(),*commonRuntime_);

    commonConstructor_=context_.nativeConstructor(u"CommonOutputJax",
        [](Context&,std::span<const Value>)->ObjectRef{
            throw Exception("TypeError",u"CommonOutputJax is an abstract output class");
        });
    context_.setPrototype(commonConstructor_,outputJax_->abstractClass());
    const auto commonPrototype=std::get<ObjectRef>(context_.get(
        Value(commonConstructor_),Key(u"prototype")).data);
    const auto abstractPrototype=std::get<ObjectRef>(context_.get(
        Value(outputJax_->abstractClass()),Key(u"prototype")).data);
    context_.setPrototype(commonPrototype,abstractPrototype);
    (void)commonClasses_->bindClassDefaults(commonConstructor_);

    svgFactories_=std::make_shared<SvgWrapperFactoryContextBindings>(context,*commonFactories_);
    svgJax_=std::make_shared<SvgOutputJaxContextBindings>(
        *commonClasses_,*commonRuntime_,*svgFactories_,*functions_,outputJax_->filterBindings(),
        outputJax_->filterRecordOwner(),*maps_);
    wrapperSet_=loadSvgWrapperConstructors(*modules_,*parser_,*commonFactories_);
    wrapperConstructors_=wrapperSet_.constructors;
    wrapperMethods_=std::make_shared<SvgWrapperSourceMethods>(
        context,*properties,*commonFactories_,commonRuntime_->bboxes,*parser_);
    wrapperMethods_->install(wrapperSet_);
    const auto commonFactory=commonFactories_->registerConstructor();
    svgFactoryConstructor_=svgFactories_->registerConstructor(
        wrapperSet_.wrappers,*commonFactories_,commonFactory);
    lite_=std::make_shared<adaptors::lite::LiteModule>(
        context,*functions_,*properties,regexp_);
    (void)tex::HtmlConfiguration::initializeModule(*parser_);
    const std::weak_ptr<adaptors::lite::LiteModule> liteOwner=lite_;
    tex::TexHtmlAdaptorResolver texHtmlAdaptorResolver=
        [liteOwner](ObjectRef identity)->std::shared_ptr<AbstractDOMAdaptor>{
            const auto owner=liteOwner.lock();
            if(!owner)
                throw Exception("TypeError",u"TexHtml LiteAdaptor module was released");
            return owner->resolveAdaptor(identity);
        };
    (void)tex::TexHtmlConfiguration::initializeModule(
        *parser_,*modules_,std::move(texHtmlAdaptorResolver));
    sceneContextNodes_=std::make_shared<SceneContextNodes>(context,u"SvgSceneNodeHandle");
    sceneContextNodes_->bindPropertyBindings(parser_->properties);
    sceneContextNodes_->bindLiteModule(*lite_);
    sceneContextNodes_->bindServiceOwner(shared_from_this());
    svgJax_->bindSceneContextNodes(sceneContextNodes_,lite_);

    const auto menclose=std::find_if(wrapperConstructors_.begin(),wrapperConstructors_.end(),
        [](const auto& entry){return entry.first==u"SvgMenclose";});
    if(menclose==wrapperConstructors_.end())
        throw Exception("TypeError",u"SVG wrapper constructor graph has no SvgMenclose class");
    mencloseSurface_=std::make_shared<SvgMencloseSourceSurface>(
        context,*maps_,commonFactories_,sceneContextNodes_,std::static_pointer_cast<void>(shared_from_this()));
    mencloseSurface_->bindLiteModule(*lite_);
    mencloseSurface_->bindPropertyBindings(parser_->properties);
    (void)mencloseSurface_->install(menclose->second);
    svgJax_->bindMencloseSurface(mencloseSurface_);


    (void)svgJax_->registerClass(commonConstructor_,parser(),newcmFontConstructor_);
}

ObjectRef SvgRenderServiceGraph::extensionObject(NewcmFontExtension extension)const{
    switch(extension){
    case NewcmFontExtension::bboldx:{
        const auto data=fonts::BboldxFontExtensionData();
        return fontSurface_->extensionObject(static_cast<const common::FontExtensionData&>(data));
    }
    case NewcmFontExtension::bbm:{
        const auto data=fonts::BbmFontExtensionData();
        return fontSurface_->extensionObject(static_cast<const common::FontExtensionData&>(data));
    }
    case NewcmFontExtension::dsfont:{
        const auto data=fonts::DsfontFontExtensionData();
        return fontSurface_->extensionObject(static_cast<const common::FontExtensionData&>(data));
    }
    }
    throw Exception("RangeError",u"Unknown NewCM font extension");
}

void SvgRenderServiceGraph::applyFontProfile(ObjectRef jaxInstance){
    (void)context_.kind(jaxInstance);
    for(const auto extension:fontProfile_){
        const auto data=extensionObject(extension);
        // The pinned extension modules export data records. Source common.ts
        // reads jax.addExtension and invokes that actual function with the
        // Jax as receiver; it does not call the FontData class static method.
        const auto method=context_.get(Value(jaxInstance),Key(u"addExtension"));
        const auto* callable=std::get_if<ObjectRef>(&method.data);
        if(!callable||context_.kind(*callable)!=ObjectKind::function)
            throw Exception("TypeError",u"this.addExtension is not a function");
        const std::array<Value,1> args{Value(data)};
        (void)context_.call(*callable,Value(jaxInstance),args);
    }
}

std::shared_ptr<SvgRenderServiceGraph> SvgRenderServiceGraph::create(
    Context& context,common::FontLoadHost host,NewcmFontProfile profile){
    auto result=std::shared_ptr<SvgRenderServiceGraph>(
        new SvgRenderServiceGraph(context,std::move(host),std::move(profile)));
    result->initialize();
    result->retainRegisteredConstructors();
    return result;
}

void SvgRenderServiceGraph::retainForConstructor(ObjectRef constructor){
    (void)context_.kind(constructor);
    if(std::find(retainedConstructors_.begin(),retainedConstructors_.end(),constructor)
       !=retainedConstructors_.end())return;
    auto lease=std::make_shared<ConstructorServiceLease>();
    lease->services=shared_from_this();
    auto& constructorPayload=context_.payload(constructor);
    if(!constructorPayload.has_value()){
        constructorPayload=std::move(lease);
    }else{
        // NativeIdentityBindings and other constructor owners use the
        // constructor's payload as a typed identity record. Preserve that
        // exact any type and keep the service graph on the constructor's
        // canonical prototype, which is owned by the same Context identity.
        const auto prototypeValue=context_.get(Value(constructor),Key(u"prototype"));
        const auto* prototype=std::get_if<ObjectRef>(&prototypeValue.data);
        if(!prototype)throw Exception("TypeError",u"Source constructor has no prototype owner");
        auto& prototypePayload=context_.payload(*prototype);
        if(prototypePayload.has_value())
            throw Exception("Unimplemented",u"Source constructor prototype already has an owner payload");
        prototypePayload=std::move(lease);
    }
    retainedConstructors_.push_back(constructor);
}

void SvgRenderServiceGraph::retainRegisteredConstructors(){
    retainForConstructor(outputJax_->abstractClass());
    retainForConstructor(commonConstructor_);
    // Font constructor payloads are the canonical NativeBindingPayload for
    // FontDataClass*. Their constructor callbacks already strongly retain this
    // graph through hostAnchor, so preserve the exact payload type here.
    retainForConstructor(commonFactories_->constructor());
    retainForConstructor(svgFactoryConstructor_);
    retainForConstructor(svgJax_->constructor());
    retainForConstructor(lite_->adaptorConstructor());
    retainForConstructor(lite_->parserConstructor());
    for(const auto& [name,constructor]:wrapperConstructors_){
        (void)name;
        retainForConstructor(constructor);
    }
}

NativeRenderSession::NativeRenderSession(common::FontLoadHost fontHost,NewcmFontProfile profile)
    :context_(std::make_shared<Context>()){
    services_=SvgRenderServiceGraph::create(*context_,std::move(fontHost),std::move(profile));
}

std::size_t NativeRenderSession::dispatchMactionTimers(){
    if(!services_)return 0;
    return services_->mactionTimers().dispatchDue();
}

std::shared_ptr<mml::MmlFactory> NativeRenderSession::mathFactory()const{
    if(!services_)return {};
    auto lease=std::make_shared<SessionFactoryLease>();
    lease->context=context_;
    lease->services=services_;
    lease->factory=std::shared_ptr<mml::MmlFactory>(services_,&services_->mmlFactory());
    return std::shared_ptr<mml::MmlFactory>(lease,lease->factory.get());
}

std::shared_ptr<adaptors::lite::LiteAdaptor> NativeRenderSession::adaptor()const{
    if(!services_||adaptorObject_==ObjectRef{})return {};
    auto actual=services_->lite().resolveAdaptor(adaptorObject_);
    auto lease=std::make_shared<SessionAdaptorLease>();
    lease->context=context_;
    lease->services=services_;
    lease->adaptor=std::move(actual);
    return std::shared_ptr<adaptors::lite::LiteAdaptor>(lease,lease->adaptor.get());
}

std::shared_ptr<SvgOutputJax> NativeRenderSession::construct(Value supplied){
    if(!services_)throw Exception("TypeError",u"Render session has no service graph");
    auto actual=services_->svgJax().construct(std::move(supplied));
    services_->applyFontProfile(actual->instanceIdentity());
    auto adaptor=services_->lite().constructAdaptor();
    adaptorObject_=services_->lite().bindAdaptor(adaptor);
    actual->setAdaptor(core::OutputAdaptorReference{Value(adaptorObject_)});
    // LiteAdaptor's Source `nodeSize()` reads the real textContent and current
    // adaptor options. Reuse one detached probe pair so each cache miss does
    // not add permanent records to its node store.
    auto textProbe=std::make_shared<LiteTextMeasurementProbe>();
    textProbe->text=adaptor->text(u"");
    textProbe->element=adaptor->node(u"text",{},PropertyArray{nodeProperty(textProbe->text)});
    const std::weak_ptr<adaptors::lite::LiteAdaptor> adaptorOwner=adaptor;
    actual->bindTextMeasurementProvider([adaptorOwner,textProbe](std::u16string_view chars){
        const auto owner=adaptorOwner.lock();
        if(!owner)throw Exception("TypeError",u"Native Lite text measurement adaptor was released");
        adaptors::lite::LiteText probeText(textProbe->text);
        const auto previous=probeText.value();
        struct RestoreProbeText {
            adaptors::lite::LiteText& probe;
            std::u16string previous;
            ~RestoreProbeText(){probe.value(std::move(previous));}
        } restore{probeText,previous};
        probeText.value(std::u16string(chars));
        return owner->nodeSize(textProbe->element,1000,true).first;
    });
    if(auto visitor=services_->modules().find<common::LinebreakVisitorContextBindings>(
        u"@mathjax/src/ts/output/common/LinebreakVisitor"))
        services_->retainForConstructor(visitor->constructor());
    if(auto style=services_->modules().find<common::StyleJsonContextBindings>(
        u"@mathjax/src/ts/util/StyleJson"))
        services_->retainForConstructor(style->constructor());
    services_->retainForConstructor(services_->lite().stylesConstructor());

    // Jax stores borrowed service references. The aliasing lease makes the
    // returned handle retain exactly this Context, graph and actual native Jax.
    auto lease=std::make_shared<SessionJaxLease>();
    lease->context=context_;
    lease->services=services_;
    lease->jax=std::move(actual);
    jax_=std::shared_ptr<SvgOutputJax>(lease,lease->jax.get());
    return jax_;
}
}
