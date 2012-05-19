Ruby/Quota
==========

This module provides Ruby access to manipulating the disk quotas.


Supported systems
-----------------

  * Linux 2.4.x, 2.6.x and newer
  * Solaris 2.6, 7, 8
  * FreeBSD
  * NetBSD
  * OpenBSD
  * Dragonfly BSD
  * Mac OS X


Usage
-----

Common parameters:

* `dev` is a device file (e.g. /dev/hda0) or a mount point (e.g. /mnt/foo), where
  supported. Mount point look up is implemented on BSD and Linux targets.

* `quotafile` is a quota(s) file

* `id` can be an integer to mean uid, otherwise it can be a UserID or a GroupID
  object to signal uid or gid respectively.

* `diskquota` is a DiskQuota structure. It stores all the fields of the quota
  settings. For possible field names and meaning of them see the local quotactl
  man pages on your system.


Functions:

* `Quota.quotaon(dev, quotafile)` to turn quota on.

* `Quota.quotaoff(dev)` to turn quota off.

* `Quota.getquota(dev, id)` returns a DiskQuota structure on success.

* `Quota.setquota(dev, id, diskquota)` updates both the limit and usage settings
  from the provided quota structure.

* `Quota.setlimit(dev, id, diskquota)` updates only the limit settings from the
  provided quota structure. This is not supported on all systems.

* `Quota.setusage(dev, id, diskquota)` updates only the usage settings from the
  provided quota structure. This is not supported on all systems.

* `Quota.sync(dev)` flushes all the changes made to the quota system.

* `Quota::UserID.new(id)` or `Quota::UserID[id]` to create a new uid object.

* `Quota::GroupID.new(id)` or `Quota::GroupID[id]` to create a new gid object.

* `Quota::DiskQuota.new` to create a new DiskQuota structure.


Constants:

* `Quota::BlockSize` (number), quota block size on the given system. Can be used for
   converting between quota blocks count and bytes.

* `Quota::VERSION` (string), version of Ruby/Quota.


DiskQuota fields
----------------

The field names depend on underlying system used, but appropriate aliases and
mappings are provided.

Preferred generic field names:
* `bhardlimit`	- hard limit in blocks
* `bsoftlimit`	- soft limit in blocks
* `curblocks`	- current usage in blocks
* `ihardlimit`	- hard limit in inodes (~files)
* `isoftlimit`	- soft limit in inodes (~files)
* `curinodes`	- current usage in inodes (~files)
* `btimelimit`	- grace period for over the soft limit of blocks
* `itimelimit`	- grace period for over the soft limit of inodes

Aliases:
* `btimelimit`  <>  `btime`
* `itimelimit`  <>  `itime`
* `curinodes`   <>  `curfiles`
* `curblocks`   <>  `curspace`
* `curblocks`   <>  `curbytes`
* `ihardlimit`  <>  `fhardlimit`
* `isoftlimit`  <>  `fsoftlimit`
* `itimelimit`  <>  `ftimelimit`


Underlying quota field names on different systems
-------------------------------------------------

BSD:
* `bhardlimit`
* `bsoftlimit`
* `curblocks`
* `ihardlimit`
* `isoftlimit`
* `curinodes`
* `btime`
* `itime`

Linux:
* `bhardlimit`
* `bsoftlimit`
* `curspace`
* `ihardlimit`
* `isoftlimit`
* `curinodes`
* `btime`
* `itime`

Linux <2.4.22:
* `bhardlimit`
* `bsoftlimit`
* `curblocks`
* `ihardlimit`
* `isoftlimit`
* `curinodes`
* `btime`
* `itime`

Solaris:
* `bhardlimit`
* `bsoftlimit`
* `curblocks`
* `fhardlimit`
* `fsoftlimit`
* `curfiles`
* `btimelimit`
* `ftimelimit`

Mac OS X:
* `bhardlimit`
* `bsoftlimit`
* `curbytes`
* `ihardlimit`
* `isoftlimit`
* `curinodes`
* `btime`
* `itime`
