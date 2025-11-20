#!python3
import freetype
import zlib
import sys
import re
import math
import argparse
from collections import namedtuple
import io
import os

parser = argparse.ArgumentParser(description="Generate a header file from a font to be used with epdiy.")
parser.add_argument("name", action="store", help="name of the font.")
parser.add_argument("size", type=int, nargs='?', help="font size to use (optional if --sizes or --size-range provided).")
parser.add_argument("fontstack", action="store", nargs='+', help="list of font files, ordered by descending priority.")
parser.add_argument("--compress", dest="compress", action="store_true", help="compress glyph bitmaps.")
parser.add_argument("--output", dest="output", help="output header file path; if omitted, writes to stdout.")
parser.add_argument("--debug", dest="debug", action="store_true", help="print per-glyph debug metrics to stderr")
parser.add_argument("--sizes", dest="sizes", type=int, nargs='+', help="explicit list of font sizes to generate (e.g. --sizes 8 10 12)")
parser.add_argument("--size-range", dest="size_range", type=int, nargs=3, metavar=("MIN","MAX","STEP"), help="range of sizes inclusive (e.g. --size-range 8 24 2)")
parser.add_argument("--output-dir", dest="output_dir", help="directory to write multiple header files when generating more than one size")
args = parser.parse_args()

# Ensure UTF-8 output regardless of platform console defaults (especially Windows)
try:
    sys.stdout.reconfigure(encoding="utf-8")
    sys.stderr.reconfigure(encoding="utf-8")
except AttributeError:
    # Fallback for older Python versions
    if hasattr(sys.stdout, "buffer"):
        sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding="utf-8")
    if hasattr(sys.stderr, "buffer"):
        sys.stderr = io.TextIOWrapper(sys.stderr.buffer, encoding="utf-8")

# If an explicit output file is provided, redirect stdout to UTF-8 file
if getattr(args, "output", None):
    sys.stdout = open(args.output, "w", encoding="utf-8", newline='\n')

GlyphProps = namedtuple("GlyphProps", ["width", "height", "advance_x", "left", "top", "compressed_size", "data_offset", "code_point"])

font_stack = [freetype.Face(f) for f in args.fontstack]
compress = args.compress
base_size = args.size
base_font_name = args.name

# inclusive unicode code point intervals
# must not overlap and be in ascending order
intervals = [
    (32, 126),
    # (160, 255),
    # (0x2500, 0x259F),
    # (0x2700, 0x27BF),
    # # powerline symbols
    # (0xE0A0, 0xE0A2),
    # (0xE0B0, 0xE0B3),
    # (0x1F600, 0x1F680),
]


def norm_floor(val):
    return int(math.floor(val / (1 << 6)))

def norm_ceil(val):
    return int(math.ceil(val / (1 << 6)))

def set_stack_size(stack, px_size):
    for face in stack:
        # shift by 6 bytes, because sizes are given as 6-bit fractions
        # the display has about 150 dpi.
        face.set_char_size(px_size << 6, px_size << 6, 150, 150)

def chunks(l, n):
    for i in range(0, len(l), n):
        yield l[i:i + n]

def load_glyph(code_point, stack):
    face_index = 0
    while face_index < len(stack):
        face = stack[face_index]
        glyph_index = face.get_char_index(code_point)
        if glyph_index > 0:
            face.load_glyph(glyph_index, freetype.FT_LOAD_RENDER)
            return face
            break
        face_index += 1
        print (f"falling back to font {face_index} for {chr(code_point)}.", file=sys.stderr)
    raise ValueError(f"code point {code_point} not found in font stack!")
def generate_header_for_size(px_size, symbol_base_name, stack, compress, debug):
    # Prepare faces for this size
    set_stack_size(stack, px_size)

    total_size = 0
    total_packed = 0
    all_glyphs = []

    # inner function uses current stack
    for i_start, i_end in intervals:
        for code_point in range(i_start, i_end + 1):
            face = load_glyph(code_point, stack)
            bitmap = face.glyph.bitmap
            pixels = []
            px = 0
            for i, v in enumerate(bitmap.buffer):
                y = i / bitmap.width
                x = i % bitmap.width
                if x % 2 == 0:
                    px = (v >> 4)
                else:
                    px = px | (v & 0xF0)
                    pixels.append(px);
                    px = 0
                # eol
                if x == bitmap.width - 1 and bitmap.width % 2 > 0:
                    pixels.append(px)
                    px = 0

            # Horizontally mirror glyph (flip along vertical axis)
            # Each byte holds two 4-bit pixels: low nibble = first (left) pixel,
            # high nibble = second pixel. We unpack a row into `width` nibbles,
            # reverse the order, then repack.
            if bitmap.width > 0 and bitmap.rows > 0:
                row_byte_len = (bitmap.width + 1) // 2
                original_rows = list(chunks(pixels, row_byte_len))
                mirrored_rows = []
                for row in original_rows:
                    nibbles = []
                    for b in row:
                        if len(nibbles) < bitmap.width:
                            nibbles.append(b & 0x0F)
                        if len(nibbles) < bitmap.width:
                            nibbles.append((b >> 4) & 0x0F)
                    nibbles.reverse()
                    repacked = []
                    i2 = 0
                    while i2 < len(nibbles):
                        low = nibbles[i2] & 0x0F
                        if i2 + 1 < len(nibbles):
                            high = (nibbles[i2 + 1] & 0x0F) << 4
                            repacked.append(low | high)
                        else:
                            repacked.append(low)  # odd width: pad high nibble with 0
                        i2 += 2
                    mirrored_rows.append(repacked)
                pixels = [b for row in mirrored_rows for b in row]

            packed = bytes(pixels);
            total_packed += len(packed)
            compressed_bytes = packed
            if compress:
                compressed_bytes = zlib.compress(packed)

            # Update left bearing for true mirror about the glyph origin:
            # new_left = -(old_left + width). Advance stays unchanged.
            advance = norm_floor(face.glyph.advance.x)
            mirrored_left = advance - face.glyph.bitmap_left - bitmap.width

            # Debug output: show original vs mirrored metrics when requested.
            if debug:
                orig_left = face.glyph.bitmap_left
                # compute bounding boxes (xMin,xMax) relative to glyph origin
                orig_xmin = orig_left
                orig_xmax = orig_left + bitmap.width
                mir_xmin = mirrored_left
                mir_xmax = mirrored_left + bitmap.width
                print(f"U+{code_point:04X} '{chr(code_point)}': size={px_size} advance={advance} orig_left={orig_left} width={bitmap.width} orig_bounds=({orig_xmin},{orig_xmax}) mirrored_left={mirrored_left} mirrored_bounds=({mir_xmin},{mir_xmax})", file=sys.stderr)

            glyph = GlyphProps(
                width = bitmap.width,
                height = bitmap.rows,
                advance_x = advance,
                left = mirrored_left,
                top = face.glyph.bitmap_top,
                compressed_size = len(compressed_bytes),
                data_offset = total_size,
                code_point = code_point,
            )
            total_size += len(compressed_bytes)
            all_glyphs.append((glyph, compressed_bytes))

    # pipe seems to be a good heuristic for the "real" descender
    face = load_glyph(ord('|'), stack)

    glyph_data = []
    glyph_props = []
    for index, glyph in enumerate(all_glyphs):
        props, compressed = glyph
        glyph_data.extend([b for b in compressed])
        glyph_props.append(props)

    print("total", total_packed, file=sys.stderr)
    print("compressed", total_size, file=sys.stderr)

    # Symbol names include size suffix to keep them unique
    sym = f"{symbol_base_name}{px_size}"

    lines = []
    lines.append("#pragma once")
    lines.append("#include \"epd_driver.h\"")
    lines.append(f"const uint8_t {sym}Bitmaps[{len(glyph_data)}] = {{")
    for c in chunks(glyph_data, 16):
        lines.append("    " + " ".join(f"0x{b:02X}," for b in c))
    lines.append("};")

    lines.append(f"const GFXglyph {sym}Glyphs[] = {{")
    for i, g in enumerate(glyph_props):
        lines.append("    { " + ", ".join([f"{a}" for a in list(g[:-1])]) + "}, " + (f"// {chr(g.code_point) if g.code_point != 92 else '<backslash>'}"))
    lines.append("};")

    lines.append(f"const UnicodeInterval {sym}Intervals[] = {{")
    offset = 0
    for i_start, i_end in intervals:
        lines.append(f"    {{ 0x{i_start:X}, 0x{i_end:X}, 0x{offset:X} }},")
        offset += i_end - i_start + 1
    lines.append("};")

    lines.append(f"const GFXfont {sym} = {{")
    lines.append(f"    (uint8_t*){sym}Bitmaps,")
    lines.append(f"    (GFXglyph*){sym}Glyphs,")
    lines.append(f"    (UnicodeInterval*){sym}Intervals,")
    lines.append(f"    {len(intervals)},")
    lines.append(f"    {1 if compress else 0},")
    lines.append(f"    {norm_ceil(face.size.height)},")
    lines.append(f"    {norm_ceil(face.size.ascender)},")
    lines.append(f"    {norm_floor(face.size.descender)},")
    lines.append("};")

    return "\n".join(lines)


# Determine sizes to generate
sizes_to_generate = None
if args.sizes:
    sizes_to_generate = sorted(set([s for s in args.sizes if s > 0]))
elif args.size_range:
    mn, mx, st = args.size_range
    if st <= 0:
        raise ValueError("--size-range STEP must be > 0")
    if mx < mn:
        raise ValueError("--size-range MAX must be >= MIN")
    sizes_to_generate = list(range(mn, mx + 1, st))
else:
    if base_size is None:
        raise SystemExit("Error: provide a single size (positional) or use --sizes/--size-range")
    sizes_to_generate = [base_size]

# Generate outputs
multiple = len(sizes_to_generate) > 1
out_dir = args.output_dir
if multiple and not out_dir and not args.output:
    # Default to current directory if not specified
    out_dir = os.getcwd()
if out_dir:
    os.makedirs(out_dir, exist_ok=True)

for s in sizes_to_generate:
    header_text = generate_header_for_size(s, base_font_name, font_stack, compress, getattr(args, "debug", False))

    if multiple or out_dir:
        # Write to file per size
        dirpath = out_dir if out_dir else os.getcwd()
        filepath = os.path.join(dirpath, f"{base_font_name}{s}.h")
        with open(filepath, "w", encoding="utf-8", newline='\n') as f:
            f.write(header_text + "\n")
        print(f"Wrote {filepath}", file=sys.stderr)
    elif args.output:
        # Single size, explicit file
        with open(args.output, "w", encoding="utf-8", newline='\n') as f:
            f.write(header_text + "\n")
    else:
        # Single size to stdout
        print(header_text)
print("};")