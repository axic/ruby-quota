/* -*- C -*-
 * $Id: quota.c,v 1.3 2004/08/25 16:27:50 ttate Exp $
 * Copyright (C) 2000,2001,2002,2003,2004 Takaaki Tateishi <ttate@ttsky.net>
 */

/*
 * TODO:
 * - should use the allocation framework of ruby-1.8.x.
 */

#include "ruby.h"

#define RUBY_QUOTA_VERSION "0.7.0"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_LINUX_QUOTA_H       /* for linux-2.4.x, 2.6.x */
# define USE_LINUX_QUOTA
#endif
#ifdef HAVE_SYS_FS_UFS_QUOTA_H  /* for Solaris-2.6,7,8 */
# define USE_SOLARIS_QUOTA
#endif
#ifdef HAVE_UFS_UFS_QUOTA_H     /* for *BSD */
# define USE_BSD_QUOTA
#endif
#ifdef __APPLE__
# define USE_MACOSX_QUOTA
#endif

#ifdef USE_LINUX_QUOTA
#include <linux/version.h>
#ifdef HAVE_LINUX_TYPES_H
#  include <linux/types.h>
#else
#  include <sys/types.h>
#endif
#ifdef HAVE_LINUX_QUOTA_H
#  include <linux/quota.h>
#  define uid_t qid_t
#  if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
#    define dqblk if_dqblk
#  else
#    define dqblk disk_dqblk
#  endif
#else
#  include <sys/quota.h>
#endif
#endif /* USE_LINUX_QUOTA */

#ifdef USE_SOLARIS_QUOTA
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/fs/ufs_quota.h>
#endif

#ifdef USE_BSD_QUOTA
#include <sys/types.h>
#include <sys/fcntl.h>
#include <ufs/ufs/quota.h>
#include <sys/param.h>
#include <sys/mount.h>
#if defined(SYS_UCRED_H)
# include <sys/ucred.h>  /* required by NetBSD,FreeBSD */
#endif
#if defined(__DragonFly__)
#  include <sys/param.h>
#  if __DragonFly_version >= 160000
#    define dqblk ufs_dqblk
#  endif
#endif
#endif

/* XXX: bad workaround for Snow Leopard/Lion */
#ifdef USE_MACOSX_QUOTA
#include <sys/quota.h>
#include <sys/mount.h>
#define USE_BSD_QUOTA
#endif

/* XXX: Ruby 1.9 workaround */
#ifndef STR2CSTR
#define STR2CSTR(x) StringValuePtr(x)
#endif

static VALUE rb_mQuota;
static VALUE rb_cUID_,  rb_cGroupID, rb_cUserID;
static VALUE rb_sDiskQuota;
static VALUE rb_eQuotaError, rb_eQuotaCtlError;

typedef struct q_uid_data {
  uid_t uid;
} q_uid_data;

static uid_t
rb_quota_uid(VALUE vuid)
{
  return ((q_uid_data*)DATA_PTR(vuid))->uid;
};

static void
rb_quota_uid_free(q_uid_data *data)
{
  if( data ) free(data);
};

static VALUE
rb_quota_uid_new(VALUE klass, uid_t uid)
{
  q_uid_data *data;
  VALUE obj;

  obj = Data_Make_Struct(klass, q_uid_data, 0, rb_quota_uid_free, data);
  data->uid = uid;

  return obj;
};

static VALUE
rb_quota_uid_s_new(int argc, VALUE argv[], VALUE klass)
{
  VALUE num, obj;

  rb_scan_args(argc, argv, "1", &num);
  obj = rb_quota_uid_new(klass, NUM2UINT(num));

  rb_obj_call_init(obj, argc, argv);

  return obj;
};

static VALUE
rb_quota_uid_initialize(int argc, VALUE argv[], VALUE self)
{
  return Qnil;
};

static VALUE
rb_quota_uid_to_i(VALUE self)
{
  return UINT2NUM(((q_uid_data*)DATA_PTR(self))->uid);
};

static void
get_uid(VALUE vuid, uid_t *uid, int *is_gid)
{
  if( (TYPE(vuid) == T_FIXNUM) || (TYPE(vuid) == T_BIGNUM) ){
    if( uid ) *uid = NUM2UINT(vuid);
    if( is_gid ) *is_gid = 0;
  }
  else if( vuid == Qnil ){
    if( uid ) *uid = 0;
    if( is_gid ) *is_gid = 0;
  }
  else if( rb_obj_is_kind_of(vuid, rb_cUserID) ){
    if( uid ) *uid = rb_quota_uid(vuid);
    if( is_gid ) *is_gid = 0;
  }
  else if( rb_obj_is_kind_of(vuid, rb_cGroupID) ){
    if( uid ) *uid = rb_quota_uid(vuid);
    if( is_gid ) *is_gid = 1;
  }
  else{
    rb_raise(rb_eTypeError, "An uid or gid is expected.");
  };
};


#if defined(USE_LINUX_QUOTA) /* for Linux */
static int
rb_quotactl(int cmd, char *dev, VALUE vuid, caddr_t addr)
{
  int is_gid;
  uid_t uid;

  get_uid(vuid, &uid, &is_gid);
#ifdef DEBUG
  printf("cmd = %d, dev = %s, uid = %d, gid? = %d\n", cmd, dev, uid, is_gid);
#endif
  if( is_gid ){
    return quotactl(QCMD(cmd,GRPQUOTA),dev,(uid_t)uid,addr);
  }
  else{
    return quotactl(QCMD(cmd,USRQUOTA),dev,(uid_t)uid,addr);
  };
};
#elif defined(USE_BSD_QUOTA) /* for *BSD */
static int
rb_quotactl(int cmd, char *dev, VALUE vuid, caddr_t addr)
{
  char *path;
  int is_gid;
  uid_t uid;
#if defined(HAVE_SYS_STATVFS_H) && defined(__NetBSD__)
  struct statvfs *buff;
#else
  struct statfs *buff;
#endif
  int i, count, ret;
  
  buff = 0;
  path = dev;
  count = getmntinfo(&buff, MNT_WAIT);
  for( i=0; i<count; i++ ){
    if( strcmp(buff[i].f_mntfromname, dev) == 0 ){
      path = buff[i].f_mntonname;
      break;
    };
  };

  get_uid(vuid, &uid, &is_gid);
  if( is_gid ){
    ret = quotactl(path,QCMD(cmd,GRPQUOTA),uid,addr);
  }
  else{
    ret = quotactl(path,QCMD(cmd,USRQUOTA),uid,addr);
  };
  /* if( buff ) free(buff); */

  return ret;
};
#elif defined(USE_SOLARIS_QUOTA) /* for Solaris */
static int
rb_quotactl(int cmd, char *dev, VALUE vuid, caddr_t addr)
{
  struct quotctl qctl;
  int fd;
  uid_t uid;

  get_uid(vuid, &uid, 0);

  qctl.op = cmd;
  qctl.uid = uid;
  qctl.addr = addr;

  switch( cmd ){
  case Q_QUOTAON:
  case Q_QUOTAOFF:
  case Q_SETQUOTA:
  case Q_GETQUOTA:
  case Q_SETQLIM:
  case Q_SYNC:
    fd = open(dev,O_RDWR);
    break;
  case Q_ALLSYNC:
    if( dev ){
      fd = open(dev,O_RDWR);
    }
    else{
      fd = open("/",O_RDWR); /* maybe is it ignored anyways? */
    };
    break;
  default:
    return -1;
  }
  if( fd < 0 ){
    return -1;
  };
  if( ioctl(fd,Q_QUOTACTL,&qctl) == -1 ){
    close(fd);
    return -1;
  };
  close(fd);

  return 0; /* success */
};
#endif

static void
rb_diskquota_get(VALUE dqb, struct dqblk * c_dqb)
{
  VALUE v;

#define GetMember(mem) \
        ((v = rb_struct_getmember(dqb,rb_intern(mem))) == Qnil) ? 0 : (NUM2UINT(v))
#if defined(USE_LINUX_QUOTA)
  c_dqb->dqb_bhardlimit = GetMember("bhardlimit");
  c_dqb->dqb_bsoftlimit = GetMember("bsoftlimit");
#if defined(HAVE_LINUX_QUOTA_H)
  c_dqb->dqb_curspace   = GetMember("curspace");
#else
  c_dqb->dqb_curblocks  = GetMember("curblocks");
#endif
  c_dqb->dqb_ihardlimit = GetMember("ihardlimit");
  c_dqb->dqb_isoftlimit = GetMember("isoftlimit");
  c_dqb->dqb_curinodes  = GetMember("curinodes");
  c_dqb->dqb_btime      = GetMember("btimelimit");
  c_dqb->dqb_itime      = GetMember("itimelimit");
#elif defined(USE_BSD_QUOTA)
  c_dqb->dqb_bhardlimit = GetMember("bhardlimit");
  c_dqb->dqb_bsoftlimit = GetMember("bsoftlimit");
#if defined(USE_MACOSX_QUOTA)
  c_dqb->dqb_curbytes  = GetMember("curbytes");
#else
  c_dqb->dqb_curblocks  = GetMember("curblocks");
#endif
  c_dqb->dqb_ihardlimit = GetMember("ihardlimit");
  c_dqb->dqb_isoftlimit = GetMember("isoftlimit");
  c_dqb->dqb_curinodes  = GetMember("curinodes");
  c_dqb->dqb_btime      = GetMember("btimelimit");
  c_dqb->dqb_itime      = GetMember("itimelimit");
#elif defined(USE_SOLARIS_QUOTA)
  c_dqb->dqb_bhardlimit = GetMember("bhardlimit");
  c_dqb->dqb_bsoftlimit = GetMember("bsoftlimit");
  c_dqb->dqb_curblocks  = GetMember("curblocks");
  c_dqb->dqb_fhardlimit = GetMember("ihardlimit");
  c_dqb->dqb_fsoftlimit = GetMember("isoftlimit");
  c_dqb->dqb_curfiles   = GetMember("curfiles");
  c_dqb->dqb_btimelimit = GetMember("btimelimit");
  c_dqb->dqb_ftimelimit = GetMember("itimelimit");
#endif
#undef GetMember
};

static VALUE
rb_diskquota_new(struct dqblk *c_dqb)
{
  VALUE dqb;

#if defined(USE_LINUX_QUOTA)
  dqb = rb_struct_new(rb_sDiskQuota,
		      UINT2NUM(c_dqb->dqb_bhardlimit),
		      UINT2NUM(c_dqb->dqb_bsoftlimit),
#if defined(HAVE_LINUX_QUOTA_H)
		      UINT2NUM(c_dqb->dqb_curspace),
#else
		      UINT2NUM(c_dqb->dqb_curblocks),
#endif
		      UINT2NUM(c_dqb->dqb_ihardlimit),
		      UINT2NUM(c_dqb->dqb_isoftlimit),
		      UINT2NUM(c_dqb->dqb_curinodes),
		      UINT2NUM(c_dqb->dqb_btime),
		      UINT2NUM(c_dqb->dqb_itime),
		      0);
#elif defined(USE_BSD_QUOTA)
  dqb = rb_struct_new(rb_sDiskQuota,
		      UINT2NUM(c_dqb->dqb_bhardlimit),
		      UINT2NUM(c_dqb->dqb_bsoftlimit),
#if defined(USE_MACOSX_QUOTA)
		      UINT2NUM(c_dqb->dqb_curbytes),
#else
		      UINT2NUM(c_dqb->dqb_curblocks),
#endif
		      UINT2NUM(c_dqb->dqb_ihardlimit),
		      UINT2NUM(c_dqb->dqb_isoftlimit),
		      UINT2NUM(c_dqb->dqb_curinodes),
		      UINT2NUM(c_dqb->dqb_btime),
		      UINT2NUM(c_dqb->dqb_itime),
		      0);
#elif defined(USE_SOLARIS)
  dqb = rb_struct_new(rb_sDiskQuota,
		      UINT2NUM(c_dqb->dqb_bhardlimit),
		      UINT2NUM(c_dqb->dqb_bsoftlimit),
		      UINT2NUM(c_dqb->dqb_curblocks),
		      UINT2NUM(c_dqb->dqb_fhardlimit),
		      UINT2NUM(c_dqb->dqb_fsoftlimit),
		      UINT2NUM(c_dqb->dqb_curfiles),
		      UINT2NUM(c_dqb->dqb_btimelimit),
		      UINT2NUM(c_dqb->dqb_ftimelimit),
		      0);
#endif
  return dqb;
};

static VALUE
rb_quota_getquota(VALUE self, VALUE dev, VALUE uid)
{
  char *c_dev = STR2CSTR(dev);
  struct dqblk c_dqb;
  VALUE dqb = Qnil;

  if( rb_quotactl(Q_GETQUOTA,c_dev,uid,(caddr_t)(&c_dqb)) == -1 ){
    rb_sys_fail("quotactl");
  };

  dqb = rb_diskquota_new(&c_dqb);

  return dqb;
};

static VALUE
rb_quota_quotaoff(VALUE self, VALUE dev)
{
  char *c_dev = STR2CSTR(dev);

  if( rb_quotactl(Q_QUOTAOFF,c_dev,Qnil,NULL) == -1 ){
    rb_sys_fail("quotactl");
  };

  return Qnil;
};

static VALUE
rb_quota_quotaon(VALUE self, VALUE dev, VALUE quotas)
{
  char *c_dev = STR2CSTR(dev);
  char *c_quotas = STR2CSTR(quotas);

  if( rb_quotactl(Q_QUOTAON,c_dev,Qnil,(caddr_t)c_quotas) == -1 ){
    rb_sys_fail("quotactl");
  };

  return Qnil;
};

static VALUE
rb_quota_setquota(VALUE self, VALUE dev, VALUE uid, VALUE dqb)
{
  char *c_dev = STR2CSTR(dev);
  struct dqblk c_dqb;

  rb_diskquota_get(dqb, &c_dqb);

  if( rb_quotactl(Q_SETQUOTA,c_dev,uid,(caddr_t)(&c_dqb)) == -1 ){
    rb_sys_fail("quotactl");
  };

  return Qnil;
};

static VALUE
rb_quota_setqlim(VALUE self, VALUE dev, VALUE uid, VALUE dqb)
{
#ifdef Q_SETQLIM
  char *c_dev = STR2CSTR(dev);
  struct dqblk c_dqb;

  rb_diskquota_get(dqb, &c_dqb);

  if( rb_quotactl(Q_SETQLIM,c_dev,uid,(caddr_t)(&c_dqb)) == -1 ){
    rb_sys_fail("quotactl");
  };

#ifdef DEBUG
  printf("bhardlimit = %d\n",c_dqb.dqb_bhardlimit);
#endif
#else
  rb_raise(rb_eQuotaError, "the system don't have Q_SETQLIM");
#endif
  return Qnil;
};

static VALUE
rb_quota_sync(VALUE self, VALUE dev)
{
  char *c_dev;

  if( dev == Qnil ){
    c_dev = NULL;
  }
  else{
    c_dev = STR2CSTR(dev);
  };

  if( rb_quotactl(Q_SYNC,c_dev,Qnil,NULL) == -1 ){ /* uid and addr are ignored */
    rb_sys_fail("quotactl");
  };

  return Qnil;
};

void
Init_quota()
{
  rb_mQuota = rb_define_module("Quota");
  rb_define_const(rb_mQuota, "VERSION", rb_tainted_str_new2(RUBY_QUOTA_VERSION));
  rb_eQuotaError = rb_define_class_under(rb_mQuota,
					 "QuotaError",rb_eRuntimeError);
  rb_eQuotaCtlError = rb_define_class_under(rb_mQuota,
					    "QuotaCtlError",rb_eQuotaError);

  rb_cUID_ = rb_define_class_under(rb_mQuota, "UID_", rb_cObject);
  rb_define_singleton_method(rb_cUID_, "new", rb_quota_uid_s_new, -1);
  rb_define_method(rb_cUID_, "initialize", rb_quota_uid_initialize, -1);
  rb_define_method(rb_cUID_, "to_i", rb_quota_uid_to_i, 0);
  rb_alias(CLASS_OF(rb_cUID_), rb_intern("[]"), rb_intern("new"));
  rb_alias(CLASS_OF(rb_cUID_), '|', rb_intern("new"));
  rb_alias(CLASS_OF(rb_cUID_), '+', rb_intern("new"));

  rb_cUserID  = rb_define_class_under(rb_mQuota, "UserID", rb_cUID_);
  rb_define_singleton_method(rb_cUserID, "new", rb_quota_uid_s_new, -1);

  rb_cGroupID = rb_define_class_under(rb_mQuota, "GroupID", rb_cUID_);
  rb_define_singleton_method(rb_cUserID, "new", rb_quota_uid_s_new, -1);


  rb_sDiskQuota = rb_struct_define("DiskQuota",
				   "bhardlimit",
				   "bsoftlimit",
				   "curblocks",
				   "ihardlimit",
				   "isoftlimit",
				   "curinodes",
				   "btimelimit",
				   "itimelimit",
				   NULL);

  /* for compatibility */
#define DQ_ALIAS(a,b) rb_alias(rb_sDiskQuota,rb_intern(#a),rb_intern(#b))
  DQ_ALIAS(fhardlimit, ihardlimit);
  DQ_ALIAS(fsoftlimit, isoftlimit);
  DQ_ALIAS(curfiles,   curinodes);
  DQ_ALIAS(ftimelimit, itimelimit);
  DQ_ALIAS(fhardlimit=, ihardlimit=);
  DQ_ALIAS(fsoftlimit=, isoftlimit=);
  DQ_ALIAS(curfiles=,   curinodes=);
  DQ_ALIAS(ftimelimit=, itimelimit=);
#if defined(HAVE_LINUX_QUOTA_H)
  DQ_ALIAS(curspace, curblocks);
  DQ_ALIAS(curspace=, curblocks=);
#endif
#if defined(USE_MACOSX_QUOTA)
  DQ_ALIAS(curbytes, curblocks);
  DQ_ALIAS(curbytes=, curblocks=);
#endif
#undef DQ_ALIAS

  rb_define_const(rb_mQuota, "DiskQuota", rb_sDiskQuota);

  rb_define_module_function(rb_mQuota,"quotaon",rb_quota_quotaon,2);
  rb_define_module_function(rb_mQuota,"quotaoff",rb_quota_quotaoff,1);
  rb_define_module_function(rb_mQuota,"getquota",rb_quota_getquota,2);
  rb_define_module_function(rb_mQuota,"setquota",rb_quota_setquota,3);
  rb_define_module_function(rb_mQuota,"setqlim",rb_quota_setqlim,3);
  rb_define_module_function(rb_mQuota,"sync",rb_quota_sync,1);
};
