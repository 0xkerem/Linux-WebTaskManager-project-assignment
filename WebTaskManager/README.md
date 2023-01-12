Geben Sie im Terminal "make" ein, um das Programm auszuführen.

Um das Programm zu schließen, führen Sie im Terminal die Tastenkombination Strg+C aus oder schließen Sie das Terminal, zusätzlich können Sie zweimal die „Enter“-Taste drücken.

Geben Sie den Befehl „make help“ im Terminal für die Hilfeoptionen (Dokumentation) ein.
Für deutsche Hilfeoptionen führen Sie "make help-de" aus.

Wichtige Notizen:

- Möglicherweise sehen Sie einige Fehlermeldungen, die vom Befehl xdg-open auf dem Terminalbildschirm stammen, dies hat jedoch keinen Einfluss auf den Betrieb des Programms.
- Ich habe dieses Projekt zunächst mit Node.js für die Serverseite erstellt und dann zur httplib-Bibliothek migriert. Obwohl ich auf alle Serveranfragen auf die gleiche Weise geantwortet habe, hat httplib verschiedene Bugs eingeführt. Diese Bugs sind kein Problem für das Programm, ich habe es geschafft, die meisten davon mit JavaScript im Frontend zu verbergen, aber dennoch können einige Fehler im Terminal erscheinen. Diese Bugs sind Dinge wie eine Anforderung zur Änderung der CPU-Affinität für einen nicht existierenden Prozess.
- Ein weiterer Fehler, der von libhttp ausgeht, ist, dass bei der Eingabe des Passworts für die Root-Berechtigung im Terminal das Passwort nicht gelesen werden kann, ein Neustart der Anwendung löst das Problem. Dieser Fehler existierte nicht, als ich mit Node.js arbeitete.
