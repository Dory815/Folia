// MathJax 4.1.3 core/FindMath.ts; Apache-2.0.
export module folia.math.core.find_math;
export import folia.math.core.math_item;
import std;

export namespace folia::math::core {
class FindMath {
public:
    virtual ~FindMath() = default;
    virtual std::vector<ProtoItem> findMath(
        std::span<std::u16string const> strings) = 0;
};

template<class Options>
class AbstractFindMath : public FindMath {
protected:
    Options options_;

    explicit AbstractFindMath(Options options = {})
        : options_(std::move(options)) {}

public:
    AbstractFindMath(AbstractFindMath const&) = delete;
    AbstractFindMath& operator=(AbstractFindMath const&) = delete;
    Options const& options() const noexcept { return options_; }
};
}
