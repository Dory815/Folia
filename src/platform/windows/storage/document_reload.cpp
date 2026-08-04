module;
#include <windows.h>

module folia.platform.document_reload;
import std;

namespace folia::platform::reload {

namespace {

constexpr std::uint64_t fnv_offset = 14695981039346656037ull;
constexpr std::uint64_t fnv_prime = 1099511628211ull;

std::uint64_t mix64(std::uint64_t value) noexcept {
    value ^= value >> 30;
    value *= 0xbf58476d1ce4e5b9ull;
    value ^= value >> 27;
    value *= 0x94d049bb133111ebull;
    return value ^ (value >> 31);
}

bool same_filename(std::wstring_view left, std::wstring_view right) noexcept {
    if (left.size() != right.size()) return false;
    return CompareStringOrdinal(
        left.data(), static_cast<int>(left.size()),
        right.data(), static_cast<int>(right.size()), TRUE) == CSTR_EQUAL;
}

} // namespace

DocumentContentFingerprint fingerprint_document_content(std::string_view source) noexcept {
    auto primary = fnv_offset;
    auto secondary = mix64(static_cast<std::uint64_t>(source.size()) + 0x9e3779b97f4a7c15ull);
    for (auto byte : source) {
        auto value = static_cast<std::uint8_t>(byte);
        primary = (primary ^ value) * fnv_prime;
        secondary = mix64(secondary ^ (static_cast<std::uint64_t>(value) + 0x100ull));
    }
    return {
        .size = static_cast<std::uint64_t>(source.size()),
        .primary = primary,
        .secondary = secondary,
    };
}

WatchedFileVersion query_watched_file_version(std::filesystem::path const& path) noexcept {
    auto handle = CreateFileW(
        path.c_str(), FILE_READ_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (handle == INVALID_HANDLE_VALUE) return {};

    FILE_ID_INFO identity{};
    FILE_BASIC_INFO basic{};
    FILE_STANDARD_INFO standard{};
    auto ok = GetFileInformationByHandleEx(handle, FileIdInfo, &identity, sizeof(identity))
        && GetFileInformationByHandleEx(handle, FileBasicInfo, &basic, sizeof(basic))
        && GetFileInformationByHandleEx(handle, FileStandardInfo, &standard, sizeof(standard));
    CloseHandle(handle);
    if (!ok) return {};

    WatchedFileVersion version;
    version.exists = true;
    version.volume_serial = identity.VolumeSerialNumber;
    std::memcpy(version.file_id.data(), identity.FileId.Identifier, version.file_id.size());
    version.size = static_cast<std::uint64_t>(standard.EndOfFile.QuadPart);
    version.last_write_ticks = static_cast<std::uint64_t>(basic.LastWriteTime.QuadPart);
    return version;
}

void DocumentReloadState::Reset(
    WatchedFileVersion version,
    DocumentContentFingerprint source) noexcept {
    knownVersion_ = version;
    pendingVersion_.reset();
    savedSource_ = source;
    hasBaseline_ = true;
}

void DocumentReloadState::MarkSaved(
    WatchedFileVersion version,
    DocumentContentFingerprint source) noexcept {
    Reset(version, source);
}

bool DocumentReloadState::IsKnown(WatchedFileVersion const& version) const noexcept {
    return (knownVersion_ && *knownVersion_ == version)
        || (pendingVersion_ && *pendingVersion_ == version);
}

bool DocumentReloadState::LocallyModified(DocumentContentFingerprint source) const noexcept {
    return !hasBaseline_ || source != savedSource_;
}

DocumentReloadAction DocumentReloadState::Stage(
    WatchedFileVersion version,
    DocumentContentFingerprint currentSource) noexcept {
    pendingVersion_ = version;
    if (!version.exists || LocallyModified(currentSource)) return DocumentReloadAction::Prompt;
    return DocumentReloadAction::Reload;
}

DocumentReloadAction DocumentReloadState::Observe(
    WatchedFileVersion version,
    DocumentContentFingerprint currentSource) noexcept {
    if (IsKnown(version)) return DocumentReloadAction::None;
    return Stage(version, currentSource);
}

DocumentReloadAction DocumentReloadState::StageManual(
    WatchedFileVersion version,
    DocumentContentFingerprint currentSource) noexcept {
    return Stage(version, currentSource);
}

void DocumentReloadState::Accept(
    WatchedFileVersion version,
    DocumentContentFingerprint loadedSource) noexcept {
    Reset(version, loadedSource);
}

void DocumentReloadState::KeepPending() noexcept {
    if (pendingVersion_) knownVersion_ = pendingVersion_;
    pendingVersion_.reset();
}

void DocumentReloadState::DiscardPending() noexcept {
    pendingVersion_.reset();
}

std::optional<WatchedFileVersion> DocumentReloadState::PendingVersion() const noexcept {
    return pendingVersion_;
}

struct FileChangeMonitor::Impl {
    HANDLE directory = INVALID_HANDLE_VALUE;
    HANDLE stopEvent = nullptr;
    HANDLE changedEvent = nullptr;
    OVERLAPPED overlapped{};
    std::wstring filename;
    std::function<void()> callback;
    std::jthread worker;
    std::atomic_bool running = false;

    void Close() noexcept {
        if (changedEvent) CloseHandle(changedEvent);
        if (stopEvent) CloseHandle(stopEvent);
        if (directory != INVALID_HANDLE_VALUE) CloseHandle(directory);
        changedEvent = nullptr;
        stopEvent = nullptr;
        directory = INVALID_HANDLE_VALUE;
        overlapped = {};
    }

    void Run() noexcept {
        std::array<std::byte, 32 * 1024> buffer{};
        while (running.load(std::memory_order_acquire)) {
            ResetEvent(changedEvent);
            DWORD ignored = 0;
            auto issued = ReadDirectoryChangesW(
                directory, buffer.data(), static_cast<DWORD>(buffer.size()), FALSE,
                FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_SIZE
                    | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION,
                &ignored, &overlapped, nullptr);
            if (!issued && GetLastError() != ERROR_IO_PENDING) break;

            HANDLE waits[] = {stopEvent, changedEvent};
            auto wait = WaitForMultipleObjects(2, waits, FALSE, INFINITE);
            if (wait == WAIT_OBJECT_0) {
                CancelIoEx(directory, &overlapped);
                break;
            }
            if (wait != WAIT_OBJECT_0 + 1) break;

            DWORD transferred = 0;
            if (!GetOverlappedResult(directory, &overlapped, &transferred, FALSE)) {
                if (GetLastError() == ERROR_OPERATION_ABORTED) break;
                continue;
            }

            bool matched = false;
            for (DWORD offset = 0; offset < transferred;) {
                auto* record = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(
                    reinterpret_cast<std::byte*>(buffer.data()) + offset);
                std::wstring_view changed(record->FileName, record->FileNameLength / sizeof(wchar_t));
                matched = matched || same_filename(changed, filename);
                if (record->NextEntryOffset == 0) break;
                offset += record->NextEntryOffset;
            }
            if (matched && running.load(std::memory_order_acquire) && callback) {
                try { callback(); } catch (...) {}
            }
        }
        running.store(false, std::memory_order_release);
    }
};

FileChangeMonitor::FileChangeMonitor() : impl_(std::make_unique<Impl>()) {}
FileChangeMonitor::~FileChangeMonitor() { Stop(); }

bool FileChangeMonitor::Start(
    std::filesystem::path const& target,
    std::function<void()> callback,
    std::error_code& error) {
    Stop();
    error.clear();
    auto absolute = std::filesystem::absolute(target, error);
    if (error) return false;
    auto parent = absolute.parent_path();
    if (parent.empty() || absolute.filename().empty()) {
        error = std::make_error_code(std::errc::invalid_argument);
        return false;
    }

    impl_->directory = CreateFileW(
        parent.c_str(), FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr, OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, nullptr);
    if (impl_->directory == INVALID_HANDLE_VALUE) {
        error = std::error_code(static_cast<int>(GetLastError()), std::system_category());
        return false;
    }
    impl_->stopEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    impl_->changedEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if (!impl_->stopEvent || !impl_->changedEvent) {
        error = std::error_code(static_cast<int>(GetLastError()), std::system_category());
        impl_->Close();
        return false;
    }
    impl_->filename = absolute.filename().wstring();
    impl_->callback = std::move(callback);
    impl_->overlapped = {};
    impl_->overlapped.hEvent = impl_->changedEvent;
    impl_->running.store(true, std::memory_order_release);
    impl_->worker = std::jthread([impl = impl_.get()] { impl->Run(); });
    return true;
}

void FileChangeMonitor::Stop() noexcept {
    if (!impl_) return;
    impl_->running.store(false, std::memory_order_release);
    if (impl_->stopEvent) SetEvent(impl_->stopEvent);
    if (impl_->directory != INVALID_HANDLE_VALUE) CancelIoEx(impl_->directory, &impl_->overlapped);
    if (impl_->worker.joinable()) impl_->worker.join();
    impl_->callback = {};
    impl_->filename.clear();
    impl_->Close();
}

bool FileChangeMonitor::Running() const noexcept {
    return impl_ && impl_->running.load(std::memory_order_acquire);
}

} // namespace folia::platform::reload
