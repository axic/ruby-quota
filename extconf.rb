#
# extconf.rb
#
# Resource
# Copyright (C) 2000 by Takaaki Tateishi <ttate@jaist.ac.jp>
# $Id: extconf.rb,v 1.1 2002/11/12 05:43:18 ttate Exp $

require "mkmf"

have_header("unistd.h")

have_header("linux/quota.h")       # for linux
have_header("linux/types.h")
have_header("sys/quota.h")
have_header("sys/types.h")

have_header("sys/fs/ufs_quota.h")  # for solaris

have_header("ufs/ufs/quota.h")     # for *bsd
have_header("sys/ucred.h")         # required by FreeBSD and NetBSD

create_makefile("quota")
