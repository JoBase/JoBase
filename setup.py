import setuptools

file = open("README.md", "r")

setuptools.setup(
    name = "JoBase",
    version = "1.2",
    author = "Reuben Grey Ford",
    author_email = "info@jobase.org",
    description = "Python Game Library",
    long_description = file.read(),
    long_description_content_type = "text/markdown",
    url = "https://jobase.org",
    package_dir = {"": "src"},
    packages = setuptools.find_packages(where = "src"),
    install_requires = ['pyglet', 'shapely'],
    python_requires = '>= 3.6',
    classifiers = [
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: GNU General Public License v3 or later (GPLv3+)",
        "Operating System :: OS Independent"
    ]
)