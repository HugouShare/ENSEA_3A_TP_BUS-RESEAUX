# Compte rendu du TP  

## Introduction  
Durant ces TPs de bus & réseaux, l'objectif est de mettre en place le système suivant :  
<img width="1333" height="614" alt="image" src="https://github.com/user-attachments/assets/5432dc45-abcd-4308-863c-21bd4d93261a" />

## Mise en place de la partie I2C du système  
Dans un premier temps, nous allons mettre en place la partie I2C du projet.  
<img width="714" height="208" alt="image" src="https://github.com/user-attachments/assets/b8640915-dbf7-40d3-b937-7191a3778152" />  
Nous utilisons une communication I2C afin de communiquer avec le capteur BMP280.  

### Capteur BMP280  
La datasheet du capteur BMP280 a été répertoriée dans le dossier "_Ressources/Datasheets_".  
A partir de la datasheet, nous obtenons alors les informations suivantes :  
- Adresses possibles pour ce composant :
  - Nous la trouvons page 28 de la datasheet
  - Connecting SDO to GND results in slave address 1110110 (0x76)
  - Connecting it to VDDIO results in slave address 1110111 (0x77)
  - ATTENTION : le pin SDO ne peut être laissé flotant sinon l'adresse du device sera non définie
- Registre et valeur permettant d'identifier ce composant :
  - Nous la trouvons page 24 de la datasheet
  - Register 0xD0 “id”
- Registre et valeur permettant de placer le composant en mode normal :
  - Nous la trouvons page 26 de la datasheet
  - Register 0xF5 “config”
- Registre et valeur contenant l'étalonnage du composant :
  - Nous la trouvons page 24 de la datasheet
    <img width="1012" height="491" alt="image" src="https://github.com/user-attachments/assets/aeae1bc2-6f52-4e1f-8372-a8b8b6ad733a" />  
  - Ainsi, les valeurs de calibrations sont contenues dans les registres 0xA1 jusque 0x88
- Registre et valeur contenant la température :
  - Nous la trouvons page 27 de la datasheet
    <img width="1003" height="412" alt="image" src="https://github.com/user-attachments/assets/555e1898-cf1a-4148-b814-3edacf96738e" />  
  - Ainsi, les valeurs de calibrations sont contenues dans les registres 0xFA jusque 0xFC et se nomme "temp"
- Registre et valeur contenant la pression :
  - Nous la trouvons page 26 de la datasheet
    <img width="1007" height="417" alt="image" src="https://github.com/user-attachments/assets/f1e04610-0a8c-4655-9bdc-58421fed378e" />  
  - Ainsi, les valeurs de calibrations sont contenues dans les registres 0xF7 jusque 0xF9 et se nomme "press"
- Fonctions permettant le calcul de la température et de la pression compensées, en format entier 32 bits :
  - Nous les trouvons page 45 et 46 de la datasheet
  - Les fonctions ont les prototypes suivants :
    - BMP280_S32_t bmp280_compensate_T_int32(BMP280_S32_t adc_T)
    - BMP280_U32_t bmp280_compensate_P_int32(BMP280_S32_t adc_P)
    <img width="935" height="610" alt="image" src="https://github.com/user-attachments/assets/06d8ba80-5315-4519-a674-fa952471575e" />
    <img width="953" height="498" alt="image" src="https://github.com/user-attachments/assets/cc29adfa-8975-487a-b960-b28b5b4f4a08" />

### Setup du STM32  
Nous configurons maintenant notre carte de développement STM. Il s'agit d'une NUCLEO-F446RE.  
Voici le pinout de la carte :  
<img width="486" height="430" alt="image" src="https://github.com/user-attachments/assets/ef70ffe8-98e6-4a36-8700-6dcad772d5dc" />  

Après configuration de la carte NUCLEO, nous avons le fichier .ioc suivant :  
<img width="626" height="593" alt="image" src="https://github.com/user-attachments/assets/379734e7-69cd-4aee-b1a9-441d4edb5905" />

Afin de pouvoir déboguer à l'aide de la fonction printf, nous ajoutons le bout de code donné dans le sujet dans le fichier "_stm32f4xx_hal_msp.c_".  
Nous testons celle-ci avec un simple Hello world. Tout fonctionne comme il se doit :  
<img width="421" height="339" alt="image" src="https://github.com/user-attachments/assets/6faafb6c-84e6-41bc-9305-0afb7450d000" />

### Communications I2C avec le BMP280  
Afin de communiquer en I2C avec le module BMP280, nous allons principalement utiliser les deux fonctions suivantes :
- HAL_StatusTypeDef HAL_I2C_Master_Transmit(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout)
- HAL_StatusTypeDef HAL_I2C_Master_Receive(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout)

Avec : 
  - I2C_HandleTypeDef hi2c: structure stockant les informations du contrôleur I²C
  - uint16_t DevAddress: adresse I³C du périphérique Slave avec lequel on souhaite interagir.
  - uint8_t *pData: buffer de données
  - uint16_t Size: taille du buffer de données
  - uint32_t Timeout: peut prendre la valeur HAL_MAX_DELAY

Ces fonctions vont nous permettre d'accéder directement aux différents registres du module et donc d'écrire ou lire des données depuis les registres du module. 
A des fins de lisibilité et de clarté de code, nous décidons de créer les fonctions :  
- HAL_StatusTypeDef BMP280_WriteReg(uint8_t reg, uint8_t value);
- HAL_StatusTypeDef BMP280_ReadReg(uint8_t reg, uint8_t *value);
- HAL_StatusTypeDef BMP280_ReadMulti(uint8_t reg, uint8_t *buf, uint16_t len);

Permettant respectivement de :
- écrire dans un registre nommé _reg_ une valeur _value_
- lire dans un registre nommé _reg_ une valeur et l'écrire dans la variable nommée _value_
- lire dans _len_ registres à partir du registre _reg_ des valeurs et les écrire dans le buffer nommé _buf_
  
Afin d'écrire dans un registre, il suffit simplement d'utiliser la fonction HAL_I2C_Master_Transmit en précisant :  
- l'adresse I2C du module auquel on souhaite accéder
- un buffer de taille 2 contenant respectivement : le registre où l'on veut écrire et la valeur que l'on veut écrire dans ce registre

Afin de lire dans un registre, il suffit simplement :
- d'utiliser la fonction HAL_I2C_Master_Transmit en précisant :  
  - l'adresse I2C du module auquel on souhaite accéder
  - l'adresse du registre que l'on souhaite lire
- d'utiliser la fonction HAL_I2C_Master_Receive en précisant :  
  - un pointeur sur la variable dans laquelle on veut écrire se qui se trouve dans le registre

#### Identification du BMP280  
Tout d'abord, nous commençons par identifier le module BMP280, c'est-à-dire lire dans son registre ID.  
Pour cela, nous utilisons les informations de la datasheet et écrivons le code correspondant dans une fonction nommée "_BMP280_Init(void)_".  
Après exécution de celle-ci, nous obtenons bien un ID de 0x58 cohérent avec ce qui est écrit dans la datasheet.  

#### Configuration du BMP280  
Suite à cela, nous configurons le module BMP280 afin de spécifier de quelle manière nous voulons utiliser le capteur.  
Dans notre cas à nous : mode normal, Pressure oversampling x16, Temperature oversampling x2.  
Nous ajoutons alors à la fonction "_BMP280_Init(void)_" la configuration du capteur.  

#### Récupération de l'étalonnage, de la température et de la pression  
Afin de récupérer en une fois le contenu des registres d'étalonnages du BMP280, nous écrivons la fonction "_BMP280_Calibration(void)_".  
Dans cette fonction, nous remplissons tout simplement le buffer "_uint8_t calibration_values [26]_" via l'appel de fonction "_BMP280_ReadMulti(BMP280_CALIBRATION, &calibration_values, 26)_".  
La fonction _BMP280_ReadMulti_ va alors remplir le buffer _calibration_values_ en commencant à lire au registre _BMP280_CALIBRATION_ (valant 0xA1) et en incrémentant automatiquement l'adresse du registre 26 fois, soit jusqu'à avoir fini de lire dans l'ensemble des registres d'étalonnage du module.  

Ensuite, nous définissons la fonction "_void BMP280_ReadRawData(int32_t *raw_temp, int32_t *raw_press)_", permettant d'obtenir respectivement les valeurs brutes de température et de pression lues par le capteur, sans traitement.  
Une fois encore, le principe est le même : nous commencons à lire à l'adresse _BMP280_PRESS_MSB_ (valant 0xF7) jusqu'au registre 0xFC, puis nous mettons en forme les données lues dans les variables raw_temp et raw_press, conformément à ce qui est écrit dans la datasheet.  

#### Calcul des températures et des pression compensées  
Pour finir, nous utilisons les fonctions données dans la datasheet page 45 et 46 afin d'appliquer un traitement sur les valeurs de température et de pression mesurée par le capteur, en vue d'obtenir des valeurs les plus correctes possible.  
Nous reprenons directement le contenu fournit dans la datasheet.  

Une fois cela fait, nous utilisons la boucle _while(1)_ du fichier "_main.c_" afin d'effectuer des mesures de pression et de température et comparer les valeurs brutes aux valeurs avec traitement.  
Nous observons que... A CONTINUER !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  

## Mise en place de l'interfaçage STM32-Raspberry  
Dans un second temps, nous allons mettre en place un interfaçage entre notre carte STM32 et Raspberry.  
<img width="866" height="479" alt="image" src="https://github.com/user-attachments/assets/4e31f893-ab4f-4cc3-8550-9a39669d01b6" />  
Nous utiliserons un script Python afin d'interroger la carte STM32 depuis la Raspberry.  

### Mise en route du Raspberry PI Zéro  

#### Préparation du Raspberry  
Informations saisies lors de la création de l'image via Raspberry Pi Imager :  
- hostname : HugoCFArthurNN
- nom utilisateur : hugoarthur
- mdp : ensea2526
- SSH : C304_DTI_AP
- mdp : ilovelinux

#### Premier démarrage  
Nous flashons alors la carte SD avec les configurations faites via Raspberry Pi Imager.  
La Raspberry a obtenu son adresse IP sur le réseau de la même manière qu’un ordinateur classique : via le protocole DHCP.  
Son adresse IP correspond à la théorie vue en cours :  
![OIP NHsl6SOwA1YLv5Rn71-dHwHaFj](https://github.com/user-attachments/assets/d01dba0f-2ff9-4451-8b7e-074d66e4c709)  

Nous nous connectons alors à notre Raspberry PI Zero en suivant le protocole suivant :  
- ouverture du terminal de cmd Windows
- ecrire dans le terminal : ssh hugoarthur@192.168.4.207
- ecrire dans le terminal le mdp : ensea2526  
Nous obtenons alors l'interface suivante :
<img width="1184" height="222" alt="image" src="https://github.com/user-attachments/assets/4fa3d9a4-2ad5-4112-aebc-d88fb25c0e67" />  
  
### Port série  

#### Loopback  
Dans un premier temps, nous rebouclons la pin RX sur la pin TX.
![BOARD-Layout-CMPLETE_800x506-768x486](https://github.com/user-attachments/assets/c5023909-3ba7-494d-9369-463907a953ff)  

Nous installons minicom via la commande : 
_sudo apt update_  
_sudo apt install minicom_  

Suite à cela, nous écrivons dans le terminal de cmd Windows :  
- sudo minicom -D /dev/ttyS0

Cela nous permet alors de configurer le port série. Nous le configurons de la manière suivante (en pressant CTRL+A suivi de O) :  
<img width="892" height="429" alt="image" src="https://github.com/user-attachments/assets/f1b7800b-caa1-465a-a40a-2741d357a99f" />  

Nous pouvons effectivement écrire et visualiser les caractères écrits en même temps : 
<img width="872" height="362" alt="image" src="https://github.com/user-attachments/assets/8d1e3471-f603-4273-9bae-c4aa8c7cd734" />

En connectant notre sortie RX de notre Rasberry avec la sortie TX du STM32, nous parvenons à lire les valeurs envoyées par notre STM32 : 
<img width="985" height="740" alt="image" src="https://github.com/user-attachments/assets/b76f8cf8-f76b-4bf6-bebf-335ca62c7af7" />  

## Interface REST  
Nous mettons maintenant en place une interface REST (Representational State Transfer) sur le Raspberry. 
<img width="524" height="459" alt="image" src="https://github.com/user-attachments/assets/55da32c4-c9f3-41c8-b604-ba4e23a27ff2" />  
Nous réaliserons cela via Python depuis la Raspberry.  

### Installation du serveur Python  

#### Installation 
Nous passons l'étape de création d'un utilisateur différent de pi, puisque nous somme déjà logé sous le nom de hugoarthur.  
<img width="441" height="50" alt="image" src="https://github.com/user-attachments/assets/24b7418c-f2c4-45fa-9f8f-d218a7a6eb17" />  

Nous installons ensuite Python sur la Raspberry via les commandes suivantes :  
```
sudo apt update
sudo apt install python3-pip
```

Une fois loggé dans notre session et python 3 installé, nous réalisons les opérations suivantes :  
- 1° : nous créons un répertoire nommé restserver depuis le chemin /home/hugoarthur via la commande :  
  - ```
    mkdir restserver
    ```
    Nous nous mettons alors dans le répertoire suivant : /home/hugoarthur/restserver
- 2° : dans le répertoire restserver, nous créons un fichier nommé "_requirement.txt_" via la commande :
  - ```
    touch requirement.txt
    ```
    Nous obtenons alors un fichier "_requirement.txt_" vierge
- 3° : nous écrivons dans le fichier "_requirement.txt_" via la commande :
  - ```
    nano requirement.txt
    ```
    Une fois cette commande exécutée, nous pouvons alors écrire dans le fichier requirement.txt. Nous y écrivons : pyserial et flask.
- 4° : pour finir, nous installons les modules pyserial et flask via les commandes :
  - ```
    sudo apt install python3-flask
    sudo apt install python3-serial
    ```

#### Premier fichier WEB  
Dans le dossier restserver, nous créons un fichier nommé "_hello.py_".  
Nous y plaçons le code suivant :  
```
from flask import Flask
app = Flask(__name__)

@app.route('/')
def hello_world():
    return 'Hello, World!\n'
```

Une fois cela fait, nous lançons notre serveur WEB via la commande :  
```
FLASK_APP=hello.py flask run --debug
```
A ce stade, le problème est que le serveur ne tourne qu'en mode loopback sur la Raspberry.  
Afin de rendre le serveur accessible depuis un navigateur, et en particulier depuis le navigateur de notre ordinateur, nous entrons en plus de la commande précédente la commande suivante :  
```
 FLASK_APP=hello.py FLASK_ENV=development flask run --host 0.0.0.0.0
```
En plus de cela, nous entrons DANS UN NOUVEAU TERMINAL, la commande suivante :  
```
curl http://127.0.0.1:5000
``` 
Le serveur devient alors accessible depuis le navigateur de notre ordinateur.  

Dans notre premier terminal, nous obtenons alors l'affichage suivant :  
<img width="466" height="67" alt="image" src="https://github.com/user-attachments/assets/033812db-e9e5-439a-846d-7862a1f645e4" />  

En entrant l'adresse http://192.168.4.207:5000 sur notre navigateur WEB, nous observons alors :  
<img width="115" height="38" alt="image" src="https://github.com/user-attachments/assets/dc9690a2-e5a1-4b10-b90e-0df84a4dfa25" />  

### Première page REST  

#### Première route  
Dans un premier temps, nous ajoutons au fichier "_hello.py_" le code suivant :  
```
welcome = "Welcome to 3ESE API!"

@app.route('/api/welcome/')
def api_welcome():
    return welcome
    
@app.route('/api/welcome/<int:index>')
def api_welcome_index(index):
    return welcome[index]
```

Le décorateur @app.route sert à associer une URL (un chemin) à une fonction python. 
Le rôle du fragment <int:index> permet de capturer un paramètre dans l'URL et de le passer à la fonction.  
En entrant dans notre navigateur les commandes suivantes, nous obtenons respectivement :  
```
http://192.168.4.207:5000/api/welcome/
```
<img width="198" height="29" alt="image" src="https://github.com/user-attachments/assets/271ab8ba-e327-451a-856d-dadf99ef7798" />
```
http://192.168.4.207:5000/api/welcome/0
```
<img width="24" height="22" alt="image" src="https://github.com/user-attachments/assets/345f35a6-1a98-445b-9ad6-a87bcb2f7ea2" />
```
http://192.168.4.207:5000/api/welcome/1
```
<img width="17" height="16" alt="image" src="https://github.com/user-attachments/assets/2c1526f2-7958-43d3-80eb-7618a401700b" />

REMARQUE : En parallèle des appels faits depuis les navigateurs WEB, nous observons l'affichage des différentes requêtes faites :  
<img width="1425" height="489" alt="image" src="https://github.com/user-attachments/assets/c9b98e04-a89d-4bdf-9fed-511fe7b68a1e" />

#### Première page REST  

##### Réponse JSON  






