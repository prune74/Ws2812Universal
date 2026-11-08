#pragma once

#include "Ws2812Bus.h"
#include "Ws2812Config.h"

/**
 * ============================================================================
 * 🚀 Ws2812Init — Initialisation centralisée des bus WS2812 (architecture CFC)
 * ============================================================================
 * Miroir de CanInit (CanUniversal) :
 *
 * Rôle :
 *   Initialiser *tous* les bus WS2812 déclarés par l'application via
 *   un Ws2812ConfigProvider.
 *
 *   La bibliothèque ne connaît :
 *     • ni les pins
 *     • ni le nombre de LED
 *     • ni le canal RMT (choisi par l'application pour cohabiter
 *       avec les canaux du DCC)
 *
 * Fonctionnement :
 *   • Pour chaque bus décrit par le provider :
 *       - tentative d'initialisation RMT
 *       - si OK → bus opérationnel
 *       - si erreur → bus marqué « invalide » (mais jamais NULL)
 *
 *   👉 Aucun accès ne provoque de crash, même si une LED est
 *      absente ou mal câblée. Le DCC continue de tourner, lui,
 *      sur ses propres canaux RMT, sans être perturbé.
 * ============================================================================
 */
class Ws2812Init
{
public:
    /**
     * ------------------------------------------------------------------------
     * 🟦 begin()
     * ------------------------------------------------------------------------
     * Initialise tous les bus WS2812 décrits par le provider.
     *
     * @param provider Configuration fournie par l'application.
     * @return Toujours true : l'initialisation est *tolérante*, les
     *         erreurs sont gérées bus par bus.
     * ------------------------------------------------------------------------
     */
    static bool begin(const Ws2812ConfigProvider &provider);
};