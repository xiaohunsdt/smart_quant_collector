#pragma once

#include <openssl/crypto.h>

#include <string>
#include <utility>

namespace sqc {

/// Minimal secure string wrapper for credentials.
///
/// Wraps a std::string and zeroes the buffer on destruction using
/// OPENSSL_cleanse (which cannot be optimized away by the compiler).
/// Move is safe (transfers ownership, source is cleansed); copy is
/// deliberately disabled.
///
/// Usage:
///   SecureString pw = SecureString::FromPlain("s3cret");
///   some_api(pw.get());   // pass const std::string& to APIs
///   // ... pw is zeroed when Destroy() is called or on destruction.
struct SecureString {
  /// Create from a plaintext string (e.g. from YAML config).
  /// The caller's plaintext is securely wiped before the argument is destroyed,
  /// so the secret only lives inside this SecureString from this point on.
  static SecureString FromPlain(std::string plain) {
    SecureString s;
    // swap() (not std::move) so we control which buffer gets cleansed. A move
    // transfers the heap pointer to s.data_ and leaves `plain` pointing at a
    // fresh empty buffer — cleansing `plain` would then wipe the wrong buffer
    // and leave the plaintext on the heap. swap() exchanges buffers in O(1):
    //   - heap (long password): pointers swap; `plain` ends up with s.data_'s
    //     original empty buffer, the plaintext heap is now owned by s.data_
    //     (cleansed when s is destroyed). Cleansing `plain` is a harmless no-op.
    //   - SSO (short password): bytes are exchanged in place; cleansing `plain`
    //     wipes the now-swapped-in bytes (the original empty content), which is
    //     also harmless — the secret bytes live in s.data_'s inline buffer.
    s.data_.swap(plain);
    OPENSSL_cleanse(plain.data(), plain.capacity());
    return s;
  }

  SecureString() = default;

  ~SecureString() { Destroy(); }

  // Non-copyable — prevents accidental plaintext duplication.
  SecureString(const SecureString&) = delete;
  SecureString& operator=(const SecureString&) = delete;

  // Movable — copies the payload then wipes the source's storage. We cannot
  // rely on std::string move leaving a non-empty source: conforming moves leave
  // it empty, so a post-move cleanse keyed on !empty() would be a no-op and the
  // plaintext bytes would survive in the moved-from allocation. Copying first
  // guarantees we still know the length to wipe. NOT noexcept: the copy may
  // throw std::bad_alloc, which must propagate rather than std::terminate.
  // CAVEAT: because copy is deleted and move is non-noexcept, SecureString
  // CANNOT be stored in a std::vector/container (reallocation needs either a
  // noexcept move or a usable copy). It is only safe as a direct member — which
  // is how it's used today (config structs, DolphinDBClient/Backend members).
  SecureString(SecureString&& other) : data_(other.data_) {
    other.Destroy();
  }
  SecureString& operator=(SecureString&& other) {
    if(this != &other) {
      Destroy();
      data_ = other.data_;
      other.Destroy();
    }
    return *this;
  }

  /// Access the underlying string for API calls.
  const std::string& get() const noexcept { return data_; }

  /// Explicitly zero and free the stored password.
  void Destroy() {
    if(!data_.empty()) {
      OPENSSL_cleanse(data_.data(), data_.size());
      data_.clear();
    }
  }

  bool empty() const noexcept { return data_.empty(); }

 private:
  std::string data_;
};

}  // namespace sqc
