# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Integrates the xpcshell test runner with mach.

from __future__ import unicode_literals

import os
import sys

from StringIO import StringIO

from mozbuild.base import (
    MachCommandBase,
    MozbuildObject,
)

from mach.decorators import (
    CommandArgument,
    CommandProvider,
    Command,
)


if sys.version_info[0] < 3:
    unicode_type = unicode
else:
    unicode_type = str


# This should probably be consolidated with similar classes in other test
# runners.
class InvalidTestPathError(Exception):
    """Exception raised when the test path is not valid."""


class XPCShellRunner(MozbuildObject):
    """Run xpcshell tests."""
    def run_suite(self, **kwargs):
        manifest = os.path.join(self.topobjdir, '_tests', 'xpcshell',
            'xpcshell.ini')

        return self._run_xpcshell_harness(manifest=manifest, **kwargs)

    def run_test(self, test_file, debug=False, interactive=False,
        keep_going=False, shuffle=False):
        """Runs an individual xpcshell test."""

        if test_file == 'all':
            self.run_suite(debug=debug, interactive=interactive,
                keep_going=keep_going, shuffle=shuffle)
            return

        # dirname() gets confused if there isn't a trailing slash.
        if os.path.isdir(test_file) and not test_file.endswith(os.path.sep):
            test_file += os.path.sep

        relative_dir = test_file

        if test_file.startswith(self.topsrcdir):
            relative_dir = test_file[len(self.topsrcdir):]

        test_dir = os.path.join(self.topobjdir, '_tests', 'xpcshell',
                os.path.dirname(relative_dir))

        xpcshell_ini_file = os.path.join(test_dir, 'xpcshell.ini')
        if not os.path.exists(xpcshell_ini_file):
            raise InvalidTestPathError('An xpcshell.ini could not be found '
                'for the passed test path. Please select a path whose '
                'directory contains an xpcshell.ini file. It is possible you '
                'received this error because the tree is not built or tests '
                'are not enabled.')

        args = {
            'debug': debug,
            'interactive': interactive,
            'keep_going': keep_going,
            'shuffle': shuffle,
            'test_dirs': [test_dir],
        }

        if os.path.isfile(test_file):
            args['test_path'] = os.path.basename(test_file)

        return self._run_xpcshell_harness(**args)

    def _run_xpcshell_harness(self, test_dirs=None, manifest=None,
        test_path=None, debug=False, shuffle=False, interactive=False,
        keep_going=False):

        # Obtain a reference to the xpcshell test runner.
        import runxpcshelltests

        dummy_log = StringIO()
        xpcshell = runxpcshelltests.XPCShellTests(log=dummy_log)
        self.log_manager.enable_unstructured()

        tests_dir = os.path.join(self.topobjdir, '_tests', 'xpcshell')
        modules_dir = os.path.join(self.topobjdir, '_tests', 'modules')

        args = {
            'xpcshell': os.path.join(self.bindir, 'xpcshell'),
            'mozInfo': os.path.join(self.topobjdir, 'mozinfo.json'),
            'symbolsPath': os.path.join(self.distdir, 'crashreporter-symbols'),
            'interactive': interactive,
            'keepGoing': keep_going,
            'logfiles': False,
            'shuffle': shuffle,
            'testsRootDir': tests_dir,
            'testingModulesDir': modules_dir,
            'profileName': 'firefox',
            'verbose': test_path is not None,
            'xunitFilename': os.path.join(self.statedir, 'xpchsell.xunit.xml'),
            'xunitName': 'xpcshell',
        }

        if manifest is not None:
            args['manifest'] = manifest
        elif test_dirs is not None:
            if isinstance(test_dirs, list):
                args['testdirs'] = test_dirs
            else:
                args['testdirs'] = [test_dirs]
        else:
            raise Exception('One of test_dirs or manifest must be provided.')

        if test_path is not None:
            args['testPath'] = test_path

        # Python through 2.7.2 has issues with unicode in some of the
        # arguments. Work around that.
        filtered_args = {}
        for k, v in args.items():
            if isinstance(v, unicode_type):
                v = v.encode('utf-8')

            if isinstance(k, unicode_type):
                k = k.encode('utf-8')

            filtered_args[k] = v

        result = xpcshell.runTests(**filtered_args)

        self.log_manager.disable_unstructured()

        return int(not result)


@CommandProvider
class MachCommands(MachCommandBase):
    @Command('xpcshell-test', help='Run an xpcshell test.')
    @CommandArgument('test_file', default='all', nargs='?', metavar='TEST',
        help='Test to run. Can be specified as a single JS file, a directory, '
             'or omitted. If omitted, the entire test suite is executed.')
    @CommandArgument('--debug', '-d', action='store_true',
        help='Run test in a debugger.')
    @CommandArgument('--interactive', '-i', action='store_true',
        help='Open an xpcshell prompt before running tests.')
    @CommandArgument('--keep-going', '-k', action='store_true',
        help='Continue running tests after a SIGINT is received.')
    @CommandArgument('--shuffle', '-s', action='store_true',
        help='Randomize the execution order of tests.')
    def run_xpcshell_test(self, **params):
        # We should probably have a utility function to ensure the tree is
        # ready to run tests. Until then, we just create the state dir (in
        # case the tree wasn't built with mach).
        self._ensure_state_subdir_exists('.')

        xpcshell = self._spawn(XPCShellRunner)

        try:
            return xpcshell.run_test(**params)
        except InvalidTestPathError as e:
            print(e.message)
            return 1

