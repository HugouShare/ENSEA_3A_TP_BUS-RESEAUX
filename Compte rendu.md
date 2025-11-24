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
  - nosjdc
