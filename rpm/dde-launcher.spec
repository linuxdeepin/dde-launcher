%global repo dde-launcher
%global sname deepin-launcher

%if 0%{?fedora}
Name:           %{sname}
%else
Name:           %{repo}
%endif
Version:        5.3.0.27
Release:        1%{?fedora:%dist}
Summary:        Deepin desktop-environment - Launcher module
License:        GPLv3
%if 0%{?fedora}
URL:            https://github.com/linuxdeepin/dde-launcher
Source0:        %{url}/archive/%{version}/%{repo}-%{version}.tar.gz
%else
URL:            http://shuttle.corp.deepin.com/cache/repos/eagle/release-candidate/RERFNS4wLjAuNjU3NQ/pool/main/d/dde-launcher/
Source0:        %{name}_%{version}.orig.tar.xz	
%endif

BuildRequires:  cmake
BuildRequires:  cmake(Qt5LinguistTools)
BuildRequires:  dtkwidget-devel
BuildRequires:  pkgconfig(dtkcore)
BuildRequires:  pkgconfig(dframeworkdbus)
BuildRequires:  pkgconfig(gsettings-qt)
BuildRequires:  pkgconfig(xcb-ewmh)
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  pkgconfig(Qt5Svg)
BuildRequires:  pkgconfig(Qt5X11Extras)
BuildRequires:  qt5-qtbase-private-devel
%{?_qt5:Requires: %{_qt5}%{?_isa} = %{_qt5_version}}
Requires:       deepin-menu
%if 0%{?fedora}
Requires:       deepin-daemon
%else
Requires:       dde-daemon
%endif
Requires:       startdde
Requires:       hicolor-icon-theme

%description
%{summary}.

%package devel
Summary:        Development package for %{sname}
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description devel
Header files and libraries for %{sname}.

%prep
%autosetup -p1 -n %{repo}-%{version}

%build
sed -i 's|lrelease|lrelease-qt5|' translate_generation.sh
%if 0%{?fedora}
%cmake -DCMAKE_INSTALL_PREFIX=%{_prefix} -DWITHOUT_UNINSTALL_APP=1
%cmake_build
%else
%cmake -DCMAKE_INSTALL_PREFIX=%{_prefix} -DWITHOUT_UNINSTALL_APP=1 .
%make_build
%endif

%install
%if 0%{?fedora}
%cmake_install
%else
%make_install INSTALL_ROOT=%{buildroot}
%endif

%files
%license LICENSE
%{_bindir}/%{repo}
%{_datadir}/%{repo}/
%{_datadir}/dbus-1/services/*.service
%{_datadir}/icons/hicolor/scalable/apps/%{sname}.svg

%files devel
%{_includedir}/%{repo}/

%changelog
* Wed Jun 10 2020 uoser <uoser@uniontech.com> - 5.1.0.6
- Update to 5.1.0.6
