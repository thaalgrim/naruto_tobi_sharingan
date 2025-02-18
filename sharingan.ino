#include <LittleFS.h>
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI(); // Create an instance of the TFT_eSPI class

void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setRotation(1); // Adjust rotation if needed


  // Initialize LittleFS
  if (!LittleFS.begin()) {
    Serial.println("LittleFS Mount Failed");
    return;
  }
}

void loop() {
  // Display each frame in sequence
  for (int i = 0; i <= 3; i++) {
    char filename[20];
    sprintf(filename, "/frame_%02d.bmp", i);
    displayBMP(filename);
    delay(125); // Adjust delay for smoother animation
  }
}

void displayBMP(const char *filename) {
  File bmpFile = LittleFS.open(filename, "r");
  if (!bmpFile) {
    Serial.print("File not found: ");
    Serial.println(filename);
    return;
  }

  uint32_t bmpWidth, bmpHeight;
  uint8_t bmpDepth;
  uint32_t bmpImageoffset;
  uint32_t rowSize;
  uint8_t sdbuffer[3 * 240]; // Buffer to read file (3 bytes per pixel, 240 pixels per line)

  bmpFile.seek(10);
  bmpImageoffset = read32(bmpFile);

  bmpFile.seek(18);
  bmpWidth = read32(bmpFile);
  bmpHeight = read32(bmpFile);

  bmpFile.seek(28);
  bmpDepth = read16(bmpFile);

 
if (bmpDepth == 24) {
    rowSize = (bmpWidth * 3 + 3) & ~3; // Calculate row size (multiple of 4 bytes)
    uint8_t sdbuffer[rowSize]; // Buffer to read file

    for (uint32_t row = 0; row < bmpHeight; row++) {
        uint32_t pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
        bmpFile.seek(pos);
        bmpFile.read(sdbuffer, rowSize); // Read entire row

        // Convert RGB888 to RGB565 in-place
        for (uint32_t i = 0; i < rowSize; i += 3) {
            uint16_t color = tft.color565(sdbuffer[i + 2], sdbuffer[i + 1], sdbuffer[i]);
            sdbuffer[i / 3 * 2] = color >> 8;
            sdbuffer[i / 3 * 2 + 1] = color & 0xFF;
        }

        // Push the entire row to the display
        tft.pushImage(0, row, bmpWidth, 1, (uint16_t*)sdbuffer);
    }
}

  bmpFile.close();
}

uint16_t read16(File &f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(File &f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}