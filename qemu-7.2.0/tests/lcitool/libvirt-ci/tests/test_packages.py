# test_packages: test the package mapping resolving code
#
# Copyright (C) 2021 Red Hat, Inc.
#
# SPDX-License-Identifier: GPL-2.0-or-later

import pytest

import test_utils.utils as test_utils

from collections import namedtuple
from functools import total_ordering

from pathlib import Path
from lcitool import util
from lcitool.inventory import Inventory
from lcitool.projects import Project, Projects, ProjectError
from lcitool.package import NativePackage, CrossPackage, PyPIPackage, CPANPackage


ALL_TARGETS = sorted(Inventory().targets)


def get_non_cross_targets():
    ret = []
    for target in ALL_TARGETS:
        if target.startswith("debian-") or target.startswith("fedora-"):
            continue

        ret.append(target)
    return ret


def packages_as_dict(raw_pkgs):
    ret = {}
    for cls in [NativePackage, CrossPackage, PyPIPackage, CPANPackage]:
        pkg_type = cls.__name__.replace("Package", "").lower()

        pkg_names = set([p.name for p in raw_pkgs.values() if isinstance(p, cls)])
        if pkg_names:
            ret[pkg_type] = sorted(pkg_names)
    return ret


@pytest.fixture
def test_project():
    return Project("packages",
                   Path(test_utils.test_data_indir(__file__), "packages.yml"))


def test_verify_all_mappings_and_packages():
    expected_path = Path(test_utils.test_data_indir(__file__), "packages.yml")
    actual = {"packages": sorted(Projects().mappings["mappings"].keys())}

    test_utils.assert_yaml_matches_file(actual, expected_path)


native_params = [
    pytest.param(target, None, id=target) for target in ALL_TARGETS
]

cross_params = [
    pytest.param("debian-10", "s390x", id="debian-10-cross-s390x"),
    pytest.param("fedora-rawhide", "mingw64", id="fedora-rawhide-cross-mingw64")
]


@pytest.mark.parametrize("target,arch", native_params + cross_params)
def test_package_resolution(test_project, target, arch):
    if arch is None:
        outfile = f"{target}.yml"
    else:
        outfile = f"{target}-cross-{arch}.yml"
    expected_path = Path(test_utils.test_data_outdir(__file__), outfile)
    pkgs = test_project.get_packages(Inventory().target_facts[target],
                                     cross_arch=arch)
    actual = packages_as_dict(pkgs)

    test_utils.assert_yaml_matches_file(actual, expected_path)


@pytest.mark.parametrize(
    "target",
    [pytest.param(target, id=target) for target in get_non_cross_targets()],
)
def test_unsupported_cross_platform(test_project, target):
    with pytest.raises(ProjectError):
        test_project.get_packages(Inventory().target_facts[target],
                                  cross_arch="s390x")


@pytest.mark.parametrize(
    "target,arch",
    [
        pytest.param("debian-sid", "mingw64", id="debian-sid-cross-mingw64"),
        pytest.param("fedora-rawhide", "s390x", id="fedora-rawhide-cross-s390x"),
    ],
)
def test_cross_platform_arch_mismatch(test_project, target, arch):
    with pytest.raises(ProjectError):
        test_project.get_packages(Inventory().target_facts[target],
                                  cross_arch=arch)


@total_ordering
class MappingKey(namedtuple('MappingKey', ['components', 'priority'])):
    def __str__(self):
        return "".join(self.components)

    def __hash__(self):
        return hash(self.components)

    def __eq__(self, other):
        return isinstance(other, MappingKey) and \
            self.priority == other.priority and \
            self.components == other.components

    def __lt__(self, other):
        if self.priority < other.priority:
            return True
        if self.priority > other.priority:
            return False

        return self.components < other.components


def mapping_keys_product():
    basekeys = set()

    basekeys.add(MappingKey(("default", ), 0))
    for target, facts in Inventory().target_facts.items():
        fmt = facts["packaging"]["format"]
        name = facts["os"]["name"]
        ver = facts["os"]["version"]

        basekeys.add(MappingKey((fmt, ), 1))
        basekeys.add(MappingKey((name, ), 2))
        basekeys.add(MappingKey((name, ver), 3))

    basekeys = [str(x) for x in sorted(basekeys)]
    crosspolicykeys = ["cross-policy-" + k for k in basekeys]
    archkeys = []
    crossarchkeys = []
    for arch in sorted(util.valid_arches()):
        archkeys.extend([arch + "-" + k for k in basekeys])
        crossarchkeys.extend(["cross-" + arch + "-" + k for k in basekeys])

    return basekeys + archkeys + crossarchkeys + crosspolicykeys


def test_project_mappings_sorting():
    mappings = Projects().mappings["mappings"]

    all_expect_keys = mapping_keys_product()
    for package, entries in mappings.items():
        got_keys = list(entries.keys())
        expect_keys = list(filter(lambda k: k in got_keys, all_expect_keys))

        msg = f"Package {package} key order was {got_keys} but should be {expect_keys}"
        assert expect_keys == got_keys, msg
