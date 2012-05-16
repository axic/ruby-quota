Gem::Specification.new do |s|
  s.name = %q{ruby-quota}
  s.version = "0.8.0"
  s.date = %q{2012-05-16}
  s.authors = ["Takaaki Tateishi, Alex Beregszaszi"]
  s.email = %q{ttate@jaist.ac.jp alex@rtfs.hu}
  s.summary = %q{Ruby-quota is a Ruby extension providing access to filesystem quota.}
  s.homepage = %q{http://github.com/axic/ruby-quota}
  s.description = %q{Ruby-quota is a Ruby extension providing access to filesystem quota. Supported systems Linux, FreeBSD, NetBSD, OpenBSD, Dragonfly BSD, Solaris and Mac OS X.}
  s.extensions = [ "extconf.rb" ]
  s.files = [ "COPYING", "README.md", "MANIFEST", "ChangeLog", "test.rb", "extconf.rb", "quota.c"]
end
