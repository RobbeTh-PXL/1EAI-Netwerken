# TCP_CHAT
**TCP Chatbox met geschiedenis via HTTP (25%)**

## Doelstellingen
### TCP_ChatClient
- [x] Client kan TCP connectie maken (bewijs via WireShark) [1/25]
- [x] Client kan TCP packetten succesvol sturen en ontvangen (bewijs via WireShark) [1/25]
- [x] Client stuurt de door de gebruiker ingegeven berichten door (bewijs Packet Sender) [1/25]
- [x] Er kan tegelijk berichten gestuurd en ontvangen worden op de Client zonder afgesproken beurtrol (e.g. multi-threaded) [4/25]

### TCP_ChatServer
- [x] Server kan luisteren naar een poort voor inkomende TCP connecties [1/25]
- [ ] Server accepteert verbinding en praat met verbonden client over TCP [1/25]
- [ ] Server applicatie stuurt ontvangen bericht door naar alle andere deelnemers [1/25]
- [ ] Server stuurt niet naar de afzender zijn eigen bericht [1/25]
- [ ] Server stuurt de afzender informatie mee met het bericht (i.e. IP-adres + poort van zender) [2/25]
- [ ] Server stuurt de laatste 16 chatberichten naar de client bij nieuwe verbinding [2/25]
- [ ] Server kan HTTP communicatie uitvoeren met de webserver (bewijs via WireShark) [2/25]
- [ ] Server stuurt over HTTP de chatberichten door naar de webserver (bewijs via WireShark) [2/25]
- [ ] Server vraagt de laatste 16 chatberichten via HTTP aan de webserver (bij start) [2/25]
- [ ] Er kunnen meerdere Clients simultaan verbonden zijn en chatten met elkaar (i.e. assynchroon of synchroon verwerkt op de server) [4/25]
