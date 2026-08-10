#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/models.h"
#include "../include/utils.h"
#include "../include/auth.h"
#include "../include/producers.h"
#include "../include/categories.h"
#include "../include/products.h"
#include "../include/orders.h"
#include "../include/deliveries.h"
#include "../include/reservations.h"
#include "../include/losses.h"
#include "../include/reports.h"

void menu_users_admin(void) {
    int choice;
    do {
        print_header("GESTION DES UTILISATEURS (ADMIN)");
        printf("1. Ajouter un nouvel utilisateur / administrateur\n");
        printf("2. Afficher tous les utilisateurs\n");
        printf("3. Changer le statut d'un utilisateur (ACTIF / BLOQUE)\n");
        printf("0. Retour au menu principal\n\n");

        choice = read_int("Votre choix : ");
        switch (choice) {
            case 1: admin_register_user(); pause_screen(); break;
            case 2: admin_list_users(); pause_screen(); break;
            case 3: admin_toggle_user_status(); pause_screen(); break;
            case 0: break;
            default: printf("[ERREUR] Choix invalide.\n"); pause_screen(); break;
        }
    } while (choice != 0);
}

void menu_admin(void) {
    int choice;
    do {
        clear_screen();
        print_header("MENU PRINCIPAL ADMINISTRATEUR");
        printf("Connecte en tant que : %s %s (%s) [ADMIN]\n\n",
               g_current_user.nom, g_current_user.prenom, g_current_user.login);

        printf("1.  Gestion des Utilisateurs\n");
        printf("2.  Gestion des Producteurs / Fournisseurs\n");
        printf("3.  Gestion des Categories de produits\n");
        printf("4.  Gestion des Produits Agricoles (Stock, SKU, Prix)\n");
        printf("5.  Gestion des Commandes\n");
        printf("6.  Gestion des Livraisons & Receptions (Calcul des penalites)\n");
        printf("7.  Gestion des Reservations de Recoltes\n");
        printf("8.  Gestion des Pertes & Penalites (Produits avaries / retards)\n");
        printf("9.  Rapports & Historique (REPORTS/HISTORY.txt & Etat Journalier)\n");
        printf("10. Modifier mon mot de passe\n");
        printf("0.  Se deconnecter\n\n");

        choice = read_int("Votre choix : ");
        switch (choice) {
            case 1: menu_users_admin(); break;
            case 2: {
                int c2;
                do {
                    print_header("GESTION DES PRODUCTEURS");
                    printf("1. Ajouter un producteur\n2. Lister les producteurs\n3. Modifier un producteur\n4. Supprimer un producteur\n0. Retour\n\n");
                    c2 = read_int("Votre choix : ");
                    if (c2 == 1) { add_producer(); pause_screen(); }
                    else if (c2 == 2) { list_producers(); pause_screen(); }
                    else if (c2 == 3) { edit_producer(); pause_screen(); }
                    else if (c2 == 4) { delete_producer(); pause_screen(); }
                } while (c2 != 0);
                break;
            }
            case 3: {
                int c3;
                do {
                    print_header("GESTION DES CATEGORIES");
                    printf("1. Ajouter une categorie\n2. Lister les categories\n3. Modifier une categorie\n0. Retour\n\n");
                    c3 = read_int("Votre choix : ");
                    if (c3 == 1) { add_category(); pause_screen(); }
                    else if (c3 == 2) { list_categories(); pause_screen(); }
                    else if (c3 == 3) { edit_category(); pause_screen(); }
                } while (c3 != 0);
                break;
            }
            case 4: {
                int c4;
                do {
                    print_header("GESTION DES PRODUITS AGRICOLES");
                    printf("1. Ajouter un produit\n2. Lister les produits\n3. Modifier un produit\n4. Reapprovisionner le stock (Declenche la reservation)\n0. Retour\n\n");
                    c4 = read_int("Votre choix : ");
                    if (c4 == 1) { add_product(); pause_screen(); }
                    else if (c4 == 2) { list_products(); pause_screen(); }
                    else if (c4 == 3) { edit_product(); pause_screen(); }
                    else if (c4 == 4) { restock_product(); pause_screen(); }
                } while (c4 != 0);
                break;
            }
            case 5: {
                int c5;
                do {
                    print_header("GESTION DES COMMANDES");
                    printf("1. Creer une nouvelle commande\n2. Lister toutes les commandes\n0. Retour\n\n");
                    c5 = read_int("Votre choix : ");
                    if (c5 == 1) { create_order(); pause_screen(); }
                    else if (c5 == 2) { list_orders(); pause_screen(); }
                } while (c5 != 0);
                break;
            }
            case 6: {
                int c6;
                do {
                    print_header("GESTION DES LIVRAISONS ET RECEPTIONS");
                    printf("1. Confirmer / Enregistrer une livraison\n2. Historique des livraisons\n0. Retour\n\n");
                    c6 = read_int("Votre choix : ");
                    if (c6 == 1) { process_delivery(); pause_screen(); }
                    else if (c6 == 2) { list_deliveries(); pause_screen(); }
                } while (c6 != 0);
                break;
            }
            case 7: {
                int c7;
                do {
                    print_header("GESTION DES RESERVATIONS DE RECOLTES");
                    printf("1. Creer une reservation (Stock = 0)\n2. Lister toutes les reservations\n0. Retour\n\n");
                    c7 = read_int("Votre choix : ");
                    if (c7 == 1) { create_reservation(); pause_screen(); }
                    else if (c7 == 2) { list_reservations(); pause_screen(); }
                } while (c7 != 0);
                break;
            }
            case 8: {
                int c8;
                do {
                    print_header("GESTION DES PERTES ET PENALITES");
                    printf("1. Declarer une perte / produit avarie\n2. Lister toutes les pertes et penalites\n0. Retour\n\n");
                    c8 = read_int("Votre choix : ");
                    if (c8 == 1) { add_manual_spoilage_loss(); pause_screen(); }
                    else if (c8 == 2) { list_losses(); pause_screen(); }
                } while (c8 != 0);
                break;
            }
            case 9: {
                int c9;
                do {
                    print_header("RAPPORTS ET HISTORIQUE");
                    printf("1. Consulter l'historique global (REPORTS/HISTORY.txt)\n2. Generer le rapport / etat journalier (REPORTS/DAILY/)\n0. Retour\n\n");
                    c9 = read_int("Votre choix : ");
                    if (c9 == 1) { view_history(); pause_screen(); }
                    else if (c9 == 2) { generate_daily_report(); pause_screen(); }
                } while (c9 != 0);
                break;
            }
            case 10: {
                print_header("MODIFICATION DU MOT DE PASSE");
                char p1[MAX_PASS_LEN], p2[MAX_PASS_LEN];
                read_password_masked("Nouveau mot de passe : ", p1, sizeof(p1));
                read_password_masked("Confirmation : ", p2, sizeof(p2));
                if (strcmp(p1, p2) == 0 && strlen(p1) >= 4) {
                    change_password(&g_current_user, p1);
                } else {
                    printf("[ERREUR] Mots de passe non identiques ou trop court (min 4 car).\n");
                }
                pause_screen();
                break;
            }
            case 0:
                logout_user();
                break;
            default:
                printf("[ERREUR] Choix invalide.\n");
                pause_screen();
                break;
        }
    } while (choice != 0);
}

void menu_user(void) {
    int choice;
    do {
        clear_screen();
        print_header("MENU UTILISATEUR / ACHATEUR");
        printf("Bienvenue %s %s (Login: %s)\n\n",
               g_current_user.nom, g_current_user.prenom, g_current_user.login);

        printf("1. Consulter le catalogue des produits agricoles\n");
        printf("2. Passer une commande / achat (Limite max 3 commandes actives)\n");
        printf("3. Reserver une recolte future (Seulement si Stock = 0)\n");
        printf("4. Consulter mes commandes\n");
        printf("5. Consulter mes reservations de recoltes\n");
        printf("6. Modifier mon mot de passe\n");
        printf("0. Se deconnecter\n\n");

        choice = read_int("Votre choix : ");
        switch (choice) {
            case 1:
                list_products();
                pause_screen();
                break;
            case 2:
                create_order();
                pause_screen();
                break;
            case 3:
                create_reservation();
                pause_screen();
                break;
            case 4:
                list_user_orders(g_current_user.id);
                pause_screen();
                break;
            case 5:
                list_user_reservations(g_current_user.id);
                pause_screen();
                break;
            case 6: {
                print_header("MODIFICATION DU MOT DE PASSE");
                char p1[MAX_PASS_LEN], p2[MAX_PASS_LEN];
                read_password_masked("Nouveau mot de passe : ", p1, sizeof(p1));
                read_password_masked("Confirmation : ", p2, sizeof(p2));
                if (strcmp(p1, p2) == 0 && strlen(p1) >= 4) {
                    change_password(&g_current_user, p1);
                } else {
                    printf("[ERREUR] Mots de passe non identiques ou trop court.\n");
                }
                pause_screen();
                break;
            }
            case 0:
                logout_user();
                break;
            default:
                printf("[ERREUR] Choix invalide.\n");
                pause_screen();
                break;
        }
    } while (choice != 0);
}

int main(void) {
    // Initialize required directories
    init_directories();

    // Ensure default admin exists
    seed_default_admin();

    int choice;
    do {
        clear_screen();
        print_header("PORTAIL D'AUTHENTIFICATION");
        printf("Centrale de Distribution et de Vente de Produits Agricoles\n\n");
        printf("1. Se connecter\n");
        printf("0. Quitter l'application\n\n");

        choice = read_int("Votre choix : ");
        if (choice == 1) {
            print_header("CONNEXION");
            char login[MAX_LOGIN_LEN + 10];
            char password[MAX_PASS_LEN];

            read_string("Login (6 lettres majuscules ex: ADMINS) : ", login, sizeof(login));
            read_password_masked("Mot de passe : ", password, sizeof(password));

            if (login_user(login, password)) {
                if (strcmp(g_current_user.role, "ADMIN") == 0) {
                    menu_admin();
                } else {
                    menu_user();
                }
            } else {
                pause_screen();
            }
        } else if (choice == 0) {
            printf("\nMerci d'avoir utilise AGRI-MANAGER. Au revoir !\n");
        } else {
            printf("[ERREUR] Choix invalide.\n");
            pause_screen();
        }
    } while (choice != 0);

    return 0;
}
