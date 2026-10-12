// Ordinary Source data descriptors backed by an actual native owner's fields.
export module folia.math.util.native_data_fields;
export import folia.math.util.option_types;
import std;
export namespace folia::math::options {
template<class Owner>
void bindNativeValue(Context& context,ObjectRef object,Key key,
                     const std::shared_ptr<Owner>& owner,Value Owner::* member){
    context.bindData(object,key,std::make_shared<NativeDataField>(owner,
        [owner,member]{return owner.get()->*member;},
        [owner,member](Value value){owner.get()->*member=std::move(value);},
        [owner,member]{owner.get()->*member=Undefined{};}));
}
}
