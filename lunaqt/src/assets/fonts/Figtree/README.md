Figtree font placeholder.

Manual step required: Download Figtree (OFL licensed) from Google Fonts and place the ZIP at:
  lunaqt/src/assets/fonts/temp-font-downloads/Figtree.zip

Then run:
  python lunaqt/tools/install_font_from_zip.py "Figtree" "lunaqt/src/assets/fonts/temp-font-downloads/Figtree.zip"

This extracts TTFs and OFL license into assets/fonts/Figtree. On next launch the font_manager will load them and "Figtree" will appear in the UI/Text/Code font selectors.