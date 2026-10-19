// MathJax 4.1.3 core/MathList.ts; Apache-2.0.
export module folia.math.core.math_list;
export import folia.math.core.math_item;
export import folia.math.core.math_location;
export import folia.math.util.linked_list;
import std;

export namespace folia::math::core {
class AbstractMathList:public util::LinkedList<MathItem*>,public MathListService {
    MathLocationService* locationAccess_;
    Property locationProperty(MathItem&,std::string_view member);
    Property readStartField(MathItem&,std::string_view field);
public:
    AbstractMathList(util::LinkedListModule& sourceModule,MathLocationService* locations=nullptr)
        :util::LinkedList<MathItem*>(sourceModule.end()),locationAccess_(locations){}
    bool isBefore(MathItem* const& a,MathItem* const& b)override;
    void forEach(const std::function<void(MathItem&)>&)override;
};
}
