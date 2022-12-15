# test_config: test defaults and validation of the config file
#
# Copyright (C) 2021 Red Hat, Inc.
#
# SPDX-License-Identifier: GPL-2.0-or-later

import pytest

import test_utils.utils as test_utils

from pathlib import Path
from lcitool.config import Config, ValidationError
from lcitool.singleton import Singleton


@pytest.fixture(autouse=True)
def destroy_config():
    # The following makes sure the Config singleton is deleted after each test
    # See https://docs.pytest.org/en/6.2.x/fixture.html#teardown-cleanup-aka-fixture-finalization
    yield
    del Singleton._instances[Config]


@pytest.mark.parametrize(
    "filename",
    [
        "full.yml",
        "minimal.yml",
        "unknown_section.yml",
        "unknown_key.yml",
    ],
)
def test_config(monkeypatch, filename):
    actual_path = Path(test_utils.test_data_indir(__file__), filename)
    expected_path = Path(test_utils.test_data_outdir(__file__), filename)

    config = Config()

    # we have to monkeypatch the '_config_file_paths' attribute, since we don't
    # support custom inventory paths
    monkeypatch.setattr(config, "_config_file_paths", [actual_path])
    actual = config.values
    test_utils.assert_yaml_matches_file(actual, expected_path)


@pytest.mark.parametrize(
    "filename",
    [
        "empty.yml",
        "missing_mandatory_section.yml",
        "missing_mandatory_key.yml",
        "missing_gitlab_section_with_gitlab_flavor.yml",
    ],
)
def test_config_invalid(monkeypatch, filename):
    actual_path = Path(test_utils.test_data_indir(__file__), filename)

    config = Config()
    monkeypatch.setattr(config, "_config_file_paths", [actual_path])

    with pytest.raises(ValidationError):
        config.values
