#! /usr/bin/env python3

# This file is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published
# by the Free Software Foundation; either version 3, or (at your
# option) any later version.
#
# This file is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# For a full copy of the GNU General Public License
# see <http://www.gnu.org/licenses/>.

import os
import re
import sys
import argparse
import datetime
import json
import textwrap
import copy

parser = argparse.ArgumentParser(
    description="Simple ASM formatter for glibc x86_64")

parser.add_argument("--file",
                    action="store",
                    default=None,
                    help="File to parse")
parser.add_argument("-l",
                    action="store_true",
                    default=None,
                    help="Parse from stdin")

parser.add_argument("--lines",
                    action="store",
                    default=None,
                    help="Lines to actually format")

parser.add_argument("--no-indent",
                    action="store_true",
                    default=False,
                    help="Turn off #define indentation")

parser.add_argument("--no-skip",
                    action="store_true",
                    default=False,
                    help="Don't skip leading comments")

parser.add_argument("--width",
                    action="store",
                    default="",
                    help="Set comment wrap width")

parser.add_argument("--config",
                    action="store",
                    default=None,
                    help="Config file")

parser.add_argument("--none",
                    action="store_true",
                    default=None,
                    help="Does nothing")


def str2bool(v):
    return v.lower() in ("yes", "true", "t", "1")


def make_tmp_objdump(config, lines):
    if config.verify_objfile is False:
        return None
    date_marker = str(datetime.datetime.now()).replace(" ", "-").replace(
        ":", "-").replace(".", "-")
    tmpfname = ".tmp-objdump-" + date_marker + ".S"
    assert os.path.exists(
        tmpfname) is False, "tmpfile already exists with same name: {}".format(
            tmpfname)
    assert os.path.exists(
        tmpfname +
        ".o") is False, "tmpfile already exists with same name: {}.o".format(
            tmpfname)

    try:
        tmpfile = open(tmpfname, "w+")
        for line in lines:
            tmpfile.write(line + "\n")
        tmpfile.flush()
        tmpfile.close()
        ret = os.system("gcc -c {} -o {}.o {}".format(tmpfname, tmpfname,
                                                      "> /dev/null 2>&1"))
        # Let this fail silently imo
        if ret != 0:
            os.system("rm -f {} {}".format(tmpfname, "> /dev/null 2>&1"))
            os.system("rm -f {}.o".format(tmpfname, {}))
            return None
        ret = os.system("objdump -d {}.o > {} {}".format(
            tmpfname, tmpfname, "2>&1"))
        os.system("rm -f {}.o {}".format(tmpfname, "> /dev/null 2>&1"))
        if ret != 0:
            os.system("rm -f {}".format(tmpfname, "> /dev/null 2>&1"))
            return None

        return tmpfname

    except IOError:
        os.system("rm -f {}".format(tmpfname))
        return ""


def verify_tmp_objdump(config, old_fname, lines):
    if config.verify_objfile is False or old_fname is None:
        return

    new_fname = make_tmp_objdump(config, lines)
    error = False
    if new_fname is None or new_fname == "":
        error = True

    old_lines = []
    new_lines = []
    try:
        if error is False:
            start = False
            for line in open(old_fname):
                if "Disassembly" in line:
                    start = True
                if start is True:
                    old_lines.append(line)
            start = False
            for line in open(new_fname):
                if "Disassembly" in line:
                    start = True
                if start is True:
                    new_lines.append(line)

    except IOError:
        os.system("rm -f {} {}".format(new_fname, "> /dev/null 2>&1"))
        os.system("rm -f {} {}".format(old_fname, "> /dev/null 2>&1"))
        assert False, "IO Error verify objdump files"

    os.system("rm -f {} {}".format(new_fname, "> /dev/null 2>&1"))
    os.system("rm -f {} {}".format(old_fname, "> /dev/null 2>&1"))

    if error is False:
        if len(old_lines) != len(new_lines):
            error = True
        for i in range(0, len(old_lines)):
            if error is True:
                break
            if old_lines[i] != new_lines[i]:
                error = True

    if error:
        print("##################################")
        print("ERROR: Formatter bug")
        print("The underlying objdump file changed after formatting")
        print("##################################")
        assert False


def make_backup(config, fname, lines):
    if config.backup_path is None or config.do_backup is False:
        return

    date_marker = str(datetime.datetime.now()).replace(" ", "-").replace(
        ":", "-").replace(".", "-")
    if fname is None:
        fname = date_marker
    else:
        fname += "-" + date_marker

    backup_path = config.backup_path
    backup_fname = "{}/{}".format(backup_path, fname)
    backup_path = os.path.dirname(backup_fname)
    assert os.system("mkdir -p {}".format(backup_path)) == 0
    try:
        backup_file = open(backup_fname, "w+")
        for line in lines:
            backup_file.write(line)
        backup_file.flush()
        backup_file.close()
    except IOError:
        assert False, "Error making backup file: {}".format(backup_fname)


def fmt_pieces(pieces, seperator):
    if pieces == []:
        return ""
    out = ""
    for i in range(0, len(pieces) - 1):
        out += pieces[i] + seperator

    out += pieces[len(pieces) - 1]

    return out.rstrip().lstrip()


class Config():
    def __init__(self, config_fname):
        self.config_fname = "{}/.config/abf.json".format(
            os.path.expanduser("~"))
        if config_fname is not None:
            self.config_fname = config_fname

        # Defaults
        self.start = None
        self.end = None
        self.backup_path = None
        self.do_backup = False
        self.padd_indent = True
        self.initial_indent = 0
        self.verify_objfile = False
        self.width = 64
        self.skip_header = 0
        self.with_LOE = -1
        self.smart_comments = False
        if os.access(self.config_fname, os.R_OK) is True:
            try:
                config_file = open(self.config_fname, "r")
                config_data = json.load(config_file)

                if "Backup_Path" in config_data:
                    self.backup_path = config_data["Backup_Path"]
                if "Backup" in config_data:
                    self.do_backup = str2bool(config_data["Backup"])
                if "Padd_Indent" in config_data:
                    self.padd_indent = str2bool(config_data["Padd_Indent"])
                if "Objdump_Verify" in config_data:
                    self.verify_objfile = str2bool(
                        config_data["Objdump_Verify"])
                if "Smart_Comments" in config_data:
                    self.smart_comments = str2bool(
                        config_data["Smart_Comments"])
                if "Skip_Header" in config_data:
                    try:
                        self.skip_header = int(config_data["Skip_Header"])
                    except ValueError:
                        assert False, "Unable to parse \"Skip_Header\""
                        pass
                if "Width" in config_data:
                    try:
                        self.width = int(config_data["Width"])
                    except ValueError:
                        assert False, "Unable to parse \"Width\""
                        pass
                if "Init_Indent" in config_data:
                    try:
                        self.initial_indent = int(config_data["Init_Indent"])
                    except ValueError:
                        assert False, "Unable to parse \"Init_Indent\""
                        pass
                if "With_LOE" in config_data:
                    try:
                        self.with_LOE = int(config_data["With_LOE"])
                    except ValueError:
                        assert False, "Unable to parse \"With_LOE\""
                        pass
            except IOError:
                return


def comment_wrap(line, wraplen, tablen):
    startline = "\t *"
    words_out = ["\t"]
    words = line.split()
    line_len = tablen + 3
    for word in words:
        wlen = len(word)
        if line_len + wlen > wraplen:
            words_out.append("\n")
            if word == "*/":
                words_out.append("\t */")
                break
            words_out.append(startline)
            line_len = tablen + 3
        words_out.append(" " + word)
        line_len += (wlen + 1)

    line = ""
    for word in words_out:
        line += word
    return line


def end_comment(line):
    line = line.rstrip().lstrip()
    spaces = "   "
    if "/*" in line:
        assert line[:2] == "/*", line
        assert line[:2] + line[2:] == line, line

        spaces = "/* "
        line = line[2:]
    line = line.rstrip().lstrip()
    assert line[len(line) - 2:] == "*/", line
    assert line[:len(line) - 2] + line[len(line) - 2:] == line
    line = line[:len(line) - 2]
    line = line.rstrip().lstrip()
    if line[len(line) - 1:] != ".":
        line += "."
    return "\t" + spaces + line + "  */"


def check_entry_end_line(line, directive):
    line = line.lstrip().rstrip()
    dlen = len(directive)
    if line[0:dlen] != directive:
        return False
    if "(" not in line or ")" not in line:
        return False
    assert line.count("(") == line.count(")")

    return True
    expec_paren = line[dlen:].lstrip().rstrip()
    return expec_paren.count("(") == 1 and expec_paren.count(")") == 1


def entry_end_line(line):
    return check_entry_end_line(line, "END") or check_entry_end_line(
        line, "ENTRY") or check_entry_end_line(
            line, "ENTRY_") or check_entry_end_line(line, "P2ALIGN_ENTRY")


class Formatter():
    def __init__(self, conf):
        self.TABLEN = 8

        self.original_line = ""
        self.init_def_count = conf.initial_indent
        self.enable_indent = conf.padd_indent
        self.with_LOE = conf.with_LOE
        self.disabled = False
        self.def_count = self.init_def_count + 1
        self.in_comment = False
        self.comment_text = ""
        self.wrap_width = conf.width
        self.abf_strip = False
        self.first_line = (conf.skip_header == -1)
        self.skip_all_leading = (conf.skip_header == 1)
        self.smart_comments = conf.smart_comments
        self.skipping_first_comment = False

        self.merge_comments = False

        self.line_count = 0
        self.start = conf.start
        self.end = conf.end
        assert conf.width == -1 or conf.width > 10
        self.lost_ifdef = ""
        self.lost_ifdef_line = 0

        self.operators = ["*", "/", "+", "<<", ">>"]
        self.expand_right_tokens = [","] + self.operators
        self.expand_left_tokens = self.expand_right_tokens

        self.cleanup_tokens = ["(", ")", "[", "]", "{", "}", ",", ";"]

    def organize_paren(self, line):
        if "(" not in line:
            return line
        first = line.find("(")
        line = line[:first] + " (" + line[first + 1:]
        first_token = line.split()[0]
        rest = " ".join(line.split()[1:])
        rest = rest.replace(" (", "(")
        out = first_token + " " + rest
        out = out.replace(",(", ", (")
        return out

    def cleanup_ws_tokens(self, line):
        for token in self.cleanup_tokens:
            re_fmt = "[\\s|\\t]*\\{}[\\s|\\t]*".format(token)
            line = re.sub(re_fmt, token, line)

        return line

    def expand_tokens(self, line):
        for token in self.expand_left_tokens:
            line = line.replace(token, " " + token)
        for token in self.expand_right_tokens:
            line = line.replace(token, token + " ")
        return line

    def incr_dc(self):
        if self.enable_indent is True:
            if self.def_count == self.init_def_count + 1:
                self.lost_ifdef = self.original_line
                self.lost_ifdef_line = self.line_count
            self.def_count += 1

    def decr_dc(self):
        if self.enable_indent is True:
            self.def_count -= 1

    def dc(self):
        if self.enable_indent is True:
            return self.def_count
        return self.init_def_count

    def valid(self):
        if not (self.init_def_count + 1) == self.def_count:
            print("Missing endif for\n\t[{}]: {} ".format(
                self.lost_ifdef_line, self.lost_ifdef))
            return False
        return self.in_comment is False

    def check_directive(self, line, content):
        pieces = line.split()
        if len(pieces) == 2:
            if pieces[0] == "//" and pieces[1] == content:
                return True
        if len(pieces) == 1:
            if pieces[0] == "//" + content:
                return True
        return False

    def check_disabled(self, line):
        content = "abf-off"
        if self.disabled is True:
            content = "abf-on"
        ret = self.check_directive(line, content)
        if ret:
            self.disabled = not self.disabled
        return ret

    def check_strip(self, line):
        ret = self.check_directive(line, "abf-strip")
        if ret:
            self.abf_strip = True
        return ret

    def fmt_line(self, line):
        self.original_line = line
        self.line_count += 1
        if self.start is not None and self.end is not None:
            if self.line_count < self.start:
                return self.original_line.replace("\n", "")
            if self.line_count > self.end:
                return self.original_line.replace("\n", "")

        line = line.replace("\n", "").lstrip().rstrip()
        if len(line) == 0:
            return ""
        if self.check_strip(line):
            self.abf_strip = True
            return None

        if self.check_disabled(line):
            if self.abf_strip is False:
                return "// " + line.replace("//", "").lstrip().rstrip()
            else:
                return None
        if self.disabled is True:
            return self.original_line.replace("\n", "")

        if self.first_line is False and line[:2] == "/*":
            if self.skip_all_leading is False:
                self.first_line = True
            if "*/" not in line:
                self.skipping_first_comment = True
            return self.original_line.rstrip()

        if self.skipping_first_comment is True:
            if "*/" in line:
                self.skipping_first_comment = False
            return self.original_line.rstrip()

        if self.in_comment is False and (line.replace("//", "") == "" or (
            ("/*" in line and "*/" in line
             and line.find("/*") < line.find("*/")) and line.replace(
                 "/*", "").replace("*/", "").lstrip().rstrip() == ""
                or line.replace("/*", "").replace(
                    "*/", "").lstrip().rstrip() == ".")):
            return ""
        if "//" in line and self.in_comment is False:
            self.first_line = True
            if "/*" not in line or ("/*" in line and
                                    (line.find("/*") > line.find("//"))):
                pos = line.find("//")

                line_content = line[:line.find("//")]
                line_comment = line[line.find("//") + 2:]
                assert "//" not in line_content
                line_comment = line_comment.lstrip().rstrip()
                self.line_count -= 1
                line_content = self.fmt_line(line_content)
                if line_content == "" and self.wrap_width != int(-1):
                    lines = textwrap.fill(line_comment,
                                          width=self.wrap_width).split("\n")
                    line_comment = lines[0].lstrip().rstrip()
                    if len(lines) != 1:
                        line_comment += "\n"
                        for i in range(1, len(lines) - 1):
                            line_comment += "\t// " + lines[i].lstrip().rstrip(
                            ) + "\n"
                        line_comment += "\t// " + lines[len(lines) -
                                                        1].lstrip().rstrip()
                return line_content + "\t// " + line_comment

        if ("/*" in line and "*/" in line) and (self.in_comment is False) and (
                line[:2] != "/*" or line[len(line) - 2:] != "*/"):
            if "//" not in line or ("//" in line
                                    and line.find("//") > line.find("/*")):
                comment_start = line.find("/*")
                comment_end = line.rfind("*/")
                line_content0 = line[:comment_start]
                line_content1 = line[comment_end + 2:]
                block_comment = line[comment_start:comment_end + 2]

                assert "/*" not in line_content0
                assert "*/" not in line_content0

                assert "/*" not in line_content1
                assert "*/" not in line_content1

                assert block_comment.count("*/") == 1
                assert block_comment.count("/*") == 1

                line_content0 = self.fmt_line(line_content0)
                line_content1 = self.fmt_line(line_content1)
                if line_content0 == "" and line_content1 == "" and self.wrap_width != int(
                        -1):
                    # Not supported for now
                    assert False
                    lines = textwrap.fill(block_comment,
                                          width=self.wrap_width).split("\n")
                    block_comment = ""
                    for i in range(0, len(lines) - 1):
                        block_comment += "\t /*" + lines[i].lstrip().rstrip(
                        ) + "\n"
                    block_comment += end_comment(lines[len(lines) - 1])
                    return block_comment
                s = line_content0 + "\t" + end_comment(
                    block_comment) + "\t" + line_content1
                s = s.rstrip()
                return s

        if self.wrap_width == -1:
            if self.in_comment is True:
                if "*/" in line:
                    self.in_comment = False
                    if "*/" == line:
                        return "\t */"
                    return end_comment(line)
                if self.smart_comments and "* " == line.lstrip()[0:2]:
                    return "\t " + line.rstrip().lstrip()[2:0]
                else:
                    return "\t   " + line.rstrip().lstrip()
            # Handle start comment
            if "/*" in line:
                if "*/" not in line:
                    self.in_comment = True
                    return "\t" + line.rstrip().lstrip()
                else:
                    return end_comment(line)
        else:
            finish_comment = False
            if "/*" in line:
                assert self.in_comment is False
                assert self.comment_text == ""
                self.comment_text = line.rstrip().lstrip()
                if "*/" in line:
                    finish_comment = True
                else:
                    self.in_comment = True
                    return None

            elif self.in_comment is True:
                if self.smart_comments and "* " == line.lstrip().rstrip()[0:2]:
                    line = line.lstrip().rstrip()[2:]

                if self.comment_text != "":
                    self.comment_text += " "
                self.comment_text += line.rstrip().lstrip()

                if "*/" in line:
                    self.in_comment = False
                    finish_comment = True
                else:
                    return None

            if finish_comment is True:
                assert self.in_comment is False
                assert self.comment_text.count("/*") == 1
                assert self.comment_text.count("*/") == 1
                line = self.comment_text.lstrip().rstrip()
                self.comment_text = ""
                #line = comment_wrap(line, self.wrap_width, self.TABLEN)
                line = end_comment(line).lstrip().rstrip()
                line = textwrap.fill(line, width=self.wrap_width)
                lines = line.split("\n")
                line = "\t" + lines[0].lstrip().rstrip()
                if len(lines) != 1:
                    line += "\n"
                    for i in range(1, len(lines) - 1):
                        line += "\t   " + lines[i].lstrip().rstrip() + "\n"

                    if lines[len(lines) - 1] == "*/":
                        line += "\t */"
                    else:
                        line += "\t   " + lines[len(lines) -
                                                1].lstrip().rstrip()
                return line
        self.first_line = True

        pieces = line.split()
        if "#" in line:
            op = pieces[0]
            start = 1
            if len(pieces[0]) == 1:
                op = pieces[1]
                start = 2
            op = op.replace("#", "")

            assert op in [
                "else", "elif", "define", "if", "ifdef", "ifndef", "endif",
                "include", "error", "undef", "LOE"
            ], "{} -> {}".format(self.line_count, self.original_line)

            if op == "LOE":
                assert self.with_LOE != -1, "With_LOE disabled (-1)"
                if self.with_LOE == 0:
                    return ""

            if "endif" == op:
                self.decr_dc()
            pad_count = self.dc()

            if self.enable_indent is True and ("else" == op or "elif" == op):
                pad_count -= 1

            line = "#".ljust(pad_count) + op
            if op == "define":
                line += " " + fmt_pieces(pieces[start:], "\t")
            elif start != len(pieces):
                line += " " + fmt_pieces(pieces[start:], " ")
            if "ifdef" == op or "ifndef" == op or "if" == op:
                self.incr_dc()
            return line

        line = self.cleanup_ws_tokens(line)
        line = self.expand_tokens(line)
        pieces = line.split()

        if (":" in line) and ("." in line) and not entry_end_line(line):
            line = line.replace(":.", ": .")
            pieces = line.split()
            return pieces[0] + "\t" + fmt_pieces(pieces[1:], " ")
        if (":" in line) or entry_end_line(line):
            return fmt_pieces(pieces, " ")

        if ".cfi_" in line:
            return "\t" + fmt_pieces(pieces, " ")

        line = self.organize_paren(line)
        pieces_init = line.split()
        pieces = []
        for piece in pieces_init:
            if "{" in piece and "}" in piece:
                if "{" in piece[0] and "}" == piece[len(piece) -
                                                    1] in piece and (
                                                        "k" in piece
                                                        or "z" in piece):
                    pieces[len(pieces) - 1] += piece
                if piece == pieces_init[len(pieces_init) - 1]:
                    pieces[len(pieces) - 1] += piece
                else:
                    pieces += piece.replace("{", " {").split()
            else:
                pieces.append(piece)
        line = "\t" + pieces[0]
        if len(pieces) != 1:
            prefix = "\t"
            if len(pieces[0]) >= self.TABLEN:
                prefix = " "
            line += prefix + fmt_pieces(pieces[1:], " ")
        line = line.replace(" ,", ",")
        return line


lines = []

args = parser.parse_args()
asm_fname = args.file
from_stdin = args.l
no_indent = args.no_indent
no_skip = args.no_skip
arg_width = args.width
line_bounds = args.lines
skip_line_bounds = args.none
if skip_line_bounds is True:
    line_bounds = None

assert asm_fname is not None or from_stdin is not None
assert asm_fname is None or from_stdin is None

config = Config(args.config)
if arg_width != "":
    try:
        config.width = int(arg_width)
    except ValueError:
        assert False, "width must be an integer"

if no_indent is True:
    config.padd_indent = False
if no_skip is True:
    config.skip_header = False
if asm_fname is None:
    lines = sys.stdin.readlines()
else:
    asm_file = None
    try:
        assert os.access(asm_fname,
                         os.R_OK), "Error {} does not exist".format(asm_fname)
        asm_file = open(asm_fname, "r")
    except IOError:
        assert False, "Error opening {}".format(asm_fname)

    assert asm_file is not None, "Error opening {}".format(asm_fname)

    for line in asm_file:
        lines.append(line)
    asm_file.close()

make_backup(config, asm_fname, lines)
if len(lines) != 0:
    tmpfname = make_tmp_objdump(config, lines)
    assert tmpfname != "", "Error making objdump"

    if line_bounds is not None and line_bounds != "":
        start = int(line_bounds.split(",")[0])
        end = int(line_bounds.split(",")[1])
        config.start = start
        config.end = end
    lines_out = []

    formatter = Formatter(config)
    for line in lines:
        out = formatter.fmt_line(line)
        if out is not None:
            lines_out.append(out)

    verify_tmp_objdump(config, tmpfname, lines_out)

    assert formatter.valid()
    for line in lines_out:
        print(line)