# Compte rendu du TP  

## Introduction  
Durant ces TPs de bus & réseaux, l'objectif est de mettre en place le système suivant :  
<img width="1333" height="614" alt="image" src="https://github.com/user-attachments/assets/5432dc45-abcd-4308-863c-21bd4d93261a" />

## Mise en place de la partie I2C du système  
Dans un premier temps, nous allons mettre en place la partie I2C du projet.  
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
  - <img width="1012" height="491" alt="image" src="https://github.com/user-attachments/assets/aeae1bc2-6f52-4e1f-8372-a8b8b6ad733a" />  
  - Ainsi, les valeurs de calibrations sont contenues dans les registres 0xA1 jusque 0x88
- Registre et valeur contenant la température :
  - Nous la trouvons page 27 de la datasheet
  - <img width="1003" height="412" alt="image" src="https://github.com/user-attachments/assets/555e1898-cf1a-4148-b814-3edacf96738e" />  
  - Ainsi, les valeurs de calibrations sont contenues dans les registres 0xFA jusque 0xFC et se nomme "temp"
- Registre et valeur contenant la pression :
  - Nous la trouvons page 26 de la datasheet
  - <img width="1007" height="417" alt="image" src="https://github.com/user-attachments/assets/f1e04610-0a8c-4655-9bdc-58421fed378e" />  
  - Ainsi, les valeurs de calibrations sont contenues dans les registres 0xF7 jusque 0xF9 et se nomme "press"
- Fonctions permettant le calcul de la température et de la pression compensées, en format entier 32 bits :
  - Nous les trouvons page 45 et 46 de la datasheet
  - Les fonctions ont les prototypes suivants :
    - BMP280_S32_t bmp280_compensate_T_int32(BMP280_S32_t adc_T)
    - BMP280_U32_t bmp280_compensate_P_int32(BMP280_S32_t adc_P)
  - <img width="935" height="610" alt="image" src="https://github.com/user-attachments/assets/06d8ba80-5315-4519-a674-fa952471575e" />
  - <img width="953" height="498" alt="image" src="https://github.com/user-attachments/assets/cc29adfa-8975-487a-b960-b28b5b4f4a08" />

### Setup du STM32  
Nous configurons maintenant notre carte de développement STM. Il s'agit d'une NUCLEO-F446RE.  
Voici le pinout de la carte :  
<img width="486" height="430" alt="image" src="https://github.com/user-attachments/assets/ef70ffe8-98e6-4a36-8700-6dcad772d5dc" />  

Après configuration de la carte NUCLEO, nous avons le fichier .ioc suivant :  
<img width="626" height="593" alt="image" src="https://github.com/user-attachments/assets/379734e7-69cd-4aee-b1a9-441d4edb5905" />

Afin de pouvoir déboguer à l'aide de la fonction printf, nous ajoutons le bout de code donné dans le sujet dans le fichier "_stm32f4xx_hal_msp.c_".  

### Communications I2C  
iosic

