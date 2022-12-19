# application.py - module containing the lcitool application code
#
# Copyright (C) 2017-2020 Red Hat, Inc.
#
# SPDX-License-Identifier: GPL-2.0-or-later

import logging
import sys

from pathlib import Path
from pkg_resources import resource_filename

from lcitool import util, LcitoolError
from lcitool.config import Config
from lcitool.inventory import Inventory
from lcitool.package import package_names_by_type
from lcitool.projects import Projects
from lcitool.formatters import DockerfileFormatter, ShellVariablesFormatter, JSONVariablesFormatter, ShellBuildEnvFormatter
from lcitool.singleton import Singleton
from lcitool.manifest import Manifest

log = logging.getLogger(__name__)


def required_deps(*deps):
    def inner_decorator(func):
        def wrapped(*args, **kwargs):
            cmd = func.__name__[len('_action_'):]
            for dep in deps:
                try:
                    import importlib
                    importlib.import_module(dep)
                except ImportError:
                    raise ApplicationError(
                        f"Command '{cmd}' requires '{dep}' module to be installed"
                    )
            func(*args, **kwargs)
        return wrapped
    return inner_decorator


class ApplicationError(LcitoolError):
    def __init__(self, message):
        super().__init__(message, "Application")


class Application(metaclass=Singleton):
    def __init__(self):
        # make sure the lcitool cache dir exists
        cache_dir_path = util.get_cache_dir()
        cache_dir_path.mkdir(parents=True, exist_ok=True)

    @staticmethod
    def _entrypoint_debug(args):
        cli_args = {}
        for arg, val in vars(args).items():
            if arg not in ['func', 'debug']:
                cli_args[arg] = val
        log.debug(f"Cmdline args={cli_args}")

    def _execute_playbook(self, playbook, hosts_pattern, projects_pattern,
                          git_revision, verbosity=0):
        from lcitool.ansible_wrapper import AnsibleWrapper, AnsibleWrapperError

        log.debug(f"Executing playbook '{playbook}': "
                  f"hosts_pattern={hosts_pattern} "
                  f"projects_pattern={projects_pattern} gitrev={git_revision}")

        base = resource_filename(__name__, "ansible")
        config = Config()
        inventory = Inventory()
        projects = Projects()

        hosts_expanded = inventory.expand_hosts(hosts_pattern)
        projects_expanded = Projects().expand_names(projects_pattern)

        if git_revision is not None:
            tokens = git_revision.split("/")
            if len(tokens) < 2:
                print(f"Missing or invalid git revision '{git_revision}'",
                      file=sys.stderr)
                sys.exit(1)

            git_remote = tokens[0]
            git_branch = "/".join(tokens[1:])
        else:
            git_remote = "default"
            git_branch = "master"

        playbook_base = Path(base, "playbooks", playbook)
        group_vars = dict()

        extra_vars = config.values
        extra_vars.update({
            "base": base,
            "selected_projects": projects_expanded,
            "git_remote": git_remote,
            "git_branch": git_branch,
        })

        log.debug("Preparing Ansible runner environment")
        ansible_runner = AnsibleWrapper()

        for host in hosts_expanded:
            facts = inventory.host_facts[host]
            target = facts["target"]

            # packages are evaluated on a target level and since the
            # host->target mapping is N-1, we can skip hosts belonging to a
            # target group for which we already evaluated the package list
            if target in group_vars:
                continue

            # resolve the package mappings to actual package names
            internal_wanted_projects = ["base", "developer", "vm"]
            if config.values["install"]["cloud_init"]:
                internal_wanted_projects.append("cloud-init")

            selected_projects = internal_wanted_projects + projects_expanded
            pkgs_install = projects.get_packages(selected_projects, facts)
            pkgs_early_install = projects.get_packages(["early_install"], facts)
            pkgs_remove = projects.get_packages(["unwanted"], facts)
            package_names = package_names_by_type(pkgs_install)
            package_names_remove = package_names_by_type(pkgs_remove)
            package_names_early_install = package_names_by_type(pkgs_early_install)

            # merge the package lists to the Ansible group vars
            packages = {}
            packages["packages"] = package_names["native"]
            packages["pypi_packages"] = package_names["pypi"]
            packages["cpan_packages"] = package_names["cpan"]
            packages["unwanted_packages"] = package_names_remove["native"]
            packages["early_install_packages"] = package_names_early_install["native"]

            group_vars[target] = packages
            group_vars[target].update(inventory.target_facts[target])

        ansible_runner.prepare_env(playbookdir=playbook_base,
                                   inventories=[inventory.ansible_inventory],
                                   group_vars=group_vars,
                                   extravars=extra_vars)
        log.debug(f"Running Ansible with playbook '{playbook_base.name}'")
        try:
            ansible_runner.run_playbook(limit=hosts_expanded, verbosity=verbosity)
        except AnsibleWrapperError as ex:
            raise ApplicationError(ex.message)

    @required_deps('ansible_runner', 'libvirt')
    def _action_hosts(self, args):
        self._entrypoint_debug(args)

        inventory = Inventory()
        for host in sorted(inventory.hosts):
            print(host)

    def _action_targets(self, args):
        self._entrypoint_debug(args)

        inventory = Inventory()
        for target in sorted(inventory.targets):
            if args.containerized:
                facts = inventory.target_facts[target]

                if facts["packaging"]["format"] not in ["apk", "deb", "rpm"]:
                    continue

            print(target)

    def _action_projects(self, args):
        self._entrypoint_debug(args)

        projects = Projects()
        for project in sorted(projects.names):
            print(project)

    @required_deps('libvirt')
    def _action_install(self, args):
        from lcitool.install import VirtInstall

        self._entrypoint_debug(args)

        facts = {}
        inventory = Inventory()
        host = args.host
        target = args.target

        try:
            facts = inventory.host_facts[host]
        except KeyError:
            if target is None:
                raise ApplicationError(
                    f"Host {host} not found in the inventory, either add {host} "
                    "to your inventory or use '--target <target>'"
                )

            if target not in inventory.targets:
                raise ApplicationError(f"Unsupported target OS '{target}'")

            facts = inventory.target_facts[target]
        else:
            if target is not None:
                raise ApplicationError(
                    f"Can't use --target with '{host}': "
                    "host already exists in the inventory"
                )
            elif not facts.get("fully_managed"):
                raise ApplicationError(
                    f"fully_managed=True not set for {host}, refusing to proceed"
                )

        virt_install = VirtInstall.from_url(name=host,
                                            facts=facts)
        virt_install(wait=args.wait)

    @required_deps('ansible_runner', 'libvirt')
    def _action_update(self, args):
        self._entrypoint_debug(args)

        self._execute_playbook("update", args.hosts, args.projects,
                               args.git_revision, args.verbose)

    def _action_build(self, args):
        self._entrypoint_debug(args)

        # we don't keep a dependencies tree for projects, hence pattern
        # expansion would break the 'build' playbook
        if args.projects == "all" or "*" in args.projects:
            raise ApplicationError(
                "'build' command doesn't support specifying projects by "
                "either wildcards or the 'all' keyword"
            )

        self._execute_playbook("build", args.hosts, args.projects,
                               args.git_revision, args.verbose)

    def _action_variables(self, args):
        self._entrypoint_debug(args)

        projects_expanded = Projects().expand_names(args.projects)

        if args.format == "shell":
            formatter = ShellVariablesFormatter()
        else:
            formatter = JSONVariablesFormatter()

        variables = formatter.format(args.target,
                                     projects_expanded,
                                     args.cross_arch)

        # No comments in json !
        if args.format != "json":
            cliargv = [args.action]
            if args.cross_arch:
                cliargv.extend(["--cross", args.cross_arch])
            cliargv.extend([args.target, args.projects])
            header = util.generate_file_header(cliargv)
        else:
            header = ""

        print(header + variables)

    def _action_dockerfile(self, args):
        self._entrypoint_debug(args)

        projects_expanded = Projects().expand_names(args.projects)

        dockerfile = DockerfileFormatter(args.base,
                                         args.layers).format(args.target,
                                                             projects_expanded,
                                                             args.cross_arch)

        cliargv = [args.action]
        if args.base is not None:
            cliargv.extend(["--base", args.base])
        cliargv.extend(["--layers", args.layers])
        if args.cross_arch:
            cliargv.extend(["--cross", args.cross_arch])
        cliargv.extend([args.target, args.projects])
        header = util.generate_file_header(cliargv)

        print(header + dockerfile)

    def _action_buildenvscript(self, args):
        self._entrypoint_debug(args)

        projects_expanded = Projects().expand_names(args.projects)

        buildenvscript = ShellBuildEnvFormatter().format(args.target,
                                                         projects_expanded,
                                                         args.cross_arch)

        cliargv = [args.action]
        if args.cross_arch:
            cliargv.extend(["--cross", args.cross_arch])
        cliargv.extend([args.target, args.projects])
        header = util.generate_file_header(cliargv)

        print(header + buildenvscript)

    def _action_manifest(self, args):
        base_path = None
        if args.base_dir is not None:
            base_path = Path(args.base_dir)
        ci_path = Path(args.ci_dir)
        manifest = Manifest(args.manifest, args.quiet, ci_path, base_path)
        manifest.generate(args.dry_run)

    def run(self, args):
        try:
            util.set_extra_data_dir(args.data_dir)
            args.func(self, args)
        except LcitoolError as ex:
            print(f"{ex.module_prefix} error:", ex, file=sys.stderr)
            sys.exit(1)
