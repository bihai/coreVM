# The MIT License (MIT)

# Copyright (c) 2015 Yanzheng Li

# Permission is hereby granted, free of charge, to any person obtaining a copy of
# this software and associated documentation files (the "Software"), to deal in
# the Software without restriction, including without limitation the rights to
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
# the Software, and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:

# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
# IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

import ast
import optparse
import simplejson
import sys

from datetime import datetime
from pprint import pprint


class Instr(object):

    def __init__(self, code, oprd1, oprd2):
        self.code = code
        self.oprd1 = oprd1
        self.oprd2 = oprd2


class BytecodeGenerator(ast.NodeVisitor):

    format = 'application/json'
    format_version = '0.1'
    target_version = '0.1'
    encoding = 'utf8'
    author = 'Yanzheng Li'

    def __init__(self, output_file, instr_str_to_code_map):
        self.output_file = output_file
        self.instr_str_to_code_map = instr_str_to_code_map
        self.vector = []

    def finalize(self):
        structured_bytecode = {
            'format': self.format,
            'format-version': self.format_version,
            'target-version': self.target_version,
            'path': '',
            'timestamp': datetime.now().strftime('%Y-%m-%dT%H:%M:%S'),
            'encoding': self.encoding,
            'author': self.author,
            'encoding_map': [],
            '__MAIN__': [
                {
                    '__name__': '__main__',
                    '__vector__': [
                        [
                            str(instr.code),
                            str(instr.oprd1),
                            str(instr.oprd2)
                        ] for instr in self.vector
                    ]
                },
            ]
        }

        print 'Writing the following to %s:\n' % self.output_file
        pprint(structured_bytecode)

        with open(self.output_file, 'w') as fd:
            fd.write(simplejson.dumps(structured_bytecode))

    def __add_instr(self, code, oprd1, oprd2):
        self.vector.append(
            Instr(
                self.instr_str_to_code_map[code],
                oprd1,
                oprd2
            )
        )

    def visit_BinOp(self, node):
        self.visit_Num(node.left)
        self.visit_Num(node.right)
        self.visit_Add(node.op)

    def visit_Add(self, node):
        self.__add_instr('add', 0, 0)

    def visit_Num(self, node):
        self.__add_instr('uint32', node.n, 0)


def main():
    parser = optparse.OptionParser(
        usage='usage: %prog filename [options]',
        version='%prog v0.1')

    parser.add_option(
        '-i',
        '--instr-info-file',
        action='store',
        dest='instr_info_file',
        help='Instruction Info File')

    parser.add_option(
        '-o',
        '--output',
        action='store',
        dest='output',
        help='Output file'
    )

    (options, args) = parser.parse_args()

    if not options.instr_info_file:
        sys.stderr.write('Instruction info file not specified\n')
        return -1

    if not args:
        sys.stderr.write('Output file not specified\n')
        return -1

    # Extract instr info
    instr_str_to_code_map = None
    with open(options.instr_info_file, 'r') as fd:
        instr_str_to_code_map = simplejson.load(fd)

    output_file = args[0]

    tree = ast.parse('1 + 2')
    generator = BytecodeGenerator(output_file, instr_str_to_code_map)
    generator.visit(tree)
    generator.finalize()


if __name__ == '__main__':
    main()
