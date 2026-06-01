from conan import ConanFile

class SmartQuantCollector(ConanFile):
    name = "smart_quant_collector"
    version = "1.0.0"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"
    default_options = {"*:shared": False}

    def requirements(self):
        self.requires("boost/1.87.0")
        self.requires("openssl/3.4.0")
        self.requires("simdjson/4.6.3")
        self.requires("yaml-cpp/0.8.0")
        self.requires("zeromq/4.3.5")
        # Aeron (DolphinDB UDP streaming) is Linux-only in this recipe.
        if self.settings.os == "Linux":
            self.requires("aeron/1.41.4")
        self.requires("libuuid/1.0.3")
        self.requires("cppzmq/4.11.0")
        self.requires("fmt/11.1.4")
        self.requires("gtest/1.17.0")
        self.requires("quill/11.1.0")
        self.requires("prometheus-cpp/1.3.0")
        self.requires("benchmark/1.9.4")

    def layout(self):
        self.folders.generators = "conan"
