%define _prefix /export/home/tinderbox2
%define _cgi_prefix /home/httpd/cgi-bin

Summary: Development Monitoring Tool
Name: tinderbox2
Version: 20010214
Release: 1
Copyright: MPL
Group: Development/Tools
Source: cvs://:pserver:anonymous@cvs-mirror.mozilla.org:/cvsroot:mozilla/webtools/tinderbox2/tinderbox2.tar.gz
Prereq: apache
Prefix: %{_prefix}
Buildroot: /var/tmp/%{name}-root

%description

Tinderbox is the first program to allow developers and management to
see at a glance what is currently going on in all aspects of the
development process. The Tinderbox server prepares HTML pages which
display the history of many different development variables. It shows
at a glance the history of: whether the HEAD branch of the source code
builds and passes all automated tests, who has checked code changes
into the version control system, whether the source tree is open or
closed and when the state of the tree last changed, what trouble
tickets have been closed, notices and messages posted by the
developers or project manager.


%prep
%setup -q -n tinderbox2


%build

# These executables are not used, and are causing problems for the
# dependency engines.

rm -f ./src/bin/bustagestats.cgi ./src/bin/gifsize

prefix='%{_prefix}' cgibin_prefix='%{_cgi_prefix}/%{name}' \
	./configure

make

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/%{_prefix}/%{name}
mkdir -p $RPM_BUILD_ROOT/%{_cgi_prefix}/%{name}

make 	prefix=$RPM_BUILD_ROOT/%{_prefix} \
	cgibin_prefix=$RPM_BUILD_ROOT/%{_cgi_prefix}/%{name} \
	install


%clean
#rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,tinderbox,root)
%{_prefix}
%{_cgi_prefix}/%{name}

