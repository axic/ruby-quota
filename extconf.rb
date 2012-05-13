#
# extconf.rb
#
# Copyright (C) 2000 by Takaaki Tateishi <ttate@jaist.ac.jp>
#

require "mkmf"

have_header("unistd.h")

# ensure we do not enable the known broken version:
# http://lkml.indiana.edu/hypermail/linux/kernel/0705.0/1234.html
if try_link("#include <ruby.h>\n#include <linux/quota.h>\nvoid main() {}")
  have_header("linux/quota.h")       # for linux
end
have_header("linux/types.h")
have_header("sys/quota.h")
have_header("sys/types.h")

have_header("sys/fs/ufs_quota.h")  # for solaris

have_header("ufs/ufs/quota.h")     # for *bsd
have_header("sys/ucred.h")         # required by FreeBSD and NetBSD
have_header("sys/statvfs.h")       # required by NetBSD

create_makefile("quota")
