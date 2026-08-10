#include "../include/reports.h"
#include "../include/utils.h"
#include "../include/products.h"
#include "../include/auth.h"

void log_operation(const char *login, const char *operation) {
    init_directories();
    FILE *f = fopen(FILE_HISTORY, "a");
    if (!f) return;

    char current_date[MAX_DATE_LEN];
    get_current_date(current_date, sizeof(current_date));

    fprintf(f, "[%s] %s %s\n", current_date, login ? login : "SYSTEM", operation);
    fclose(f);
}

void view_history(void) {
    print_header("HISTORIQUE GLOBAL DES OPERATIONS (REPORTS/HISTORY.txt)");

    FILE *f = fopen(FILE_HISTORY, "r");
    if (!f) {
        printf("Aucun historique disponible.\n");
        return;
    }

    char line[512];
    int count = 0;
    while (fgets(line, sizeof(line), f) != NULL) {
        printf("%s", line);
        count++;
    }
    fclose(f);

    printf("-----------------------------------------------------------------------------------\n");
    printf("Total: %d entree(s) d'historique\n", count);
}

void generate_daily_report(void) {
    print_header("GENERATION DE L'ETAT JOURNALIER AUTOMATIQUE");

    char today_date[20];
    get_date_only(today_date, sizeof(today_date));

    char filename[256];
    snprintf(filename, sizeof(filename), "%s/REPORT_%s.txt", DIR_REPORTS_DAILY, today_date);

    char today_str[20];
    get_current_date(today_str, sizeof(today_str));
    
    char today_prefix[12];
    strncpy(today_prefix, today_str, 10);
    today_prefix[10] = '\0';

    int orders_count = 0;
    int deliveries_count = 0;
    int reservations_count = 0;
    int new_users_count = 0;
    int new_producers_count = 0;

    FILE *fo = fopen(FILE_ORDERS, "rb");
    if (fo) {
        Order o;
        while (fread(&o, sizeof(Order), 1, fo) == 1) {
            if (strncmp(o.date_commande, today_prefix, 10) == 0) {
                orders_count++;
            }
        }
        fclose(fo);
    }

    FILE *fd = fopen(FILE_DELIVERIES, "rb");
    if (fd) {
        Delivery d;
        while (fread(&d, sizeof(Delivery), 1, fd) == 1) {
            if (strncmp(d.date_reelle_livraison, today_prefix, 10) == 0) {
                deliveries_count++;
            }
        }
        fclose(fd);
    }

    FILE *fr = fopen(FILE_RESERVATIONS, "rb");
    if (fr) {
        Reservation r;
        while (fread(&r, sizeof(Reservation), 1, fr) == 1) {
            if (strncmp(r.date_reservation, today_prefix, 10) == 0) {
                reservations_count++;
            }
        }
        fclose(fr);
    }

    FILE *fu = fopen(FILE_USERS, "rb");
    if (fu) {
        User u;
        while (fread(&u, sizeof(User), 1, fu) == 1) {
            if (strncmp(u.date_creation, today_prefix, 10) == 0) {
                new_users_count++;
            }
        }
        fclose(fu);
    }

    double total_losses = 0.0;
    FILE *fl = fopen(FILE_LOSSES, "rb");
    if (fl) {
        Loss l;
        while (fread(&l, sizeof(Loss), 1, fl) == 1) {
            total_losses += l.montant_financier;
        }
        fclose(fl);
    }

    char top_product_name[100] = "Aucun";
    double max_qty_sold = 0.0;
    FILE *fp = fopen(FILE_PRODUCTS, "rb");
    if (fp) {
        Product p;
        while (fread(&p, sizeof(Product), 1, fp) == 1) {
           
            double product_total_qty = 0.0;
            FILE *fo2 = fopen(FILE_ORDERS, "rb");
            if (fo2) {
                Order o2;
                while (fread(&o2, sizeof(Order), 1, fo2) == 1) {
                    if (o2.idProduit == p.id && (strcmp(o2.etat, "EN_COURS") == 0 || strcmp(o2.etat, "LIVRÉ") == 0)) {
                        product_total_qty += o2.quantite_commandee;
                    }
                }
                fclose(fo2);
            }
            if (product_total_qty > max_qty_sold) {
                max_qty_sold = product_total_qty;
                snprintf(top_product_name, sizeof(top_product_name), "%s (Code: %s)", p.designation, p.codeProduit);
            }
        }
        fclose(fp);
    }

    char top_buyer_name[100] = "Aucun";
    int max_buyer_orders = 0;
    FILE *fu2 = fopen(FILE_USERS, "rb");
    if (fu2) {
        User u2;
        while (fread(&u2, sizeof(User), 1, fu2) == 1) {
            if (strcmp(u2.role, "USER") == 0) {
                int user_orders = 0;
                FILE *fo3 = fopen(FILE_ORDERS, "rb");
                if (fo3) {
                    Order o3;
                    while (fread(&o3, sizeof(Order), 1, fo3) == 1) {
                        if (o3.idUtilisateur == u2.id) user_orders++;
                    }
                    fclose(fo3);
                }
                if (user_orders > max_buyer_orders) {
                    max_buyer_orders = user_orders;
                    snprintf(top_buyer_name, sizeof(top_buyer_name), "%s %s (Login: %s)", u2.nom, u2.prenom, u2.login);
                }
            }
        }
        fclose(fu2);
    }

    FILE *fout = fopen(filename, "w");
    if (!fout) {
        printf("[ERREUR] Impossible de creer le rapport journalier.\n");
        return;
    }

    fprintf(fout, "=======================================================\n");
    fprintf(fout, "      AGRI-MANAGER - RAPPORT ET ETAT JOURNALIER       \n");
    fprintf(fout, "=======================================================\n");
    fprintf(fout, "Date du rapport               : %s\n", today_prefix);
    fprintf(fout, "Heure de generation           : %s\n", today_str);
    fprintf(fout, "-------------------------------------------------------\n");
    fprintf(fout, "Nombre de commandes du jour   : %d\n", orders_count);
    fprintf(fout, "Nombre de livraisons          : %d\n", deliveries_count);
    fprintf(fout, "Nombre de reservations recolte: %d\n", reservations_count);
    fprintf(fout, "Nouveaux utilisateurs/prod.   : %d\n", new_users_count + new_producers_count);
    fprintf(fout, "Pertes totales enregistrees   : %.2f FCFA\n", total_losses);
    fprintf(fout, "-------------------------------------------------------\n");
    fprintf(fout, "Produit le plus vendu         : %s (Total: %.2f units)\n", top_product_name, max_qty_sold);
    fprintf(fout, "Acheteur le plus actif        : %s (%d commande(s))\n", top_buyer_name, max_buyer_orders);
    fprintf(fout, "=======================================================\n");

    fclose(fout);

    log_operation(g_current_user.login, "Generation du rapport journalier");

    printf("[SUCCES] Etat journalier genere dans le fichier :\n%s\n", filename);
    printf("\n--- RECAPITULATIF RAPPORT ---\n");
    printf("- Commandes du jour : %d\n", orders_count);
    printf("- Livraisons du jour : %d\n", deliveries_count);
    printf("- Pertes totales : %.2f FCFA\n", total_losses);
    printf("- Top Produit : %s\n", top_product_name);
    printf("- Top Acheteur : %s\n", top_buyer_name);
}
