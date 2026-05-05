

#mostly cooked up by some ai btw
#takes a logo as a .png (must be only black, white, and transparent)
#lets you name it and puts the entire header contents into your clipboard
#for displaying with the MAX7456 OSD chip

#you then need to load the custom characters into a free space in
#the chip's EEPROM. this only needs to be done once since it is kept
#across power cycles but there is no issue with doing it each startup.
#you would use OSD_WriteCustomCharacter() to load each one of the
#generated arrays into consecutive character IDs (ideally with IDs
#between 200 and 255). you would then use OSD_DrawLogo to draw it.
#the width and height of OSD_DrawLogo is measured in characters.


import sys
import os
try:
    import tkinter as tk
    from tkinter import filedialog, simpledialog
except ImportError:
    print("you gotta install tkinter (pip install tkinter)")
    os.system("pause")
    sys.exit(1)

try:
    from PIL import Image
except ImportError:
    print("you gotta install PIL (pip install pillow)")
    os.system("pause")
    sys.exit(1)




# Attempt to import pyperclip for clipboard support
try:
    import pyperclip
except ImportError:
    print("Error: 'pyperclip' library not found. Install it with: pip install pyperclip")
    os.system("pause")
    sys.exit(1)

def get_max7456_pixel(r, g, b, a=255):
    # Alpha channel check: if pixel is transparent, return MAX7456 transparent pattern
    if a < 128:
        return 0b01
        
    # Black: 0b00, White: 0b10, Everything else: Transparent (0b01)
    if r < 50 and g < 50 and b < 50:
        return 0b00
    elif r > 200 and g > 200 and b > 200:
        return 0b10
    else:
        return 0b01

def main():
    # Set up tkinter
    root = tk.Tk()
    root.withdraw() 
    root.attributes("-topmost", True)

    print("select the .png of your logo (black, white, transparent only). must be some multiple of 12(w)x18(h) pixels")
    # 1. Prompt for File
    file_path = filedialog.askopenfilename(
        title="Select Image (PNG/BMP)",
        filetypes=[("Image files", "*.png *.bmp"), ("All files", "*.*")]
    )
    if not file_path:
        return

    # 2. Prompt for Logo Name
    logo_name = simpledialog.askstring("Logo Name", "Enter a name for this logo")
    if not logo_name:
        logo_name = "custom_logo"
    
    # Sanitize name for C variables
    clean_name = logo_name.strip().replace(" ", "_").lower()
    guard_name = clean_name.upper()

    try:
        img = Image.open(file_path).convert("RGBA")
    except Exception as e:
        print(f"Error opening image: {e}")
        return

    width, height = img.size
    chars_wide = width // 12
    
    header_lines = [
        f"// Auto-generated MAX7456 {logo_name} Header ({chars_wide}x1)",
        f"#ifndef {guard_name}_H",
        f"#define {guard_name}_H\n",
        "#include <stdint.h>\n"
    ]

    for char_idx in range(chars_wide):
        # Replaces 'logo' with your custom name
        header_lines.append(f"const uint8_t {clean_name}_tile_{char_idx}[54] = {{")
        
        byte_array = []
        for y in range(18):
            for byte_col in range(3):
                byte_val = 0
                for p in range(4):
                    x = (char_idx * 12) + (byte_col * 4) + p
                    if x < width and y < height:
                        r, g, b, a = img.getpixel((x, y))
                    else:
                        r, g, b, a = (0, 0, 0, 0)
                        
                    pixel_bits = get_max7456_pixel(r, g, b, a)
                    shift = 6 - (p * 2) # MAX7456 packs 4 pixels per byte
                    byte_val |= (pixel_bits << shift)
                
                byte_array.append(f"0x{byte_val:02X}")
                
        # Format 54 bytes into the array
        for i in range(0, 54, 6):
            chunk = ", ".join(byte_array[i:i+6])
            suffix = "," if i < 48 else ""
            header_lines.append(f"    {chunk}{suffix}")
            
        header_lines.append("};\n")

    header_lines.append(f"#endif // {guard_name}_H")
    final_output = "\n".join(header_lines)

    try:
        pyperclip.copy(final_output)
        print(f"\nSuccessfully processed: {file_path}")
        print(f"Variable prefix: {clean_name}")
        print("The header has been copied to your clipboard!")
    except Exception as e:
        print(f"Clipboard Error: {e}\n\n{final_output}")

if __name__ == "__main__":
    main()
    os.system("pause")
