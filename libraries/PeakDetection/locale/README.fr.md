# PeakDetection

**PeakDetection** est une bibliothèque Arduino pour la détection de pics en temps réel dans les données de capteurs.

[🇺🇸 Translate](https://github.com/leandcesar/PeakDetection/blob/master/README.md) | [🇧🇷 Traduzir](https://github.com/leandcesar/PeakDetection/blob/master/locale/README.pt-BR.md)

## Algorithme

Il est basé sur le principe de dispersion: si un nouveau point de données est à une distance de `x` écarts types d'une moyenne mobile donnée, l'algorithme signale (également appelé score-z).

L'algorithme prend 3 entrées:

* `lag`: est le décalage de la fenêtre mobile. Ce paramètre détermine dans quelle mesure vos données seront lissées et à quel point l'algorithme est adaptable aux changements de la moyenne à long terme des données. Plus vos données sont stationnaires, plus vous devriez inclure de décalages. Si vos données contiennent des tendances variant dans le temps, vous devez tenir compte de la rapidité avec laquelle vous souhaitez que l'algorithme s'adapte à ces tendances.

* `threshold`: ce paramètre correspond au nombre d'écart-types par rapport à la moyenne mobile au-dessus duquel l'algorithme classera un nouveau point de données comme étant un signal. Ce paramètre doit être défini en fonction du nombre de signaux que vous attendez. Le seuil influence donc directement la sensibilité de l'algorithme et donc la fréquence à laquelle l'algorithme signale.

* `influence`: est le score-z auquel l'algorithme signale. Ce paramètre détermine l'influence des signaux sur le seuil de détection de l'algorithme. S'il est réglé à 0, les signaux n'ont aucune influence sur le seuil, de sorte que les signaux futurs sont détectés en fonction d'un seuil calculé avec une moyenne et un écart type qui ne sont pas influencés par les signaux passés. Vous devez placer le paramètre d'influence quelque part entre 0 et 1, en fonction de la mesure dans laquelle les signaux peuvent influencer systématiquement la tendance variant dans le temps des données.

## Bibliothèque

* `begin()`: initialise l'objet `PeakDetection` et configure les paramètres personnalisés. Si aucun paramètre n'a été défini, les valeurs par défaut sont utilisées. Paramètres: lag (par défaut=32), threshold (par défaut=2), influence (par défaut=0.5).

* `add()`: ajoute un nouveau point de données à l'algorithme, calcule les écarts types et la moyenne mobile.

* `getPeak()`: retourne l'état de crête du dernier point de données ajouté. {-1, 0, 1}, représentant respectivement en dessous, dans ou au-dessus du seuil d'écart-type.

* `getFilt()`: retourne le dernier point de données filtré par la moyenne mobile.

## Installation

Pour utiliser la bibliothèque:

1. Téléchargez le fichier zip et décompressez-le.
2. Copiez le dossier dans le dossier des bibliothèques d'Arduino (`C:/Users/username/Documents/Arduino/libraries`).
3. Renommez-le en `PeakDetection`.

## Exemple

```C++
#include <PeakDetection.h>                       // importation de la bibliothèque

PeakDetection peakDetection;                     // création de l'objet PeakDetection

void setup() {
  Serial.begin(9600);                            // définition du débit de données pour la communication série
  pinMode(A0, INPUT);                            // broche analogique utilisée pour connecter le capteur
  peakDetection.begin(48, 3, 0.6);               // définition du lag, du seuil et de l'influence
}

void loop() {
    double data = (double)analogRead(A0)/512-1;  // convertit la valeur du capteur en une plage entre -1 et 1
    peakDetection.add(data);                     // ajoute une nouvelle valeur
    int peak = peakDetection.getPeak();          // 0, 1 ou -1
    double filtered = peakDetection.getFilt();   // moyenne mobile
    Serial.print(data);                          // affichage de la valeur
    Serial.print(",");
    Serial.print(peak);                          // affichage du statut de pic
    Serial.print(",");
    Serial.println(filtered);                    // affichage de la moyenne mobile
}
```

![Example](https://github.com/leandcesar/PeakDetection/blob/master/docs/assets/example.gif)

## Remerciements

* [StackOverFlow](https://stackoverflow.com/questions/22583391/peak-signal-detection-in-realtime-timeseries-data)
