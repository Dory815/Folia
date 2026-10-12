// Portable host edge for Source String.normalize('NFD'). No approximation.
export module folia.math.util.unicode_normalization;
import std;
export namespace folia::math {
class UnicodeNormalization {
public:
    virtual ~UnicodeNormalization()=default;
    virtual std::u16string nfd(std::u16string_view)=0;
};
// Same Unicode 16.0 data version as the pinned original Node/ICU reference.
class Unicode16Normalization final:public UnicodeNormalization {
public:
    std::u16string nfd(std::u16string_view)override;
};
}
