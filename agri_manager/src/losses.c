#include "../include/losses.h"
#include "../include/products.h"
#include "../include/auth.h"
#include "../include/utils.h"
#include "../include/reports.h"

int get_next_loss_id(void) {
    FILE *f = fopen(FILE_LOSSES, "rb");
    if (!f) return 1;
    Loss l;
    int max_id = 0;
    while (fread(&l, sizeof(Loss), 1, f) == 1) {
        if (l.id > max_id) max_id = l.id;
    }
    fclose(f);
    return max_id + 1;
}

double get_total_losses_amount(void) {
    FILE *f = fopen(FILE_LOSSES, "rb");
    if (!f) return 0.0;
    Loss l;
    double total = 0.0;
    while (fread(&l, sizeof(Loss), 1, f) == 1) {
        total += l.montant_financier;
    }
    fclose(f);
    return total;
}

void record_loss(const Loss *loss) {
    FILE *f = fopen(FILE_LOSSES, "ab");
    if (!f) return;
    Loss l = *loss;
    if (l.id == 0) {
        l.id = get_next_loss_id();
    }
    fwrite(&l, sizeof(Loss), 1, f);
    fclose(f);
}

void add_manual_spoilage_loss(void) {
    print_header("DECLARATION DE PERTES / PRODUITS AVARIES");

    list_products();

    int id_product = read_int("\nEntrez l'ID du produit avarie / perimé : ");
    Product prd;
    if (!find_product_by_id(id_product, &prd)) {
        printf("[ERREUR] Produit ID %d introuvable.\n", id_product);
        return;
    }

    double qty_spoiled = read_double("Quantite avariee / perdue : ");
    if (qty_spoiled <= 0) {
        printf("[ERREUR] La quantite doit etre superieure a 0.\n");
        return;
    }

    if (qty_spoiled > prd.quantite_disponible) {
        printf("[ATTENTION] La quantite avariee (%.2f) depasse le stock disponible (%.2f). Stock ajuste a 0.\n",
               qty_spoiled, prd.quantite_disponible);
    }

    Loss l;
    memset(&l, 0, sizeof(Loss));
    l.id = get_next_loss_id();
    l.idUtilisateur = prd.idProducteur;
    l.idCommande = 0; // Spoilage loss
    l.nb_jours_retard_ou_quantite_avariee = qty_spoiled;
    l.montant_financier = qty_spoiled * prd.prix_unitaire;
    get_current_date(l.date_enregistrement, sizeof(l.date_enregistrement));

    record_loss(&l);

    update_product_stock(prd.id, -qty_spoiled, -qty_spoiled);

    char log_msg[250];
    snprintf(log_msg, sizeof(log_msg), "Declaration de perte: %.2f %s de \"%s\" avaries (Montant: %.2f FCFA)",
             qty_spoiled, prd.unite_mesure, prd.designation, l.montant_financier);
    log_operation(g_current_user.login, log_msg);

    printf("\n[SUCCES] Perte declaree avec succes ! Montant financier evalue : %.2f FCFA\n", l.montant_financier);
}

void list_losses(void) {
    print_header("GESTION DES PERTES ET PENALITES");

    FILE *f = fopen(FILE_LOSSES, "rb");
    if (!f) {
        printf("Aucune perte enregistree.\n");
        return;
    }

    Loss l;
    printf("%-4s | %-10s | %-12s | %-18s | %-15s | %-19s\n",
           "ID", "ID USER/PRD", "ID COMMANDE", "RETARD/QTE AVARIE", "MONTANT (FCFA)", "DATE ENREG.");
    printf("--------------------------------------------------------------------------------------------\n");

    int count = 0;
    double total = 0.0;
    while (fread(&l, sizeof(Loss), 1, f) == 1) {
        printf("%-4d | %-10d | %-12d | %-18.2f | %-15.2f | %-19s\n",
               l.id, l.idUtilisateur, l.idCommande,
               l.nb_jours_retard_ou_quantite_avariee, l.montant_financier, l.date_enregistrement);
        total += l.montant_financier;
        count++;
    }
    fclose(f);
    printf("--------------------------------------------------------------------------------------------\n");
    printf("Total: %d entree(s) | PERTE TOTALE ENREGISTREE : %.2f FCFA\n", count, total);
}
