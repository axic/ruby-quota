Gem::Specification.new do |s|
  s.name = %q{ruby-quota}
  s.version = "0.6.0"
  s.date = %q{2009-11-24}
  s.authors = ["Takaaki Tateishi"]
  s.email = %q{ttate@jaist.ac.jp}
  s.summary = %q{Ruby-quota is a Ruby extension providing access to filesystem quota under Linux, FreeBSD, NetBSD and Solaris.}
  s.homepage = %q{http://ruby-quota.sf.net/}
  s.description = %q{Ruby-quota is a Ruby extension providing access to filesystem quota under Linux, FreeBSD, NetBSD and Solaris.}
  s.extensions = [ "extconf.rb" ]
  s.files = [ "COPYING", "README", "MANIFEST", "ChangeLog", "test.rb", "extconf.rb", "quota.c"]
end
