#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <FS.h>

/*---------------------------------------------------------------
 * SD card SPI configuration
 * These pins connect the ESP32 SPI bus to the card socket.
 *--------------------------------------------------------------*/
#define SD_MOSI 23
#define SD_MISO 19
#define SD_SCK 18
#define SD_CS 5

/**
 * @brief Start the serial monitor, SPI bus, and SD card.
 *
 * Arduino calls this function once after startup or reset. The return code
 * from SD_init() selects the status message shown to the learner.
 *
 * @param None.
 * @return Nothing.
 */
void setup() {
  Serial.begin(9600);
  SPI.begin(SD_SCK, SD_MISO, SD_MOSI);
  delay(100);
  if (SD_init() == 1)
  {
    Serial.println("Card Mount Failed");
  }
  else
    Serial.println("initialize SD Card successfully");
}

/**
 * @brief Leave the sketch idle after the one-time card inspection.
 *
 * Arduino calls this function repeatedly after setup() finishes.
 *
 * @param None.
 * @return Nothing.
 */
void loop() {
}

/**
 * @brief Mount the SD card and print its capacity and file list.
 *
 * setup() calls this function once after the SPI bus is ready. A nonzero
 * result tells setup() that mounting or card detection failed.
 *
 * @param None.
 * @return 0 when the card is ready, or 1 when initialization fails.
 */
int SD_init()
{
  if (!SD.begin(SD_CS))
  {
    Serial.println("Card Mount Failed");
    return 1;
  }
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE)
  {
    Serial.println("No TF card attached");
    return 1;
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("TF Card Size: %lluMB\n", cardSize);
  listDir(SD, "/", 2);

  // These optional operations are retained as reference exercises.
  //  listDir(SD, "/", 0);
  //  createDir(SD, "/mydir");
  //  listDir(SD, "/", 0);
  //  removeDir(SD, "/mydir");
  //  listDir(SD, "/", 2);
  //  writeFile(SD, "/hello.txt", "Hello ");
  //  appendFile(SD, "/hello.txt", "World!\n");
  //  readFile(SD, "/hello.txt");
  //  Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
  //  Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
  //  Serial.println("SD init over.");

  return 0;
}

/**
 * @brief Print files from a directory and optionally visit subdirectories.
 *
 * SD_init() calls this function after mounting the card. Recursive calls
 * reduce levels so traversal cannot continue beyond the requested depth.
 *
 * @param fs Mounted file system to inspect.
 * @param dirname Directory path to open.
 * @param levels Maximum number of subdirectory levels to visit.
 * @return Nothing.
 */
void listDir(fs::FS & fs, const char *dirname, uint8_t levels)
{
  //  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root)
  {
    //Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory())
  {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  //  i = 0;
  while (file)
  {
    if (file.isDirectory())
    {
      //      Serial.print("  DIR : ");
      //      Serial.println(file.name());
      if (levels)
      {
        listDir(fs, file.name(), levels - 1);
      }
    }
    else
    {
      Serial.print("FILE: ");
      Serial.print(file.name());
      //      lcd.setCursor(0, 2 * i);
      //      lcd.printf("FILE:%s", file.name());

      Serial.print("SIZE: ");
      Serial.println(file.size());
      //      lcd.setCursor(180, 2 * i);
      //      lcd.printf("SIZE:%d", file.size());
      //      i += 16;
    }

    file = root.openNextFile();
  }
}
