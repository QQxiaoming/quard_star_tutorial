# inventory - module containing Ansible inventory handling primitives
#
# Copyright (C) 2017-2020 Red Hat, Inc.
#
# SPDX-License-Identifier: GPL-2.0-or-later

import copy
import logging
import yaml

from pathlib import Path
from pkg_resources import resource_filename

from lcitool import util, LcitoolError
from lcitool.singleton import Singleton

log = logging.getLogger(__name__)


class InventoryError(LcitoolError):
    """Global exception type for the inventory module."""

    def __init__(self, message):
        super().__init__(message, "Inventory")


class Inventory(metaclass=Singleton):

    @property
    def ansible_inventory(self):
        if self._ansible_inventory is None:
            self._ansible_inventory = self._get_ansible_inventory()
        return self._ansible_inventory

    @property
    def target_facts(self):
        if self._target_facts is None:
            self._target_facts = self._load_target_facts()
        return self._target_facts

    @property
    def targets(self):
        return list(self.target_facts.keys())

    @property
    def host_facts(self):
        if self._host_facts is None:
            self._host_facts = self._load_host_facts()
        return self._host_facts

    @property
    def hosts(self):
        return list(self.host_facts.keys())

    def __init__(self):
        self._target_facts = None
        self._host_facts = None
        self._ansible_inventory = None

    @staticmethod
    def _read_facts_from_file(yaml_path):
        log.debug(f"Loading facts from '{yaml_path}'")
        with open(yaml_path, "r") as infile:
            return yaml.safe_load(infile)

    def _get_ansible_inventory(self):
        from lcitool.ansible_wrapper import AnsibleWrapper, AnsibleWrapperError

        inventory_sources = []
        inventory_path = Path(util.get_config_dir(), "inventory")
        if inventory_path.exists():
            inventory_sources.append(inventory_path)

        log.debug("Querying libvirt for lcitool hosts")
        inventory_sources.append(self._get_libvirt_inventory())

        ansible_runner = AnsibleWrapper()
        ansible_runner.prepare_env(inventories=inventory_sources,
                                   group_vars=self.target_facts)

        log.debug(f"Running ansible-inventory on '{inventory_sources}'")
        try:
            inventory = ansible_runner.get_inventory()
        except AnsibleWrapperError as ex:
            log.debug("Failed to load Ansible inventory")
            raise InventoryError(f"Failed to load Ansible inventory: {ex}")

        return inventory

    def _get_libvirt_inventory(self):
        from lcitool.libvirt_wrapper import LibvirtWrapper

        inventory = {"all": {"children": {}}}
        children = inventory["all"]["children"]

        for host, target in LibvirtWrapper().hosts.items():
            inventory_target = children.setdefault(target, {})
            inventory_hosts = inventory_target.setdefault("hosts", {})
            inventory_hosts.setdefault(host, {})

        return inventory

    @staticmethod
    def _validate_target_facts(target_facts, target):
        fname = target + ".yml"

        actual_osname = target_facts["os"]["name"].lower()
        if not target.startswith(actual_osname + "-"):
            raise InventoryError(f'OS name "{target_facts["os"]["name"]}" does not match file name {fname}')
        target = target[len(actual_osname) + 1:]

        actual_version = target_facts["os"]["version"].lower()
        expected_version = target.replace("-", "")
        if expected_version != actual_version:
            raise InventoryError(f'OS version "{target_facts["os"]["version"]}" does not match version in file name {fname} ({expected_version})')

    def _load_target_facts(self):
        def merge_dict(source, dest):
            for key in source.keys():
                if key not in dest:
                    dest[key] = copy.deepcopy(source[key])
                    continue

                if isinstance(source[key], list) or isinstance(dest[key], list):
                    raise InventoryError("cannot merge lists")
                if isinstance(source[key], dict) != isinstance(dest[key], dict):
                    raise InventoryError("cannot merge dictionaries with non-dictionaries")
                if isinstance(source[key], dict):
                    merge_dict(source[key], dest[key])

        facts = {}
        targets_path = Path(resource_filename(__name__, "facts/targets/"))
        targets_all_path = Path(targets_path, "all.yml")

        # first load the shared facts from targets/all.yml
        shared_facts = self._read_facts_from_file(targets_all_path)

        # then load the rest of the facts
        for entry in targets_path.iterdir():
            if not entry.is_file() or entry.suffix != ".yml" or entry.name == "all.yml":
                continue

            target = entry.stem
            facts[target] = self._read_facts_from_file(entry)
            self._validate_target_facts(facts[target], target)
            facts[target]["target"] = target

            # missing per-distro facts fall back to shared facts
            merge_dict(shared_facts, facts[target])

        return facts

    def _load_host_facts(self):
        facts = {}
        groups = {}

        def _rec(inventory, group_name):
            for key, subinventory in inventory.items():
                if key == "hosts":
                    for host_name, host_facts in subinventory.items():
                        log.debug(f"Host '{host_name}' is in group '{group_name}'")

                        # Keep track of all the groups we've seen each host
                        # show up in so that we can perform some validation
                        # later
                        if host_name not in groups:
                            groups[host_name] = set()
                        groups[host_name].add(group_name)

                        # ansible-inventory only includes the full list of facts
                        # the first time a host shows up, no matter how deeply
                        # nested that happens to be, and all other times just uses
                        # an empty dictionary as a position marker
                        if host_name not in facts:
                            log.debug(f"Facts for host '{host_name}': {host_facts}")
                            facts[host_name] = host_facts

                # Recurse into the group's children to look for more hosts
                elif key == "children":
                    _rec(subinventory, group_name)
                else:
                    log.debug(f"Group '{key}' is a children of group '{group_name}'")
                    _rec(subinventory, key)

        _rec(self.ansible_inventory["all"], "all")

        targets = set(self.targets)
        for host_name, host_groups in groups.items():
            host_targets = host_groups.intersection(targets)

            # Each host should have shown up in exactly one of the groups
            # that are defined based on the target OS
            if len(host_targets) == 0:
                raise InventoryError(
                    f"Host '{host_name}' not found in any target OS group"
                )
            elif len(host_targets) > 1:
                raise InventoryError(
                    f"Host '{host_name}' found in multiple target OS groups: {host_targets}"
                )

        return facts

    def expand_hosts(self, pattern):
        try:
            return util.expand_pattern(pattern, self.hosts, "hosts")
        except InventoryError as ex:
            raise ex
        except Exception as ex:
            log.debug(f"Failed to load expand '{pattern}'")
            raise InventoryError(f"Failed to expand '{pattern}': {ex}")
