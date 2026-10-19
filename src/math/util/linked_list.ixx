// MathJax 4.1.3 util/LinkedList.ts; Apache-2.0.
export module folia.math.util.linked_list;
export import folia.math.util.options;
import std;

export namespace folia::math::util {
namespace detail {
template<class T> bool sameValueZero(const T& left,const T& right){
    if constexpr(std::is_floating_point_v<T>){
        return left==right||(std::isnan(left)&&std::isnan(right));
    }else if constexpr(std::same_as<T,options::Value>){
        if(left.data.index()!=right.data.index())return false;
        return std::visit([&](const auto& value){
            using V=std::remove_cvref_t<decltype(value)>;
            if constexpr(std::same_as<V,double>){const auto other=std::get<double>(right.data);return value==other||(std::isnan(value)&&std::isnan(other));}
            else return value==std::get<V>(right.data);
        },left.data);
    }else if constexpr(requires{left.value.index();}){
        if(left.value.index()!=right.value.index())return false;
        return std::visit([&](const auto& value){
            using V=std::remove_cvref_t<decltype(value)>;
            if constexpr(std::same_as<V,double>){const auto other=std::get<double>(right.value);return value==other||(std::isnan(value)&&std::isnan(other));}
            else return value==std::get<V>(right.value);
        },left.value);
    }else return left==right;
}
}
// The pinned Source exports one module-level END = Symbol().  The native
// module loader owns one of these per loaded Context and passes end() to every
// list, including singleton lists created by sort().  Lists never mint their
// own sentinel identity.
class LinkedListModule {
    options::Context* context_;
    options::SymbolRef end_;
public:
    explicit LinkedListModule(options::Context& context)
        :context_(&context),end_(context.symbol()){}
    LinkedListModule(const LinkedListModule&)=delete;
    LinkedListModule& operator=(const LinkedListModule&)=delete;
    options::Context& context()const{return *context_;}
    options::SymbolRef end()const{return end_;}
};
// The loaded native module owns this cache. It is deliberately not static:
// unloading the Source module clears its Context-owned symbol references.
template<class T> struct ListItem {
    static_assert(!std::same_as<T,options::SymbolRef>,"ListItem payload must not alias END");
    std::variant<T,options::SymbolRef> data;
    ListItem* next=nullptr;
    ListItem* prev=nullptr;
    ListItem() requires std::default_initializable<T> :data(T{}){}
    explicit ListItem(T value):data(std::move(value)){}
    explicit ListItem(options::SymbolRef end):data(end){}
};

template<class T> class ListItemRef {
    // Keep the owning list arena alive without coupling this public handle to
    // LinkedList's private arena type.  The node is still the actual list node;
    // this erased pointer is only its graph lease.
    std::shared_ptr<void> arena_;
    std::shared_ptr<ListItem<T>> item_;
public:
    ListItemRef()=default;
    template<class Arena>
    ListItemRef(std::shared_ptr<Arena> arena,std::shared_ptr<ListItem<T>> item)
        :arena_(std::move(arena)),item_(std::move(item)){}
    ListItem<T>* operator->()const{return item_.get();}
    ListItem<T>& operator*()const{return *item_;}
    explicit operator bool()const noexcept{return bool(item_);}
    // Weak probes expose lifetime only; they do not grant ownership or alter
    // Source-visible list behavior. They let native acceptance verify that the
    // arena and nodes are reclaimed after the final list/iterator/node lease.
    std::weak_ptr<void> weakArena()const{return arena_;}
    std::weak_ptr<ListItem<T>> weakItem()const{return item_;}
    friend bool operator==(const ListItemRef& a,const ListItemRef& b){return a.item_==b.item_;}
};

template<class T> class LinkedList {
public:
    using Item=ListItem<T>;
    using Node=ListItemRef<T>;
    using Compare=std::function<bool(const T&,const T&)>;
private:
    struct Arena:std::enable_shared_from_this<Arena>{
        using Ref=std::shared_ptr<Arena>;
        std::vector<std::shared_ptr<Item>> items;
        Ref parent;
        Ref root(){auto value=this->shared_from_this();while(value->parent)value=value->parent;return value;}
        std::shared_ptr<Item> make(T value){auto item=std::make_shared<Item>(std::move(value));root()->items.push_back(item);return item;}
        std::shared_ptr<Item> makeEnd(options::SymbolRef end){auto item=std::make_shared<Item>(end);root()->items.push_back(item);return item;}
        static void unite(const Ref& into,const Ref& from){
            const auto target=into->root(),source=from->root();
            if(target==source)return;
            target->items.insert(target->items.end(),
                std::make_move_iterator(source->items.begin()),
                std::make_move_iterator(source->items.end()));
            source->items.clear();
            source->parent=target;
        }
    };
    std::shared_ptr<Arena> arena_=std::make_shared<Arena>();
    std::shared_ptr<Item> list_;
    options::SymbolRef end_;
    void initialize(){list_=arena_->makeEnd(end_);list_->next=list_.get();list_->prev=list_.get();}
    static void detach(Item* item){item->next=nullptr;item->prev=nullptr;}
    static void spliceBefore(Item* at,Item* item){
        item->prev=at->prev;item->next=at;at->prev->next=item;at->prev=item;
    }
    static void linkBetween(Item* left,Item* right,Item* first,Item* last){
        left->next=first;first->prev=left;last->next=right;right->prev=last;
    }
    Compare defaultCompare(){return [this](const T& a,const T& b){return isBefore(a,b);};}
    bool isEnd(const Item* item)const{
        if(!item)return false;
        const auto* symbol=std::get_if<options::SymbolRef>(&item->data);
        return symbol&&*symbol==end_;
    }
    std::shared_ptr<Item> own(Item* item)const{
        if(!item)return {};
        const auto root=arena_->root();
        const auto found=std::find_if(root->items.begin(),root->items.end(),[&](const auto& value){return value.get()==item;});
        return found==root->items.end()?std::shared_ptr<Item>{}:*found;
    }

public:
    class Iterator {
        std::shared_ptr<Arena> arena_;
        Item* current_=nullptr;
        options::SymbolRef marker_;
        bool atEnd()const{
            if(!current_)return false;
            const auto* symbol=std::get_if<options::SymbolRef>(&current_->data);
            return symbol&&*symbol==marker_;
        }
        static void nullCurrent(){throw options::Exception("TypeError",u"Cannot read properties of null (reading 'data')");}
    public:
        using iterator_category=std::forward_iterator_tag;
        using value_type=T;
        using difference_type=std::ptrdiff_t;
        using reference=const T&;
        using pointer=const T*;
        Iterator()=default;
        Iterator(std::shared_ptr<Arena> arena,Item* current,options::SymbolRef marker)
            :arena_(std::move(arena)),current_(current),marker_(marker){}
        reference operator*()const{if(!current_)nullCurrent();return std::get<T>(current_->data);}
        pointer operator->()const{return &**this;}
        Iterator& operator++(){if(!current_)nullCurrent();current_=current_->next;return *this;}
        Iterator operator++(int){auto copy=*this;++*this;return copy;}
        bool operator==(const Iterator& other)const{
            if(marker_!=other.marker_)return false;
            return current_==other.current_||(atEnd()&&other.atEnd());
        }
        bool isEnd()const{return atEnd();}
    };
    class ReverseIterator {
        std::shared_ptr<Arena> arena_;
        Item* current_=nullptr;
        options::SymbolRef marker_;
        bool atEnd()const{
            if(!current_)return false;
            const auto* symbol=std::get_if<options::SymbolRef>(&current_->data);
            return symbol&&*symbol==marker_;
        }
    public:
        using iterator_category=std::forward_iterator_tag;
        using value_type=T;
        using difference_type=std::ptrdiff_t;
        using reference=const T&;
        using pointer=const T*;
        ReverseIterator()=default;
        ReverseIterator(std::shared_ptr<Arena> arena,Item* current,options::SymbolRef marker)
            :arena_(std::move(arena)),current_(current),marker_(marker){}
        reference operator*()const{if(!current_)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'data')");return std::get<T>(current_->data);}
        pointer operator->()const{return &**this;}
        ReverseIterator& operator++(){if(!current_)throw options::Exception("TypeError",u"Cannot read properties of null (reading 'data')");current_=current_->prev;return *this;}
        ReverseIterator operator++(int){auto copy=*this;++*this;return copy;}
        bool operator==(const ReverseIterator& other)const{
            if(marker_!=other.marker_)return false;
            return current_==other.current_||(atEnd()&&other.atEnd());
        }
        bool isEnd()const{return atEnd();}
    };

    explicit LinkedList(options::SymbolRef end):end_(end){initialize();}
    LinkedList(options::SymbolRef end,std::span<const T> values):end_(end){initialize();push(values);}
    template<class... U> requires(sizeof...(U)>0&&(std::constructible_from<T,U&&>&&...))
    LinkedList(options::SymbolRef end,U&&... values):end_(end){initialize();(push(T(std::forward<U>(values))),...);}
    LinkedList(const LinkedList&)=delete;
    LinkedList& operator=(const LinkedList&)=delete;
    virtual ~LinkedList()=default;
    virtual bool isBefore(const T& a,const T& b){return a<b;}
    Node sentinel()const{return Node(arena_->root(),own(list_.get()));}
    options::SymbolRef endSymbol()const{return end_;}
    Iterator begin()const{return Iterator(arena_->root(),list_->next,end_);}
    Iterator end()const{return Iterator(arena_->root(),list_.get(),end_);}
    ReverseIterator rbegin()const{return ReverseIterator(arena_->root(),list_->prev,end_);}
    ReverseIterator rend()const{return ReverseIterator(arena_->root(),list_.get(),end_);}
    bool empty()const{return isEnd(list_->next);}
    LinkedList& push(std::span<const T> values){
        for(const auto& value:values){auto item=arena_->make(value);item->next=list_.get();item->prev=list_->prev;list_->prev->next=item.get();list_->prev=item.get();}
        return *this;
    }
    LinkedList& push(){return *this;}
    LinkedList& push(T value){const std::array<T,1> values{std::move(value)};return push(values);}
    template<class... U> requires(sizeof...(U)>1&&(std::constructible_from<T,U&&>&&...))
    LinkedList& push(U&&... values){(push(T(std::forward<U>(values))),...);return *this;}
    LinkedList& unshift(std::span<const T> values){
        for(auto i=values.rbegin();i!=values.rend();++i){auto item=arena_->make(*i);item->next=list_->next;item->prev=list_.get();list_->next->prev=item.get();list_->next=item.get();}
        return *this;
    }
    LinkedList& unshift(){return *this;}
    LinkedList& unshift(T value){const std::array<T,1> values{std::move(value)};return unshift(values);}
    template<class... U> requires(sizeof...(U)>1&&(std::constructible_from<T,U&&>&&...))
    LinkedList& unshift(U&&... values){const std::array<T,sizeof...(U)> snapshot{T(std::forward<U>(values))...};return unshift(snapshot);}
    std::optional<T> pop(){
        const auto item=list_->prev;if(isEnd(item))return {};
        list_->prev=item->prev;item->prev->next=list_.get();auto value=std::get<T>(item->data);detach(item);return value;
    }
    std::optional<T> shift(){
        const auto item=list_->next;if(isEnd(item))return {};
        list_->next=item->next;item->next->prev=list_.get();auto value=std::get<T>(item->data);detach(item);return value;
    }
    LinkedList& remove(std::span<const T> values){
        auto item=list_->next;
        while(!isEnd(item)){const auto next=item->next;
            if(std::any_of(values.begin(),values.end(),[&](const T& value){return detail::sameValueZero(value,std::get<T>(item->data));})){item->prev->next=item->next;item->next->prev=item->prev;detach(item);}
            item=next;
        }
        return *this;
    }
    LinkedList& remove(){return *this;}
    template<class... U> requires(sizeof...(U)>0&&(std::constructible_from<T,U&&>&&...))
    LinkedList& remove(U&&... values){const std::array<T,sizeof...(U)> snapshot{T(std::forward<U>(values))...};return remove(snapshot);}
    LinkedList& clear(){list_->next->prev=list_->prev->next=nullptr;list_->next=list_->prev=list_.get();return *this;}
    LinkedList& insert(T value,Compare compare={}){
        if(!compare)compare=defaultCompare();auto item=arena_->make(std::move(value));auto cur=list_->next;
        while(!isEnd(cur)&&compare(std::get<T>(cur->data),std::get<T>(item->data)))cur=cur->next;
        spliceBefore(cur,item.get());return *this;
    }
    LinkedList& sort(Compare compare={}){
        if(!compare)compare=defaultCompare();std::vector<std::shared_ptr<LinkedList>> lists;
        for(const auto& item:*this){auto single=std::make_shared<LinkedList>(end_);single->push(item);lists.push_back(std::move(single));}
        list_->next=list_->prev=list_.get();
        while(lists.size()>1){auto left=lists.front();lists.erase(lists.begin());auto right=lists.front();lists.erase(lists.begin());left->merge(*right,compare);lists.push_back(std::move(left));}
        if(!lists.empty()){list_=lists.front()->list_;arena_=lists.front()->arena_;}
        return *this;
    }
    LinkedList& merge(LinkedList& other,Compare compare={}){
        if(!compare)compare=defaultCompare();Arena::unite(arena_,other.arena_);
        auto left=list_->next,right=other.list_->next;
        while(!isEnd(left)&&!other.isEnd(right)){
            if(compare(std::get<T>(right->data),std::get<T>(left->data))){
                const auto leftPrev=left->prev,rightPrev=right->prev;
                leftPrev->next=right;right->prev=leftPrev;rightPrev->next=left;left->prev=rightPrev;
                const auto thisTail=list_->prev,otherTail=other.list_->prev;
                thisTail->next=other.list_.get();other.list_->prev=thisTail;
                otherTail->next=list_.get();list_->prev=otherTail;
                auto nextRight=right->next;right=left;left=nextRight;
            }else left=left->next;
        }
        if(!other.isEnd(right)){
            const auto first=other.list_->next,last=other.list_->prev;
            linkBetween(list_->prev,list_.get(),first,last);
            other.list_->next=other.list_->prev=other.list_.get();
        }
        return *this;
    }
};
}
