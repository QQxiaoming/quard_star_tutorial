# test_projects: test the project package definitions
#
# Copyright (C) 2021 Red Hat, Inc.
#
# SPDX-License-Identifier: GPL-2.0-or-later

import pytest

from lcitool.projects import Projects
from lcitool.inventory import Inventory


projects = Projects()
ALL_PROJECTS = sorted(projects.names + list(projects.internal_projects.keys()))


@pytest.mark.parametrize(
    "name",
    ALL_PROJECTS
)
def test_project_packages(name):
    try:
        project = projects.projects[name]
    except KeyError:
        project = projects.internal_projects[name]
    target = Inventory().targets[0]
    facts = Inventory().target_facts[target]
    project.get_packages(facts)


@pytest.mark.parametrize(
    "name",
    ALL_PROJECTS
)
def test_project_package_sorting(name):
    try:
        project = projects.projects[name]
    except KeyError:
        project = projects.internal_projects[name]
    pkgs = project._load_generic_packages()

    otherpkgs = sorted(pkgs)

    assert otherpkgs == pkgs
