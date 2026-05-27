#define SSD1306_NO_SPLASH

#include <EEPROM.h>
#include <Wire.h>
#include <SPI.h>
#include <lmic.h>
#include <hal/hal.h>
#include <SSD1306Ascii.h>
#include <SSD1306AsciiWire.h>

static const u4_t DEVADDR = 0x260B37E4;

static const PROGMEM u1_t NWKSKEY[16] = {
  0xC8, 0x25, 0x50, 0xD9, 0x54, 0x6A, 0x5D, 0xBB,
  0x46, 0x48, 0xD3, 0x18, 0x4C, 0x52, 0x64, 0x62
};

static const PROGMEM u1_t APPSKEY[16] = {
  0x20, 0xB6, 0xC9, 0x66, 0xCE, 0xF1, 0xCD, 0x56,
  0xD0, 0x1B, 0x8E, 0xC0, 0xEE, 0x57, 0x15, 0x07
};

void os_getArtEui(u1_t* buf) {}
void os_getDevEui(u1_t* buf) {}
void os_getDevKey(u1_t* buf) {}

const lmic_pinmap lmic_pins = {
  .nss = 10,
  .rxtx = LMIC_UNUSED_PIN,
  .rst = 8,
  .dio = {6, 6, 6},
};

const int trigPin = A3;
const int echoPin = A2;
const int seuilPanier = 15;
const int seuilReArmement = 25;
const unsigned long timeoutEcho = 30000UL;
const unsigned long dureePartie = 60000UL;
const unsigned long intervalleMesure = 30UL;
const uint8_t portLoRaWan = 1;
const uint8_t taillePayloadLoRaWan = 4;
const uint8_t tailleFileLoRaWan = 4;
const bool envoyerEtatAuDemarrage = true;
const int adresseCompteurLoRaWan = 0;
const uint32_t magicCompteurLoRaWan = 0xB45C0A21;
const uint32_t compteurInitialLoRaWan = 1000;

const int oledAddress = 0x3C;

float duration, distance;
bool ballonPresent = false;
int score = 0;
unsigned long debutPartie = 0;
int dernierTempsRestant = -1;
bool finPartieAnnoncee = false;
unsigned long derniereMesure = 0;
uint8_t fileLoRaWan[tailleFileLoRaWan][taillePayloadLoRaWan];
uint8_t debutFileLoRaWan = 0;
uint8_t nbPaquetsLoRaWan = 0;

struct EtatCompteurLoRaWan {
  uint32_t magic;
  uint32_t seqnoUp;
};

SSD1306AsciiWire display;

float mesurerDistanceCm() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH, timeoutEcho);
  if (duration == 0) {
    return 999;
  }

  return (duration * 0.0343) / 2;
}

int calculerTempsRestant() {
  unsigned long tempsEcoule = millis() - debutPartie;

  if (tempsEcoule >= dureePartie) {
    return 0;
  }

  return (dureePartie - tempsEcoule + 999) / 1000;
}

void afficherEcran(int tempsRestant) {
  display.clear();
  display.set1X();
  display.setCursor(0, 0);
  display.print(F("TEMPS"));
  display.setCursor(84, 0);
  display.print(F("SCORE"));

  display.set2X();
  display.setCursor(0, 2);
  display.print(tempsRestant);
  display.print(F("s"));
  display.setCursor(84, 2);
  display.print(score);

  display.set1X();
  display.setCursor(42, 7);
  if (tempsRestant == 0) {
    display.print(F("FIN JEU"));
  } else {
    display.print(F("EN JEU"));
  }
}

void envoyerLoRaWan() {
  if (nbPaquetsLoRaWan == 0) {
    return;
  }

  if (LMIC.opmode & (OP_TXRXPEND | OP_TXDATA | OP_POLL)) {
    return;
  }

  uint8_t* payload = fileLoRaWan[debutFileLoRaWan];

  int resultat = LMIC_setTxData2(portLoRaWan, payload, taillePayloadLoRaWan, 0);
  if (resultat != 0) {
    Serial.print(F("LoRaWAN erreur file TX: "));
    Serial.println(resultat);
    return;
  }

  Serial.print(F("LoRaWAN paquet en file: score="));
  Serial.print((payload[0] << 8) | payload[1]);
  Serial.print(F(" temps="));
  Serial.print(payload[2]);
  Serial.print(F(" etat="));
  Serial.println(payload[3]);

  debutFileLoRaWan = (debutFileLoRaWan + 1) % tailleFileLoRaWan;
  nbPaquetsLoRaWan--;
}

void demanderEnvoiLoRaWan(int tempsRestant, bool finPartie) {
  if (nbPaquetsLoRaWan == tailleFileLoRaWan) {
    debutFileLoRaWan = (debutFileLoRaWan + 1) % tailleFileLoRaWan;
    nbPaquetsLoRaWan--;
    Serial.println(F("LoRaWAN file pleine, ancien paquet remplace"));
  }

  uint8_t index = (debutFileLoRaWan + nbPaquetsLoRaWan) % tailleFileLoRaWan;
  fileLoRaWan[index][0] = highByte(score);
  fileLoRaWan[index][1] = lowByte(score);
  fileLoRaWan[index][2] = tempsRestant;
  fileLoRaWan[index][3] = finPartie ? 1 : 0;
  nbPaquetsLoRaWan++;

  envoyerLoRaWan();
}

void configurerCanauxLoRaWan() {
#if defined(CFG_eu868)
  LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);
  LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);
  LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);
  LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);
  LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);
  LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);
  LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);
  LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);
  LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK, DR_FSK), BAND_MILLI);
#else
#error "La configuration LMIC doit etre en CFG_eu868 pour TTN Europe."
#endif
}

void restaurerCompteurLoRaWan() {
  EtatCompteurLoRaWan etat;
  EEPROM.get(adresseCompteurLoRaWan, etat);

  if (etat.magic == magicCompteurLoRaWan) {
    LMIC.seqnoUp = etat.seqnoUp;
    Serial.print(F("LoRaWAN FCnt restaure: "));
    Serial.println(LMIC.seqnoUp);
  } else {
    LMIC.seqnoUp = compteurInitialLoRaWan;
    sauvegarderCompteurLoRaWan();
    Serial.print(F("LoRaWAN FCnt EEPROM initialise: "));
    Serial.println(LMIC.seqnoUp);
  }
}

void sauvegarderCompteurLoRaWan() {
  EtatCompteurLoRaWan etat = {magicCompteurLoRaWan, LMIC.seqnoUp};
  EEPROM.put(adresseCompteurLoRaWan, etat);
}

void configurerLoRaWan() {
  os_init();
  LMIC_reset();
  LMIC_setClockError(MAX_CLOCK_ERROR * 2 / 100);

  uint8_t appskey[sizeof(APPSKEY)];
  uint8_t nwkskey[sizeof(NWKSKEY)];
  memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
  memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
  LMIC_setSession(0x1, DEVADDR, nwkskey, appskey);
  restaurerCompteurLoRaWan();

  configurerCanauxLoRaWan();
  LMIC_setLinkCheckMode(0);
  LMIC.dn2Dr = DR_SF9;
  LMIC_setDrTxpow(DR_SF7, 14);

  Serial.println(F("LoRaWAN ABP pret"));
}

void onEvent(ev_t ev) {
  switch (ev) {
    case EV_TXCOMPLETE:
      Serial.println(F("LoRaWAN emission terminee"));
      sauvegarderCompteurLoRaWan();
      if (LMIC.txrxFlags & TXRX_ACK) {
        Serial.println(F("LoRaWAN ACK recu"));
      }
      if (LMIC.dataLen) {
        Serial.print(F("LoRaWAN downlink: "));
        Serial.print(LMIC.dataLen);
        Serial.println(F(" octets"));
      }
      envoyerLoRaWan();
      break;
    default:
      break;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  Wire.begin();
  Wire.setClock(400000L);
  display.begin(&Adafruit128x64, oledAddress);
  display.setFont(System5x7);
  display.clear();

  configurerLoRaWan();
  debutPartie = millis();
  afficherEcran(calculerTempsRestant());
  Serial.println(F("Compteur pret: 60 secondes"));
  if (envoyerEtatAuDemarrage) {
    demanderEnvoiLoRaWan(calculerTempsRestant(), false);
  }
}

void loop() {
  os_runloop_once();
  envoyerLoRaWan();

  int tempsRestant = calculerTempsRestant();

  if (tempsRestant != dernierTempsRestant) {
    dernierTempsRestant = tempsRestant;
    afficherEcran(tempsRestant);
  }

  if (tempsRestant == 0) {
    if (!finPartieAnnoncee) {
      finPartieAnnoncee = true;
      Serial.print(F("Temps ecoule, score final = "));
      Serial.println(score);
      demanderEnvoiLoRaWan(0, true);
    }
    return;
  }

  if (millis() - derniereMesure < intervalleMesure) {
    return;
  }
  derniereMesure = millis();

  distance = mesurerDistanceCm();

  if (distance < seuilPanier && distance > 0 && !ballonPresent) {
    ballonPresent = true;
    score += 2;
    afficherEcran(tempsRestant);
    Serial.print(F("Panier detecte, score = "));
    Serial.println(score);
    demanderEnvoiLoRaWan(tempsRestant, false);
  }

  if (distance > seuilReArmement) {
    ballonPresent = false;
  }
}
    
