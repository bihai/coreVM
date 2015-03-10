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
import sys
import traceback


class CodeTransformer(ast.NodeVisitor):

    TAB = '\t'

    def __init__(self, options):
        self.options = options
        self.indent_level = 0

    def __indent(self):
        self.indent_level += 1

    def __dedent(self):
        self.dedent_level -= 1

    def transform(self):
        with open(self.options.input_file, 'r') as fd:
            tree = ast.parse(fd.read())

        transformed_str = self.visit(tree)

        if self.options.debug_mode:
            print 'Writing the following to %s' % self.options.output_file
            print transformed_str

        with open(self.options.output_file, 'w') as fd:
            fd.write(transformed_str)

    def visit_FunctionDef(self, node):
        base_str = 'def {func_name}({arguments}):\n{body}'

        return base_str.format(
            func_name=node.name,
            arguments=self.visit(node.args),
            body=''.join([self.visit(stmt) for stmt in node.body])
        )

    """ ------------------------------ mod --------------------------------- """

    def visit_Module(self, node):
        return ''.join([self.visit(stmt) for stmt in node.body])

    """ ----------------------------- stmt --------------------------------- """

    def visit_ClassDef(self, node):
        base_str = 'class {class_name}:\n{body}'

        return base_str.format(
            class_name=node.name,
            body=''.join([self.visit(stmt) for stmt in node.body])
        )

    def visit_Return(self, node):
        base_str = 'return'

        if node.value:
            base_str += (' ' + self.visit(node.value))

        return base_str

    def visit_Print(self, node):
        base_str = 'print'

        if node.values:
            values_str = '.'.join([self.visit(value) for value in node.values])
            base_str += (' ' + values_str)

        return base_str

    def visit_Expr(self, node):
        return self.visit(node.value)

    """ ----------------------------- expr --------------------------------- """

    def visit_BinOp(self, node):
        base_str = '{lhs}.{func}({lhs}, {rhs})'

        return base_str.format(
            lhs=self.visit(node.left),
            func=self.visit(node.op),
            rhs=self.visit(node.right)
        )

    def visit_Call(self, node):
        base_str = '__call({caller}'

        base_str = base_str.format(
            caller=self.visit(node.func)
        )

        if node.args:
            base_str += (', ' + ', '.join([self.visit(arg) for arg in node.args]))

        if node.kwargs:
            kwargs_str = ', '.join(
                [
                    '{keyword}={value}'.format(
                        keyword=keyword.arg,
                        value=self.visit(keyword.value)
                    )
                    for keyword in node.keywords
                ]
            )
            base_str += (', ' + kwargs_str)

        if node.starargs:
            base_str += (', ' + '*' + self.visit(node.starargs))

        if node.kwargs:
            base_str += (', ' + '**' + self.visit(node.kwargs))

        base_str += ')'

        return base_str

    def visit_Num(self, node):
        return str(node.n)

    def visit_Name(self, node):
        return node.id

    def visit_Str(self, node):
        return '\'{s}\''.format(s=node.s)

    """ --------------------------- operator ------------------------------- """

    def visit_Add(self, node):
        return '__add__'

    def visit_Sub(self, node):
        return '__sub__'

    def visit_Mult(self, node):
        return '__mul__'

    def visit_Div(self, node):
        return '__div__'

    def visit_Mod(self, node):
        return '__mod__'

    """ ---------------------------- unaryop ------------------------------- """

    def visit_Not(self, node):
        return 'not'

    """ --------------------------- arguments ------------------------------ """

    def visit_arguments(self, node):
        # A mapping of the closest arguments to their default values by their
        # column offset, for explicit kwargs.
        base_str = ''

        closest_args_to_defaults = {}

        # Traverse through the default values, and find the closest arguments
        # for each of them, and put them into the mapping above.
        for default in node.defaults:
            closest_arg = None
            for arg in node.args:
                if closest_arg is None:
                    closest_arg = arg
                elif arg.col_offset > closest_arg.col_offset and arg.col_offset < default.col_offset:
                    closest_arg = arg

            assert closest_arg

            closest_args_to_defaults[closest_arg.col_offset] = default

        # Iterate through the arguments.
        for arg in node.args:
            default = closest_args_to_defaults.get(arg.col_offset)
            if default:
                base_str += (arg.id + '=' + self.visit(default) + ', ')
            else:
                base_str += (arg.id + ', ')

        if node.vararg:
            base_str += ('*' + node.vararg + ', ')

        if node.kwarg:
            base_str += ('*' + node.kwarg + ', ')

        base_str.replace(', )', ')')

        return base_str


def main():
    parser = optparse.OptionParser(
        usage='usage: %prog [options]',
        version='%prog v0.1')

    parser.add_option(
        '-i',
        '--input',
        action='store',
        dest='input_file',
        help='Input file'
    )

    parser.add_option(
        '-o',
        '--output',
        action='store',
        dest='output_file',
        help='Output file'
    )

    parser.add_option(
        '-d',
        '--debug',
        action='store_true',
        dest='debug_mode',
        help='Debug mode'
    )

    options, _ = parser.parse_args()

    if not options.input_file:
        sys.stderr.write('Input file not specified\n')
        return -1

    if not options.output_file:
        sys.stderr.write('Output file not specified\n')
        return -1

    try:
        transformer = CodeTransformer(options)
        transformer.transform()
    except Exception as ex:
        sys.stderr.write('Failed to process %s\n' % options.input_file)
        sys.stderr.write(str(ex))
        sys.stderr.write('\n')
        if options.debug_mode:
            print traceback.format_exc()
        sys.exit(-1)


if __name__ == '__main__':
    main()
