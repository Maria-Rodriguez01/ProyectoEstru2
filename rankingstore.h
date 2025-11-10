#ifndef RANKINGSTORE_H
#define RANKINGSTORE_H

#include <QObject>
#include <QVector>
#include <QString>
#include <QDateTime>
#include <QStandardPaths>
#include <QDir>
#include <QFile>

// Registro binario fijo (simple y portable)
struct ScoreRecord {
    char   name[32];   // UTF-8 truncado
    qint64 millis;     // tiempo total en ms
    qint64 whenUtcMs;  // marca de tiempo (opcional)
};

// Almacén + quicksort
class RankingStore : public QObject
{
    Q_OBJECT
public:
    explicit RankingStore(QObject *parent = nullptr);

    // Carga todos los registros del archivo binario.
    QVector<ScoreRecord> load() const;

    // Agrega un registro y guarda.
    void add(const QString& playerName, qint64 millis);

    // Retorna los N mejores (ordenados por millis ascendente)
    QVector<ScoreRecord> topN(int n) const;

    // Ruta completa al archivo
    QString filePath() const;

private:
    // Quicksort in-place (por millis asc)
    static void quickSort(QVector<ScoreRecord>& a, int lo, int hi);

    static int  partition(QVector<ScoreRecord>& a, int lo, int hi);
    static bool lessEq(const ScoreRecord& x, const ScoreRecord& y);

    // util: escribe/lee binario
    static bool writeAll(const QString& path, const QVector<ScoreRecord>& v);
    static QVector<ScoreRecord> readAll(const QString& path);

    static QByteArray toFixedUtf8(const QString& s, int maxBytes);
};

#endif // RANKINGSTORE_H
