# -*- ruby -*-
# this script is intended to be run by root on the solaris.

require 'quota'
require 'etc'

# edit for your OS
case `uname -s`
when /^Linux/
  $DEV = "/dev/hda9"
  $QUOTAS = "/mnt/hda9/aquota.user"
when /^SunOS/
  $DEV = "/quotas"
  $QUOTAS = "/quotas"
when /BSD/
  $DEV = "/mnt/test"
  $QUOTAS = "/mnt/test/quota.user"
end

print("user id: ")
uid = gets.chop
if( uid =~ /\d+/ )
  $USER = Etc.getpwuid(uid).name
  $UID  = uid.to_i
else
  $USER = uid
  $UID  = Etc.getpwnam(uid).uid
end
print("uid = #{$USER}(#{$UID})\n")

begin
  Quota.quotaon($DEV, $QUOTAS)
rescue Errno::EBUSY
  Quota.quotaoff($DEV)
  Quota.quotaon($DEV, $QUOTAS)
end

begin
  dq = Quota.getquota($DEV, $UID)
rescue Errno::ESRCH
  dq = Quota::DiskQuota.new
end

print("quota = #{dq.inspect}\n")
print("softlimit: ")
softlimit = gets.to_i

dq.bsoftlimit = softlimit # 1block = 1024byte (SunOS 5.6, edquota(1M))
Quota.setquota($DEV, $UID, dq)

other = Quota.getquota($DEV, $UID)
print("quota = #{dq.inspect}\n")
