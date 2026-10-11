/*************************************************************
 *
 *  Copyright (c) 2017-2026 The MathJax Consortium
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

// Native port of MathJax 4.1.3 ts/util/PrioritizedList.ts.
export module folia.math.util.prioritized_list;
import std;

namespace folia::math::detail {
    inline double prioritized_list_default_priority = 5;
}

export namespace folia::math {

template<class T>
struct PrioritizedListItem {
    double priority;
    T item;
};

template<class T>
class PrioritizedList {
public:
    inline static double& DEFAULTPRIORITY = detail::prioritized_list_default_priority;
    using Storage = std::vector<std::shared_ptr<PrioritizedListItem<T>>>;
    using StorageRef = std::shared_ptr<Storage>;

    class iterator {
    public:
        using iterator_category = std::input_iterator_tag;
        using value_type = PrioritizedListItem<T>;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;

        iterator() = default;

        reference operator*() const {
            cache_current();
            return *current_;
        }

        pointer operator->() const {
            cache_current();
            return current_.get();
        }

        iterator& operator++() {
            ++cursor_;
            current_.reset();
            return *this;
        }

        iterator operator++(int) {
            auto previous = *this;
            ++*this;
            return previous;
        }

        bool operator==(const iterator& other) const {
            if (owner_ != other.owner_) return false;
            if (!owner_) return true;
            if (end_sentinel_ && other.end_sentinel_) return true;
            if (end_sentinel_) return other.cursor_ >= other.storage_->size();
            if (other.end_sentinel_) return cursor_ >= storage_->size();
            return storage_ == other.storage_ && cursor_ == other.cursor_;
        }

        bool operator!=(const iterator& other) const {
            return !(*this == other);
        }

    private:
        friend class PrioritizedList;

        iterator(PrioritizedList* owner, std::size_t cursor, bool end_sentinel)
            : owner_(owner), storage_(owner ? owner->items_ : nullptr),
              cursor_(cursor), end_sentinel_(end_sentinel) {}

        void cache_current() const {
            if (!current_) current_ = (*storage_)[cursor_];
        }

        PrioritizedList* owner_ = nullptr;
        StorageRef storage_;
        std::size_t cursor_ = 0;
        bool end_sentinel_ = false;
        mutable std::shared_ptr<value_type> current_;
    };

    PrioritizedList() = default;

    [[nodiscard]] iterator begin() {
        return iterator(this, 0, false);
    }

    [[nodiscard]] iterator end() {
        // This is a sentinel, not a captured size: iteration observes mutations
        // to items.length just as the source iterator does on each next().
        return iterator(this, 0, true);
    }

    T add(T item, double priority = DEFAULTPRIORITY) {
        std::size_t index = items_->size();
        while (index > 0 && priority < (*items_)[index - 1]->priority) {
            --index;
        }

        // Keep entries separately owned so ordered-array mutations do not
        // invalidate a record already yielded to an iterator.
        auto result = item;
        items_->insert(items_->begin() + static_cast<std::ptrdiff_t>(index),
            std::make_shared<PrioritizedListItem<T>>(PrioritizedListItem<T>{priority, std::move(item)}));
        return result;
    }

    PrioritizedList& remove(const T& item) {
        for (std::size_t index = items_->size(); index > 0; --index) {
            if ((*items_)[index - 1]->item == item) {
                items_->erase(items_->begin() + static_cast<std::ptrdiff_t>(index - 1));
                break;
            }
        }
        return *this;
    }

protected:
    StorageRef& itemStorage() { return items_; }
    const StorageRef& itemStorage() const { return items_; }
private:
    StorageRef items_ = std::make_shared<Storage>();
};

}
