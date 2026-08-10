#ifndef MODELS_H
#define MODELS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LOGIN_LEN 7
#define MAX_NAME_LEN 50
#define MAX_PHONE_LEN 20
#define MAX_ADDR_LEN 100
#define MAX_EMAIL_LEN 50
#define MAX_PASS_LEN 64
#define MAX_ROLE_LEN 10
#define MAX_STATE_LEN 15
#define MAX_DATE_LEN 20
#define MAX_CODE_LEN 30
#define MAX_DESC_LEN 200


#define DIR_DATABASE "DATABASE"
#define DIR_REPORTS "REPORTS"
#define DIR_REPORTS_ORDERS "REPORTS/ORDERS"
#define DIR_REPORTS_DELIVERIES "REPORTS/DELIVERIES"
#define DIR_REPORTS_DAILY "REPORTS/DAILY"

#define FILE_USERS "DATABASE/USERS.dat"
#define FILE_PRODUCERS "DATABASE/PRODUCERS.dat"
#define FILE_CATEGORIES "DATABASE/CATEGORIES.dat"
#define FILE_PRODUCTS "DATABASE/PRODUCTS.dat"
#define FILE_ORDERS "DATABASE/ORDERS.dat"
#define FILE_DELIVERIES "DATABASE/DELIVERIES.dat"
#define FILE_RESERVATIONS "DATABASE/RESERVATIONS.dat"
#define FILE_LOSSES "DATABASE/LOSSES.dat"
#define FILE_HISTORY "REPORTS/HISTORY.txt"

typedef struct {
    int id;
    char nom[MAX_NAME_LEN];
    char prenom[MAX_NAME_LEN];
    char telephone[MAX_PHONE_LEN];
    char adresse[MAX_ADDR_LEN];
    char email[MAX_EMAIL_LEN];
    char login[MAX_LOGIN_LEN]; 
    char password[MAX_PASS_LEN]; 
    char role[MAX_ROLE_LEN]; 
    char etat[MAX_STATE_LEN]; 
    char date_creation[MAX_DATE_LEN];
    char date_derniere_connexion[MAX_DATE_LEN];
    int first_login; 
} User;

typedef struct {
    int id;
    char nom_complet[MAX_NAME_LEN * 2];
    char region[MAX_NAME_LEN];
    char telephone[MAX_PHONE_LEN];
    char description_cultures[MAX_DESC_LEN];
    int nb_produits_enregistres;
} Producer;

typedef struct {
    int id;
    char libelle[MAX_NAME_LEN];
    char description[MAX_DESC_LEN];
    char date_creation[MAX_DATE_LEN];
} Category;

typedef struct {
    int id;
    char codeProduit[MAX_CODE_LEN]; 
    char designation[MAX_NAME_LEN * 2];
    int idProducteur;
    int idCategorie;
    char unite_mesure[MAX_PHONE_LEN]; 
    double prix_unitaire; 
    double quantite_totale_stock;
    double quantite_disponible;
    char emplacement[MAX_NAME_LEN];
    int duree_conservation; 
    char date_recolte[MAX_DATE_LEN];
} Product;

typedef struct {
    int id;
    char numero_commande[MAX_CODE_LEN]; 
    int idUtilisateur;
    int idProduit;
    double quantite_commandee;
    double montant_total;
    char date_commande[MAX_DATE_LEN];
    char date_prevue_livraison[MAX_DATE_LEN];
    char etat[MAX_STATE_LEN]; 
} Order;


typedef struct {
    int idLivraison;
    int idCommande;
    char date_reelle_livraison[MAX_DATE_LEN];
    int nb_jours_retard;
    double montant_penalite;
} Delivery;


typedef struct {
    int id;
    int idUtilisateur;
    int idProduit;
    double quantite_reservee;
    char date_reservation[MAX_DATE_LEN];
    char etat[MAX_STATE_LEN]; ÉE
} Reservation;


typedef struct {
    int id;
    int idUtilisateur; 
    int idCommande;
    double nb_jours_retard_ou_quantite_avariee;
    double montant_financier;
    char date_enregistrement[MAX_DATE_LEN];
} Loss;

#endif 
