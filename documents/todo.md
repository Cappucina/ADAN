# ADAN TODO

## Overview

### Finished
- Pointers (pointer, dereference, address of, array)
- Break/Continue

### In Development
- adan.io
- adan.string
- adan.net (cross-platform)
- variadic functions implementation in ir and codegen

### Not Started
- 
- vectors (agree on a vector/dynamic array implementation)
- talk about changing string implementation

---

## std lib

### adan.io
- [x] print(str) - ! Replace with print(f)(fmt::string, ...);
- [x] print_int(x) - ! Replace with print(f)(fmt::string, ...);
- [x] read_file_source(path)
- [x] write_file_source(path, content)
- [x] append_file_source(path, content)
- [x] file_exists(path)
- [x] delete_file(path)
- [x] create_directory(path)
- [x] delete_directory(path)
- [x] is_directory(path)
- [x] is_file(path)
- [x] copy_file(src, dst)
- [x] move_file(src, dst)
- [ ] read_line() - Or similar input

---

### adan.string
- [x] concat(s1, s2)
- [x] repeat(s, n)
- [x] reverse(s)
- [x] upper(s)
- [x] lower(s)
- [x] trim(s)
- [x] replace(s, old, new)
- [x] replace_all(s, old, new)
- [x] split(s, delim)
- [x] join(s, delim)
- [x] find(s, sub)
- [x] starts_with(s, prefix)
- [x] to_string(x)
- [x] to_int(s)
- [x] to_bool(s)
- [x] to_char(s)
- [x] to_float(s)
- [ ] contains(s, sub)
- [ ] substring(s, start, len)
- [ ] char_at(s, idx)
- [ ] length(s)
- [ ] is_empty(s)
- [ ] format(fmt, ...)

---

### adan.net
- [x] tcp_listen(port)
- [x] tcp_accept(fd)
- [x] tcp_connect(ip, port)
- [x] tcp_read(fd, buf, len)
- [x] tcp_write(fd, buf, len)
- [x] tcp_read_request(fd, buf, len)
- [x] tcp_write_all(fd, buf, len)
- [x] tcp_read_request_str(fd)
- [x] tcp_write_string(fd, s)
- [x] tcp_close(fd)
- [ ] udp_socket()
- [ ] udp_bind(fd, port)
- [ ] udp_send(fd, ip, port, data, len)
- [ ] udp_recv(fd, buf, len)

---

### New
**adan.math**
**adan.vector**
**adan.map**
**adan.algorithm**
**adan.thread** or **adan.async**