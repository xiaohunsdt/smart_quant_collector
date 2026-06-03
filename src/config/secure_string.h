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
  static SecureString FromPlain(std::string plain) {
    SecureString s;
    s.data_ = std::move(plain);
    return s;
  }

  SecureString() = default;

  ~SecureString() { Destroy(); }

  // Non-copyable — prevents accidental plaintext duplication.
  SecureString(const SecureString&) = delete;
  SecureString& operator=(const SecureString&) = delete;

  // Movable — cleanses the source object after transfer.
  SecureString(SecureString&& other) noexcept : data_(std::move(other.data_)) {
    if(!other.data_.empty()) {
      OPENSSL_cleanse(const_cast<char*>(other.data_.data()), other.data_.size());
      other.data_.clear();
    }
  }
  SecureString& operator=(SecureString&& other) noexcept {
    if(this != &other) {
      Destroy();
      data_ = std::move(other.data_);
      if(!other.data_.empty()) {
        OPENSSL_cleanse(const_cast<char*>(other.data_.data()), other.data_.size());
        other.data_.clear();
      }
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
