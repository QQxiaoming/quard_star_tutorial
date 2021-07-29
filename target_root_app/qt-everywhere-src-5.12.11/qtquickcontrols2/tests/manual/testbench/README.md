# Style Testbench

The Style Testbench provides a way to quickly see how controls look in different states with a given style.

When run with the Imagine style, the testbench will react to changes to image assets and automatically fix and reload them.
To use this functionality, click on the settings icon and then the settings menu item.
From there, check "Use Custom Imagine Style Assets" and provide a path to your custom assets in the text field.
If you want your 9-patch assets to be automatically fixed up (e.g. turn 4 pixel-thick 9-patch lines exported at
@4x to a 1 pixel-thick line), check "Fix Imagine Style Assets".

## Custom Text colors

To use custom text colors, enable the "Use Custom Palette" option in the settings dialog, and then provide colors in the relevant fields. As an example, the WindowText palette role affects text that is displayed directly against the window (labels, flat button text, etc.), ButtonText affects text that is displayed against a button's background, and so on. A full list of color roles can be found here:

http://doc.qt.io/qt-5/qpalette.html#ColorRole-enum

The custom text color settings apply to every style that supports palettes, until turned off.

The "Copy Palette Settings To Clipboard" button in the settings dialog can be used to copy the current colors and share them with others. Once someone has your custom colors, they can import them with the "Imort Palette Settings From Clipboard" button.

## Tips

- Ensure that 9-patch lines are black (#000000) or red (#ff0000). If these colors are slightly
  off (e.g. #010101), the assets won't display correctly when run in the application.
- Don't use pure black in assets that have black 9-patch lines, as this will confuse the asset fixer.

## Shortcuts

| Name              | Shortcut     | Shortcut (Mac) | Description                                                                                                                                      |
|-------------------|--------------|----------------|--------------------------------------------------------------------------------------------------------------------------------------------------|
| Use Custom Assets | Ctrl+Shift+C | ⌘+Shift+C      | If using the Imagine style, toggles between the default assets and the user's custom assets. Custom assets are specified in the settings dialog. |
| Fix Custom Assets | Ctrl+Shift+X | ⌘+Shift+X      | If using the Imagine style, manually fixes the assets. This is done automatically when "Automatically Fix Custom Assets" checkbox is checked. |
| Reload Assets     | Ctrl+R       | ⌘+R            | If using the Imagine style, reloads the assets from disk. This is usually done automatically.                                                    |
| Search/Filter     | Ctrl+F       | ⌘+F            | Filter the controls that are shown. Case-sensitive.                                                                                              |
| Quit              | Ctrl+Q       | ⌘+Q            | Quit the application.                                                                                                                            |
