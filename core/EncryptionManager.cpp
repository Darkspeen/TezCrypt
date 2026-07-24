#include "EncryptionManager.h"

void EncryptionManager::registerAlgorithm(EncryptionAlgorithmPtr algorithm) {
    if (algorithm) {
        m_algorithms[algorithm->id().toStdString()] = algorithm;
    }
}

EncryptionAlgorithmPtr EncryptionManager::getAlgorithm(const QString& id) const {
    auto it = m_algorithms.find(id.toStdString());
    if (it != m_algorithms.end()) {
        return it->second;
    }
    return nullptr;
}

QStringList EncryptionManager::getAlgorithmNames() const {
    QStringList names;
    for (const auto& [id, algorithm] : m_algorithms) {
        names.append(algorithm->name());
    }
    names.sort();
    return names;
}

QStringList EncryptionManager::getAlgorithmIds() const {
    QStringList ids;
    for (const auto& [id, algorithm] : m_algorithms) {
        ids.append(algorithm->id());
    }
    ids.sort();
    return ids;
}

bool EncryptionManager::hasAlgorithm(const QString& id) const {
    return m_algorithms.find(id.toStdString()) != m_algorithms.end();
}

size_t EncryptionManager::algorithmCount() const {
    return m_algorithms.size();
}
