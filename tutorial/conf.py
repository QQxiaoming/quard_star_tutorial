# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
# import os
# import sys
# sys.path.insert(0, os.path.abspath('.'))


project = 'quard_star_tutorial'
copyright = '2021, Quard(qiaoqiming)'
author = 'Quard(qiaoqiming)'

extensions = ['myst_parser']
templates_path = ['_templates']
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store', 'index.md']

language = 'zh_CN'
html_theme = 'sphinx_material'
html_title = '主页'

html_sidebars = {
    "**": ["logo-text.html", "globaltoc.html", "localtoc.html", "searchbox.html"]
}

html_theme_options = {
    'nav_title': '基于qemu从0开始构建嵌入式linux系统',

    #'color_primary': 'blue',
    #'color_accent': 'light-blue',

    'repo_url': 'https://github.com/QQxiaoming/quard_star_tutorial',
    'repo_name': 'quard_star_tutorial',
    "logo_icon": "&#xe02f",

    'globaltoc_depth': 1,
    'globaltoc_collapse': True,
    'globaltoc_includehidden': False,

    "nav_links": [
        {"href": "ch0","internal": True,"title": "正文"},
        {"href": "ext1","internal": True,"title": "扩展文章"},
    ],
}
