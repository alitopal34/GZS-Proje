//ESP8266-01 Modülünün internete bağlanması için ve bildirimleri göndereceği serverin api bilgileri.
#define ag_ismi "alitopal"   //Bağlanılacak olan wifi adı
#define ag_sifresi "313211333"  //Wifi sifresi
#define SERVER "api.pushetta.com"  //Server apisi.
String API("a80d273cc285a88a994812f366eea77f8eeef950"); //Server'in api keyi
String CHANNEL("gercekzamanli");  //Kanal ismi


#include <LiquidCrystal.h>  //LCD ekran kütüphanesi
#include <dht11.h>   //DHT11 Sıcaklık ve Nem Sensörü Kütüphanesi
#define DHT11PIN 36 // DHT11PIN olarak Dijital 36"yi belirliyoruz.
dht11 DHT11; //DHT11 nesnesi

LiquidCrystal lcd(A0, A1, A2, A3, A4, A5);  //LCD Ekran pin girişleri.

//Sıcaklık Ledleri tanımlandı.
int sicaklikLedGreen = 32;
int sicaklikLedRed = 30;

//Gaz sensoru için sensor data portunun tanımlanması
int analogGaz = A7;  //Gaz sensörünün sağlıklı ölçümler yapabilmesi için analog olarak ölçüm yapması gerekir.
double okunanGaz = 0;
int gazLedGreen = 40;
int gazLedRed = 38;

//Pır sensör için tanımlamalar
int pirPin = 42;
int ledPin = 44;
int deger = 0;

//LDR sensör için tanımlamalar
int ldrPin = A6;
int ldrledPin = 46;
int deger2 = 0;



void setup()
{
  Serial.begin(9600); //Seri port ekranının aktif edilmesi.
  lcd.begin(16, 2);  //LCD ekranın aktif edilmesi.
  lcd.setCursor(0,0);
  lcd.print("Sensorler");
  lcd.setCursor(0,1);
  lcd.print("Calistiriliyor..");
  
  pinMode(sicaklikLedGreen, OUTPUT);
  pinMode(sicaklikLedRed, OUTPUT);

  pinMode(gazLedGreen, OUTPUT);
  pinMode(gazLedRed, OUTPUT);

  pinMode(ldrledPin, OUTPUT);
  pinMode(ldrPin, INPUT);

  pinMode(pirPin, INPUT);
  pinMode(ledPin, OUTPUT);

  //Kartımız Mega olduğu için 3 Rx 3 Tx pini vardır. ilk rx ve tx üzerinden sensörlere ilişkin veri aktarımı yapıldığı için
  //ESP8266-01 modülü ile Rx1 Tx1 üzerinden haberleşme sağlanacaktır. Bu yüzden Serial1 şeklinde ikinci bir seri monitor ekranı aktif edilmiştir.
  Serial1.begin(9600);
  Serial1.println("AT");  //Wifi modülü bağlanmaya hazır hale getiriliyor.
  delay(5000);  //Firmware güncellemesi yapılmadıysa 5 sn beklemek gerekir. Bizde yapıldığı için beklemeye gerek yok.

  if (Serial1.find("OK")) {       //esp modülü ile bağlantıyı kurabilmişsek modül "AT" komutuna "OK" komutu ile geri dönüş yapıyor.
    Serial1.println("AT+CWMODE=1"); //esp modülümüzün WiFi modunu STA şekline getiriyoruz. Bu mod ile modülümüz başka ağlara bağlanabilecek.
    delay(2000);
    String baglantiKomutu = String("AT+CWJAP=\"") + ag_ismi + "\",\"" + ag_sifresi + "\"";
    Serial1.println(baglantiKomutu);

    delay(5000);
  }

}

void loop()
{
  ldrSensor();

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////SICAKLIK & NEM SENSÖRÜ/////////////////////////////////////////////////////////

  
  //Sensörün okunup okunmadığını konrol ediyoruz.
  // chk 0 ise sorunsuz okunuyor demektir. Sorun yaşarsanız
  // chk değerini serial monitörde yazdırıp kontrol edebilirsiniz.

  int chk = DHT11.read(DHT11PIN);

  //Eğer belirlenmiş olan sıcaklıkların üstüne çıkarsa ledi yakıyor, altında kalırsa led sönüyor.
  if ((float)DHT11.temperature >= 30 || (float)DHT11.humidity >= 60)
  {
    Serial.println("20");
    digitalWrite(sicaklikLedGreen, LOW);
    digitalWrite(sicaklikLedRed, HIGH);

ldrSensor();
    
    veriPushetta(CHANNEL, "SICAKLIK YUKSELDI !");  //Sıcaklığın ortalama üzerinde olduğunu gösteren bildirim.
  }
  else if ((float)DHT11.temperature < 30 || (float)DHT11.humidity < 60)
  {
    Serial.println("21");
    digitalWrite(sicaklikLedGreen, HIGH);
ldrSensor();
    digitalWrite(sicaklikLedRed, LOW);
  }

 ldrSensor();

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////GAZ SENSÖRÜ///////////////////////////////////////////////////////////////////////////////

  okunanGaz = analogRead(analogGaz);  //Sensörden gaz değeri okunuyor.
  
  if (okunanGaz >= 550) //Okunan gaz değerinin tehlike arz ettiği değerden fazla olması
  {
    Serial.println("30");
    digitalWrite(gazLedRed, HIGH);

ldrSensor();

    veriPushetta(CHANNEL, "GAZ YOGUNLUGU YUKSELDI !");  //Herhangi bir gaz kaçağına ilişkin bildirim.

    digitalWrite(gazLedGreen, LOW);
  }
  else  //Gaz durumu normalse
  {
    Serial.println("31");
    digitalWrite(gazLedGreen, HIGH);
ldrSensor();
    digitalWrite(gazLedRed, LOW);
  }

 ldrSensor();


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////LCD EKRAN//////////////////////////////////////////////////////////////

 //Sıcaklık ve Nem yazdırılıyor.
  lcd.setCursor(0,0);
  lcd.print("N:");
  lcd.setCursor(2,0);
  lcd.print((float)DHT11.humidity,0);
  lcd.setCursor(4,0);
  lcd.print("% ");
  lcd.setCursor(6,0);
  lcd.print("D:");
  lcd.setCursor(8,0);
  lcd.print((float)DHT11.temperature,0);
  lcd.setCursor(10,0);
  lcd.print((char)223); //Derece işaretini yapmak için.
  lcd.print("C ");

ldrSensor();

  //Gaz sensörü yazdırılıyor.
  lcd.setCursor(0,1);
  lcd.print("G:");
  lcd.setCursor(2,1);
  lcd.print(okunanGaz / 10000); //Gaz çözünürlüğü hesaplanıp ekrana basılıyor-ppm değer- 4 indis
  lcd.setCursor(6,1);
  lcd.print("% ");
  lcd.setCursor(8,1);
  lcd.print("PPM:");
  lcd.setCursor(12,1);
  lcd.print(okunanGaz,0); //3 indis
  lcd.print(" ");

}

void ldrSensor()
{
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////LDR SENSOR/////////////////////////////////////////////////////////////

  deger2 = analogRead(ldrPin); //sensördeki değeri oku
  Serial.println(deger2);

  //300 den küçük olup plmadığını kontrol et
  if (deger2 <=300) {
    digitalWrite(ldrledPin, HIGH);    //ledi yak
    Serial.println("LDR karanlik, LED acik");
    lcd.setCursor(13,0);
    lcd.print("Var");
    }
  else {
   digitalWrite(ldrledPin, LOW);     //ledi söndür
   Serial.println("---------------");
   lcd.setCursor(13, 0);
   lcd.print("Yok");
  }

  
}

//Bilgilerin server'a gönderilmesi
void veriPushetta(String CHANNEL, String text) {

  //ldrSensor();

  Serial1.println(String("AT+CIPSTART=\"TCP\",\"") + SERVER + "\",80"); //server'a bağlanma

  delay(3000);
  String   deneme = ("POST /api/pushes/");
  deneme += (CHANNEL);
  deneme += ("/ HTTP/1.1");
  deneme += ("\r\n");
  deneme += ("Host: "); //Bağlanılan server
  deneme += (SERVER);
  deneme += ("\r\n");
  deneme += ("Authorization: Token "); //Server'dan alınan api key
  deneme += (API);
  deneme += ("\r\n");
  deneme += ("Content-Type: application/json"); //JSON basma
  deneme += ("\r\n");
  deneme += ("Content-Length: "); //uzunluk
  deneme += (text.length() + 46);
  deneme += ("\r\n\r\n");
  deneme += ("{ \"body\" : \"");
  deneme += (text);
  deneme += ("\", \"message_type\" : \"text/plain\" }"); //Gönderilen içerik.
  deneme += ("\r\n\r\n");

ldrSensor();

  Serial1.print("AT+CIPSEND=");  //›veriyi Gönder komutu
  delay(100);
  Serial1.println(deneme.length()); //›verinin sonuna kadar tamamını alma.
  // Sorun oluşmadıysa veriyi gönder.
  if (Serial1.find(">")) {
    Serial1.println(deneme);
ldrSensor();
    Serial1.println("AT+CIPCLOSE=0"); //Veriyi gönderdikten sonra server ile olan bağlantıyı kes.
    delay(100);

  }
  else {
ldrSensor();
    Serial1.println("AT+CIPCLOSE=0"); //Veriyi gonderemediyse de server ile olan baglanti kesilecektir.
  }

}


