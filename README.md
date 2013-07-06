# Maschinelles Sehen / Computer Vision

Bearbeitung der Übungsaufgaben des Kurses *Maschinelles Sehen* bei [Dr.-Ing. Anko Börner](http://www.dlr.de/os/desktopdefault.aspx/tabid-3459/5342_read-13663/sortby-lastname/) an der Beuth Hochschule für Technik Berlin.

Die Projekte sind in C++11 (soweit von meinem verwendeten Compiler unterstützt) mittels OpenMP bearbeitet. OpenMP-Unterstützung ist nicht zwingend notwendig, aber empfohlen; Ebenso ist es **nicht** ratsam, die Beispiele in `DEBUG`-Konfiguration zu starten.

Die Projekte verwenden `OpenCV`~2.4.1 für die Darstellung der Ergebnisse.

## Parts

### `mas01`: Start-up

`mas01` is about the general use of OpenCV for loading and storing images, as well as basic image transformations through pixel manipulation.

### `mas02`: Image Statistics

`mas02` is about dynamic memory management and simple statistics (minimum and maximum, mean, standard deviation) of an image. This project also covers radiometric transformations in the context of high dynamic range imaging.

### `mas03`: Feature Detection

`mas03` is about feature detection by using image/template cross-correlation and absolute differences.

## License

### General License

Copyright &copy; 2013 Markus Mayer &lt;widemeadows@gmail.com&gt;

State-Space Sandbox is licensed under the EUPL, Version 1.1 or - as soon they will be approved by the European Commission -
subsequent versions of the EUPL (the "Licence"); you may not use this work except in compliance with the Licence.
You may obtain a copy of the Licence at:

[http://joinup.ec.europa.eu/software/page/eupl/licence-eupl](http://joinup.ec.europa.eu/software/page/eupl/licence-eupl)

Unless required by applicable law or agreed to in writing, software distributed under the Licence is
distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the Licence for the specific language governing permissions and limitations under the Licence.

### License of Images

I do not claim copyright of the images used in these examples, they are used here solely on an academic *fair use* basis. If in doubt consider them to be copyrighted by [Dr.-Ing. Anko Börner](http://www.dlr.de/os/desktopdefault.aspx/tabid-3459/5342_read-13663/sortby-lastname/).