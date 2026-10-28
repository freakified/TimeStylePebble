#!/usr/bin/env python3
"""Convert the clock digits of a TTF/OTF into an SVG font for fctx-compiler.

The watch only ever draws the clock's digits, so only digits, colon and
space are written. Glyphs are rescaled so the digits come out at a fixed
height in fctx's 72-unit em, which lets every clock font share the same
layout metrics in clock_area.c.

Requires fontTools (pip install fonttools). Typical use:

    python3 tools/ttf_to_svgfont.py "EBGaramond[wght].ttf" build/garamond.svg \\
        --id EBGaramondRegular --wght 400
    (cd tools && pnpm install)
    tools/node_modules/.bin/fctx-compiler build/garamond.svg -r "[0-9: ]"

--like copies the advances and digit placement of an existing .ffont, for
adding another weight of a font whose .ffont was built by other means
(LECO's regular weight predates the current LECO TTFs and is monospaced).
"""

import argparse
import struct
from xml.sax.saxutils import quoteattr

from fontTools.pens.boundsPen import BoundsPen
from fontTools.pens.svgPathPen import SVGPathPen
from fontTools.pens.transformPen import TransformPen
from fontTools.ttLib import TTFont

GLYPHS = '0123456789:'
# metrics below are in fctx's 72-unit em, but the SVG is written at 16x
# that: fctx-compiler parseInt()s advances, and 1/16 unit is the most
# precision the .ffont format keeps anyway
EM = 72.0
SVG_SCALE = 16

# the existing Avenir .ffonts: digit height and ascent at a 72 em
AVENIR_DIGIT_HEIGHT = 51.0
AVENIR_ASCENT = 50.375
AVENIR_DESCENT = -17.5625


def read_ffont(path):
    """Advance and xMin (72-unit em) of each glyph in a compiled .ffont."""
    nargs = {'M': 2, 'Z': 0, 'L': 2, 'H': 1, 'V': 1, 'C': 6, 'S': 4, 'Q': 4, 'T': 2}
    data = open(path, 'rb').read()
    header = struct.unpack('<hhhhHH', data[:12])
    index_len, table_len = header[4], header[5]
    ranges = [struct.unpack('<HH', data[12 + 4 * i:16 + 4 * i]) for i in range(index_len)]
    table_off = 12 + 4 * index_len
    path_data = data[table_off + 6 * table_len:]
    codepoints = [cp for begin, end in ranges for cp in range(begin, end)]

    glyphs = {}
    for i, cp in enumerate(codepoints):
        offset, length, adv = struct.unpack('<HHH', data[table_off + 6 * i:table_off + 6 * i + 6])
        pd = path_data[offset:offset + length]
        pos, xs = 0, []
        while pos < len(pd):
            cmd = chr(struct.unpack('<h', pd[pos:pos + 2])[0])
            n = nargs[cmd]
            args = struct.unpack('<%dh' % n, pd[pos + 2:pos + 2 + 2 * n])
            pos += 2 + 2 * n
            if cmd == 'H':
                xs.append(args[0])
            elif cmd != 'V':
                xs.extend(args[0::2])
        glyphs[chr(cp)] = (adv / 16.0, min(xs) / 16.0 if xs else None)
    return glyphs


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('font')
    ap.add_argument('output')
    ap.add_argument('--id', required=True, help='font id; names the .ffont')
    ap.add_argument('--wght', type=float, help='instance a variable font at this weight')
    ap.add_argument('--digit-height', type=float, default=AVENIR_DIGIT_HEIGHT,
                    help='height of "0" in a 72-unit em (default: match Avenir)')
    ap.add_argument('--like', help='copy advances and digit x placement from this .ffont')
    args = ap.parse_args()

    font = TTFont(args.font)
    if args.wght is not None:
        from fontTools.varLib.instancer import instantiateVariableFont
        font = instantiateVariableFont(font, {'wght': args.wght})

    glyph_set = font.getGlyphSet()
    cmap = font.getBestCmap()

    def bounds(name):
        pen = BoundsPen(glyph_set)
        glyph_set[name].draw(pen)
        return pen.bounds

    zero = bounds(cmap[ord('0')])
    scale = args.digit_height / (zero[3] - max(zero[1], 0)) * SVG_SCALE
    like = read_ffont(args.like) if args.like else None

    out = []
    for ch in GLYPHS:
        name = cmap[ord(ch)]
        adv = glyph_set[name].width * scale
        dx = 0.0
        if like:
            ref_adv, ref_xmin = like[ch]
            dx = ref_xmin * SVG_SCALE - bounds(name)[0] * scale
            adv = ref_adv * SVG_SCALE
        pen = SVGPathPen(glyph_set, ntos=lambda v: ('%.3f' % v).rstrip('0').rstrip('.'))
        glyph_set[name].draw(TransformPen(pen, (scale, 0, 0, scale, dx, 0)))
        out.append('<glyph unicode=%s glyph-name=%s horiz-adv-x="%d" d=%s/>'
                   % (quoteattr(ch), quoteattr(name), round(adv), quoteattr(pen.getCommands())))

    space_adv = like[' '][0] * SVG_SCALE if like else glyph_set[cmap[ord('0')]].width * scale
    out.append('<glyph unicode=" " glyph-name="space" horiz-adv-x="%d"/>' % round(space_adv))

    svg = ('<?xml version="1.0" standalone="no"?>\n'
           '<svg xmlns="http://www.w3.org/2000/svg"><defs>\n'
           '<font id=%s horiz-adv-x="%d">\n'
           '<font-face units-per-em="%g" ascent="%g" descent="%g" cap-height="%g"/>\n'
           '%s\n</font></defs></svg>\n'
           % (quoteattr(args.id), round(space_adv), EM * SVG_SCALE,
              AVENIR_ASCENT * SVG_SCALE, AVENIR_DESCENT * SVG_SCALE,
              args.digit_height * SVG_SCALE, '\n'.join(out)))
    with open(args.output, 'w') as f:
        f.write(svg)


if __name__ == '__main__':
    main()
