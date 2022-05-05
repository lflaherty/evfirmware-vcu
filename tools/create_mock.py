#!/usr/bin/env python3
from pathlib import Path
import sys
import os
import re
import json
import datetime
import argparse

"""
Reads header file and creates:
{
    "include_guard": name of #ifndef include guard
    "methods": list of methods to be mocked
        {
            "return_type":
            "method_name":
            "params":
        }
    "header_filename": name of original header (relative to evfirmware-lib/src)
}
"""
def parse(filename):
    def get_header_filename(filename):
        abs_path = os.path.abspath(filename)
        if '/src/' not in abs_path:
            raise ValueError('The input file is not within the src directory')
        rel_path = abs_path.split('/src/')[1]
        return rel_path
    def get_include_guard(text):
        _REGEX_1 = '#ifndef (\w+_H_)'
        _REGEX_2 = '#define (\w+_H_)'
        _REGEX_3 = '#endif \/\* (\w+_H_) \*\/'

        res_1 = re.findall(_REGEX_1, text)
        res_2 = re.findall(_REGEX_2, text)
        res_3 = re.findall(_REGEX_3, text)

        guards_found = all([
            len(res_1) == 1,
            len(res_2) == 1,
            len(res_3) == 1,
        ])
        if not guards_found:
            raise ValueError('Could not find any include guards')

        guards_consistent = all([
            res_1 == res_2,
            res_2 == res_3
        ])
        if not guards_consistent:
            raise ValueError('Multiple options for include guard value')

        # By this point, they're all the same:
        return res_1[0]
    def get_method_names(text):
        # Regex to roughly match a C method
        # _REGEX_METHOD = '\w+\s+(\w+)\s*\([\w &*,\/]*\);'
        _REGEX_METHOD = '\w*\s*(\w+)\s+(\w+)(\s*\([\w &*,\/]*\));'
        # Explainer:
        # \w*   Optional qualifier(s) before type
        # \s*   Optional space between qualifier(s) and type
        # (\w+) Return typename (e.g. void or a type) (capture in a group)
        # \s+   Whitespace between typename and method name
        # (\w+) Method name (captured in a group)
        # \s*   Optional whitespace between method name and (
        # \(    Open bracket
        # [\w &*,\/\\\]*  any of: characters, whitespace, &, *, /
        # \);   close bracket and ;
        results = re.findall(_REGEX_METHOD, text)
        results = [
            {
                'return_type': match[0],
                'method_name': match[1],
                'params': match[2]
            }
            for match in results
        ]
        return results

    with open(filename) as file:
        text = file.read()

    data = {}
    data['header_filename'] = get_header_filename(filename)
    data['methods'] = get_method_names(text)
    data['include_guard'] = get_include_guard(text)

    return data


def _caps_first(string):
    if len(string) == 0:
        return string
    elif len(string) == 1:
        return string[0].upper()
    else:
        return string[0].upper() + string[1:]


def create_header_file(parse_data, output_dir, author):
    output_path = '{}/Mock{}.h'.format(
        output_dir,
        _caps_first(Path(parse_data['header_filename']).stem))
    output_basename = os.path.basename(output_path)
    date = datetime.datetime.now().strftime('%-d %b %Y')

    lines = []

    # Create content
    header_comment = """/*
 * {}
 *
 *  Created on: {}
 *      Author: {}
 */""".format(output_basename, date, author)
    lines.append(header_comment)
    lines.append('')
    lines.append('#ifndef _MOCK_{}'.format(parse_data['include_guard']))
    lines.append('#define _MOCK_{}'.format(parse_data['include_guard']))
    lines.append('')

    lines.append('// Redefine methods to be mocked')
    for method in parse_data['methods']:
        method_name = method['method_name']
        method_redefine = f'#define {method_name} stub_{method_name}'
        lines.append(method_redefine)
    
    lines.append('')
    lines.append('// Bring in the header to be mocked')
    lines.append('#include "{}"'.format(parse_data['header_filename']))

    lines.append('')
    lines.append('// ============= Mock control methods =============')

    # Add default return status overrides
    for method in parse_data['methods']:
        return_type = method['return_type']
        method_name = method['method_name']

        if return_type != 'void':
            set_status_method = f'void mockSet_{method_name}_Status({return_type} status);'
            lines.append(set_status_method)

    lines.append('')
    lines.append('#endif // _MOCK_{}'.format(parse_data['include_guard']))
    
    print('Writing %s' % output_path)
    with open(output_path, 'w') as file:
        contents = '\n'.join(lines)
        file.writelines(contents)

    return output_basename


def create_source_file(parse_data, output_dir, mock_header, author):
    output_path = '{}/Mock{}.c'.format(
        output_dir,
        _caps_first(Path(parse_data['header_filename']).stem))
    output_basename = os.path.basename(output_path)
    date = datetime.datetime.now().strftime('%-d %b %Y')

    lines = []
    
    # Create content
    header_comment = """/*
 * {}
 *
 *  Created on: {}
 *      Author: {}
 */
""".format(output_basename, date, author)
    lines.append(header_comment)
    lines.append('#include "{}"'.format(mock_header))

    lines.append('')
    lines.append('// ------------------- Static data -------------------')
    # Add default return status overrides
    for method in parse_data['methods']:
        return_type = method['return_type']
        method_name = method['method_name']

        if return_type != 'void':
            static_field = f'static {return_type} mStatus_{method_name} = TODO_SET_VALUE;'
            lines.append(static_field)

    lines.append('')
    lines.append('// ------------------- Methods -------------------')
    for method in parse_data['methods']:
        return_type = method['return_type']
        method_name = method['method_name']
        params = method['params']
        method_declaration = f"""{return_type} stub_{method_name}{params}
{{
    // TODO
    return mStatus_{method_name};
}}"""
        lines.append(method_declaration)
        lines.append('')

    for method in parse_data['methods']:
        return_type = method['return_type']
        method_name = method['method_name']
        params = method['params']

        if return_type != 'void':
            method_declaration = f"""void mockSet_{method_name}_Status({return_type} status)
{{
    mStatus_{method_name} = status;
}}"""
            lines.append(method_declaration)
            lines.append('')
    
    print('Writing %s' % output_path)
    with open(output_path, 'w') as file:
        contents = '\n'.join(lines)
        file.writelines(contents)


def main(input_file, output_dir, author):
    if not os.path.exists(output_dir):
        raise ValueError('Ouptut directory does not exist')

    parse_data = parse(input_file)
    print(json.dumps(parse_data, indent=4))

    mock_header = create_header_file(
        parse_data=parse_data,
        output_dir=output_dir,
        author=author
    )
    create_source_file(
        parse_data=parse_data,
        output_dir=output_dir,
        author=author,
        mock_header=mock_header
    )

    print('Files require review and modification before use')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Generate mock templates')
    parser.add_argument('author', help='Author to label mock files')
    parser.add_argument('input_file', help='Input header file')
    parser.add_argument('output_dir', help='Directory to store generated mocks')
    args = parser.parse_args()
    
    main(
        input_file=args.input_file,
        output_dir=args.output_dir,
        author=args.author
    )