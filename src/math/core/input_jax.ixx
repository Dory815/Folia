// MathJax 4.1.3 core/InputJax.ts; native string compilation service. Apache-2.0.
export module folia.math.core.input_jax;
export import folia.math.core.math_item;
export import folia.math.util.function_list;
import std;

export namespace folia::math::core {
template <class Data> struct InputJaxFilterData {
    MathItem &math;
    mml::MmlDocument &document;
    Data &data;
    MathOutputJaxService *output{}; // borrowed only for this compile/filter invocation
};
template <class Options, class Data> class AbstractInputJax : public MathInputJaxService {
  protected:
    mml::MmlFactory *mmlFactory_;
    AbstractInputJax(mml::MmlFactory &factory, Options supplied)
        : mmlFactory_(&factory), options(std::move(supplied)) {
        if (options.preFilters)
            preFilters.addList(*options.preFilters);
        if (options.postFilters)
            postFilters.addList(*options.postFilters);
    }
    Data &executeFilters(FunctionList<InputJaxFilterData<Data> &> &filters, MathItem &math,
                         mml::MmlDocument &document, Data &data,
                         MathOutputJaxService *output = nullptr) {
        InputJaxFilterData<Data> arguments{math, document, data, output};
        (void)filters.execute(arguments);
        return arguments.data;
    }

  public:
    Options options;
    FunctionList<InputJaxFilterData<Data> &> preFilters, postFilters;
    virtual std::string_view name() const noexcept = 0;
    virtual bool processStrings() const noexcept {
        return true;
    }
    virtual void setMmlFactory(mml::MmlFactory &factory) {
        mmlFactory_ = &factory;
    }
    virtual void initialize() {}
};
} // namespace folia::math::core
