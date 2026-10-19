#include "support/folia_test.hpp"
import folia.math.core.mml_tree.mml_document;
import folia.math.core.mml_tree.mml_nodes.mrow;

using namespace boost::ut;
using namespace folia::math::mml;

namespace {
class ThrowingRow final : public MmlMrow {
    int &live_;
    bool const &fail_;

  protected:
    void copyNativeStateTo(MmlNode &destination, MmlClone &clone) const override {
        MmlMrow::copyNativeStateTo(destination, clone);
        if (fail_)
            throw std::runtime_error("controlled clone failure");
    }

  public:
    ThrowingRow(MmlDocument &document, int &live, bool const &fail)
        : MmlMrow(document), live_(live), fail_(fail) {
        ++live_;
    }

    ~ThrowingRow() override {
        --live_;
    }
};

void registerRow(MmlDocument &document, int &live, bool const &fail) {
    document.factory().setNodeClass(
        NodeKind::mrow,
        {[&live, &fail](MmlCreationArguments const &arguments) {
             return std::make_unique<ThrowingRow>(arguments.document, live, fail);
         },
         [](MmlNode const &node) { return dynamic_cast<ThrowingRow const *>(&node) != nullptr; }});
}
} // namespace

suite mml_exact_clone_document = [] {
    "exact clone rollback destroys partial destination nodes and keeps existing tree"_test = [] {
        int sourceLive = 0, targetLive = 0;
        bool fail = true;
        MmlDocument source, target;
        registerRow(source, sourceLive, fail);
        registerRow(target, targetLive, fail);
        auto &row = source.factory().make(NodeKind::mrow);
        auto &child = source.factory().make(NodeKind::mrow);
        source.append(row, child);
        source.setRoot(row);
        auto &existing = target.factory().make(NodeKind::mrow);
        auto &existingChild = target.factory().make(NodeKind::mi);
        target.append(existing, existingChild);
        target.setRoot(existing);
        const auto before = target.size();

        bool caught = false;
        try {
            target.cloneSubtree(row);
        } catch (std::runtime_error const &) {
            caught = true;
        }
        expect(caught);
        expect(target.size() == before);
        expect(targetLive == 1);
        expect(sourceLive == 2);
        expect(target.root() == &existing);
        expect(existing.children().size() == 1u);
        expect(existing.children().front() == &existingChild);
        expect(existingChild.parent() == &existing);
        expect(source.root() == &row);
        expect(child.parent() == &row);

        fail = false;
        auto &copied = target.cloneSubtree(row);
        expect(target.size() == before + 2);
        expect(targetLive == 3);
        expect(copied.parent() == nullptr);
        expect(copied.children().front()->parent() == &copied);
        expect(target.root() == &existing);
    };

    "exact clone rollback in the source document preserves source links and permits retry"_test =
        [] {
            int live = 0;
            bool fail = true;
            MmlDocument document;
            registerRow(document, live, fail);
            auto &row = document.factory().make(NodeKind::mrow);
            auto &child = document.factory().make(NodeKind::mtext);
            row.appendChild(child);
            document.setRoot(row);
            const auto before = document.size();
            bool caught = false;
            try {
                document.cloneSubtree(row);
            } catch (std::runtime_error const &) {
                caught = true;
            }
            expect(caught);
            expect(live == 1);
            expect(document.size() == before);
            expect(document.root() == &row);
            expect(row.children().front() == &child);
            expect(child.parent() == &row);

            fail = false;
            auto &copied = document.cloneSubtree(row);
            expect(document.size() == before + 2);
            expect(live == 2);
            expect(&copied != &row);
            expect(copied.children().front() != &child);
            expect(child.parent() == &row);
            expect(document.root() == &row);
        };
};
