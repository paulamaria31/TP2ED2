#include "ordenaQuick.h"
#include <float.h>
#include <string.h>
#include <time.h>

static QuickMetrics g_qm = {0,0,0};
void QuickResetMetrics(void) { g_qm = (QuickMetrics){0,0,0}; }
QuickMetrics QuickGetMetrics(void) { return g_qm; }

static inline int cmp_nota(double a, double b) {
    g_qm.comparacoes++;
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}


static void SmallSort(FILE **Arq, int Esq, int Dir) {
    int n = Dir - Esq + 1;
    if (n <= 1) return;

    Aluno buf[TAM_AREA];
    fseek(*Arq, (Esq - 1) * (long)sizeof(Aluno), SEEK_SET);
    for (int i = 0; i < n; i++) {
        fread(&buf[i], sizeof(Aluno), 1, *Arq);
        g_qm.leituras++;
    }

    // insertion sort (ASC por nota)
    for (int i = 1; i < n; i++) {
        Aluno key = buf[i];
        int j = i - 1;
        while (j >= 0 && cmp_nota(buf[j].nota, key.nota) > 0) {
            buf[j + 1] = buf[j];
            j--;
        }
        buf[j + 1] = key;
    }

    fseek(*Arq, (Esq - 1) * (long)sizeof(Aluno), SEEK_SET);
    for (int i = 0; i < n; i++) {
        fwrite(&buf[i], sizeof(Aluno), 1, *Arq);
        g_qm.escritas++;
    }
}

static void LeSup(FILE **ArqLEs, Aluno *UltLido, int *Ls, bool *OndeLer);
static void LeInf(FILE **ArqLi, Aluno *UltLido, int *Li, bool *OndeLer);
static void EscreveMax(FILE **ArqLEs, Aluno R, int *Es);
static void EscreveMin(FILE **ArqEi, Aluno R, int *Ei);
static void InserirArea(TipoArea *Area, const Aluno *UltLido);
static void RetiraMax(TipoArea *Area, Aluno *R);
static void RetiraMin(TipoArea *Area, Aluno *R);
static void Particao(FILE **ArqLi, FILE **ArqEi, FILE **ArqLEs, TipoArea *Area, int Esq, int Dir, int *i, int *j);

void QuicksortExterno(FILE **ArqLi, FILE **ArqEi, FILE **ArqLEs, int Esq, int Dir) {
    if (Dir - Esq < 1) return;
    if ((Dir - Esq + 1) <= TAM_AREA) {
        // subarquivo cabe na memória interna do TP (10 registros)
        SmallSort(ArqEi, Esq, Dir);
        return;
    }

    int i, j;
    TipoArea Area;
    Area.n = 0;

    Particao(ArqLi, ArqEi, ArqLEs, &Area, Esq, Dir, &i, &j);

    // Se a partição já produziu um subarquivo completo (sem dividir), não há mais o que recursar
    if (i >= Dir || j <= Esq) return;

    // Recurse apenas em intervalos válidos (evita recursão infinita quando a partição não "quebra")
    int leftL = Esq, leftR = i;
    int rightL = j, rightR = Dir;

    int leftSize  = leftR - leftL;
    int rightSize = rightR - rightL;

    if (leftSize < rightSize) {
        if (leftL < leftR)   QuicksortExterno(ArqLi, ArqEi, ArqLEs, leftL, leftR);
        if (rightL < rightR) QuicksortExterno(ArqLi, ArqEi, ArqLEs, rightL, rightR);
    } else {
        if (rightL < rightR) QuicksortExterno(ArqLi, ArqEi, ArqLEs, rightL, rightR);
        if (leftL < leftR)   QuicksortExterno(ArqLi, ArqEi, ArqLEs, leftL, leftR);
    }
}


static void Particao(FILE **ArqLi, FILE **ArqEi, FILE **ArqLEs,
                     TipoArea *Area, int Esq, int Dir, int *i, int *j)
{
    int Li = Esq, Ls = Dir;   // cursores de leitura
    int Ei = Esq, Es = Dir;   // cursores de escrita

    double Linf = -DBL_MAX;
    double Lsup =  DBL_MAX;

    bool OndeLer = true;      // true = inferior, false = superior
    Aluno UltLido, R;

    // posiciona para leitura inferior sequencial
    fseek(*ArqLi, (Li - 1) * (long)sizeof(Aluno), SEEK_SET);
    // posiciona escrita inferior
    fseek(*ArqEi, (Ei - 1) * (long)sizeof(Aluno), SEEK_SET);

    while (Li <= Ls) {
        // 1) enche a área
        if (Area->n < TAM_AREA) {
            if (OndeLer) LeInf(ArqLi, &UltLido, &Li, &OndeLer);
            else         LeSup(ArqLEs, &UltLido, &Ls, &OndeLer);
            InserirArea(Area, &UltLido);
            continue;
        }

        // 2) área cheia: ler com trava anti-sobrescrita
        if (Ls == Es) {
            LeSup(ArqLEs, &UltLido, &Ls, &OndeLer);
        } else if (Li == Ei) {
            LeInf(ArqLi, &UltLido, &Li, &OndeLer);
        } else if (OndeLer) {
            LeInf(ArqLi, &UltLido, &Li, &OndeLer);
        } else {
            LeSup(ArqLEs, &UltLido, &Ls, &OndeLer);
        }

        // 3) classifica pelo intervalo (Linf, Lsup)
        if (cmp_nota(UltLido.nota, Lsup) > 0) {
            EscreveMax(ArqLEs, UltLido, &Es);
        } else if (cmp_nota(UltLido.nota, Linf) < 0) {
            EscreveMin(ArqEi, UltLido, &Ei);
        } else {
            InserirArea(Area, &UltLido);

            // descarrega do lado menor para balancear
            if ((Ei - Esq) <= (Dir - Es)) {
                RetiraMin(Area, &R);
                EscreveMin(ArqEi, R, &Ei);
                Linf = R.nota;
            } else {
                RetiraMax(Area, &R);
                EscreveMax(ArqLEs, R, &Es);
                Lsup = R.nota;
            }
        }
    }

    // 4) descarrega área no "miolo" (sempre enquanto houver itens!)
    while (Area->n > 0) {
        RetiraMin(Area, &R);
        EscreveMin(ArqEi, R, &Ei);
    }

    // 5) limites corretos para recursão
    *i = Ei - 1;
    *j = Es + 1;
}

/* -------------------- I/O -------------------- */

static void LeSup(FILE **ArqLEs, Aluno *UltLido, int *Ls, bool *OndeLer) {
    fseek(*ArqLEs, (*Ls - 1) * (long)sizeof(Aluno), SEEK_SET);
    fread(UltLido, sizeof(Aluno), 1, *ArqLEs);
    g_qm.leituras++;
    (*Ls)--;
    *OndeLer = true;
}

static void LeInf(FILE **ArqLi, Aluno *UltLido, int *Li, bool *OndeLer) {
    fread(UltLido, sizeof(Aluno), 1, *ArqLi);
    g_qm.leituras++;
    (*Li)++;
    *OndeLer = false;
}

static void EscreveMax(FILE **ArqLEs, Aluno R, int *Es) {
    fseek(*ArqLEs, (*Es - 1) * (long)sizeof(Aluno), SEEK_SET);
    fwrite(&R, sizeof(Aluno), 1, *ArqLEs);
    g_qm.escritas++;
    (*Es)--;
}

static void EscreveMin(FILE **ArqEi, Aluno R, int *Ei) {
    // escrita inferior é sequencial (ponteiro já está posicionado)
    fwrite(&R, sizeof(Aluno), 1, *ArqEi);
    g_qm.escritas++;
    (*Ei)++;
}

/* -------------------- Área Interna -------------------- */

static void InserirArea(TipoArea *Area, const Aluno *UltLido) {
    // insertion mantendo Área ordenada por nota (ASC)
    int k = Area->n - 1;
    while (k >= 0 && cmp_nota(Area->itens[k].nota, UltLido->nota) > 0) {
        Area->itens[k + 1] = Area->itens[k];
        k--;
    }
    Area->itens[k + 1] = *UltLido;
    Area->n++;
}

static void RetiraMin(TipoArea *Area, Aluno *R) {
    *R = Area->itens[0];
    // desloca
    for (int k = 0; k < Area->n - 1; k++) Area->itens[k] = Area->itens[k + 1];
    Area->n--;
}

static void RetiraMax(TipoArea *Area, Aluno *R) {
    *R = Area->itens[Area->n - 1];
    Area->n--;
}