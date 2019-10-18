#
# Do NOT Edit the Auto-generated Part!
# Generated by: spectacle version 0.27
#

Name:       situations-sonar

# >> macros
# << macros

Summary:    Companion Daemon for Situations
Version:    0.0.3
Release:    3
Group:      Qt/Qt
License:    Copyright (C) Pastilli Labs - All Rights Reserved
URL:        http://www.pastillilabs.com
Source0:    %{name}-%{version}.tar.bz2
Source100:  situations-sonar.yaml
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(libmkcal-qt5)
BuildRequires:  pkgconfig(libkcalcoren-qt5)
BuildRequires:  pkgconfig(libical)

%description
Companion Daemon for Situations


%prep
%setup -q -n %{name}-%{version}

# >> setup
# << setup

%build
# >> build pre
# << build pre

%qmake5

make %{?_smp_mflags}

# >> build post
# << build post

%install
rm -rf %{buildroot}
# >> install pre
# << install pre
%qmake5_install

# >> install post
# << install post

%preun
# >> preun
systemctl disable situations-sonar
systemctl stop situations-sonar
# << preun

%post
# >> post
systemctl enable situations-sonar
systemctl restart situations-sonar
systemctl daemon-reload
# << post

%postun
# >> postun
systemctl daemon-reload
# << postun

%files
%defattr(-,root,root,-)
%{_bindir}/%{name}
%{_libdir}/systemd/user/harbour-situations2application.service
%{_sysconfdir}/systemd/system/situations-sonar.service
# >> files
# << files
