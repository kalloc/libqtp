Version: 1.4.0
Summary: Q-Lang template preprocessor (QTP)
Name: qtp
Release: 1
Copyright: GPL
Group: Development/Languages/QLang
Source: http://ainmarh.com/download/%{name}-%{version}.tar.gz
URL: http://ainmarh.com/download/
Packager: Denis Slyusarev
BuildRoot: /var/tmp/%{name}-buildroot/
Requires: fcgi >= 2.4.0

%description
Q-Lang template preprocessor.(QTP)
2.1 dialect test.

%prep
%setup

%build
./configure --prefix=/usr
make

%install
rm -rf $RPM_BUILD_ROOT
make prefix=$RPM_BUILD_ROOT/usr sysconfdir=$RPM_BUILD_ROOT/etc install
find $RPM_BUILD_ROOT/usr -type f -print | grep -v '\/(README|INSTALL)$' | \
    sed "s@^$RPM_BUILD_ROOT@@g" | sed "s/^\(.*\)$/\1\*/" > %{name}-%{version}-filelist

%clean
rm -rf $RPM_BUILD_ROOT

%files -f %{name}-%{version}-filelist
%defattr(-, root, root)

%doc README INSTALL ChangeLog

%changelog
