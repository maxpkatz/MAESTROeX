#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# MAESTROeX documentation build configuration file, created by
# sphinx-quickstart on Mon Dec 25 18:42:54 2017.
#
# This file is execfile()d with the current directory set to its
# containing dir.
#
# Note that not all possible configuration values are present in this
# autogenerated file.
#
# All configuration values have a default; values that are commented out
# serve to show the default.

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
import os
import re
import sys
import sphinx_rtd_theme
import breathe
import shlex
import subprocess

sys.path.insert(0, os.path.abspath('../../'))
sys.path.append(os.path.dirname(breathe.__file__))


def get_version():
    prog = shlex.split("git describe --tags --abbrev=0")
    p0 = subprocess.Popen(prog, stdout=subprocess.PIPE,
                          stderr=subprocess.PIPE)
    stdout0, stderr0 = p0.communicate()
    rc = p0.returncode
    stdout = stdout0.decode('utf-8')
    return stdout


# -- General configuration ------------------------------------------------

# If your documentation needs a minimal Sphinx version, state it here.
#
# needs_sphinx = '1.0'

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = ['sphinx.ext.autodoc',
              'sphinx.ext.mathjax',
              'sphinx.ext.ifconfig',
              'sphinx.ext.viewcode',
              'sphinxcontrib.bibtex',
              'sphinx.ext.autosummary',
              'sphinx.ext.githubpages',
              'breathe',
              'sphinxfortran.fortran_domain',
              'sphinxfortran.fortran_autodoc',
              'sphinx.ext.intersphinx']

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# The suffix(es) of source filenames.
# You can specify multiple suffix as a list of string:
#
# source_suffix = ['.rst', '.md']
source_suffix = '.rst'

# see https://github.com/phn/pytpm/issues/3#issuecomment-12133978
numpydoc_show_class_members = False

# The master toctree document.
master_doc = 'index'

# General information about the project.
project = 'MAESTROeX'
copyright = '2018-2019, MAESTROeX development tem'
author = 'MAESTROeX development team'

html_logo = "maestroex_logo.png"

# The version info for the project you're documenting, acts as replacement for
# |version| and |release|, also used in various other places throughout the
# built documents.
#
# The short X.Y version.
version = get_version()
# The full version, including alpha/beta/rc tags.
release = version

# The language for content autogenerated by Sphinx. Refer to documentation
# for a list of supported languages.
#
# This is also used if you do content translation via gettext catalogs.
# Usually you set "language" from the command line for these cases.
language = None

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This patterns also effect to html_static_path and html_extra_path
exclude_patterns = []

# The name of the Pygments (syntax highlighting) style to use.
pygments_style = 'sphinx'

# If true, `todo` and `todoList` produce output, else they produce nothing.
todo_include_todos = False


# -- Options for MathJax
mathjax_config = {'TeX': {'Macros': {}}}

with open('mathsymbols.tex', 'r') as f:
    for line in f:
        macros = re.findall(r'\\newcommand{\\(.*?)}(\[(\d)\])?{(.+)}', line)
        for macro in macros:
            if len(macro[1]) == 0:
                mathjax_config['TeX']['Macros'][macro[0]
                                                ] = "{" + macro[3] + "}"
            else:
                mathjax_config['TeX']['Macros'][macro[0]] = [
                    "{" + macro[3] + "}", int(macro[2])]


math_eqref_format = "Eq.{number}"
math_number_all = True
num_fig = True
math_num_fig = True
numfig = True

# -- Options for HTML output ----------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = 'sphinx_rtd_theme'
html_theme_path = [sphinx_rtd_theme.get_html_theme_path()]

# Theme options are theme-specific and customize the look and feel of a theme
# further.  For a list of options available for each theme, see the
# documentation.
#
# html_theme_options = {}

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ['_static']

html_context = {
    'css_files': [
        '_static/theme_overrides.css',  # override wide tables in RTD theme
    ],
}

# Custom sidebar templates, must be a dictionary that maps document names
# to template names.
#
# This is required for the alabaster theme
# refs: http://alabaster.readthedocs.io/en/latest/installation.html#sidebars
html_sidebars = {
    '**': [
        'relations.html',  # needs 'show_related': True theme option to display
        'searchbox.html',
    ]
}


# -- Options for HTMLHelp output ------------------------------------------

# Output file base name for HTML help builder.
htmlhelp_basename = 'MAESTROeXdoc'


# -- Options for LaTeX output ---------------------------------------------

latex_elements = {
    # The paper size ('letterpaper' or 'a4paper').
    #
    # 'papersize': 'letterpaper',

    # The font size ('10pt', '11pt' or '12pt').
    #
    # 'pointsize': '10pt',

    # Additional stuff for the LaTeX preamble.
    #
    # 'preamble': '',

    # Latex figure (float) alignment
    #
    # 'figure_align': 'htbp',
}

# Grouping the document tree into LaTeX files. List of tuples
# (source start file, target name, title,
#  author, documentclass [howto, manual, or own class]).
latex_documents = [
    (master_doc, 'MAESTROeX.tex', 'MAESTROeX Documentation',
     'MAESTROeX development team', 'manual'),
]


# -- Options for manual page output ---------------------------------------

# One entry per manual page. List of tuples
# (source start file, name, description, authors, manual section).
man_pages = [
    (master_doc, 'MAESTROeX', 'MAESTROeX Documentation',
     [author], 1)
]


# -- Options for Texinfo output -------------------------------------------

# Grouping the document tree into Texinfo files. List of tuples
# (source start file, target name, title, author,
#  dir menu entry, description, category)
texinfo_documents = [
    (master_doc, 'MAESTROeX', 'MAESTROeX Documentation',
     author, 'MAESTROeX', 'One line description of project.',
     'Miscellaneous'),
]


# -- Options for breathe -------------------------------------------------

breathe_projects = {
    "maestroex": "../doxy_files/xml",
}

breathe_default_project = "maestroex"

breathe_default_members = ('members', 'undoc-members', 'protected-members',
                           'private-members')

breathe_doxygen_config_options = {'EXTRACT_ALL': 'YES',
                                  'SHOW_USED_FILES': 'YES', 'RECURSIVE': 'YES'
                                  }

# -- Options for sphinx-fortran -----------------------------------------

fortran_src = [os.path.abspath('preprocessed_files')]

fortran_ext = ['f90', 'F90']


# -- Options for intersphinx --------------------------------------------

intersphinx_mapping = {
    'amrex':  ('https://amrex-codes.github.io/amrex/docs_html/', None)
}
