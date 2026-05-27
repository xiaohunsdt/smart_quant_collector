#include "logger_init.h"

#include <memory>
#include <mutex>
#include <vector>

#include "quill/Backend.h"
#include "quill/Frontend.h"
#include "quill/LogMacros.h"
#include "quill/sinks/ConsoleSink.h"
#include "quill/sinks/FileSink.h"

namespace sqc {

static quill::Logger* g_logger = nullptr;
static std::once_flag g_logger_init_flag;

void InitLogger(const std::string& log_file_path, const std::string& log_level_str) {
  quill::Backend::start();

  quill::LogLevel level = quill::LogLevel::Info;
  if (log_level_str == "debug") level = quill::LogLevel::Debug;
  else if (log_level_str == "trace_l3") level = quill::LogLevel::TraceL3;
  else if (log_level_str == "trace_l2") level = quill::LogLevel::TraceL2;
  else if (log_level_str == "trace_l1") level = quill::LogLevel::TraceL1;
  else if (log_level_str == "warning") level = quill::LogLevel::Warning;
  else if (log_level_str == "error") level = quill::LogLevel::Error;

  auto stdout_sink = quill::Frontend::create_or_get_sink<quill::ConsoleSink>("stdout");
  auto file_sink = quill::Frontend::create_or_get_sink<quill::FileSink>(
      log_file_path, quill::FileSinkConfig{});

  std::vector<std::shared_ptr<quill::Sink>> sinks;
  sinks.push_back(std::move(stdout_sink));
  sinks.push_back(std::move(file_sink));

  g_logger = quill::Frontend::create_or_get_logger("root", std::move(sinks));
  g_logger->set_log_level(level);
}

quill::Logger* GetLogger() {
  if (!g_logger) {
    // Lazy-init a console-only fallback so LOG_* calls never crash on a null
    // pointer, even when no explicit InitLogger() has been called (e.g. in tests).
    std::call_once(g_logger_init_flag, []() {
      quill::Backend::start();
      g_logger = quill::Frontend::create_or_get_logger(
          "fallback",
          quill::Frontend::create_or_get_sink<quill::ConsoleSink>("fallback_sink"));
      g_logger->set_log_level(quill::LogLevel::Error);
    });
  }
  return g_logger;
}

}  // namespace sqc
