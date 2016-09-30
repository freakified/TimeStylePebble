#!/usr/bin/env python
import zipfile
import os
import argparse
import logging

def copy_zip_file(filename, zip_from, zip_to):
    logging.info("Copying (%s --> %s): %s ", zip_from.filename, zip_to.filename, filename)
    zip_to.writestr(filename, zip_from.read(filename))

def copy_zip_directories(directories, zip_from, zip_to):
    """
    Copy a list of directories from one zip to another
    :param directories: a list of directories to copy over
    :param zip_to: zip to copy to
    :param zip_from: zip to copy from
    :return: yields successfully copied filenames
    """
    for f in zip_from.infolist():
        # Copy the file if it is one level deep in a directory with one of the requested names
        if os.path.split(f.filename)[0] in directories:
            copy_zip_file(f.filename, zip_from, zip_to)
            yield f.filename

def combine_zips(output_filename, zips_and_folders, default_filename):
    """ Copy folders from various zip files to one zip file. Then,
    for any filenames not copied in that step, copy remaining files
    from a default zipfile
    :param output: name of zip file to create
    :param pbws: map of 'zipfiles'->[foldernames]
    :param default: name of zip file to copy all 'other' files from
    """
    with zipfile.ZipFile(output_filename, 'w') as output:
        copied_files = set()
        # Copy the requested folders from each zip in to the output zip
        for filename, folders in zips_and_folders.iteritems():
            with zipfile.ZipFile(filename, 'r') as input:
                input.filename = filename
                # Remember which files we've copied
                copied_files.update(copy_zip_directories(folders, input, output))

        # Then copy remaining files from the default zip to the output zip
        with zipfile.ZipFile(default_filename, 'r') as input:
            input.filename = default_filename
            for f in input.infolist():
                if f.filename not in copied_files and '__MACOSX' not in f.filename:
                    copy_zip_file(f.filename, input, output)

def main():
    parser = argparse.ArgumentParser(description="Combine two PBWs, one targeted toward Aplite, and one targeted toward everything else, into one franken-PBW")
    parser.add_argument('legacypbw', help='path to .pbw file for Aplite')
    parser.add_argument('modernpbw', help='path to .pbw file for Basalt, Chalk, and Diorite')
    parser.add_argument('out', help='filename for output file')
    parser.add_argument('-v', '--verbose', action="store_true", help="Output all copy operations")
    args = parser.parse_args()
    if args.verbose:
        logging.basicConfig(level=logging.DEBUG)

    combine_zips(args.out, {
        args.legacypbw: ['aplite'],
        args.modernpbw: ['basalt','chalk','diorite']
    }, args.modernpbw)

if __name__ == '__main__':
    main()
