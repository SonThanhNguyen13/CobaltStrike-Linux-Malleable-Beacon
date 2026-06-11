#!/usr/bin/env python3

import sys
import os
import tempfile
import re

TRANSFORM_TYPES = {
    "base64": "TRANSFORM_BASE64",
    "base64url": "TRANSFORM_BASE64URL",
    "mask": "TRANSFORM_MASK",
    "netbios": "TRANSFORM_NETBIOS",
    "netbiosu": "TRANSFORM_NETBIOSU",
    "prepend": "TRANSFORM_PREPEND",
    "append": "TRANSFORM_APPEND",
}

TERMINATOR_TYPES = {
    "print": "TERMINATOR_PRINT",
    "header": "TERMINATOR_HEADER",
    "parameter": "TERMINATOR_PARAMETER",
    "uri-append": "TERMINATOR_URI_APPEND",
}

def c_string_escape(s):
    result = []
    for ch in s:
        if ch == '"':
            result.append('\\"')
        elif ch == '\\':
            result.append('\\\\')
        elif ch == '\n':
            result.append('\\n')
        elif ch == '\r':
            result.append('\\r')
        elif ch == '\t':
            result.append('\\t')
        elif ord(ch) < 32 or ord(ch) > 126:
            result.append(f'\\x{ord(ch):02x}')
        else:
            result.append(ch)
    return ''.join(result)


def extract_quoted_string(text, start_quote):
    i = start_quote + 1
    result = []
    while i < len(text):
        ch = text[i]
        if ch == '\\' and i + 1 < len(text):
            next_ch = text[i + 1]
            if next_ch == '"':
                result.append('"')
                i += 2
            elif next_ch == '\\':
                result.append('\\')
                i += 2
            elif next_ch == 'n':
                result.append('\n')
                i += 2
            elif next_ch == 'r':
                result.append('\r')
                i += 2
            elif next_ch == 't':
                result.append('\t')
                i += 2
            elif next_ch == 'x' and i + 3 < len(text):
                hex_str = text[i+2:i+4]
                try:
                    result.append(chr(int(hex_str, 16)))
                    i += 4
                except ValueError:
                    result.append(ch)
                    i += 1
            else:
                result.append(ch)
                result.append(next_ch)
                i += 2
        elif ch == '"':
            return ''.join(result), i + 1
        else:
            result.append(ch)
            i += 1
    return ''.join(result), i


def strip_comments(text):
    result = []
    i = 0
    in_string = False
    while i < len(text):
        if not in_string and text[i] == '#' and (i == 0 or text[i-1] != '$'):
            while i < len(text) and text[i] != '\n':
                i += 1
            continue
        if text[i] == '"':
            in_string = not in_string
        result.append(text[i])
        i += 1
    return ''.join(result)


class ProfileParser:
    def __init__(self):
        self.data = {
            "sample_name": "",
            "useragent": "",
            "sleeptime": None,
            "jitter": None,
            "http_get": {
                "uri": [],
                "verb": "GET",
                "client_headers": [],
                "metadata_transforms": [],
                "metadata_terminator": None,
                "metadata_terminator_arg": None,
                "server_headers": [],
                "server_output_transforms": [],
                "server_output_terminator": None,
                "server_output_terminator_arg": None,
            },
            "http_post": {
                "uri": [],
                "verb": "POST",
                "client_headers": [],
                "id_transforms": [],
                "id_terminator": None,
                "id_terminator_arg": None,
                "output_transforms": [],
                "output_terminator": None,
                "output_terminator_arg": None,
                "server_headers": [],
                "server_output_transforms": [],
                "server_output_terminator": None,
                "server_output_terminator_arg": None,
            },
        }

    def parse(self, text):
        self._parse_set_directives(text)
        self._parse_http_get(text)
        self._parse_http_post(text)
        self._validate()
        return self.data

    def _parse_set_directives(self, text):
        m = re.search(r'set\s+sample_name\s+"([^"]*)"', text)
        if m:
            self.data["sample_name"] = m.group(1)

        m = re.search(r'set\s+useragent\s+"((?:[^"\\]|\\.)*)"', text)
        if m:
            self.data["useragent"] = m.group(1)

        m = re.search(r'set\s+sleeptime\s+"([^"]*)"', text)
        if m:
            try:
                self.data["sleeptime"] = int(m.group(1))
            except ValueError:
                pass

        m = re.search(r'set\s+jitter\s+"([^"]*)"', text)
        if m:
            try:
                self.data["jitter"] = int(m.group(1))
            except ValueError:
                pass

    def _parse_header_directive(self, text):
        headers = []
        clean = strip_comments(text)
        for m in re.finditer(r'header\s+"((?:[^"\\]|\\.)*)"\s+"((?:[^"\\]|\\.)*)"\s*;', clean):
            headers.append({"name": m.group(1), "value": m.group(2)})
        return headers

    def _parse_transform_block(self, block_text):
        transforms = []
        terminator = None
        terminator_arg = None

        clean = strip_comments(block_text)

        i = 0
        while i < len(clean):
            while i < len(clean) and clean[i] in ' \t\r\n':
                i += 1
            if i >= len(clean):
                break

            word_match = re.match(r'[a-zA-Z][-a-zA-Z0-9]*', clean[i:])
            if not word_match:
                i += 1
                continue

            word = word_match.group(0)
            i += len(word)

            while i < len(clean) and clean[i] in ' \t':
                i += 1

            if word in TRANSFORM_TYPES:
                arg = None
                if word in ("prepend", "append"):
                    if i < len(clean) and clean[i] == '"':
                        arg, i = extract_quoted_string(clean, i)
                transforms.append({"type": word, "arg": arg})

                while i < len(clean) and clean[i] != ';':
                    i += 1
                if i < len(clean):
                    i += 1

            elif word in TERMINATOR_TYPES:
                terminator = word
                if word != "print":
                    if i < len(clean) and clean[i] == '"':
                        terminator_arg, i = extract_quoted_string(clean, i)
                    else:
                        terminator_arg = None
                else:
                    terminator_arg = None

                while i < len(clean) and clean[i] != ';':
                    i += 1
                if i < len(clean):
                    i += 1
            else:
                while i < len(clean) and clean[i] != ';' and clean[i] != '\n':
                    i += 1
                if i < len(clean) and clean[i] == ';':
                    i += 1

        return transforms, terminator, terminator_arg

    def _extract_brace_block(self, text, keyword):
        pattern = re.compile(keyword + r'\s*\{')
        m = pattern.search(text)
        if not m:
            return ""
        start = m.end()
        depth = 1
        i = start
        while i < len(text) and depth > 0:
            if text[i] == '{':
                depth += 1
            elif text[i] == '}':
                depth -= 1
            elif text[i] == '"':
                i += 1
                while i < len(text) and text[i] != '"':
                    if text[i] == '\\':
                        i += 1
                    i += 1
            i += 1
        return text[start:i-1]

    def _parse_http_get(self, text):
        get_block = self._extract_brace_block(text, r'http-get')
        if not get_block:
            return

        for m in re.finditer(r'set\s+uri\s+"([^"]+)"', get_block):
            self.data["http_get"]["uri"] = [u.strip() for u in m.group(1).split(' ')]

        m = re.search(r'set\s+verb\s+"([^"]+)"', get_block)
        if m:
            self.data["http_get"]["verb"] = m.group(1)

        client_block = self._extract_brace_block(get_block, r'client')
        if client_block:
            self.data["http_get"]["client_headers"] = self._parse_header_directive(client_block)

            metadata_block = self._extract_brace_block(client_block, r'metadata')
            if metadata_block:
                transforms, terminator, terminator_arg = self._parse_transform_block(metadata_block)
                self.data["http_get"]["metadata_transforms"] = transforms
                self.data["http_get"]["metadata_terminator"] = terminator
                self.data["http_get"]["metadata_terminator_arg"] = terminator_arg

        server_block = self._extract_brace_block(get_block, r'server')
        if server_block:
            self.data["http_get"]["server_headers"] = self._parse_header_directive(server_block)

            output_block = self._extract_brace_block(server_block, r'output')
            if output_block:
                transforms, terminator, terminator_arg = self._parse_transform_block(output_block)
                self.data["http_get"]["server_output_transforms"] = transforms
                self.data["http_get"]["server_output_terminator"] = terminator
                self.data["http_get"]["server_output_terminator_arg"] = terminator_arg

    def _parse_http_post(self, text):
        post_block = self._extract_brace_block(text, r'http-post')
        if not post_block:
            return

        for m in re.finditer(r'set\s+uri\s+"([^"]+)"', post_block):
            self.data["http_post"]["uri"] = [u.strip() for u in m.group(1).split(' ')]

        m = re.search(r'set\s+verb\s+"([^"]+)"', post_block)
        if m:
            self.data["http_post"]["verb"] = m.group(1)

        client_block = self._extract_brace_block(post_block, r'client')
        if client_block:
            self.data["http_post"]["client_headers"] = self._parse_header_directive(client_block)

            id_block = self._extract_brace_block(client_block, r'\bid\b')
            if id_block:
                transforms, terminator, terminator_arg = self._parse_transform_block(id_block)
                self.data["http_post"]["id_transforms"] = transforms
                self.data["http_post"]["id_terminator"] = terminator
                self.data["http_post"]["id_terminator_arg"] = terminator_arg

            output_block = self._extract_brace_block(client_block, r'output')
            if output_block:
                transforms, terminator, terminator_arg = self._parse_transform_block(output_block)
                self.data["http_post"]["output_transforms"] = transforms
                self.data["http_post"]["output_terminator"] = terminator
                self.data["http_post"]["output_terminator_arg"] = terminator_arg

        server_block = self._extract_brace_block(post_block, r'server')
        if server_block:
            self.data["http_post"]["server_headers"] = self._parse_header_directive(server_block)

            output_block = self._extract_brace_block(server_block, r'output')
            if output_block:
                transforms, terminator, terminator_arg = self._parse_transform_block(output_block)
                self.data["http_post"]["server_output_transforms"] = transforms
                self.data["http_post"]["server_output_terminator"] = terminator
                self.data["http_post"]["server_output_terminator_arg"] = terminator_arg

    def _validate(self):
        def check_chain(chain, term, block_name, chain_name):
            if not chain:
                return
            is_binary = False
            for t in chain:
                if t["type"] == "mask":
                    is_binary = True
                elif t["type"] in ("base64", "base64url", "netbios", "netbiosu"):
                    is_binary = False
            if is_binary and term != "print":
                print(f"Warning: {block_name} {chain_name} produces binary output but doesn't use 'print' terminator")

        check_chain(self.data["http_get"].get("metadata_transforms", []),
                     self.data["http_get"].get("metadata_terminator"), "GET", "metadata")
        check_chain(self.data["http_get"].get("server_output_transforms", []),
                     self.data["http_get"].get("server_output_terminator"), "GET", "server_output")
        check_chain(self.data["http_post"].get("id_transforms", []),
                     self.data["http_post"].get("id_terminator"), "POST", "id")
        check_chain(self.data["http_post"].get("output_transforms", []),
                     self.data["http_post"].get("output_terminator"), "POST", "output")
        check_chain(self.data["http_post"].get("server_output_transforms", []),
                     self.data["http_post"].get("server_output_terminator"), "POST", "server_output")


class ProfileCodeGenerator:
    def __init__(self, data):
        self.data = data
        self.var_counter = 0

    def _unique_name(self, prefix):
        self.var_counter += 1
        return f"{prefix}_{self.var_counter}"

    def _emit_transform_steps(self, steps, var_name):
        lines = []
        lines.append(f"static const transform_step_t {var_name}[] = {{")
        for step in steps:
            type_enum = TRANSFORM_TYPES.get(step["type"], "TRANSFORM_BASE64")
            if step["arg"] is not None:
                escaped = c_string_escape(step["arg"])
                lines.append(f'    {{ {type_enum}, "{escaped}" }},')
            else:
                lines.append(f"    {{ {type_enum}, NULL }},")
        lines.append("};")
        return lines

    def _emit_transform_chain(self, chain_steps, terminator, terminator_arg, chain_name, steps_name):
        lines = []
        if not chain_steps:
            lines.append(f"static const transform_step_t {steps_name}[] = {{")
            lines.append(f"    {{ {TRANSFORM_TYPES['base64']}, NULL }},")
            lines.append(f"}};")
            lines.append(f"static const transform_chain_t {chain_name} = {{")
            lines.append(f"    .steps = {steps_name},")
            lines.append(f"    .step_count = 1,")
            lines.append(f"    .terminator = TERMINATOR_PRINT,")
            lines.append(f"    .terminator_arg = NULL,")
            lines.append(f"}};")
            return lines

        lines.extend(self._emit_transform_steps(chain_steps, steps_name))

        term_enum = TERMINATOR_TYPES.get(terminator, "TERMINATOR_PRINT") if terminator else "TERMINATOR_PRINT"
        term_arg_str = f'"{c_string_escape(terminator_arg)}"' if terminator_arg else "NULL"

        lines.append(f"static const transform_chain_t {chain_name} = {{")
        lines.append(f"    .steps = {steps_name},")
        lines.append(f"    .step_count = {len(chain_steps)},")
        lines.append(f"    .terminator = {term_enum},")
        lines.append(f"    .terminator_arg = {term_arg_str},")
        lines.append(f"}};")
        return lines

    def _emit_uri_array(self, uris, var_name):
        lines = []
        lines.append(f"static const char *{var_name}[] = {{")
        for uri in uris:
            lines.append(f'    "{c_string_escape(uri)}",')
        lines.append("};")
        return lines

    def _emit_header_array(self, headers, var_name):
        lines = []
        lines.append(f"static const profile_header_t {var_name}[] = {{")
        for h in headers:
            lines.append(f'    {{ "{c_string_escape(h["name"])}", "{c_string_escape(h["value"])}" }},')
        lines.append("};")
        return lines

    def generate(self):
        lines = []
        lines.append("#ifndef PROFILE_GENERATED_H")
        lines.append("#define PROFILE_GENERATED_H")
        lines.append("")
        lines.append('#include "transform.h"')
        lines.append('#include "profile.h"')
        lines.append("")

        name = self.data.get("sample_name", "custom") or "custom"
        lines.append(f'#define PROFILE_NAME "{c_string_escape(name)}"')
        lines.append("#define PROFILE_VERSION 1")
        lines.append('#define PROFILE_GENERATED_BY "InsertProfile.py v1.0"')
        lines.append("")

        sleeptime = self.data.get("sleeptime")
        if sleeptime is not None:
            lines.append(f"#define PROFILE_SLEEP_TIME {sleeptime}")
        else:
            lines.append("#define PROFILE_SLEEP_TIME 60000")

        jitter = self.data.get("jitter")
        if jitter is not None:
            lines.append(f"#define PROFILE_JITTER {jitter}")
        else:
            lines.append("#define PROFILE_JITTER 0")
        lines.append("")

        http_get = self.data["http_get"]
        http_post = self.data["http_post"]

        lines.extend(self._emit_uri_array(http_get["uri"] or ["/"], "gen_get_uris"))
        lines.append("")
        lines.extend(self._emit_uri_array(http_post["uri"] or ["/"], "gen_post_uris"))
        lines.append("")

        if http_get["client_headers"]:
            lines.extend(self._emit_header_array(http_get["client_headers"], "gen_get_headers"))
        else:
            lines.append("static const profile_header_t gen_get_headers[] = {};")
        lines.append("")

        if http_post["client_headers"]:
            lines.extend(self._emit_header_array(http_post["client_headers"], "gen_post_headers"))
        else:
            lines.append("static const profile_header_t gen_post_headers[] = {};")
        lines.append("")

        lines.extend(self._emit_transform_chain(
            http_get["metadata_transforms"],
            http_get["metadata_terminator"],
            http_get["metadata_terminator_arg"],
            "gen_metadata_chain", "gen_metadata_steps"))
        lines.append("")

        lines.extend(self._emit_transform_chain(
            http_post["id_transforms"],
            http_post["id_terminator"],
            http_post["id_terminator_arg"],
            "gen_id_chain", "gen_id_steps"))
        lines.append("")

        lines.extend(self._emit_transform_chain(
            http_post["output_transforms"],
            http_post["output_terminator"],
            http_post["output_terminator_arg"],
            "gen_output_chain", "gen_output_steps"))
        lines.append("")

        lines.extend(self._emit_transform_chain(
            http_get["server_output_transforms"],
            http_get["server_output_terminator"],
            http_get["server_output_terminator_arg"],
            "gen_server_get_chain", "gen_server_get_steps"))
        lines.append("")

        lines.extend(self._emit_transform_chain(
            http_post["server_output_transforms"],
            http_post["server_output_terminator"],
            http_post["server_output_terminator_arg"],
            "gen_server_post_chain", "gen_server_post_steps"))
        lines.append("")

        lines.append("#define GENERATED_GET_URIS gen_get_uris")
        lines.append(f"#define GENERATED_GET_URI_COUNT {len(http_get['uri'] or ['/'])}")
        lines.append(f'#define GENERATED_GET_VERB "{c_string_escape(http_get.get("verb", "GET"))}"')
        lines.append("#define GENERATED_GET_HEADERS gen_get_headers")
        lines.append(f"#define GENERATED_GET_HEADER_COUNT {len(http_get.get('client_headers', []))}")
        lines.append("#define GENERATED_METADATA_TRANSFORM gen_metadata_chain")
        lines.append("")

        lines.append("#define GENERATED_POST_URIS gen_post_uris")
        lines.append(f"#define GENERATED_POST_URI_COUNT {len(http_post['uri'] or ['/'])}")
        lines.append(f'#define GENERATED_POST_VERB "{c_string_escape(http_post.get("verb", "POST"))}"')
        lines.append("#define GENERATED_POST_HEADERS gen_post_headers")
        lines.append(f"#define GENERATED_POST_HEADER_COUNT {len(http_post.get('client_headers', []))}")
        lines.append("#define GENERATED_ID_TRANSFORM gen_id_chain")
        lines.append("#define GENERATED_OUTPUT_TRANSFORM gen_output_chain")
        lines.append("")

        lines.append("#define GENERATED_SERVER_GET_TRANSFORM gen_server_get_chain")
        lines.append("#define GENERATED_SERVER_POST_TRANSFORM gen_server_post_chain")
        lines.append("")

        ua = self.data.get("useragent", "")
        if not ua:
            ua = "Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; Win64; x64; Trident/5.0)"
        lines.append(f'#define GENERATED_USER_AGENT "{c_string_escape(ua)}"')
        lines.append("")
        lines.append("#endif")
        lines.append("")

        return "\n".join(lines)


def main():
    if len(sys.argv) < 3:
        print(f"Usage: {sys.argv[0]} <input.profile> <output_header.h>")
        sys.exit(1)

    input_file = sys.argv[1]
    output_file = sys.argv[2]

    if not os.path.exists(input_file):
        print(f"Error: Profile file not found: {input_file}")
        sys.exit(1)

    with open(input_file, 'r') as f:
        profile_text = f.read()

    parser = ProfileParser()
    try:
        data = parser.parse(profile_text)
    except Exception as e:
        print(f"Error parsing profile: {e}")
        sys.exit(1)

    generator = ProfileCodeGenerator(data)
    header_content = generator.generate()

    tmp_fd, tmp_path = tempfile.mkstemp(suffix='.h', dir=os.path.dirname(output_file) or '.')
    try:
        with os.fdopen(tmp_fd, 'w') as f:
            f.write(header_content)
        os.rename(tmp_path, output_file)
    except Exception:
        os.unlink(tmp_path)
        raise

    print(f"Generated: {output_file}")
    print(f"Profile: {data.get('sample_name', 'unknown')}")
    print(f"Sleep: {data.get('sleeptime', 'default')}ms, Jitter: {data.get('jitter', 'default')}%")
    print(f"GET URI: {data['http_get']['uri']}")
    print(f"POST URI: {data['http_post']['uri']}")
    print(f"User-Agent: {data.get('useragent', 'default')[:80]}...")
    for name, chain in [("GET metadata", data['http_get']['metadata_transforms']),
                        ("POST id", data['http_post']['id_transforms']),
                        ("POST output", data['http_post']['output_transforms']),
                        ("GET server output", data['http_get']['server_output_transforms']),
                        ("POST server output", data['http_post']['server_output_transforms'])]:
        parts = [f"{t['type']}({repr(t['arg'][:30])}...)" if t['arg'] and len(t['arg']) > 30 else f"{t['type']}({repr(t['arg'])})" for t in chain]
        print(f"  {name}: {' -> '.join(parts) if parts else '(none)'}")


if __name__ == '__main__':
    main()
