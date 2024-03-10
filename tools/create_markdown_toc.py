#!/usr/bin/env python3

import argparse
import re

HEADER_REGEX = r'^(#+)(.+)+$'


def header_text_to_link(line: str):
    REPLACE_LIST = {
        ' ': '-',
        '/': '-',
        '?': '-',
        '&': '-',
    }
    link = line.lstrip().rstrip()
    for key,val in REPLACE_LIST.items():
        link = link.replace(key, val)
    return link


"""
Returns a string which contains the markdown for the table of contents.
"""
def add_toc(filename: str, tag: str):
    LINE_TOC_BEGIN = f'<!-- {tag} -->'
    LINE_TOC_END = f'<!-- END_{tag} -->'

    s = set()

    with open(filename, 'r+') as file:
        lines = [line for line in file]
        line_n_toc_begin = -1
        line_n_toc_end = -1
        md_toc_lines = []

        for i,line in enumerate(lines):
            if LINE_TOC_BEGIN in line:
                if line_n_toc_begin >= 0:
                    raise ValueError(f'begin tag already found at line {i}')
                line_n_toc_begin = i
            if LINE_TOC_END in line:
                if line_n_toc_end >= 0:
                    raise ValueError(f'end tag already found at line {i}')
                line_n_toc_end = i

            header_matches = re.findall(HEADER_REGEX, line)
            if len(header_matches) > 0:
                header_level = len(header_matches[0][0])
                header_label = header_matches[0][1].lstrip()
                header_link_name = header_text_to_link(header_label)

                if header_link_name in s:
                    print('warning: duplicated header link')
                s.add(header_link_name)

                # skip the top level header
                if header_level > 1:
                    # Create a line item in the TOC
                    toc_item_text = f'[{header_label}](#{header_link_name})'
                    toc_line = (header_level - 2) * '    ' + f'1. {toc_item_text}\n'
                    md_toc_lines.append(toc_line)

                    # Replace the header with an inline link
                    header_link = f'<a name="{header_link_name}"/>'
                    header_line = f'{header_matches[0][0]} {header_label} {header_link}\n'
                    lines[i] = header_line

        if line_n_toc_begin == -1:
            raise ValueError(f'beginning tag {LINE_TOC_BEGIN} not found')
        if line_n_toc_end == -1:
            raise ValueError(f'beginning tag {LINE_TOC_END} not found')

        # insert TOC between tags
        lines = lines[:line_n_toc_begin+1] + \
                md_toc_lines + \
                lines[line_n_toc_end:]

        # re-write file
        file.seek(0)
        for line in lines:
            file.write(line)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Generates TOC markdown for a given markdown doc. Optionally modifies the md file to add TOC header links.')
    parser.add_argument('filename', help='Name of Markdown file')
    parser.add_argument('toc_tag', help='Table of contents tag. E.g. for "TAG", place <!-- TOC --> and <!-- END_TAG --> around what the script will replace with the table of contents.')
    args = parser.parse_args()

    add_toc(filename=args.filename, tag=args.toc_tag)
