// Quiz.h
#pragma once
#include <QString>
#include <QStringList>
#include <vector>

// HABILITA el sufijo "_qs" para literales QString (Qt 6)
using namespace Qt::Literals::StringLiterals;

struct MCQ {
    QString question;
    QStringList options; // "A) ...", "B) ...", ...
    int correctIndex;    // 0-based
};

// 5 preguntas mapeadas a D1..D5 (índices 0..4)
inline const std::vector<MCQ>& quizBank() {
    static const std::vector<MCQ> bank = {
        { u"Durante el Renacimiento, el modelo de gobierno es uno de los siguientes:"_qs,
            { u"A) Monarquía absoluta."_qs,
             u"B) Tiranía republicana."_qs,
             u"C) Democracia participativa."_qs,
             u"D) Liberalismo político."_qs },
            0
        },
        { u"De los siguientes acontecimientos, seleccione el que inicia el período moderno:"_qs,
            { u"A) Toma de Constantinopla."_qs,
             u"B) Tratado de Paz de Westfalia."_qs,
             u"C) Toma de la Bastilla."_qs,
             u"D) La ruta de la seda."_qs },
            1
        },
        { u"Durante el siglo XV, la sociedad se estratifica en tres estamentos definidos:"_qs,
            { u"A) Clase media, baja y alta."_qs,
             u"B) Nobleza, clero y estado llano."_qs,
             u"C) Artesanos, guardianes y gobernantes."_qs },
            1
        },
        { u"Aparece el realismo político, basado en el orden establecido y recomendaciones de cómo gobernar:"_qs,
            { u"A) Tomás Moro."_qs,
             u"B) Jean Bodín."_qs,
             u"C) Nicolás Maquiavelo."_qs,
             u"D) Erasmo de Rotterdam."_qs },
            2
        },
        { u"Terminada la Edad Media, en política resulta que:"_qs,
            { u"A) La Iglesia resalta su poder."_qs,
             u"B) La Iglesia pierde el papel rector en la política."_qs,
             u"C) La Iglesia evangélica se posiciona en la política."_qs,
             u"D) La política desaparece."_qs },
            1
        }
    };
    return bank;
}
