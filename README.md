# ext-zones for KWin

Here you can find an implementation of the ext-zones protocol for KWin.

The protocol itself is still in discussion, this discussion can be found here:
https://gitlab.freedesktop.org/wayland/wayland-protocols/-/merge_requests/264

You can find:
- src/ a kwin plugin that will bring in the feature
- src/client that brings a QML plugin to implement it into clients
- tests/main.qml a test that uses it to make sure everything is in place.

