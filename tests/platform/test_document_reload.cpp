#include "support/folia_test.hpp"

import folia.platform.document_reload;

using namespace boost::ut;
using namespace folia::platform::reload;

namespace {

WatchedFileVersion version(std::uint64_t stamp, bool exists = true) {
    WatchedFileVersion result;
    result.exists = exists;
    result.size = stamp;
    result.last_write_ticks = stamp * 17;
    return result;
}

} // namespace

suite document_reload_tests = [] {

"content fingerprints are deterministic and byte exact"_test = [] {
    auto first = fingerprint_document_content("alpha");
    expect(first == fingerprint_document_content("alpha"));
    expect(first != fingerprint_document_content("Alpha"));
    expect(first != fingerprint_document_content(std::string_view{"alpha\0", 6}));
    expect(first.size == 5_u);
};

"clean external changes auto reload and duplicate events are suppressed"_test = [] {
    DocumentReloadState state;
    auto source = fingerprint_document_content("saved");
    state.Reset(version(1), source);
    expect(state.Observe(version(1), source) == DocumentReloadAction::None);
    expect(state.Observe(version(2), source) == DocumentReloadAction::Reload);
    expect(state.PendingVersion() == std::optional{version(2)});
    expect(state.Observe(version(2), source) == DocumentReloadAction::None);
    state.Accept(version(2), fingerprint_document_content("external"));
    expect(!state.PendingVersion().has_value());
    expect(state.Observe(version(2), fingerprint_document_content("external"))
        == DocumentReloadAction::None);
};

"dirty external changes prompt and keeping local suppresses only that version"_test = [] {
    DocumentReloadState state;
    auto saved = fingerprint_document_content("saved");
    auto dirty = fingerprint_document_content("edited");
    state.Reset(version(1), saved);
    expect(state.LocallyModified(dirty));
    expect(state.Observe(version(2), dirty) == DocumentReloadAction::Prompt);
    state.KeepPending();
    expect(state.Observe(version(2), dirty) == DocumentReloadAction::None);
    expect(state.Observe(version(3), dirty) == DocumentReloadAction::Prompt);
};

"saved versions suppress the monitor event produced by the application"_test = [] {
    DocumentReloadState state;
    state.Reset(version(1), fingerprint_document_content("old"));
    auto saved = fingerprint_document_content("new");
    state.MarkSaved(version(2), saved);
    expect(!state.LocallyModified(saved));
    expect(state.Observe(version(2), saved) == DocumentReloadAction::None);
};

"manual reload stages known versions and still protects dirty content"_test = [] {
    DocumentReloadState state;
    auto saved = fingerprint_document_content("saved");
    state.Reset(version(1), saved);
    expect(state.StageManual(version(1), saved) == DocumentReloadAction::Reload);
    state.DiscardPending();
    expect(state.StageManual(version(1), fingerprint_document_content("dirty"))
        == DocumentReloadAction::Prompt);
};

"deleted files always prompt and pending state can be discarded"_test = [] {
    DocumentReloadState state;
    auto saved = fingerprint_document_content("saved");
    state.Reset(version(1), saved);
    auto missing = version(0, false);
    expect(state.Observe(missing, saved) == DocumentReloadAction::Prompt);
    expect(state.PendingVersion() == std::optional{missing});
    state.DiscardPending();
    expect(!state.PendingVersion().has_value());
};

"file versions distinguish missing files and atomic replacements"_test = [] {
    auto root = std::filesystem::temp_directory_path()
        / ("folia-version-"
            + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::create_directories(root);
    auto target = root / "target.md";
    auto replacement = root / "replacement.md";
    expect(!query_watched_file_version(target).exists);
    { std::ofstream(target) << "same"; }
    auto original = query_watched_file_version(target);
    expect(original.exists);
    expect(original.size == 4_u);
    { std::ofstream(replacement) << "same"; }
    std::filesystem::remove(target);
    std::filesystem::rename(replacement, target);
    auto replaced = query_watched_file_version(target);
    expect(replaced.exists);
    expect(replaced != original);
    std::filesystem::remove_all(root);
};

"accepting a conflict resets the baseline and keeping it does not"_test = [] {
    DocumentReloadState state;
    auto original = fingerprint_document_content("original");
    auto local = fingerprint_document_content("local");
    auto external = fingerprint_document_content("external");
    state.Reset(version(1), original);
    expect(state.Observe(version(2), local) == DocumentReloadAction::Prompt);
    state.KeepPending();
    expect(state.LocallyModified(local));
    expect(state.Observe(version(3), local) == DocumentReloadAction::Prompt);
    state.Accept(version(3), external);
    expect(!state.LocallyModified(external));
    expect(state.LocallyModified(local));
};

"file monitor filters sibling changes and stops deterministically"_test = [] {
    auto root = std::filesystem::temp_directory_path()
        / ("folia-reload-"
            + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::create_directories(root);
    auto target = root / "target.md";
    auto sibling = root / "sibling.md";
    { std::ofstream(target) << "one"; }

    std::mutex mutex;
    std::condition_variable changed;
    std::size_t notifications = 0;
    FileChangeMonitor monitor;
    std::error_code error;
    expect(fatal(monitor.Start(target, [&] {
        std::scoped_lock lock(mutex);
        ++notifications;
        changed.notify_all();
    }, error)));
    expect(!error);

    { std::ofstream(sibling) << "ignore"; }
    {
        std::unique_lock lock(mutex);
        expect(!changed.wait_for(lock, std::chrono::milliseconds(150), [&] { return notifications != 0; }));
    }
    { std::ofstream(target, std::ios::trunc) << "two"; }
    {
        std::unique_lock lock(mutex);
        expect(changed.wait_for(lock, std::chrono::seconds(3), [&] { return notifications != 0; }));
    }
    monitor.Stop();
    std::size_t stoppedCount = 0;
    {
        std::scoped_lock lock(mutex);
        stoppedCount = notifications;
    }
    { std::ofstream(target, std::ios::trunc) << "three"; }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    {
        std::scoped_lock lock(mutex);
        expect(notifications == stoppedCount);
    }
    std::filesystem::remove_all(root);
};

};
