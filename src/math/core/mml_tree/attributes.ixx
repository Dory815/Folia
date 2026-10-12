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

// Port of MathJax 4.1.3 ts/core/MmlTree/Attributes.ts.
// Source SHA256: c1e363944e0c059c7ed554b4b6aeb8ea56d43c844f84934c267ccc79b090468a.
export module folia.math.core.mml_tree.attributes;
import std;
import folia.math.core.tree.property;

export namespace folia::math {
inline constexpr std::string_view INHERIT = "_inherit_";

/**
 * Attribute values are kept in four live PropertyList layers, matching the
 * upstream prototype chain: explicit -> inherited -> defaults -> globals.
 * PropertyList copies are handles, so the all-layer accessors intentionally
 * expose mutations to subsequent lookups.
 */
class Attributes {
    PropertyList attributes_;
    PropertyList inherited_;
    PropertyList defaults_;
    PropertyList global_;

public:
    virtual ~Attributes() = default;
    Attributes(const PropertyList& defaults, const PropertyList& global)
        : global_(global) {
        defaults_ = PropertyList::withPrototype(global_);
        // Object.assign(this.defaults, defaults) copies own enumerable entries
        // while preserving the global prototype and JS key ordering.
        defaults_.assign(defaults);
        inherited_ = PropertyList::withPrototype(defaults_);
        attributes_ = PropertyList::withPrototype(inherited_);
    }

    void set(std::string_view name, Property value) {
        attributes_.set(name, std::move(value));
    }

    void setList(const PropertyList& list) {
        attributes_.assign(list);
    }

    void unset(std::string_view name) {
        attributes_.erase(name);
    }

    virtual Property get(std::string_view name) const {
        Property value = attributes_.get(name);
        if (value == Property{std::string(INHERIT)}) {
            value = global_.get(name);
        }
        return value;
    }

    Property getExplicit(std::string_view name) const {
        return hasExplicit(name) ? attributes_.getOwn(name) : Property{Missing{}};
    }

    bool hasExplicit(std::string_view name) const {
        return attributes_.hasOwn(name);
    }

    bool hasOneOf(const std::vector<std::string>& names) const {
        for (const auto& name : names) {
            if (hasExplicit(name)) return true;
        }
        return false;
    }

    PropertyList getList(const std::vector<std::string>& names) const {
        PropertyList values;
        for (const auto& name : names) {
            values.set(name, get(name));
        }
        return values;
    }

    void setInherited(std::string_view name, Property value) {
        inherited_.set(name, std::move(value));
    }

    Property getInherited(std::string_view name) const {
        return inherited_.get(name);
    }

    Property getDefault(std::string_view name) const {
        return defaults_.get(name);
    }

    virtual bool isSet(std::string_view name) const {
        return attributes_.hasOwn(name) || inherited_.hasOwn(name);
    }

    bool hasDefault(std::string_view name) const {
        return defaults_.has(name);
    }

    std::vector<std::string> getExplicitNames() const {
        return attributes_.keys();
    }

    std::vector<std::string> getInheritedNames() const {
        return inherited_.keys();
    }

    std::vector<std::string> getDefaultNames() const {
        return defaults_.keys();
    }

    std::vector<std::string> getGlobalNames() const {
        return global_.keys();
    }

    PropertyList getAllAttributes() const {
        return attributes_;
    }

    PropertyList getAllInherited() const {
        return inherited_;
    }

    PropertyList getAllDefaults() const {
        return defaults_;
    }

    PropertyList getAllGlobals() const {
        return global_;
    }
};
}
