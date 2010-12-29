Name:           brasd
Version:        0.1.1
Release:        1%{?dist}
Summary:        Southeast University BRAS Client

Group:          System/Daemons
License:        Public Domain
URL:            http://launchpad.net/brasd
Source0:        http://launchpad.net/brasd/0.1/0.1.1/+download/brasd-0.1.1.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:  autoconf, automake, gtk2-devel, libevent-devel, xl2tpd
Requires:       xl2tpd, libevent, gtkmm24

%description
This is an unofficial client of Southeast University BRAS service, which helps Linux users quickly use BRAS.

%prep
%setup -q


%build
%configure
make %{?_smp_mflags}


%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT


%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root,-)
%doc



%changelog
* Tue Dec 28 2010 Zhao Cheng <zcbenz@gmail.com> 0.1.1-1
- Will show a no_response dialog when brasd didn't response
- brasd only accpets connections from local programes
- Change configuration files's permission to 0600
- Debian init script's output is more standard

