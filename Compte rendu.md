# Compte rendu du TP  

## Introduction  

Durant ces TPs de bus & réseaux, l'objectif est de mettre en place le système suivant :  
<img width="1333" height="614" alt="image" src="https://github.com/user-attachments/assets/5432dc45-abcd-4308-863c-21bd4d93261a" />

## Mise en place de la partie I2C du système  

Dans un premier temps, nous allons mettre en place la partie I2C du projet.  
<img width="714" height="208" alt="image" src="https://github.com/user-attachments/assets/b8640915-dbf7-40d3-b937-7191a3778152" />  
<img src="https://github.com/user-attachments/assets/2dd9cfb0-b5ac-4326-9cdd-b6cea80a035f" width="50%">  
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
    - ```BMP280_S32_t bmp280_compensate_T_int32(BMP280_S32_t adc_T)```
    - ```BMP280_U32_t bmp280_compensate_P_int32(BMP280_S32_t adc_P)```
  - Contenu des fonctions :
    - ```C
      // Returns temperature in DegC, resolution is 0.01 DegC. Output value of “5123” equals 51.23 DegC.
      // t_fine carries fine temperature as global value
      BMP280_S32_t t_fine;
      BMP280_S32_t bmp280_compensate_T_int32(BMP280_S32_t adc_T)
      {
          BMP280_S32_t var1, var2, T;
          var1 = ((((adc_T>>3) – ((BMP280_S32_t)dig_T1<<1))) * ((BMP280_S32_t)dig_T2)) >> 11;
          var2 = (((((adc_T>>4) – ((BMP280_S32_t)dig_T1)) * ((adc_T>>4) – ((BMP280_S32_t)dig_T1))) >> 12) *
          ((BMP280_S32_t)dig_T3)) >> 14;
          t_fine = var1 + var2;
          T = (t_fine * 5 + 128) >> 8;
          return T;
      }
      ```
    - ```C
      // Returns pressure in Pa as unsigned 32 bit integer. Output value of “96386” equals 96386 Pa = 963.86 hPa
      BMP280_U32_t bmp280_compensate_P_int32(BMP280_S32_t adc_P)
      {
          BMP280_S32_t var1, var2;
          BMP280_U32_t p;
          var1 = (((BMP280_S32_t)t_fine)>>1) – (BMP280_S32_t)64000;
          var2 = (((var1>>2) * (var1>>2)) >> 11 ) * ((BMP280_S32_t)dig_P6);
          var2 = var2 + ((var1*((BMP280_S32_t)dig_P5))<<1);
          var2 = (var2>>2)+(((BMP280_S32_t)dig_P4)<<16);
          var1 = (((dig_P3 * (((var1>>2) * (var1>>2)) >> 13 )) >> 3) + ((((BMP280_S32_t)dig_P2) * var1)>>1))>>18;
          var1 =((((32768+var1))*((BMP280_S32_t)dig_P1))>>15);
          if (var1 == 0)
          {
              return 0; // avoid exception caused by division by zero
          }
          p = (((BMP280_U32_t)(((BMP280_S32_t)1048576)-adc_P)-(var2>>12)))*3125;
          if (p < 0x80000000)
          {
              p = (p << 1) / ((BMP280_U32_t)var1);
          }
          else
          {
              p = (p / (BMP280_U32_t)var1) * 2;
          }
          var1 = (((BMP280_S32_t)dig_P9) * ((BMP280_S32_t)(((p>>3) * (p>>3))>>13)))>>12;
          var2 = (((BMP280_S32_t)(p>>2)) * ((BMP280_S32_t)dig_P8))>>13;
          p = (BMP280_U32_t)((BMP280_S32_t)p + ((var1 + var2 + dig_P7) >> 4));
          return p;
      }
      ```  

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
```C
HAL_StatusTypeDef HAL_I2C_Master_Transmit(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout)
HAL_StatusTypeDef HAL_I2C_Master_Receive(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout)
```
Avec : 
- I2C_HandleTypeDef hi2c : structure stockant les informations du contrôleur I²C
- uint16_t DevAddress : adresse I³C du périphérique Slave avec lequel on souhaite interagir
- uint8_t *pData : buffer de données
- uint16_t Size : taille du buffer de données
- uint32_t Timeout : peut prendre la valeur HAL_MAX_DELAY

Ces fonctions vont nous permettre d'accéder directement aux différents registres du module et donc d'écrire ou lire des données depuis les registres du module.  

A des fins de lisibilité et de clarté de code, nous décidons de créer les fonctions :  
```C
HAL_StatusTypeDef BMP280_WriteReg(uint8_t reg, uint8_t value);
HAL_StatusTypeDef BMP280_ReadReg(uint8_t reg, uint8_t *value);
HAL_StatusTypeDef BMP280_ReadMulti(uint8_t reg, uint8_t *buf, uint16_t len);
```
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
Pour cela, nous utilisons les informations de la datasheet et écrivons le code correspondant dans une fonction nommée ```bmp280_init()```.  
Après exécution de celle-ci, nous obtenons bien un ID de 0x58 cohérent avec ce qui est écrit dans la datasheet.  

#### Configuration du BMP280  

Suite à cela, nous configurons le module BMP280 afin de spécifier de quelle manière nous voulons utiliser le capteur.  
Dans notre cas à nous : mode normal, Pressure oversampling x16, Temperature oversampling x2.  
Nous ajoutons alors à la fonction ```bmp280_init()``` la configuration du capteur.  

#### Récupération de l'étalonnage, de la température et de la pression  

Afin de récupérer en une fois le contenu des registres d'étalonnages du BMP280, nous écrivons la fonction ```bmp280_read_calibration()```.  
Dans cette fonction, nous remplissons tout simplement le buffer _buf_ via l'appel de fonction ```bmp280_read_registers(&hi2c1, BMP280_REG_CALIB_START, buf, 24)```.  
La fonction _bmp280_read_registers_ va alors remplir le buffer _buf_ en commencant à lire au registre _BMP280_CALIBRATION_ (valant 0xA1) et en incrémentant automatiquement l'adresse du registre 26 fois, soit jusqu'à avoir fini de lire dans l'ensemble des registres d'étalonnage du module.  

Ensuite, nous définissons la fonction ```bmp280_read_raw(int32_t *raw_temp, int32_t *raw_press)```, permettant d'obtenir respectivement les valeurs brutes de température et de pression lues par le capteur, sans traitement.  
Une fois encore, le principe est le même : nous commencons à lire à l'adresse _BMP280_PRESS_MSB_ (valant 0xF7) jusqu'au registre 0xFC, puis nous mettons en forme les données lues dans les variables raw_temp et raw_press, conformément à ce qui est écrit dans la datasheet.  

#### Calcul des températures et des pression compensées  

Pour finir, nous utilisons les fonctions données dans la datasheet page 45 et 46 afin d'appliquer un traitement sur les valeurs brutes de température et de pression mesurées par le capteur, en vue d'obtenir des valeurs compensées les plus correctes possibles.  
Nous reprenons directement le contenu fournit dans la datasheet.  

Nous implémentons alors la fonction ```bmp280_read_temp_press_int(int32_t* temperature_raw_100, uint32_t* pressure_raw_100, int32_t* temperature_compensate_100, uint32_t* pressure_compensate_100)```.  
Dont le fonctionnement est le suivant :  
- on lit les valeurs brutes de température et de pression via l'appel ```bmp280_read_raw(&raw_T, &raw_P)``` et on inscrit le résultat dans les buffers _raw_T_ et _raw_P_
- on applique une compension sur la valeur brute de température mesurée via l'appel ```bmp280_compensate_T_int32(raw_T)```
- on applique une compension sur la valeur brute de pression mesurée via l'appel ```bmp280_compensate_P_int32(raw_P)```
- pour finir, on inscrit les valeurs brutes et compensées de température et de pression en pointant directement sur les variables passées en paramètres :
  ```C
  *temperature_raw_100 = raw_T;
  *pressure_raw_100 = raw_P;
  *temperature_compensate_100 = bmp280_compensate_T_int32(raw_T);
  *pressure_compensate_100    = bmp280_compensate_P_int32(raw_P);
  ```

Une fois tout cela fait, nous testons alors notre fonction dans le fichier "main.c".  
```C
	// Code BMP280 pour capture et affichage de température et pression compensées et non compensées
	bmp280_init();
	bmp280_print_temperature_pressure();
```
Nous obtenons alors le résultat suivant :  
<img width="702" height="62" alt="image" src="https://github.com/user-attachments/assets/8e05e355-69b1-475a-9a83-73dfd6ad7e3c" />  

Nous observons donc que les valeurs brutes ne sont clairement pas exploitables. Les valeurs compensées, quant à elles, sont parfaitement correctes et représentatives de la réalité.  

> NOTE : comme vu en cours, nous utilisons des entiers directement et venons intercaller des virgules afin de donner "l'illusion" de nombres à virgules. Cela nous permet de libérer de l'espace mémoire et est donc moins énergivore.

Cela est fait ici :
```C
	if (bmp280_read_temp_press_int(&temp_raw_100, &press_raw_100, &temp_compensate_100, &press_compensate_100) == HAL_OK)
	{
		printf("\r\n raw temperature = %ld.%02ld degres C, compensate temperature = %ld.%02ld degres C, raw pressure = %lu.%02lu hPa, compensate pressure = %lu.%02lu hPa \r\n",
				temp_raw_100 / 100, temp_raw_100 % 100,
				temp_compensate_100 / 100, temp_compensate_100 % 100,
				press_raw_100 / 100, press_raw_100 % 100,
				press_compensate_100 / 100, press_compensate_100 % 100);
	}
```  

## Mise en place de l'interfaçage STM32-Raspberry  

Dans un second temps, nous allons mettre en place un interfaçage entre notre carte STM32 et Raspberry.  
<img width="866" height="479" alt="image" src="https://github.com/user-attachments/assets/4e31f893-ab4f-4cc3-8550-9a39669d01b6" />  
Nous utiliserons un script Python afin d'interroger la carte STM32 depuis la Raspberry.  

### Mise en route du Raspberry PI Zéro  

#### Préparation du Raspberry et configuration de l'image

Informations saisies lors de la création de l'image via ```Raspberry Pi Imager``` :  
```
- hostname : HugoCFArthurNN
- nom utilisateur : hugoarthur
- mdp : ensea2526
- SSH : C304_DTI_AP
- mdp : ilovelinux
```

#### Premier démarrage  

Nous flashons alors la carte SD avec les configurations faites via Raspberry Pi Imager.  

La Raspberry a obtenu son adresse IP sur le réseau de la même manière qu’un ordinateur classique : via le protocole DHCP.  
Son adresse IP correspond à la théorie vue en cours :  
![OIP NHsl6SOwA1YLv5Rn71-dHwHaFj](https://github.com/user-attachments/assets/d01dba0f-2ff9-4451-8b7e-074d66e4c709)  

Nous nous connectons alors à notre Raspberry PI Zero en suivant le protocole suivant :  
```
- ouverture du terminal de cmd Windows
- ecrire dans le terminal : ssh hugoarthur@192.168.4.207
- ecrire dans le terminal le mdp : ensea2526
```

Nous obtenons alors l'interface suivante :
<img width="1184" height="222" alt="image" src="https://github.com/user-attachments/assets/4fa3d9a4-2ad5-4112-aebc-d88fb25c0e67" />  
  
### Port série  

#### Loopback  

Dans un premier temps, nous rebouclons la pin RX sur la pin TX.
![BOARD-Layout-CMPLETE_800x506-768x486](https://github.com/user-attachments/assets/c5023909-3ba7-494d-9369-463907a953ff)  

Nous installons minicom via les commandes :  
```
sudo apt update  
sudo apt install minicom
```    

Suite à cela, nous écrivons dans le terminal de cmd Windows :  
```
sudo minicom -D /dev/ttyS0
```

Cela nous permet alors de configurer le port série. Nous le configurons de la manière suivante (en pressant CTRL+A suivi de O) :  
<img width="892" height="429" alt="image" src="https://github.com/user-attachments/assets/f1b7800b-caa1-465a-a40a-2741d357a99f" />  

Nous pouvons effectivement écrire et visualiser les caractères écrits en même temps : 
<img width="872" height="362" alt="image" src="https://github.com/user-attachments/assets/8d1e3471-f603-4273-9bae-c4aa8c7cd734" />

#### Communication avec la STM32

Nous voulons établir le protocole suivant entre la Raspberry PI et la STM32 :  
<img width="680" height="195" alt="image" src="https://github.com/user-attachments/assets/d8f5fedb-7882-42f6-9069-33e5ce2f92e2" />  

Pour ce faire, nous ajoutons les fichiers ```interface_stm32_raspberry.c``` et ```interface_stm32_raspberry.h```.  
Nous implémentons alors dans le fichier ```interface_stm32_raspberry.c``` les fonctions suivantes :  
- ```interface_stm32_raspberry_process_command(char *cmd)``` : permettant l'interface entre la Raspberry PI et la STM32
- ```HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)``` : permettant de traiter la commande reçue suite à une interruption déclenchée via l'USART  

Le fonctionnement est donc le suivant : depuis le terminal de la Raspberry PI, nous pouvons désormais entrer une des requêtes RPI citées précédemment. Cette requête est alors transmise via USART jusqu'à la STM32, qui reçoit alors cette requête la traite et retourne une réponse semblable à celles citées précédemment via USART.  

Nous testons maintenant le bon fonctionnement du protocole.  
Nous écrivons dans le terminal cmd Windows la commande suivante :  
```
GET_T
```
Nous obtenons alors :  
<img width="104" height="22" alt="image" src="https://github.com/user-attachments/assets/f7edeca8-05ef-4f49-8584-46e586ca6539" />  
Et si nous écrivons dans le terminal cmd Windows la commande suivante :  
```
GET_P
```
Nous obtenons alors :  
<img width="162" height="21" alt="image" src="https://github.com/user-attachments/assets/e4a5a068-ee40-46b2-850a-920bbd51adf6" />  

Le protocole STM32-RPI fonctionne donc parfaitement !  

### Commande depuis Python  

Nous commençons par installer Python sur la Raspberry via les commandes suivantes :  
```
sudo apt update
sudo apt install python3-pip
```  

Suite à cela, nous installons pyserial afin de pouvoir communiquer en USART entre la Raspberry PI et la STM32 via la commande suivante :  
`pip3 install pyserial`  

Nous avons bien installé la bibliothèque pyserial :  
<img width="687" height="287" alt="image" src="https://github.com/user-attachments/assets/6627dfae-4acf-47bb-9488-4abc0b7b5374" />  

Suite à cela, nous créons à l'emplacement suivant : `home/hugoarthur` le fichier suivant : `interface_stm32_raspberry.py`.  
Dans ce fichier nous insérons alors le code suivant :  
```PYTHON
#!/usr/bin/env python3
import serial
import time
import sys

def open_serial(port="/dev/ttyAMA0", baudrate=115200):
    try:
        ser = serial.Serial(port, baudrate, timeout=1, write_timeout=1)
        print(f"[OK] Port ouvert : {port} @ {baudrate} bauds")
        return ser
    except Exception as e:
        print(f"[ERREUR] Impossible d'ouvrir le port série : {e}")
        sys.exit(1)

def send_command(ser, cmd):
    ser.write((cmd + "\n").encode())
    time.sleep(0.05)
    response = ser.read_all().decode(errors="ignore")
    print(f"→ Réponse à {cmd} : {response if response else '(aucune)'}")
    return response

def menu(ser):
    while True:
        print("""
===========================
   Communication STM32
===========================
1 - Obtenir température (GET_T)
2 - Obtenir pression (GET_P)
0 - Quitter
""")
        choix = input("Choix : ").strip()
        if choix == "1":
            send_command(ser, "GET_T")
        elif choix == "2":
            send_command(ser, "GET_P")
        elif choix == "0":
            print("Fermeture et sortie…")
            return
        else:
            print("Choix invalide.")

if __name__ == "__main__":
    ser = open_serial("/dev/ttyS0", 115200)
    try:
        menu(ser)
    finally:
        ser.close()
        print("[OK] Port série fermé.")
```  

Suite à cela, nous testons alors le résultat en écrivant dans le shell :  
`python3 interface_stm32_raspberry.py`  

Nous obtenons alors :  
<img width="798" height="256" alt="image" src="https://github.com/user-attachments/assets/a3cffca0-c799-4592-85e7-08f50ea8d502" />  

En écrivant dans le shell :   
`1`  
Nous obtenons :  
<img width="307" height="76" alt="image" src="https://github.com/user-attachments/assets/fc8f8b09-fcb4-4c4b-b463-42fdd41c13a3" />  

Et en écrivant dans le shell :  
`2`  
Nous obtenons :  
<img width="310" height="77" alt="image" src="https://github.com/user-attachments/assets/b7566df6-19e9-4441-af83-97d0632d183c" />

Ainsi, notre communication via Python entre la Raspberry PI et la STM32 fonctionne bel et bien !  

## Interface REST  

Nous mettons maintenant en place une interface REST (Representational State Transfer) sur le Raspberry. 
<img width="524" height="459" alt="image" src="https://github.com/user-attachments/assets/55da32c4-c9f3-41c8-b604-ba4e23a27ff2" />  
Nous réaliserons cela via Python depuis la Raspberry.  

### Installation du serveur Python  

#### Installation 

Nous passons l'étape de création d'un utilisateur différent de pi, puisque nous somme déjà logé sous le nom de ```hugoarthur```.  
<img width="441" height="50" alt="image" src="https://github.com/user-attachments/assets/24b7418c-f2c4-45fa-9f8f-d218a7a6eb17" />  

Une fois loggé dans notre session et python 3 installé, nous réalisons les opérations suivantes :  
- 1° : nous créons un répertoire nommé ```restserver``` depuis le chemin `/home/hugoarthur` via la commande :  
  - ```
    mkdir restserver
    ```
    Nous nous mettons alors dans le répertoire suivant : `/home/hugoarthur/restserver`  
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
`http://192.168.4.207:5000/api/welcome/`  
<img width="198" height="29" alt="image" src="https://github.com/user-attachments/assets/271ab8ba-e327-451a-856d-dadf99ef7798" />  
`http://192.168.4.207:5000/api/welcome/0`  
<img width="24" height="22" alt="image" src="https://github.com/user-attachments/assets/345f35a6-1a98-445b-9ad6-a87bcb2f7ea2" />  
`http://192.168.4.207:5000/api/welcome/1`    
<img width="17" height="16" alt="image" src="https://github.com/user-attachments/assets/2c1526f2-7958-43d3-80eb-7618a401700b" />  

> REMARQUE : En parallèle des appels faits depuis les navigateurs WEB, nous observons l'affichage des différentes requêtes faites :  
<img width="1425" height="489" alt="image" src="https://github.com/user-attachments/assets/c9b98e04-a89d-4bdf-9fed-511fe7b68a1e" />

#### Première page REST  

##### Réponse JSON  

Nous allons maintenant nous interesseer au module JSON.  
Un module JSON est un composant logiciel (souvent une bibliothèque) qui permet de lire, écrire, analyser et manipuler des données au format JSON (JavaScript Object Notation).  

Afin de générer du JSON, nous utilisons la fonction python _json.dumps()_ en insérant la ligne suivante dans la fonction _api_welcome_index_ :  
```
return json.dumps({"index": index, "val": welcome[index]})
```
à la place de la ligne :  
```
return welcome[index]
```

Lorsque nous entrons la commande suivante dans notre navigateur :  
```
http://192.168.4.207:5000/api/welcome/1
```
Nous obtenons le résultat suivant en utilisant les outils de développement (accessible via F12) :  
<img width="1854" height="877" alt="image" src="https://github.com/user-attachments/assets/e66b8988-9d89-42d2-8a99-ef38575c8d0a" />
Nous observons donc qu'il s'agit d'un type html et non d'un type JSON.  

##### 1ère solution  

Nous remplaçons la ligne précédente :  
```
return json.dumps({"index": index, "val": welcome[index]})
```
Par la ligne suivante :  
```
return json.dumps({"index": index, "val": welcome[index]}), {"Content-Type": "application/json"}
```
Nous obtenons maintenant le résultat suivant :  
<img width="1851" height="878" alt="image" src="https://github.com/user-attachments/assets/5433a71f-41ce-4f81-a6b7-021301b00917" />  
Il s'agit bien d'une réponse JSON !  

##### 2ème solution  

Nous remplaçons maintenant la ligne :  
```
return json.dumps({"index": index, "val": welcome[index]})
```
Par la ligne suivante :  
```
return jsonify({"index": index, "val": welcome[index]})
```
Nous obtenons alors :  
<img width="1850" height="868" alt="image" src="https://github.com/user-attachments/assets/38b0d41a-0cb1-4e47-b1b7-213a34063a40" />  
Il s'agit à nouveau bel et bien d'une réponse JSON !  

##### Erreur 404  

Nous téléchargons d'abord le fichier "_page_not_found.html_" et le téléversons dans le dossier "_templates_".
Nous ajoutons maitenant dans le fichier "_hello.py_" le code suivant :  
```
@app.errorhandler(404)
def page_not_found(error):
    return render_template('page_not_found.html'), 404
```
Et modifions la fonction  _api_welcome_index_ de manière à générer une erreur 404 si l'index entré n'est pas correct. Voici les modifications apportées :  
```
@app.route('/api/welcome/<int:index>')
def api_welcome_index(index):
    if (index<0 or index>len(welcome)):
        abort(404)
    return jsonify({"index": index, "val": welcome[index]})
```

### Nouvelles méthodes HTTP  

#### Méthodes POST, PUT, DELETE...  

##### Méthode POST  

Nous entrons dans notre terminal la ligne suivante :  
```
curl -X POST http://192.168.4.207:5000/api/welcome/14
```
Nous obtenons alors :  
<img width="1140" height="141" alt="image" src="https://github.com/user-attachments/assets/69968181-5065-439c-ada1-7148cac06d5f" />

Nous ajoutons à notre fichier "_hello.py_" le code suivant :  
```
@app.route('/api/request/', methods=['GET', 'POST'])
@app.route('/api/request/<path>', methods=['GET','POST'])
def api_request(path=None):
    resp = {
            "method":   request.method,
            "url" :  request.url,
            "path" : path,
            "args": request.args,
            "headers": dict(request.headers),
    }
    if request.method == 'POST':
        resp["POST"] = {
                "data" : request.get_json(),
                }
    return jsonify(resp)
```

Suite à cela, nous utilisons l'extension Firefox _RESTED_ afin d'interroger notre serveur.  
Nous obtenons alors :  
<img width="1215" height="778" alt="image" src="https://github.com/user-attachments/assets/09a3996a-a649-4acf-a1ca-ac95c899607e" />  

#### API CRUD  

Dans le fichier `hello.py`, nous plaçons le code suivant :  
```PYTHON
from flask import Flask, jsonify, abort, send_from_directory, render_template
import serial
import time
import sys

app = Flask(__name__)

# --------------------
# SERIAL COMMUNICATION
# --------------------

def open_serial(port="/dev/ttyAMA0", baudrate=115200):
    try:
        ser = serial.Serial(port, baudrate, timeout=1, write_timeout=1)
        print(f"[OK] Port série ouvert : {port} @ {baudrate}")
        return ser
    except Exception as e:
        print(f"[ERREUR] Port série : {e}")
        sys.exit(1)


def send_command(cmd):
    ser.write((cmd + "\n").encode())
    time.sleep(0.05)
    response = ser.read_all().decode(errors="ignore").strip()
    print(f"→ {cmd} → {response}")
    return response


# Ouverture du port série UNE SEULE FOIS
ser = open_serial("/dev/ttyAMA0", 115200)


# --------------------
# DATA STORAGE
# --------------------

temperatures = []
pressures = []
scale = 1.0


# --------------------
# CREATE
# --------------------

@app.route('/temp/', methods=['POST'])
def create_temperature():
    response = send_command("GET_T")

    try:
        value = float(response)
    except ValueError:
        return jsonify({"error": "Invalid temperature from STM32"}), 500

    temperatures.append(value)
    return jsonify({
        "message": "Temperature retrieved",
        "index": len(temperatures) - 1,
        "value": value
    }), 201


@app.route('/pres/', methods=['POST'])
def create_pressure():
    response = send_command("GET_P")

    try:
        value = float(response)
    except ValueError:
        return jsonify({"error": "Invalid pressure from STM32"}), 500

    pressures.append(value)
    return jsonify({
        "message": "Pressure retrieved",
        "index": len(pressures) - 1,
        "value": value
    }), 201


# --------------------
# RETRIEVE
# --------------------

@app.route('/temp/', methods=['GET'])
def get_all_temperatures():
    return jsonify(temperatures)


@app.route('/temp/<int:x>', methods=['GET'])
def get_temperature(x):
    if x < 0 or x >= len(temperatures):
        abort(404)
    return jsonify({"index": x, "value": temperatures[x]})


@app.route('/pres/', methods=['GET'])
def get_all_pressures():
    return jsonify(pressures)


@app.route('/pres/<int:x>', methods=['GET'])
def get_pressure(x):
    if x < 0 or x >= len(pressures):
        abort(404)
    return jsonify({"index": x, "value": pressures[x]})


@app.route('/scale/', methods=['GET'])
def get_scale():
    return jsonify({"scale": scale})


@app.route('/angle/', methods=['GET'])
def get_angle():
    if not temperatures:
        return jsonify({"error": "No temperature available"}), 400

    angle = temperatures[-1] * scale
    return jsonify({
        "temperature": temperatures[-1],
        "scale": scale,
        "angle": angle
    })


# --------------------
# UPDATE
# --------------------

@app.route('/scale/<int:x>', methods=['POST'])
def update_scale(x):
    global scale
    data = app.current_request.get_json(silent=True)

    if not data or 'value' not in data:
        return jsonify({"error": "Missing 'value'"}), 400

    scale = data['value']
    return jsonify({
        "message": f"Scale updated for {x}",
        "scale": scale
    })


# --------------------
# DELETE
# --------------------

@app.route('/temp/<int:x>', methods=['DELETE'])
def delete_temperature(x):
    if x < 0 or x >= len(temperatures):
        abort(404)

    return jsonify({
        "message": "Temperature deleted",
        "value": temperatures.pop(x)
    })


@app.route('/pres/<int:x>', methods=['DELETE'])
def delete_pressure(x):
    if x < 0 or x >= len(pressures):
        abort(404)

    return jsonify({
        "message": "Pressure deleted",
        "value": pressures.pop(x)
    })


# --------------------
# ERRORS
# --------------------

@app.errorhandler(404)
def not_found(error):
    return jsonify({"error": "Not found"}), 404


# --------------------
# MAIN
# --------------------

if __name__ == '__main__':
    try:
        app.run(debug=True)
    finally:
        ser.close()
        print("[OK] Port série fermé")
```

> Nous le testerons plus tard, lors de l'intégration finale...  

## Bus CAN  

Notre objectif est maitenant de mettre en place une API Rest et un périphérique sur bus CAN.  
Nous nous focalisons donc sur la partie système suivante :  
<img width="682" height="258" alt="image" src="https://github.com/user-attachments/assets/d3ff5e7c-1c91-4d84-ae3c-9c4cc6fd5bc5" />  

Notre STM32L476RG possède un CAN intégré mais nécessite un transceiver afin de faire l'interface entre la STM32 et le bus CAN.  
Notre modèle est le TJA1050 dont la datasheet se trouve dans le dossier "Ressources".  

Nous commençons par activer le CAN dans le ".ioc". Nous voulons une **vitesse CAN de 500kbit/s PRECISEMENT**.  
A l'aide du calculateur en ligne fournit dans le sujet nous configurons en conséquent le fichier ".ioc" de la manière suivante :  
<img width="445" height="195" alt="image" src="https://github.com/user-attachments/assets/3372f05b-60ce-4d72-a2a4-d5e99279fba0" />
<img width="529" height="494" alt="image" src="https://github.com/user-attachments/assets/6bb014cf-a47e-4d99-a16a-2b76055bf3fa" />  

### Pilotage du moteur  

Nous nous intéressons maintenant au pilotage du moteur via bus CAN.  

Notre setup est le suivant :  
![7cc50a0e-8551-474a-9a90-8a0d9741bd2f~1](https://github.com/user-attachments/assets/bb86aa08-a417-4158-90a8-c812e793cdac)  

Pour ce faire, nous commençons d'abord par initialiser une structure correspondant au header du message TX que nous allons transmettre sur le bus CAN.  
Nous le configurons comme suit :  
```C
CAN_TxHeaderTypeDef tx_header =
{
    .StdId = 0x61,
    .TransmitGlobalTime = DISABLE,
    .IDE = CAN_ID_STD,
    .RTR = CAN_RTR_DATA,
    .DLC = 1
};
``` 
D'après STM32CubeIDE, nous obtenons les informations suivantes quant à la structure _CAN_TxHeaderTypeDef_ :  
```C
typedef struct
{
  uint32_t StdId;    /*!< Specifies the standard identifier.
                          This parameter must be a number between Min_Data = 0 and Max_Data = 0x7FF. */

  uint32_t ExtId;    /*!< Specifies the extended identifier.
                          This parameter must be a number between Min_Data = 0 and Max_Data = 0x1FFFFFFF. */

  uint32_t IDE;      /*!< Specifies the type of identifier for the message that will be transmitted.
                          This parameter can be a value of @ref CAN_identifier_type */

  uint32_t RTR;      /*!< Specifies the type of frame for the message that will be transmitted.
                          This parameter can be a value of @ref CAN_remote_transmission_request */

  uint32_t DLC;      /*!< Specifies the length of the frame that will be transmitted.
                          This parameter must be a number between Min_Data = 0 and Max_Data = 8. */

  FunctionalState TransmitGlobalTime; /*!< Specifies whether the timestamp counter value captured on start
                          of frame transmission, is sent in DATA6 and DATA7 replacing pData[6] and pData[7].
                          @note: Time Triggered Communication Mode must be enabled.
                          @note: DLC must be programmed as 8 bytes, in order these 2 bytes are sent.
                          This parameter can be set to ENABLE or DISABLE. */

} CAN_TxHeaderTypeDef;
```
Explication de la configuration du driver : 
- .StdId = 0x61 : correspond à l'ID du moteur pas-à-pas
- .TransmitGlobalTime = DISABLE : permet de mesurer les temps de réponse du bus CAN. Nous ne l'utilisons pas. Nous le configurons donc à DISABLE
- .IDE = CAN_ID_STD : permet d'identifier le type d'identifiant du message qui va être transmis. Dans notre cas à nous, il s'agit d'un message standart => CAN_ID_STD
- .RTR = CAN_RTR_DATA : permet de spécifier le type de trame qui va être transmis via le bus CAN. Dans notre cas : CAN_RTR_DATA (trame avec données payload)
- .DLC = 1 : permet de spécifier la longueur de la trame que l'on va transmettre. Dans notre cas : 1

Une fois le driver configuré, nous écrivons alors les deux fonctions aux prototypes suivants :  
```C
void motor_command_send(int8_t angle_cmd)
void motor_test_loop(void)
```
Permettant respectivement de :  
- faire tourner le moteur d'_angle_cmd_ par rapport au 0°
- faire tourner le moteur de +90°, attendre 1s, faire tourner le moteur de -90° et attendre une seconde

Pour tester le bon fonctionnement de notre code, nous ajoutons alors la ligne de code suivante dans le fichier "_main.c_" :  
```C
motor_test_loop();
```
Nous observons alors que le moteur fonctionne bel et bien comme désiré :  
![PXL_20251210_141720348(2)](https://github.com/user-attachments/assets/f8dd9cd3-7fd7-4323-b0ed-a0b7ed6158ce)  

### Interfaçage avec le capteur  

Nous voulons maintenant que le moteur tourne de manière proportionnelle à la valeur de température qui lui est fournie.  

Nous écrivons alors la fonction suivante `motor_temperature_to_angle` dont le code est :  
```C
#define TEMP_MIN   (-20.0f) // °C
#define TEMP_MAX   (80.0f)  // °C

#define ANGLE_MIN  (-180)   // En °
#define ANGLE_MAX  (180)    // En °

void motor_temperature_to_angle(float temperature)
{
    float angle;

    /* Saturation température */
    if (temperature < TEMP_MIN)
        temperature = TEMP_MIN;
    if (temperature > TEMP_MAX)
        temperature = TEMP_MAX;

    angle = (temperature - TEMP_MIN) *
            (ANGLE_MAX - ANGLE_MIN) /
            (TEMP_MAX - TEMP_MIN) +
            ANGLE_MIN;

    motor_command_send(angle);
}
```  

## Intégration  

Nous allons maintenant réaliser l'intégration de l'ensemble du travail fournit durant les séances de TP.  

Nous commençons par créer une page HTML intitulée `interface_stm32_raspberry.html` afin de pouvoir réaliser des requêtes directment depuis cette page.  

Nous ajoutons, en parallèle de cela, dans le fichier `hello.py` le morceau de code suivant :  
```PYTHON
# --------------------
# HTML
# --------------------

@app.route('/')
def index():
    return send_from_directory('.', 'interface_stm32_raspberry.html')
```
C'est ce morceau de code qui va nous permettre d'accéder à la page web que nous avons depuis notre naviguateur personnel directment.  

Malgré de nombreux efforts faits, nous n'avons malheuresement pas réussi à aboutir avec l'étape d'intégration.  
