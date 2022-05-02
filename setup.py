from setuptools import setup, Extension
from sys import platform, maxsize
from os import getenv, system

glfw = "3.3.7"

libraries = []
extra_compile_args = []
include_dirs = []
library_dirs = []

if platform == "win32":
    base = getenv("BASE") if getenv("BASE") else maxsize > 2 ** 32
    folder = "/glfw-" + glfw + ".bin.WIN" + base

    command = "curl github.com/glfw/glfw/releases/download/" + glfw + folder + ".zip -L -o src/glfw.zip"
    print(command)
    system(command)

    command = "unzip src/glfw.zip -d src/glfw"
    print(command)
    system(command)

    include_dirs = ["include", "src/glfw" + folder + "/include"]
    library_dirs = ["src/glfw" + folder + "/lib-vc2022"]

    libraries = [
        "glfw3", "opengl32", "kernel32", "user32", "gdi32", "winspool",
        "shell32", "ole32", "oleaut32", "uuid", "comdlg32", "advapi32"
    ]

elif platform == "linux":
    extra_compile_args = ["-Wextra", "-Wno-comment", "-Wfloat-conversion"]
    include_dirs = ["include"]

    libraries = [
        "glfw3", "GL", "m", "X11", "pthread",
        "Xi", "Xrandr", "dl", "rt", "png"
    ]

elif platform == "darwin":
    folder = "src/glfw/glfw-" + getenv("GLFW") + ".bin.MACOS"

    include_dirs = ["include", folder + "/include"]
    library_dirs = [folder + "/lib-x86_64"]
    libraries = ["glfw3"]

setup(
    name = "JoBase",
    version = "1.3",
    author = "Reuben Ford",
    author_email = "hello@jobase.org",
    description = "Fast Python Game Library",
    long_description = open("README.md", "r").read(),
    long_description_content_type = "text/markdown",
    url = "https://jobase.org",
    python_requires = ">=3.6",
    license = "GPL-3.0-or-later",
    packages = ["JoBase"],
    package_data = {"JoBase": ["images/*.png", "examples/*.py"]},
    include_package_data = True,

    keywords = [
        "fast", "beginner", "extension",
        "library", "opengl", "glfw",
        "games", "c", "children"
    ],

    project_urls = {
        "Source": "https://github.com/Grey41/JoBase",
        "Tracker": "https://github.com/Grey41/JoBase/issues",
        "Documentation": "https://jobase.org/tutorials",
    },

    classifiers = [
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: GNU General Public License v3 or later (GPLv3+)",
        "Operating System :: OS Independent",
    ],
    
    ext_modules = [
        Extension(
            "JoBase.__init__", ["src/module.c", "src/glad.c"],
            extra_compile_args = extra_compile_args,
            include_dirs = include_dirs,
            library_dirs = library_dirs,
            libraries = libraries)
    ])