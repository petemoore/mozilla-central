#!/usr/bin/env python

import os
import shutil
import subprocess
import tempfile
import unittest
from mozprofile.prefs import Preferences
from mozprofile.profile import Profile

class PreferencesTest(unittest.TestCase):
    """test mozprofile"""

    def run_command(self, *args):
        """
        runs mozprofile;
        returns (stdout, stderr, code)
        """
        process = subprocess.Popen(args,
                                   stdout=subprocess.PIPE,
                                   stderr=subprocess.PIPE)
        stdout, stderr = process.communicate()
        stdout = stdout.strip()
        stderr = stderr.strip()
        return stdout, stderr, process.returncode

    def compare_generated(self, _prefs, commandline):
        """
        writes out to a new profile with mozprofile command line
        reads the generated preferences with prefs.py
        compares the results
        cleans up
        """
        profile, stderr, code = self.run_command(*commandline)
        prefs_file = os.path.join(profile, 'user.js')
        self.assertTrue(os.path.exists(prefs_file))
        read = Preferences.read_prefs(prefs_file)
        if isinstance(_prefs, dict):
            read = dict(read)
        self.assertEqual(_prefs, read)
        shutil.rmtree(profile)

    def test_basic_prefs(self):
        _prefs = {"browser.startup.homepage": "http://planet.mozilla.org/"}
        commandline = ["mozprofile"]
        _prefs = _prefs.items()
        for pref, value in _prefs:
            commandline += ["--pref", "%s:%s" % (pref, value)]
        self.compare_generated(_prefs, commandline)

    def test_ordered_prefs(self):
        """ensure the prefs stay in the right order"""
        _prefs = [("browser.startup.homepage", "http://planet.mozilla.org/"),
                  ("zoom.minPercent", 30),
                  ("zoom.maxPercent", 300),
                  ("webgl.verbose", 'false')]
        commandline = ["mozprofile"]
        for pref, value in _prefs:
            commandline += ["--pref", "%s:%s" % (pref, value)]
        _prefs = [(i, Preferences.cast(j)) for i, j in _prefs]
        self.compare_generated(_prefs, commandline)

    def test_ini(self):

        # write the .ini file
        _ini = """[DEFAULT]
browser.startup.homepage = http://planet.mozilla.org/

[foo]
browser.startup.homepage = http://github.com/
"""
        fd, name = tempfile.mkstemp(suffix='.ini')
        os.write(fd, _ini)
        os.close(fd)
        commandline = ["mozprofile", "--preferences", name]

        # test the [DEFAULT] section
        _prefs = {'browser.startup.homepage': 'http://planet.mozilla.org/'}
        self.compare_generated(_prefs, commandline)

        # test a specific section
        _prefs = {'browser.startup.homepage': 'http://github.com/'}
        commandline[-1] = commandline[-1] + ':foo'
        self.compare_generated(_prefs, commandline)

        # cleanup
        os.remove(name)

    def test_reset_should_remove_added_prefs(self):
        """Check that when we call reset the items we expect are updated"""

        profile = Profile()
        prefs_file = os.path.join(profile.profile, 'user.js')

        # we shouldn't have any initial preferences
        initial_prefs = Preferences.read_prefs(prefs_file)
        self.assertFalse(initial_prefs)
        initial_prefs = file(prefs_file).read().strip()
        self.assertFalse(initial_prefs)

        # add some preferences
        prefs1 = [("mr.t.quotes", "i aint getting on no plane!")]
        profile.set_preferences(prefs1)
        self.assertEqual(prefs1, Preferences.read_prefs(prefs_file))
        lines = file(prefs_file).read().strip().splitlines()
        self.assertTrue(bool([line for line in lines
                              if line.startswith('#MozRunner Prefs Start')]))
        self.assertTrue(bool([line for line in lines
                              if line.startswith('#MozRunner Prefs End')]))

        profile.reset()
        self.assertNotEqual(prefs1, \
                    Preferences.read_prefs(os.path.join(profile.profile, 'user.js')),\
                            "I pity the fool who left my pref")

    def test_magic_markers(self):
        """ensure our magic markers are working"""

        profile = Profile()
        prefs_file = os.path.join(profile.profile, 'user.js')

        # we shouldn't have any initial preferences
        initial_prefs = Preferences.read_prefs(prefs_file)
        self.assertFalse(initial_prefs)
        initial_prefs = file(prefs_file).read().strip()
        self.assertFalse(initial_prefs)

        # add some preferences
        prefs1 = [("browser.startup.homepage", "http://planet.mozilla.org/"),
                   ("zoom.minPercent", 30)]
        profile.set_preferences(prefs1)
        self.assertEqual(prefs1, Preferences.read_prefs(prefs_file))
        lines = file(prefs_file).read().strip().splitlines()
        self.assertTrue(bool([line for line in lines
                              if line.startswith('#MozRunner Prefs Start')]))
        self.assertTrue(bool([line for line in lines
                              if line.startswith('#MozRunner Prefs End')]))

        # add some more preferences
        prefs2 = [("zoom.maxPercent", 300),
                   ("webgl.verbose", 'false')]
        profile.set_preferences(prefs2)
        self.assertEqual(prefs1 + prefs2, Preferences.read_prefs(prefs_file))
        lines = file(prefs_file).read().strip().splitlines()
        self.assertTrue(len([line for line in lines
                             if line.startswith('#MozRunner Prefs Start')]) == 2)
        self.assertTrue(len([line for line in lines
                             if line.startswith('#MozRunner Prefs End')]) == 2)

        # now clean it up
        profile.clean_preferences()
        final_prefs = Preferences.read_prefs(prefs_file)
        self.assertFalse(final_prefs)
        lines = file(prefs_file).read().strip().splitlines()
        self.assertTrue('#MozRunner Prefs Start' not in lines)
        self.assertTrue('#MozRunner Prefs End' not in lines)

    def test_preexisting_preferences(self):
        """ensure you don't clobber preexisting preferences"""

        # make a pretend profile
        tempdir = tempfile.mkdtemp()

        try:
            # make a user.js
            contents = """
user_pref("webgl.enabled_for_all_sites", true);
user_pref("webgl.force-enabled", true);
"""
            user_js = os.path.join(tempdir, 'user.js')
            f = file(user_js, 'w')
            f.write(contents)
            f.close()

            # make sure you can read it
            prefs = Preferences.read_prefs(user_js)
            original_prefs = [('webgl.enabled_for_all_sites', True), ('webgl.force-enabled', True)]
            self.assertTrue(prefs == original_prefs)

            # now read this as a profile
            profile = Profile(tempdir, preferences={"browser.download.dir": "/home/jhammel"})

            # make sure the new pref is now there
            new_prefs = original_prefs[:] + [("browser.download.dir", "/home/jhammel")]
            prefs = Preferences.read_prefs(user_js)
            self.assertTrue(prefs == new_prefs)

            # clean up the added preferences
            profile.cleanup()
            del profile

            # make sure you have the original preferences
            prefs = Preferences.read_prefs(user_js)
            self.assertTrue(prefs == original_prefs)
        except:
            shutil.rmtree(tempdir)
            raise

    def test_json(self):
        _prefs = {"browser.startup.homepage": "http://planet.mozilla.org/"}
        json = '{"browser.startup.homepage": "http://planet.mozilla.org/"}'

        # just repr it...could use the json module but we don't need it here
        fd, name = tempfile.mkstemp(suffix='.json')
        os.write(fd, json)
        os.close(fd)

        commandline = ["mozprofile", "--preferences", name]
        self.compare_generated(_prefs, commandline)


if __name__ == '__main__':
    unittest.main()
