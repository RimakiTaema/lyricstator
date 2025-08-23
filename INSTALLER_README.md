# Qt Installer Framework Setup

This document explains how to set up and use Qt Installer Framework for creating professional installers for Lyricstator.

## What is Qt Installer Framework?

Qt Installer Framework is Qt's official tool for creating cross-platform installers. It provides:

- **Professional appearance** - Modern wizard-style installation
- **Cross-platform support** - Windows, macOS, and Linux
- **Customizable branding** - Your application's look and feel
- **Dependency management** - Automatic Qt6 runtime handling
- **Update support** - Built-in update mechanisms
- **Clean uninstallation** - Proper cleanup when removing

## Installation

### Option 1: Qt Online Installer (Recommended)

1. Download [Qt Online Installer](https://www.qt.io/download)
2. Run the installer
3. During installation, select "Qt Installer Framework" component
4. Complete the installation

### Option 2: Package Manager

**Ubuntu/Debian:**
```bash
sudo apt install qt6-tools-dev qt6-tools-dev-tools
```

**Fedora:**
```bash
sudo dnf install qt6-qttools-devel
```

**Arch Linux:**
```bash
sudo pacman -S qt6-tools
```

**macOS:**
```bash
brew install qt6-tools
```

## Usage

### 1. Build the Application

First, build the Qt6 application:

```bash
# Build and create installer project
./build_qt_desktop.sh Release installer
```

This creates an `installer/` directory in your build folder.

### 2. Build the Installer

Navigate to the installer directory and build:

```bash
cd build_qt_desktop/installer
./build_installer.sh
```

### 3. Customize the Installer

The installer project structure:

```
installer/
├── config/
│   └── config.xml              # Main installer configuration
├── packages/
│   └── com.lyricstator.core/   # Main application package
│       ├── data/               # Application files
│       └── meta/               # Package metadata
│           ├── package.xml     # Package information
│           ├── installscript.qs # Installation logic
│           └── installer.ui    # UI configuration
├── lyricstator_installer.qbs   # Build configuration
└── build_installer.sh          # Build script
```

## Configuration Files

### config.xml

Main installer configuration:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<Installer>
    <Name>Lyricstator</Name>
    <Version>1.0.0</Version>
    <Title>Lyricstator Installer</Title>
    <Publisher>Lyricstator Team</Publisher>
    <ProductUrl>https://github.com/lyricstator/lyricstator</ProductUrl>
    <Icon>icon.ico</Icon>
    <Banner>installer_banner.png</Banner>
    <Background>installer_background.png</Background>
    <WizardStyle>Modern</WizardStyle>
    <StartMenuDir>Lyricstator</StartMenuDir>
    <TargetDir>@ApplicationsDir@/Lyricstator</TargetDir>
</Installer>
```

### package.xml

Package information:

```xml
<?xml version="1.0"?>
<Package>
    <DisplayName>Lyricstator Core</DisplayName>
    <Description>Lyricstator karaoke application core files</Description>
    <Version>1.0.0</Version>
    <ReleaseDate>2024-01-01</ReleaseDate>
    <Default>true</Default>
    <Script>installscript.qs</Script>
</Package>
```

### installscript.qs

Installation logic (JavaScript):

```javascript
function Component()
{
    // Default constructor
}

Component.prototype.createOperations = function()
{
    // Call the base createOperations function
    component.createOperations();
    
    // Create desktop shortcut on Linux
    if (installer.value("os") === "x11") {
        component.addOperation("CreateShortcut", 
            "@TargetDir@/bin/Lyricstator_Qt6",
            "@HomeDir@/Desktop/Lyricstator.desktop",
            "workingDirectory=@TargetDir@/bin",
            "icon=@TargetDir@/share/icons/hicolor/256x256/apps/lyricstator.png",
            "description=Lyricstator Karaoke Application"
        );
    }
}
```

## Customization Options

### Branding

1. **Icons**: Replace `icon.ico` with your application icon
2. **Banner**: Replace `installer_banner.png` with your banner image
3. **Background**: Replace `installer_background.png` with your background
4. **Colors**: Modify the installer theme in `installer.ui`

### Installation Options

1. **Components**: Add multiple packages for different features
2. **Dependencies**: Specify required system packages
3. **Shortcuts**: Customize desktop and start menu shortcuts
4. **Registry**: Add Windows registry entries
5. **Services**: Install system services (Windows/Linux)

### Advanced Features

1. **Updates**: Configure automatic update repositories
2. **Licensing**: Add license agreements
3. **Localization**: Support multiple languages
4. **Silent Installation**: Command-line installation support
5. **Rollback**: Undo failed installations

## Troubleshooting

### Common Issues

**Installer build fails:**
```bash
# Check if qbs is available
qbs --version

# Install qbs if needed
sudo apt install qbs
```

**Missing Qt Installer Framework:**
```bash
# Check installation
ls /usr/lib/qt6/bin/binarycreator

# Reinstall if missing
sudo apt install --reinstall qt6-tools-dev
```

**Permission errors:**
```bash
# Make scripts executable
chmod +x build_installer.sh
chmod +x create_installer.sh.in
```

### Debug Mode

Enable verbose output:

```bash
# Build with debug information
qbs build --file lyricstator_installer.qbs --log-level debug

# Or use Qt Installer Framework directly
binarycreator --verbose --online-only -c config/config.xml -p packages/ installer.exe
```

## Best Practices

1. **Test thoroughly** - Test on all target platforms
2. **Version management** - Use semantic versioning
3. **Dependencies** - Document all requirements
4. **Updates** - Plan for future versions
5. **Documentation** - Include user guides
6. **Signing** - Code sign your installers (Windows/macOS)

## Resources

- [Qt Installer Framework Documentation](https://doc.qt.io/qtinstallerframework/)
- [Qt Installer Framework Examples](https://github.com/qt/qtinstallerframework)
- [QBS Build System](https://doc.qt.io/qbs/)
- [Qt Online Installer](https://www.qt.io/download)

## Support

For issues with Qt Installer Framework:

1. Check the [Qt Documentation](https://doc.qt.io/)
2. Search [Qt Forums](https://forum.qt.io/)
3. Report bugs to [Qt Bug Tracker](https://bugreports.qt.io/)
4. Check [Stack Overflow](https://stackoverflow.com/questions/tagged/qt-installer-framework)