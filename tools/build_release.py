#!/usr/bin/env python3
"""Build the release PBW: this branch for every platform except aplite, plus
the legacy/aplite branch's aplite build spliced in. See docs/aplite-legacy.md."""
import argparse
import json
import os
import shutil
import subprocess
import sys
import tempfile
import zipfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
LEGACY_REF = 'legacy/aplite'


def run(cmd, cwd=ROOT):
    print('+ ' + ' '.join(cmd), flush=True)
    subprocess.check_call(cmd, cwd=cwd)


def load_package(path):
    with open(path) as f:
        return json.load(f)


def write_package(path, package):
    with open(path, 'w') as f:
        json.dump(package, f, indent=2)
        f.write('\n')


def check_keys(package):
    legacy = json.loads(subprocess.check_output(
        ['git', 'show', LEGACY_REF + ':package.json'], cwd=ROOT))
    keys = set(package['pebble']['messageKeys'])
    missing = [k for k in legacy['pebble']['messageKeys'] if k not in keys]
    if missing:
        sys.exit('messageKeys is missing legacy keys: ' + ', '.join(missing))


def build_legacy(package, worktree):
    run(['git', 'worktree', 'add', '--detach', worktree, LEGACY_REF])
    legacy_path = os.path.join(worktree, 'package.json')
    legacy = load_package(legacy_path)
    legacy['version'] = package['version']
    legacy['pebble']['messageKeys'] = package['pebble']['messageKeys']
    legacy['pebble']['targetPlatforms'] = ['aplite']
    write_package(legacy_path, legacy)
    run(['pebble', 'build'], cwd=worktree)
    return os.path.join(worktree, 'build', os.path.basename(worktree) + '.pbw')


def splice(new_pbw, legacy_pbw, output):
    with zipfile.ZipFile(new_pbw) as new, zipfile.ZipFile(legacy_pbw) as legacy, \
            zipfile.ZipFile(output, 'w', zipfile.ZIP_DEFLATED) as out:
        if any(n.startswith('aplite/') for n in new.namelist()):
            sys.exit('new build unexpectedly contains aplite/')
        for info in new.infolist():
            data = new.read(info.filename)
            if info.filename == 'appinfo.json':
                appinfo = json.loads(data)
                appinfo['targetPlatforms'] = ['aplite'] + appinfo['targetPlatforms']
                data = json.dumps(appinfo, indent=2).encode()
            out.writestr(info.filename, data)
        aplite = [n for n in legacy.namelist() if n.startswith('aplite/')]
        if not aplite:
            sys.exit('legacy build has no aplite/ folder')
        for name in aplite:
            out.writestr(name, legacy.read(name))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--keep-worktree', action='store_true',
                        help='leave the legacy worktree in place for debugging')
    args = parser.parse_args()

    package = load_package(os.path.join(ROOT, 'package.json'))
    if 'aplite' in package['pebble']['targetPlatforms']:
        sys.exit('this branch must not target aplite')
    check_keys(package)

    worktree = os.path.join(tempfile.mkdtemp(prefix='timestyle-'), 'legacy-aplite')
    try:
        legacy_pbw = build_legacy(package, worktree)
        # waf doesn't regenerate message_keys.auto.h when messageKeys changes
        run(['pebble', 'clean'])
        run(['pebble', 'build'])
        new_pbw = os.path.join(ROOT, 'build', os.path.basename(ROOT) + '.pbw')

        output = os.path.join(ROOT, 'Releases',
                              'TimeStyle-Release-%s.pbw' % package['version'])
        os.makedirs(os.path.dirname(output), exist_ok=True)
        splice(new_pbw, legacy_pbw, output)
        print('wrote ' + output)
    finally:
        if not args.keep_worktree and os.path.exists(worktree):
            run(['git', 'worktree', 'remove', '--force', worktree])
            shutil.rmtree(os.path.dirname(worktree), ignore_errors=True)


if __name__ == '__main__':
    main()
