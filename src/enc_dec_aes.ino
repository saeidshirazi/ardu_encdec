#include <LiquidCrystal.h>
#include <AESLib.h>
#include <SPI.h>
#include <SD.h>

#define outputA 9
#define outputB 8

int counter = 0;//---------------------------------------------
int curs = 0;
int state; 
int laststate;

const int rs = 7, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2, cs = 10;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

uint8_t key[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};

File root;
File myFile;
String names [16];
int num;//----------------------------------------------------

int pinCS = 10;

void setup() 
{
  lcd.begin(16, 2);

  SD_init();
  
  lcd.clear();
  lcd.print("initialization ");
  lcd.setCursor(15,1);
  lcd.print("done!");
    
  pinMode (outputA,INPUT);
  pinMode (outputB,INPUT);
  pinMode (A0,INPUT_PULLUP);

  
  pinMode (A1,INPUT_PULLUP);
  pinMode (A2,INPUT_PULLUP);
  
  laststate = digitalRead(outputA);
  delay(1000);
  frame(counter);
}

void loop() 
{
  rotary(counter);
  if(digitalRead(A1) == 0)
    Right_Key();
  if(digitalRead(A2) == 0)
    Left_Key();
}

void frame(int &count)
{
  if(count > num - 1)
    count = 0;
  else if(count < 0)
    count = num -1;
  curs = 0;
  lcd.clear();
  lcd.setCursor(15,0);
  lcd.print(count+1);lcd.print("/");lcd.print(num);
  lcd.setCursor(0,0);
  myFile = SD.open(names[counter]);
  if(myFile) 
  {
  lcd.print(myFile.name());
  lcd.setCursor(1,1);
  lcd.print(" Enc    ");lcd.print("Dec    ");lcd.print("Del");
  lcd.setCursor(0,1);
  lcd.print("->");
  curs = 0;
  }
  else
  {
  lcd.print("can't open ");
  lcd.setCursor(0,1);
  lcd.print("the file");
  }
  myFile.close();
}

void Directory(File dir, String (& names) [16], int &num) 
{
  while (true) 
  {
    pinMode(pinCS, OUTPUT);
    File entry =  dir.openNextFile();
    if (! entry) {
      break;
    }  
    if (entry.isDirectory()) 
    {
      Directory(entry, names, num);
    } 
    else 
    {
      names[num] = entry.name();
    }
    entry.close();
    num = num + 1;
  }
}

void rotary(int &count)
{
  state = digitalRead(outputA);
  if(state != laststate)
  {
    if(digitalRead(outputB) != state)
    {
      count = count + 1;
      frame(count);
    }
    else
    {
      count = count - 1;
      frame(count);
    }
  }
  laststate = state;
  if(digitalRead(A0) == LOW)
  {
  myFile = SD.open(names[counter]);
  char filename[16] = {0};
  if(curs == 0)
  {
    F_NAME("E_"+names[counter],filename);
    Enc(myFile,filename);
  }
  else if(curs == 7)
  {
    F_NAME(names[counter],filename);
    filename[0] = 'D';
    Dec(myFile,filename);
  }
  else if(curs == 14)
  {
    Del(myFile);
  }
  //SD_init();
  counter = 0;
  while(digitalRead(A0) == LOW);
  }
}

void Enc(File DecFile, char filename[16])
{
  File EncFile;
  char data[16];
  float ts = DecFile.size();
  float s = 0;
  if (SD.exists(filename))
    SD.remove(filename);
  EncFile = SD.open(filename, FILE_WRITE);
  if(EncFile)
  {
    if(DecFile) 
    {
      lcd.clear();
      lcd.print("Encrypting:");
      lcd.setCursor(0,1);
      lcd.print(DecFile.name());
      while(DecFile.available()) 
      {
        DecFile.read(data,16);
        aes128_enc_single(key, data);
        EncFile.write(data,16);
        s +=16;
        lcd.setCursor(14,1);
        lcd.print((s/ts)*100);
        lcd.print("%");
      }
      DecFile.close();
      lcd.clear();
      lcd.setCursor(16,1);
      lcd.print("done");
    } 
    else 
    {
      lcd.clear();
      lcd.print("error opening" );
      lcd.setCursor(0,1);
      lcd.print(DecFile.name());
    }
    EncFile.close();
  }
  else 
  {
    lcd.clear();
    lcd.print("error opening ");
    lcd.setCursor(0,1);
    lcd.print(filename);
  }
}

void Dec(File EncFile, char filename[16])
{
  File DecFile;
  char data[16];
  float ts = EncFile.size();
  float s = 0;
  if (SD.exists(filename))
    SD.remove(filename);
  DecFile = SD.open(filename, FILE_WRITE);
  if(DecFile)
  {
    if(EncFile) 
    {
      lcd.clear();
      lcd.print("Decrypting:");
      lcd.setCursor(0,1);
      lcd.print(EncFile.name());
      while(EncFile.available())
      {
        EncFile.read(data,16);
        aes128_dec_single(key, data);
        DecFile.write(data,16);
        s +=16;
        lcd.setCursor(14,1);
        lcd.print((s/ts)*100);
        lcd.print("%");
      }
      EncFile.close();
      lcd.clear();
      lcd.setCursor(16,1);
      lcd.print("done");
    } 
    else 
    {
      lcd.clear();
      lcd.print("error opening ");
      lcd.setCursor(0,1);
      lcd.print(EncFile.name());
    }
    DecFile.close();
  }
  else 
  {
    lcd.clear();
    lcd.print("error opening ");
    lcd.setCursor(0,1);
    lcd.print(filename);
  }
}

void Del(File DelFile)
{
  if(DelFile) 
    {
      lcd.clear();
      lcd.print("Encrypting:");
      lcd.setCursor(0,1);
      lcd.print(DelFile.name());
      SD.remove(DelFile.name());
      if (SD.exists(DelFile.name())) 
      {
        lcd.clear();
        lcd.print("error deleting ");
        lcd.setCursor(0,1);
        lcd.print(DelFile.name());
      } 
      else 
      {
        lcd.clear();
        lcd.setCursor(16,1);
        lcd.print("done");
      }
    } 
    else 
    {
      lcd.clear();
      lcd.print("error opening" );
      lcd.setCursor(0,1);
      lcd.print(DelFile.name());
    }
}

int F_NAME(String NAME, char (&filename)[16])
{
  int i = NAME.indexOf('.');
  int j =0;
  int n = i;
  int l = NAME.length();
  int fl = l;
  if(fl > 10) fl = 10;
  char Name[32] = "";
  NAME.toCharArray(Name,32);
  while(n < l)
  {
    filename[fl-(l-i)+j] = Name[n];
    j++;
    n++;
  }
  j = 0;
  while(j<fl-(l-i))
  {
    filename[j] = Name[j];
    j++;
  }
  return l-i;
}

void SD_init()
{
    digitalWrite(pinCS, HIGH);
    delay(500);
    digitalWrite(pinCS, LOW);
    if (!SD.begin(10)) 
    {
    lcd.print("initialization");
    lcd.setCursor(13,1);
    lcd.print("failed!");
    while (1);
    }
    root = SD.open("/");
    num = 0;
    Directory(root, names, num);
}

void Right_Key()
{
  lcd.setCursor(curs,1);
  lcd.print("  ");
  curs = curs + 7;
  if(curs > 14)
    curs = 0;
  lcd.setCursor(curs,1);
  lcd.print("->");
  while(digitalRead(A1) == 0);
}

void Left_Key()
{
  lcd.setCursor(curs,1);
  lcd.print("  ");
  curs = curs - 7;
  if(curs < 0)
    curs = 14;
  lcd.setCursor(curs,1);
  lcd.print("->");
  while(digitalRead(A2) == 0);
}
