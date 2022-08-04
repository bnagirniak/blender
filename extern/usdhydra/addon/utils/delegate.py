# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

# <pep8 compliant>

import zipfile
from concurrent import futures

from ..engine import session_get_render_plugins
from . import update_ui, logging
log = logging.Log('utils.delegate')


class Manager:
    def __init__(self):
        self.delegate_executor = None
        self.in_progress = False
        self.filepath = ""
        self.delegates = None

    def set_progress(self, msg):
        self.in_progress = msg
        update_ui(area_type='PREFERENCES')

    def update_delegates(self):
        self.delegates = session_get_render_plugins()

    def register_delegates(self):
        from .. import engine
        engine.exit()
        engine.init()

    def build(self):
        from ..properties.preferences import get_addon_pref
        delegates_dir = get_addon_pref().delegates_dir

        with zipfile.ZipFile(self.filepath) as z:
            self.in_progress = True
            z.extractall(path=delegates_dir)

        self.register_delegates()
        self.update_delegates()

        self.in_progress = False

    def install_delegate(self):
        def delegate_build():
            try:
                self.build()

            except Exception as err:
                log.error(err)

            finally:
                self.in_progress = False

        if not self.delegate_executor:
            self.delegate_executor = futures.ThreadPoolExecutor()

        self.delegate_executor.submit(delegate_build)


manager = Manager()
