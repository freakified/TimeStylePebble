"""Packs resources/data/languages.json into the LANGUAGES raw resource.

One fixed-size record per language, in id order, laid out like the C
LanguageStrings struct (languages.h): 7 day names and 12 month names of
LANGUAGE_NAME_SIZE bytes each, then the word for "week" in
LANGUAGE_WEEK_SIZE bytes. Strings are UTF-8, NUL-padded.

After the records comes the locale table the "automatic" language setting
matches the watch's locale against: one LANGUAGE_LOCALE_SIZE entry per
ISO 639-1 code or alias, holding the two code letters then the language id.

The watch loads only the selected language's record, rather than keeping
every language's strings in app RAM. wscript runs this on every build.
"""
import json
import os

NAME_SIZE = 8  # LANGUAGE_NAME_SIZE
WEEK_SIZE = 12  # LANGUAGE_WEEK_SIZE
COUNT = 37  # LANGUAGE_COUNT


def pack(text, size, where):
    data = text.encode('utf-8')
    if len(data) >= size:
        raise ValueError('%s: "%s" is %d bytes, max %d' % (where, text, len(data), size - 1))
    return data + b'\0' * (size - len(data))


def generate(json_path, bin_path):
    with open(json_path, encoding='utf-8') as f:
        languages = json.load(f)

    if len(languages) != COUNT:
        raise ValueError('%d languages, but LANGUAGE_COUNT is %d' % (len(languages), COUNT))

    out = bytearray()
    locales = bytearray()
    seen = {}
    for index, lang in enumerate(languages):
        where = 'language %d (%s)' % (index, lang['code'])
        if lang['id'] != index:
            raise ValueError('%s: listed out of id order' % where)
        if len(lang['days']) != 7 or len(lang['months']) != 12:
            raise ValueError('%s: needs 7 days and 12 months' % where)
        for name in lang['days'] + lang['months']:
            out += pack(name, NAME_SIZE, where)
        out += pack(lang['week'], WEEK_SIZE, where)
        for code in [lang['code']] + lang.get('aliases', []):
            if len(code) != 2 or not code.isascii() or not code.islower():
                raise ValueError('%s: "%s" is not a lowercase ISO 639-1 code' % (where, code))
            if code in seen:
                raise ValueError('%s: "%s" is already language %d' % (where, code, seen[code]))
            seen[code] = index
            locales += code.encode('ascii') + bytes([index])
    out += locales

    # skip the write when nothing changed, so waf doesn't rebuild resources
    if os.path.exists(bin_path):
        with open(bin_path, 'rb') as f:
            if f.read() == out:
                return
    with open(bin_path, 'wb') as f:
        f.write(out)


if __name__ == '__main__':
    root = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')
    generate(os.path.join(root, 'resources/data/languages.json'),
             os.path.join(root, 'resources/data/languages.bin'))
