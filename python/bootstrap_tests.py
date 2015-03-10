"""
The MIT License (MIT)

Copyright (c) 2015 Yanzheng Li

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
"""

"""
Bootstrap Python tests.

Run as:

python python/bootstrap_tests.py
"""

import glob
import os
import subprocess


PYTHON = 'python'
PYTHON_TESTS_DIR = './python/tests/'
PYTHON_COMPILER = './python/python_compiler.py'
PYTHON_CODE_TRANSFORMER = './python/code_transformer.py'
INFO_FILE = './info.json'
COREVM = './coreVM'
BYTECODE_EXTENSION = '.core'


class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'




def code_transformer_input_to_output_path(path):
    return os.path.splitext(path)[0] + '.tmp' + '.py'


def compiler_input_to_output_path(path):
    return code_transformer_input_to_output_path(path) + BYTECODE_EXTENSION


def code_transformer_cmdl_args(path):
    return [
        PYTHON,
        PYTHON_CODE_TRANSFORMER,
        '--input',
        path,
        '--output',
        code_transformer_input_to_output_path(path)
    ]


def compiler_cmdl_args(path):
    return [
        PYTHON,
        PYTHON_COMPILER,
        '--input',
        path,
        '--info-file',
        INFO_FILE,
        '--output',
        compiler_input_to_output_path(path)
    ]


def corevm_cmdl_args(path):
    return [COREVM, '--input', compiler_input_to_output_path(path)]


def main():
    inputs = glob.glob(PYTHON_TESTS_DIR + '*.py')
    real_inputs = []

    print 'Bootstrapping Python tests...'
    print 'Testing using the following %d input file(s):' % len(inputs)
    for path in inputs:
        if not path.endswith('.tmp.py'):
            real_inputs.append(path)
            print path

    # Bring blank line.
    print

    for path in real_inputs:
        info = path

        retcode = subprocess.call(code_transformer_cmdl_args(path))

        if retcode != 0:
            info += (bcolors.WARNING + ' [FAILED]' + bcolors.ENDC)
            print info
            continue

        retcode = subprocess.call(compiler_cmdl_args(path))
        if retcode != 0:
            info += (bcolors.WARNING + ' [FAILED]' + bcolors.ENDC)
            print info
            continue

        retcode = subprocess.call(corevm_cmdl_args(path))

        if retcode == 0:
            info += (bcolors.OKGREEN + ' [SUCCESS]' + bcolors.ENDC)
        else:
            info += (bcolors.FAIL + ' [FAILED]' + bcolors.ENDC)

        print info

    outputs = glob.glob(PYTHON_TESTS_DIR + '*.tmp.py')
    for output in outputs:
        os.remove(output)

    outputs = glob.glob(PYTHON_TESTS_DIR + '*.core')
    for output in outputs:
        os.remove(output)


if __name__ == '__main__':
    main()
