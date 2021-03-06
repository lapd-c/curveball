#!/usr/bin/env python
#
# This material is based upon work supported by the Defense Advanced
# Research Projects Agency under Contract No. N66001-11-C-4017.
#
# Copyright 2014 - Raytheon BBN Technologies Corp.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
# implied.  See the License for the specific language governing
# permissions and limitations under the License.

"""
dhexp watcher, based on DirWatcher, for the dp.
"""

import datetime
import logging
import os.path

import cb.noc.file
import cb.util.cblogging

from cb.noc.check_dhexp import CheckDHexp
from cb.util.dir_watcher import DirWatcher
from cb.util.dir_watcher import DirWatcherHelper

class DHexpManagerHelper(DirWatcherHelper):
    """
    The DirWatcher helper class used by the DHexpManager.

    Knows implicitly which dhexp files are currently interesting
    (based on the current time--whether or not the files actually exist)
    and how to add/delete files from the DHexpManager instance
    """

    def __init__(self, checker):
        self.logger = logging.getLogger('cb.noc')
        self.checker = checker

    def load(self, path):
        """
        Add the dhexp entries in the file at the given path to the checker
        """
        self.logger.info('loading (%s)' % (path,))
        self.checker.add_file(path)
        print 'LOADED DHEXP FILE [%s]' % path

    def unload(self, path):
        """
        Delete the sentinels in the file at the given path from the checker
        """

        self.logger.info('unloading (%s)' % (path,))
        self.checker.delete_file(path)

    def choose_fnames(self, fnames):
        """
        For the sentinel files, we always try to load the
        files for the current and previous hours, and ignore the
        rest.

        This method ignores the given fnames and simply returns
        the fnames that we want, based on the current time.
        """

        utc = datetime.datetime.utcnow()
        utc_last_hour = utc - datetime.timedelta(hours=1)

        fnames = [
                cb.noc.file.dhexp_fname(utc),
                cb.noc.file.dhexp_fname(utc_last_hour)
                ]

        #print 'FNAMES: '
        #print fnames

        self.logger.info('choosing fnames %s' % (str(fnames),))

        return fnames


class DHexpManager(CheckDHexp):
    """
    Subclass of CheckDHexp that adds a DirWatcher to initialize
    the sentinel->DH map and keep the map up to date over time
    """

    def __init__(self, dirname):
        """
        dirname: the path to the directory where the sentinel files are
        located.

        Note that the polling interval is hardwired to 3 seconds.
        This should be reasonable for now, but it's also easy to change.
        """

        CheckDHexp.__init__(self)

        self.dirname = dirname

        self.dirwatch_helper = DHexpManagerHelper(self)
        self.dirwatcher = DirWatcher(self.dirname, self.dirwatch_helper,
                poll_interval=3)


if __name__ == '__main__':
    import os
    import time

    def main():
        """
        Test of basic functionality

        Doesn't test whether unload actually works.
        """

        print "this must be run by hand.  Read the comments."

        dirname = './dd'

        try:
            os.makedirs(dirname)
        except OSError:
            print "dir already exists?"

        utc = datetime.datetime.utcnow()
        hour = datetime.timedelta(hours=1)

        now = utc
        then = now - hour

        fname_now = cb.noc.file.dhexp_fname(now)
        fname_then = cb.noc.file.dhexp_fname(then)

        # fnames: the hourly sentinel filenames, starting 5 hours ago,
        # until 4 houcrs from now.
        #
        fnames = [cb.noc.file.dhexp_fname(utc + (hour * t))
                  for t in range(-5, 5)]

        # Create the "mapping".  For this test, each sentinel file
        # contains exactly one mapping, and it is a completely bogus
        # mapping: fnameXX -> fnameYY, where fname is the name of the
        # file.
        #
        for fname in fnames:
            open(os.path.join(dirname, fname), 'w+').write(
                    '%sXX %sYY\n' % (fname, fname))

        manager = DHexpManager('dd')

        # We need to sleep, at least for a moment, in order to give
        # the DirWatcher a chance to run.
        #
        time.sleep(2)

        print "Expect these to be true, all others false:"
        print "last hour: " + fname_then
        print "curr hour: " + fname_now

        print "Testing:"
        for fname in fnames:
            print '%s: %s' % (fname, str(fname + 'XX' in manager))


    exit(main())

