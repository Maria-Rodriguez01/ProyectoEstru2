#include "rankingstore.h"

RankingStore::RankingStore(QObject *parent) : QObject(parent) {}

QString RankingStore::filePath() const
{
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(base);
    return base + QDir::separator() + "ranking.bin";
}

QVector<ScoreRecord> RankingStore::readAll(const QString& path)
{
    QVector<ScoreRecord> out;
    QFile f(path);
    if (!f.exists() || !f.open(QIODevice::ReadOnly)) return out;

    while (true) {
        ScoreRecord rec;
        const qint64 need = static_cast<qint64>(sizeof(ScoreRecord));
        const qint64 got  = f.read(reinterpret_cast<char*>(&rec), need);
        if (got == need) out.push_back(rec);
        else break;
    }
    return out;
}

bool RankingStore::writeAll(const QString& path, const QVector<ScoreRecord>& v)
{
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) return false;

    for (const auto& rec : v) {
        if (f.write(reinterpret_cast<const char*>(&rec), sizeof(ScoreRecord)) !=
            static_cast<qint64>(sizeof(ScoreRecord))) return false;
    }
    f.flush();
    f.close();
    return true;
}

QByteArray RankingStore::toFixedUtf8(const QString& s, int maxBytes)
{
    QByteArray u = s.toUtf8();
    if (u.size() >= maxBytes) {
        u.truncate(maxBytes - 1); // deja 1 byte para terminador
    }
    // garantizamos longitud fija
    QByteArray out(maxBytes, '\0');
    memcpy(out.data(), u.constData(), u.size());
    return out;
}

bool RankingStore::lessEq(const ScoreRecord& x, const ScoreRecord& y)
{
    // primero por tiempo; si empatan, por nombre para estabilidad
    if (x.millis != y.millis) return x.millis < y.millis;
    return strncmp(x.name, y.name, sizeof(x.name)) <= 0;
}

int RankingStore::partition(QVector<ScoreRecord>& a, int lo, int hi)
{
    const ScoreRecord pivot = a[hi];
    int i = lo - 1;
    for (int j = lo; j < hi; ++j) {
        if (lessEq(a[j], pivot)) {
            ++i; std::swap(a[i], a[j]);
        }
    }
    std::swap(a[i+1], a[hi]);
    return i+1;
}

void RankingStore::quickSort(QVector<ScoreRecord>& a, int lo, int hi)
{
    if (lo >= hi) return;
    int p = partition(a, lo, hi);
    quickSort(a, lo, p - 1);
    quickSort(a, p + 1, hi);
}

QVector<ScoreRecord> RankingStore::load() const
{
    QVector<ScoreRecord> v = readAll(filePath());
    if (!v.isEmpty()) quickSort(v, 0, v.size() - 1);
    return v;
}

void RankingStore::add(const QString& playerName, qint64 millis)
{
    QVector<ScoreRecord> v = readAll(filePath());

    ScoreRecord rec{};
    const QByteArray fixed = toFixedUtf8(playerName.trimmed(), int(sizeof(rec.name)));
    memcpy(rec.name, fixed.constData(), fixed.size());
    rec.millis   = millis;
    rec.whenUtcMs = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();

    v.push_back(rec);
    if (!v.isEmpty()) quickSort(v, 0, v.size() - 1);
    writeAll(filePath(), v);
}

QVector<ScoreRecord> RankingStore::topN(int n) const
{
    QVector<ScoreRecord> v = load();
    if (n > 0 && v.size() > n) v.resize(n);
    return v;
}

