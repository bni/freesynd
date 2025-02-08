from conan import ConanFile
from conan.tools.cmake import cmake_layout


class ExampleRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps"

    def requirements(self):
        self.requires("libpng/1.6.44")
        self.requires("cli11/2.4.2")
        self.requires("crcpp/1.2.0.0")
        self.requires("catch2/3.8.0")

    def layout(self):
        cmake_layout(self)
