-- This code was adapted to serve the application needs.

on run argv
  set image_name to item 1 of argv

  tell application "Finder"
    tell disk image_name

    -- Wait for the image to finish mounting.
      set open_attempts to 0
      repeat while open_attempts < 4
        try
          open
          delay 1
          set open_attempts to 5
          close
        on error errStr number errorNumber
          set open_attempts to open_attempts + 1
          delay 10
        end try
      end repeat
      delay 5

      -- Open the image and save a .DS_Store with
      -- background and icon setup.
      open
      set current view of container window to icon view
      set theViewOptions to the icon view options of container window
      set background picture of theViewOptions to file ".background:background.tiff"
      set arrangement of theViewOptions to not arranged
      set icon size of theViewOptions to 128
      delay 5
      close

      -- Setup the position of the app and Applications symlink
      -- and hide all the window decoration.
      open
      tell container window
        set sidebar width to 0
        set statusbar visible to false
        set toolbar visible to false
        -- Those bounds are defined as:
        -- x-start, y-start, x-end, y-end (aka. x, z, width, height)
        set the bounds to {400, 100, 1100, 528}
        set position of item "FreeSynd.app" to {140, 200}
        set position of item "Applications" to {580, 200}
      end tell
      delay 5
      close

      -- Open and close for visual verification.
      open
      delay 5
      close

    end tell
    delay 1
  end tell
end run