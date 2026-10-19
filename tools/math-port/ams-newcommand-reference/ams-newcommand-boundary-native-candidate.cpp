// Supplemental real-runtime candidate. Reuse the strict serializers and actual
// RuntimeGraph from the frozen 24-case candidate; this does not alter its case
// list, output, or acceptance path.
#define main ams_newcommand_frozen_candidate_main
#include "ams-newcommand-native-candidate.cpp"
#undef main

namespace {
struct ParseOptionsAddNodeSpy final:tex::NodeFactoryConfiguration{
 tex::ParseOptions& owner;
 std::vector<std::string> kinds;
 std::vector<mml::NodeValue> underovers;
 explicit ParseOptionsAddNodeSpy(tex::ParseOptions& actualOwner):owner(actualOwner){}
 void addNode(Property property,mml::NodeValue node)override{
  if(const auto* value=std::get_if<std::string>(&property.value))kinds.push_back(*value);
  else kinds.push_back("<non-string>");
  if(const auto* value=std::get_if<std::string>(&property.value);value&&*value=="munderover")underovers.push_back(node);
  owner.addNode(std::move(property),node);
 }
};
struct NodeConfigurationRestore{
 std::shared_ptr<tex::NodeFactory> factory;
 tex::NodeFactoryConfigurationReference previous;
 NodeConfigurationRestore(std::shared_ptr<tex::NodeFactory> value,tex::NodeFactoryConfigurationReference prior)
  :factory(std::move(value)),previous(std::move(prior)){}
 ~NodeConfigurationRestore(){if(factory)factory->configuration=previous;}
};
const Case boundaryCases[]={
 {"newcommand-standard-name-redefinition",u"\\newcommand{\\frac}{x}",{u"base",u"newcommand",u"ams"}},
 {"newcommand-missing-definition",u"\\newcommand{\\zz}[2]",{u"base",u"newcommand",u"ams"}},
 {"newcommand-illegal-control-sequence-name",u"\\newcommand{bad!}{x}",{u"base",u"newcommand",u"ams"}},
 {"newcommand-empty-optional-default",u"\\newcommand{\\zz}[2][]{#1/#2}\\zz{q}",{u"base",u"newcommand",u"ams"}},
 {"newcommand-explicit-empty-optional",u"\\newcommand{\\zz}[2][d]{#1/#2}\\zz[]{q}",{u"base",u"newcommand",u"ams"}},
 {"def-delimited-argument-match",u"\\def\\zz#1,#2;{#2/#1}\\zz a,b;",{u"base",u"newcommand",u"ams"}},
 {"def-delimited-argument-mismatch",u"\\def\\zz#1,#2;{#1}\\zz a;c",{u"base",u"newcommand",u"ams"}},
 {"def-runaway-delimited-argument",u"\\def\\zz#1,{#1}\\zz abc",{u"base",u"newcommand",u"ams"}},
 {"def-missing-control-sequence",u"\\def foo{x}",{u"base",u"newcommand",u"ams"}},
 {"let-undefined-control-sequence",u"\\let\\zz=\\notDefined\\zz",{u"base",u"newcommand",u"ams"}},
 {"let-existing-newcommand",u"\\newcommand{\\orig}{x}\\let\\copy=\\orig\\copy",{u"base",u"newcommand",u"ams"}},
 {"let-existing-character-token",u"\\let\\zz=\\alpha\\zz",{u"base",u"newcommand",u"ams"}},
 {"let-special-character",u"\\let\\zz=^\\zz x",{u"base",u"newcommand",u"ams"}},
 {"let-special-delimiter",u"\\let\\paren=(\\paren",{u"base",u"newcommand",u"ams"}},
 {"new-environment-omitted-optional-default",u"\\newenvironment{xx}[1][d]{(#1)}{!}\\begin{xx}q\\end{xx}",{u"base",u"newcommand",u"ams"}},
 {"new-environment-explicit-empty-optional",u"\\newenvironment{xx}[1][d]{(#1)}{!}\\begin{xx}[]q\\end{xx}",{u"base",u"newcommand",u"ams"}},
 {"new-environment-mismatched-end",u"\\newenvironment{xx}{a}{b}\\begin{xx}x\\end{yy}",{u"base",u"newcommand",u"ams"}},
 {"declare-math-operator-unstarred",u"\\DeclareMathOperator{\\rank}{rank}\\rank(A)",{u"base",u"newcommand",u"ams"}},
 {"declare-math-operator-starred",u"\\DeclareMathOperator*{\\limfoo}{lim}\\limfoo_n",{u"base",u"newcommand",u"ams"}},
 {"ams-arrow-empty-labels",u"\\xrightarrow[]{}+\\xleftarrow{}",{u"base",u"newcommand",u"ams"}},
 {"ams-shove-outside-multline",u"\\shoveleft{x}",{u"base",u"newcommand",u"ams"}},
 {"ams-shove-after-row-content",u"\\begin{multline}a\\shoveleft b\\end{multline}",{u"base",u"newcommand",u"ams"}},
 {"ams-cfrac-illegal-alignment",u"\\cfrac[x]{a}{b}",{u"base",u"newcommand",u"ams"}},
 {"ams-cfrac-right-alignment",u"\\cfrac[r]{a}{b}",{u"base",u"newcommand",u"ams"}},
 {"ams-tag-duplicate",u"\\begin{equation}\\tag{a}\\tag{b}x\\end{equation}",{u"base",u"newcommand",u"ams"}},
 {"ams-tag-starred",u"\\begin{equation}\\tag*{A}x\\end{equation}",{u"base",u"newcommand",u"ams"}},
 {"ams-tag-untaggable-environment",u"\\begin{split}a&=b\\tag{x}\\end{split}",{u"base",u"newcommand",u"ams"}},
 {"ams-alignat-invalid-column-count",u"\\begin{alignat}{x}a&=b\\end{alignat}",{u"base",u"newcommand",u"ams"}},
 {"ams-genfrac-invalid-style",u"\\genfrac{(}{)}{0pt}{4}{a}{b}",{u"base",u"newcommand",u"ams"}},
 {"ams-xalignat-row",u"\\begin{xalignat}{1}a&=b\\end{xalignat}",{u"base",u"newcommand",u"ams"}},
 {"ams-alignedat-two-pairs",u"\\begin{alignedat}{2}a&=b&c&=d\\end{alignedat}",{u"base",u"newcommand",u"ams"}},
 {"ams-shoveright-first-row",u"\\begin{multline}\\shoveright a+b\\\\c+d\\end{multline}",{u"base",u"newcommand",u"ams"}},
 {"ams-arrow-bottom-label",u"\\xleftarrow[a]{b}",{u"base",u"newcommand",u"ams"}},
 {"ams-genfrac-text-style",u"\\genfrac{(}{)}{0pt}{1}{a}{b}",{u"base",u"newcommand",u"ams"}},
 {"ams-operatorname-limits",u"\\operatorname{lim}\\limits_{n}",{u"base",u"newcommand",u"ams"}},
 {"ams-multint-with-limits",u"\\iint\\limits_a^b",{u"base",u"newcommand",u"ams"}},
};
}

int main(int argc,char**argv){
 try{
  RuntimeGraph graph;Serializer serializer{graph.context,graph.functions};
  std::string output="{\"cases\":{";
  for(std::size_t i=0;i<std::size(boundaryCases);++i){
   const auto& c=boundaryCases[i];if(i)output+=',';output+=quote(c.name)+":";
   std::string result="null",error="null",maps="null",packageData="null";tex::TeXRef jax;
   ParseOptionsAddNodeSpy* activeAddNodeSpy=nullptr;
   std::optional<NodeConfigurationRestore> restoreNodeConfiguration;
   std::optional<ParseOptionsAddNodeSpy> addNodeSpy;
   try{
    jax=graph.jax(c.packages);
    CandidateMathItem math(Property(property_host::debugText(c.input)),
      core::MathInputJaxReference(static_cast<core::MathInputJaxService*>(jax.get())),Property(true));
    tex::ParseOptionsRef arrowOptions;
    if(c.name=="ams-arrow-empty-labels"){
     const auto sharedOptions=jax->parseOptions();
     const auto* options=std::get_if<tex::ParseOptionsRef>(&sharedOptions);
     if(!options||!*options)throw std::runtime_error("arrow addNode spy has no real ParseOptions");
     arrowOptions=*options;
     const auto nodeFactory=(*options)->nodeFactoryOwnerRef();
     if(!nodeFactory)throw std::runtime_error("arrow addNode spy has no real NodeFactory");
     addNodeSpy.emplace(**options);
     activeAddNodeSpy=&*addNodeSpy;
     restoreNodeConfiguration.emplace(nodeFactory,nodeFactory->configuration);
     nodeFactory->configuration=activeAddNodeSpy;
    }
    if(c.name=="ams-arrow-empty-labels"){
     auto observer=std::make_shared<Function>();
     const auto expectedOwner=arrowOptions;
     observer->call=[&graph,expectedOwner](const FunctionReceiver&,FunctionArgs args)->std::any{
      if(args.empty())throw std::runtime_error("arrow prefilter observer missing data");
      const auto* data=std::any_cast<core::InputJaxFilterDataRef>(&args[0]);
      if(!data||!*data)throw std::runtime_error("arrow prefilter observer invalid data");
      const auto current=(*data)->currentData();
      const auto* owner=std::any_cast<tex::ParseOptionsRef>(&current);
      if(!owner||!*owner)throw std::runtime_error("arrow prefilter observer missing ParseOptions");
      const auto raw=graph.context.get((*owner)->nodeLists,opts::Key(u"munderover"));
      std::uint32_t length=0;
      std::string storage="non-array";
      if(const auto* array=std::get_if<opts::ObjectRef>(&raw.data);
         array&&graph.context.kind(*array)==opts::ObjectKind::array){
       length=graph.context.arrayLength(*array);
       storage="context-array";
      }
      const auto listOwner=(*owner)->nodeLists;
      const auto* listObject=std::get_if<opts::ObjectRef>(&listOwner.data);
      const auto expectedLists=expectedOwner->nodeLists;
      const auto* expectedObject=std::get_if<opts::ObjectRef>(&expectedLists.data);
      std::cerr<<"arrow-before-cleanSubSup raw-munderover-count="<<length
               <<" storage="<<storage<<" value-variant="<<raw.data.index()
               <<" same-options="<<((*owner).get()==expectedOwner.get())
               <<" same-nodeLists="<<(listObject&&expectedObject&&*listObject==*expectedObject)
               <<" nodeLists-kind="<<(listObject?static_cast<int>(graph.context.kind(*listObject)):-1)
               <<" keys=";
      if(listObject){
       const auto keys=graph.context.ownKeys(opts::Value(*listObject));
       for(std::size_t i=0;i<keys.size();++i){
        if(i)std::cerr<<',';
        if(const auto* name=std::get_if<std::u16string>(&keys[i].data))std::cerr<<utf8(*name);
        else std::cerr<<"<symbol>";
       }
      }
      std::cerr<<'\n';
      return std::any(std::monostate{});
     };
     jax->postFilters->add(FunctionValue(FunctionRef(observer)),-7.5);
    }
    result=serializer.node(jax->compile(math,core::MathDocumentReference(Missing{})));
   }catch(const tex::TexError&e){
    error="{\"name\":\"TexError\",\"id\":"+quote(utf8(e.id))+",\"message\":"+quote(utf8(e.message))+"}";
   }catch(const opts::Exception&e){
    error="{\"name\":"+quote(e.name)+",\"id\":null,\"message\":"+quote(utf8(e.message))+"}";
   }catch(const std::exception&e){
    error="{\"name\":\"Error\",\"id\":null,\"message\":"+quote(e.what())+"}";
   }
   if(activeAddNodeSpy){
    std::size_t count=0;for(const auto& kind:addNodeSpy->kinds)if(kind=="munderover")++count;
    std::cerr<<"arrow-addNode-observer total="<<addNodeSpy->kinds.size()<<" munderover="<<count<<" kinds=";
    for(std::size_t k=0;k<addNodeSpy->kinds.size();++k){if(k)std::cerr<<',';std::cerr<<addNodeSpy->kinds[k];}
    std::cerr<<'\n';
   }
   if(c.name=="ams-arrow-empty-labels"&&jax){
    const auto raw=jax->parseOptions();
    if(const auto* po=std::get_if<tex::ParseOptionsRef>(&raw);po&&*po){
     const auto root=(*po)->root;
     const auto list=(*po)->getList(u"munderover");
     const auto rawList=graph.context.get((*po)->nodeLists,opts::Key(u"munderover"));
     std::uint32_t rawCount=0;
     if(const auto* rawArray=std::get_if<opts::ObjectRef>(&rawList.data);
        rawArray&&graph.context.kind(*rawArray)==opts::ObjectKind::array)
      rawCount=graph.context.arrayLength(*rawArray);
     std::cerr<<"arrow-diagnostic root="<<(root.isNode()?root.requireNode("kind")->kind():"missing")
              <<" raw-munderover-count="<<rawCount
              <<" root-tree="<<(root.isNode()?serializer.node(root):"null")
              <<" munderover-count="<<(list?list->size():0);
     if(list)for(std::size_t k=0;k<list->size();++k){
      const auto item=list->get(k);const auto children=item.isNode()?item.requireNode("childNodes")->childNodes:mml::Children{};
      std::cerr<<" ["<<k<<":"<<(item.isNode()?item.requireNode("kind")->kind():"non-node")
               <<" children="<<(children?children->size():0);
      if(children)for(std::size_t n=0;n<children->size();++n){const auto child=children->get(n);std::cerr<<","<<(child.isNode()?child.requireNode("kind")->kind():child.kind()==tree::ChildKind::Undefined?"undefined":"null");}
      std::cerr<<"]";
     }
     std::cerr<<'\n';
     for(std::size_t k=0;k<addNodeSpy->underovers.size();++k){
      const auto old=addNodeSpy->underovers[k];
      const auto rawParent=old.requireNode("parent")->getParent();
      const auto logicalParent=old.requireNode("parent")->Parent();
      const auto rawIndex=rawParent.isNode()?rawParent.requireNode("childIndex")->childIndex(old):std::nullopt;
      const auto logicalIndex=logicalParent.isNode()?logicalParent.requireNode("childIndex")->childIndex(old):std::nullopt;
      const auto parentChildren=rawParent.isNode()?rawParent.requireNode("childNodes")->childNodes:mml::Children{};
      std::cerr<<"arrow-old-node["<<k<<"] raw-parent="<<(rawParent.isNode()?rawParent.requireNode("kind")->kind():"null")
               <<" raw-index="<<(rawIndex?std::to_string(*rawIndex):"missing")
               <<" logical-parent="<<(logicalParent.isNode()?logicalParent.requireNode("kind")->kind():"null")
               <<" logical-index="<<(logicalIndex?std::to_string(*logicalIndex):"missing")
               <<" raw-parent-child-count="<<(parentChildren?std::to_string(parentChildren->size()):"null")<<'\n';
     }
    }
   }
   if(jax){
    maps=mapState(serializer,*jax,graph.runtime);auto po=jax->parseOptions();auto*p=std::get_if<tex::ParseOptionsRef>(&po);
    if(!p||!*p)throw std::runtime_error("missing ParseOptions after compile");
    const auto*data=std::get_if<std::shared_ptr<SourceStringMap<opts::Value>>>(&(*p)->packageData);
    if(!data||!*data)throw std::runtime_error("missing packageData map");
    packageData="{\"$\":\"map\",\"entries\":[";auto keys=(*data)->keys();bool first=true;
    while(auto entry=keys.next()){
     if(!first)packageData+=',';first=false;auto val=(*data)->get(*entry);
     if(!val)throw std::runtime_error("packageData key disappeared");
     packageData+="["+quote(utf8(*entry))+","+serializer.value(*val)+"]";
    }
    packageData+="]}";
   }
   output+="{\"result\":"+result+",\"error\":"+error+",\"dynamicMaps\":"+maps+",\"packageData\":"+packageData+"}";
  }
  output+="}}\n";const std::filesystem::path path=argc>1?argv[1]:"build/math-port/ams-newcommand-boundary-native.json";
  std::filesystem::create_directories(path.parent_path());std::ofstream file(path,std::ios::binary);
  if(!file)throw std::runtime_error("cannot open AMS boundary candidate output");file<<output;
  if(!file)throw std::runtime_error("failed writing AMS boundary candidate output");return 0;
 }catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}
}
