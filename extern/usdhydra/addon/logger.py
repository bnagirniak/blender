# SPDX-License-Identifier: Apache-2.0
# Copyright 2011-2022 Blender Foundation

# <pep8 compliant>

import sys
import logging.handlers


FORMAT_STR = "%(asctime)s %(levelname)s %(name)s [%(thread)d]:  %(message)s"

# root logger for the addon
logger = logging.getLogger('usdhydra')
logger.setLevel('DEBUG')

console_handler = logger.StreamHandler(stream=sys.stdout)
console_handler.setFormatter(logger.Formatter(FORMAT_STR))
logger.addHandler(console_handler)


def msg(args):
    return ", ".join(str(arg) for arg in args)


class Log:
    def __init__(self, tag):
        self.logger = logger.getChild(tag)

    def __call__(self, *args):
        self.debug(*args)

    def debug(self, *args):
        self.logger.debug(msg(args))

    def info(self, *args):
        self.logger.info(msg(args))

    def warn(self, *args):
        self.logger.warning(msg(args))

    def error(self, *args):
        self.logger.error(msg(args))

    def critical(self, *args):
        self.logger.critical(msg(args))

    def dump_args(self, func):
        """This decorator dumps out the arguments passed to a function before calling it"""
        arg_names = func.__code__.co_varnames[:func.__code__.co_argcount]

        def echo_func(*args, **kwargs):
            self.debug(f"<{func.__name__}>: "
                       f"{tuple(f'{name}={arg}' for name, arg in zip(arg_names, args))}"
                       f"{f' {kwargs.items()}' if kwargs else ''}")
            return func(*args, **kwargs)

        return echo_func
