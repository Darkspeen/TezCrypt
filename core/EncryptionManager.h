#ifndef ENCRYPTION_MANAGER_H
#define ENCRYPTION_MANAGER_H

#include "EncryptionAlgorithm.h"
#include <QStringList>
#include <memory>
#include <unordered_map>

/**
 * @class EncryptionManager
 * @brief Central registry for all encryption algorithms
 * 
 * Manages registration, storage, and retrieval of encryption algorithms.
 * Provides a single point of contact for the GUI to access algorithms
 * without knowing their concrete implementations.
 * 
 * This design follows the Registry pattern and enables:
 * - Easy addition of new algorithms without modifying GUI code
 * - Polymorphic algorithm selection
 * - Centralized algorithm lifecycle management
 */
class EncryptionManager {
public:
    /**
     * @brief Register a new encryption algorithm
     * @param algorithm Shared pointer to an algorithm instance
     */
    void registerAlgorithm(EncryptionAlgorithmPtr algorithm);

    /**
     * @brief Get an algorithm by its ID
     * @param id The algorithm identifier
     * @return Shared pointer to the algorithm, or nullptr if not found
     */
    EncryptionAlgorithmPtr getAlgorithm(const QString& id) const;

    /**
     * @brief Get all available algorithm names
     * @return List of algorithm names suitable for UI display
     */
    QStringList getAlgorithmNames() const;

    /**
     * @brief Get all available algorithm IDs
     * @return List of algorithm IDs for internal use
     */
    QStringList getAlgorithmIds() const;

    /**
     * @brief Check if an algorithm is registered
     * @param id The algorithm identifier
     * @return True if the algorithm exists
     */
    bool hasAlgorithm(const QString& id) const;

    /**
     * @brief Get the number of registered algorithms
     * @return Count of registered algorithms
     */
    size_t algorithmCount() const;

private:
    // Map: algorithm_id -> algorithm_instance
    std::unordered_map<std::string, EncryptionAlgorithmPtr> m_algorithms;
};

#endif // ENCRYPTION_MANAGER_H
