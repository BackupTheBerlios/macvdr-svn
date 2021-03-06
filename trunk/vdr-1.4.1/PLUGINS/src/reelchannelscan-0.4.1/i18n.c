/*
 * i18n.c: Internationalization
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id:
 */

#include "i18n.h"

 const tI18nPhrase Phrases[] = {
  { "Search Transponders for DVB Channels", //EN
    "Durchsucht Transponder nach DVB Kan�len.",// DE
    "", // Slovenski
    "Ricerca Transponders di Canali DVB",// IT
    "Doorzoek Transponders naar DVB kanalen", // Nederlands
    "", // Portugu�s
    "", // Fran�ais
    "", // Norsk
    "Kanavahaku DVB-transpondereille",// FI
    "", // Polski
    "", // Espa�ol
    "", // �������� (Greek)
    "", // Svenska
    "", // Romaneste
    "", // Magyar
    "", // Catal�
    "", // ������� (Russian)
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // �esky (Czech)
  },
  { "Search Mode",
    "Suchmodus",
    "",
    "Modalita' di Ricerca",
    "Zoek mode",
    "",
    "",
    "",
    "Hakutapa",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // �esky (Czech)
  },
  { "scanning on transponder",
    "Suche auf Transponder",
    "",
    "Ricerca trasponder",
    "Scan op transponder",
    "",
    "",
    "",
    "haetaan transponderilta",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // �esky (Czech)
  },
  { "Scanning configured satellites",
    "Durchsuche eingerichtete Satelliten",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // �esky (Czech)
  },
  { "DiSEqC",
    "DiSEqC",
    "DiSEqC",
    "DiSEqC",
    "DiSEqC",
    "",
    "",
    "",
    "DiSEqC-kytkin",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // �esky (Czech)
  },
  { "FEC",
    "FEC",
    "",
    "FEC",
    "FEC",
    "",
    "",
    "",
    "FEC",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // �esky (Czech)
  },
  { "None",
    "Keine",
    "",
    "Nessun",
    "Geen",
    "",
    "",
    "",
    "ei",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // �esky (Czech)
  },
  { "Auto",
    "Auto",
    "",
    "Auto",
    "Auto",
    "",
    "",
    "",
    "auto",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // �esky (Czech)
  },
  { "SearchMode$Auto",
    "Automatisch",
    "",
    "Ricerca automatica",
    "Automatisch",
    "",
    "",
    "",
    "automaattinen",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // �esky (Czech)
  },
  { "Manual",
    "Manuell",
    "",
    "Manuale",
    "Handmatig",
    "",
    "",
    "",
    "manuaalinen",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // �esky (Czech)
  },
  { "Channelscan",
    "Kanalsuche",
    "",
    "Scansione canali",
    "Kanaal scan",
    "",
    "",
    "",
    "Kanavahaku",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "����� �������",
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // �esky (Czech)
  },
  { "Detailed search",
    "Detaillierte Suche",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // �esky (Czech)
  },
  { "Position",
    "Position",
    "",
    "Posizione",
    "Positie",
    "",
    "",
    "",
    "Sijainti",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // �esky (Czech)
  },
  { "Tuner Error",
    "Tuner Fehler",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // �esky (Czech)
  },
  { "Scanning on transponder",
    "Durchsuche Transponder",
    "",
    "Ricerca Transponders",
    "Scannen op transponder",
    "",
    "",
    "",
    "Haetaan transponderilta",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // �esky (Czech)
  },
  { "channels in current list",
    "Kan�le in aktueller Liste",
    "",
    "Canali presenti nella lista",
    "Kanalen in huidige lijst",
    "",
    "",
    "",
    "T�m�nhetkiset kanavat",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // �esky (Czech)
  },
  { "TV CHANNELS",
    "TV KAN�LE",
    "",
    "CANALI TV",
    "TV KANALEN",
    "",
    "",
    "",
    "TV-kanavat",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // �esky (Czech)
  },
  { "No new channels found",
    "Keine neuen Kan�le gefunden",
    "",
    "Non sono stati trovati nuovi canali",
    "Geen nieuwe kanalen gevonden",
    "",
    "",
    "",
    "Uusia kanavia ei l�ydetty",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // �esky (Czech)
  },
  { "Scanning aborted by User",
    "Suche abgebrochen",
    "",
    "Ricerca interrotta dall'Utente",
    "Scannen afgebroken door User",
    "",
    "",
    "",
    "Haku keskeytetty",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // �esky (Czech)
  },

  { "TV CHANNELS                     RADIO",
    "TV KAN�LE                       RADIO",
    "",
    "CANALI TV                       RADIO",
    "TV KANALEN                      RADIO",
    "",
    "",
    "",
    "TV-kanavat                      Radio",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // �esky (Czech)
  },
  { "Symbolrate",
    "Symbolrate",
    "",
    "Symbolrate",
    "Symbolrate",
    "",
    "",
    "",
    "Symbolinopeus",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "����.��������",
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // �esky (Czech)
  },
  { "Frequency",
    "Frequenz",
    "",
    "Frequenza",
    "Frequentie",
    "",
    "",
    "",
    "Taajuus",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "�������",
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // �esky (Czech)
  },
  { "Scanning %s\tPlease Wait",
    "Durchsuche %s\tBitte warten",
    "",
    "",
    "",
    "",
    "",
    "",
    "Haku k�ynniss� %s.\tOdota hetkinen...",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // �esky (Czech)
  },
  { "Scanning %s (%iMHz) \tPlease Wait",
    "Durchsuche %s (%iMHz) \tBitte warten",
    "",
    "",
    "",
    "",
    "",
    "",
    "Haku k�ynniss� %s.\tOdota hetkinen...",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // �esky (Czech)
  },
  { "Scanning %s (%.3fMHz) \tPlease Wait",
    "Durchsuche %s (%.3fMHz) \tBitte warten",
    "",
    "",
    "",
    "",
    "",
    "",
    "Haku k�ynniss� %s.\tOdota hetkinen...",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // �esky (Czech)
  },
  { "Button$Start",
    "Start",
    "",
    "Start",
    "Start",
    "",
    "",
    "",
    "Aloita",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // �esky (Czech)
  },
  { "Radio only",
    "nur Radio",
    "",
    "Solo radio",
    "Alleen Radio",
    "",
    "",
    "",
    "vain radio",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // �esky (Czech)
  },
  { "TV only",
    "nur TV",
    "",
    "Solo TV",
    "Alleen TV",
    "",
    "",
    "",
    "vain TV",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // �esky (Czech)
  },
  { "Radio + TV",
    "Radio + TV",
    "",
    "Radio + TV",
    "Radio + TV",
    "",
    "",
    "",
    "radio + TV",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // �esky (Czech)
  },
  { "Radio + TV + NVOD",
    "Radio + TV + NVOD",
    "",
    "Radio + TV + NVOD",
    "Radio + TV + NVOD",
    "",
    "",
    "",
    "radio + TV + NVOD",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // �esky (Czech)
  },
  { "Service Type",
    "Service Arten",
    "",
    "Tipo servizio",
    "Service type",
    "",
    "",
    "",
    "Haettavat palvelut",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // �esky (Czech)
  },
  { "enabled",
    "aktiviert",
    "",
    "abilitato",
    "ingeschakeld",
    "",
    "",
    "",
    "p��ll�",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // �esky (Czech)
  },
  { "disabled",
    "deaktiviert",
    "",
    "disabilitato",
    "uitgeschakeld",
    "",
    "",
    "",
    "pois",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // �esky (Czech)
  },
  { "Added new channels",
    "Neue Kan�le hinzugef�gt",
    "",
    "Aggiunti nuovi canali",
    "Nieuwe kanalen toegevoegd",
    "",
    "",
    "",
    "Uudet kanavat lis�tty",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // �esky (Czech)
  },
  { "Frequency (kHz)",
    "Frequenz (kHz)",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // �esky (Czech)
  },
  { "Frequency (MHz)",
    "Frequenz (MHz)",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // �esky (Czech)
  },
  { "Expert",                                                                                               
    "Experten",                                                                                             
    "",                                                                                                     
    "Esperto",                                                                                              
    "Expert",                                                                                               
    "",                                                                                                     
    "",                                                                                                     
    "",                                                                                                     
    "",                                                                                                     
    "",                                                                                                     
    "",                                                                                                     
    "",                                                                                                     
    "",                                                                                                     
    "",                                                                                                     
    "",                                                                                                     
    "",                                                                                                     
    "",                                                                                                     
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // �esky (Czech)
  },
  { "DVB-S - Satellite",
    "DVB-S - Satellit",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // �esky (Czech)
  },
  { "DVB-C - Cable",
    "DVB-C - Kabel",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // �esky (Czech)
  },
  { "DVB-T - Terrestrial",
    "DVB-T - Terrestrisch",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // �esky (Czech)
  },
  { "Press OK to proceede",
    "Dr�cken Sie OK um fortzufahren",
    "",
    "Premere OK per continuare",
    "Druk OK om te vervolgen",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // �esky (Czech)
  },
  { "Press OK to finish or Exit for new scan",
    "Dr�cken Sie OK zum Beenden oder Exit f�r eine neue Kanalsuche.",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // �esky (Czech)
  },
  { "Standard",
    "Standard",
    "",
    "Standard", // Italiano
    "Standaard",
    "",
    "Standart",
    "",
    "Vakio",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "", // Hrvatski (Croatian)
    "", // Eesti
    "", // Dansk
    "", // �esky (Czech)
  },
  { "Extended",
    "Erweitert",
    "",
    "",
    "",
    "",
    "Pr�cision",
    "",
    "Laaja",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "", // Eesti
    "", // Dansk
    "", // �esky (Czech)
  },
  { "Terrestrial",
    "Terr.",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "", // Eesti
    "", // Dansk
    "", // �esky (Czech)
  },
  { "Cable",
    "Kabel",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "", // Eesti
    "", // Dansk
    "", // �esky (Czech)
  },
  { "Running services on transponder: %i / %i",
    "Aktive Dienste auf Transponder: %i / %i",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "", // Eesti
    "", // Dansk
    "", // �esky (Czech)
  },
  { "Scanning %s (%iMHz)\t%s",
    "Durchsuche %s (%iMHz)\t%s",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "", // Eesti
    "", // Dansk
    "", // �esky (Czech)
  },
  { "Scanning %s (%.3fMHz)\t%s",
    "Durchsuche %s (%.3fMHz)\t%s",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "", // Eesti
    "", // Dansk
    "", // �esky (Czech)
  },
  { NULL }
  };

