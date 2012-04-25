Gem::Specification.new do |s|
  s.name = %q{ruby-quota}
  s.version = "0.7.0"
  s.date = %q{2012-04-25}
  s.authors = ["Takaaki Tateishi, Alex Beregszaszi"]
  s.email = %q{ttate@jaist.ac.jp}
  s.summary = %q{Ruby-quota is a Ruby extension providing access to filesystem quota.}
  s.homepage = %q{http://ruby-quota.sf.net/}
  s.description = %q{Ruby-quota is a Ruby extension providing access to filesystem quota. Supported systems Linux, FreeBSD, NetBSD, Dragonfly BSD, Solaris and Mac OS X.}
  s.extensions = [ "extconf.rb" ]
  s.files = [ "COPYING", "README", "MANIFEST", "ChangeLog", "test.rb", "extconf.rb", "quota.c"]
end
