// Borrowed edges into the one ParseOptions/session, no parallel parser state.
export module folia.math.input.tex.stack_services;
export import folia.math.input.tex.node_factory;
import std;
export namespace folia::math::tex {
using TeXNodeFactoryReference=std::variant<Missing,PropertyNull,NodeFactory*>;
class StackItemFactoryConfiguration {
public:
    virtual ~StackItemFactoryConfiguration()=default;
    virtual TeXNodeFactoryReference nodeFactoryReference()const=0;
};
using StackConfigurationReference=std::variant<Missing,PropertyNull,StackItemFactoryConfiguration*>;
class StackItemFactoryService {
public:
    virtual ~StackItemFactoryService()=default;
    StackConfigurationReference configuration=PropertyNull{};
};
using StackFactoryReference=std::variant<Missing,PropertyNull,StackItemFactoryService*>;
}
