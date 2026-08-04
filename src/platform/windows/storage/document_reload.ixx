// folia.platform.document_reload — deterministic reload policy and Windows file notifications.
export module folia.platform.document_reload;
import std;

export namespace folia::platform::reload {

struct DocumentContentFingerprint {
    std::uint64_t size = 0;
    std::uint64_t primary = 0;
    std::uint64_t secondary = 0;

    bool operator==(DocumentContentFingerprint const&) const = default;
};

DocumentContentFingerprint fingerprint_document_content(std::string_view source) noexcept;

struct WatchedFileVersion {
    bool exists = false;
    std::uint64_t volume_serial = 0;
    std::array<std::byte, 16> file_id{};
    std::uint64_t size = 0;
    std::uint64_t last_write_ticks = 0;

    bool operator==(WatchedFileVersion const&) const = default;
};

WatchedFileVersion query_watched_file_version(std::filesystem::path const& path) noexcept;

enum class DocumentReloadAction {
    None,
    Reload,
    Prompt,
};

class DocumentReloadState {
public:
    void Reset(WatchedFileVersion version, DocumentContentFingerprint source) noexcept;
    void MarkSaved(WatchedFileVersion version, DocumentContentFingerprint source) noexcept;
    bool IsKnown(WatchedFileVersion const& version) const noexcept;
    bool LocallyModified(DocumentContentFingerprint source) const noexcept;

    DocumentReloadAction Observe(
        WatchedFileVersion version,
        DocumentContentFingerprint currentSource) noexcept;
    DocumentReloadAction StageManual(
        WatchedFileVersion version,
        DocumentContentFingerprint currentSource) noexcept;

    void Accept(WatchedFileVersion version, DocumentContentFingerprint loadedSource) noexcept;
    void KeepPending() noexcept;
    void DiscardPending() noexcept;
    std::optional<WatchedFileVersion> PendingVersion() const noexcept;

private:
    DocumentReloadAction Stage(
        WatchedFileVersion version,
        DocumentContentFingerprint currentSource) noexcept;

    std::optional<WatchedFileVersion> knownVersion_;
    std::optional<WatchedFileVersion> pendingVersion_;
    DocumentContentFingerprint savedSource_{};
    bool hasBaseline_ = false;
};

class FileChangeMonitor {
public:
    FileChangeMonitor();
    ~FileChangeMonitor();
    FileChangeMonitor(FileChangeMonitor const&) = delete;
    FileChangeMonitor& operator=(FileChangeMonitor const&) = delete;
    FileChangeMonitor(FileChangeMonitor&&) = delete;
    FileChangeMonitor& operator=(FileChangeMonitor&&) = delete;

    bool Start(
        std::filesystem::path const& target,
        std::function<void()> callback,
        std::error_code& error);
    void Stop() noexcept;
    bool Running() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace folia::platform::reload
