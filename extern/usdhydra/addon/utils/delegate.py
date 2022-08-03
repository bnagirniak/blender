# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

# <pep8 compliant>

import zipfile
from concurrent import futures
from time import sleep

from ..engine import session_get_render_plugins

from . import update_ui


class Manager:
    def __init__(self):
        self.delegate_executor = None
        self.progress = None
        self.filepath = ""
        self.delegates = None

    @property
    def is_available(self):
        if manager.progress is None:
            return True

        if manager.progress >= 0:
            return False

    def set_progress(self, msg):
        self.progress = msg
        update_ui(area_type='PREFERENCES')

    def update_delegates(self):
        self.delegates = session_get_render_plugins()

    def register_delegates(self):
        from ..engine import init, exit
        exit()
        init()

    def build(self):
        from ..properties.preferences import get_addon_pref
        delegates_dir = get_addon_pref().delegates_dir

        with zipfile.ZipFile(self.filepath) as z:
            self.set_progress(True)
            z.extractall(path=delegates_dir)

        self.register_delegates()
        self.update_delegates()

        self.set_progress(False)

    def install_delegate(self):
        def delegate_build():
            try:
                self.build()

            except Exception as err:
                #log.error(err)
                self.set_progress(None)

        if not self.delegate_executor:
            self.delegate_executor = futures.ThreadPoolExecutor()

        self.delegate_executor.submit(delegate_build)


manager = Manager()
