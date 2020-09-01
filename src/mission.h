/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2010  Benoit Blancard <benblan@users.sourceforge.net>*
 *   Copyright (C) 2010  Bohdan Stelmakh <chamel@users.sourceforge.net> *
 *                                                                      *
 *    This program is free software;  you can redistribute it and / or  *
 *  modify it  under the  terms of the  GNU General  Public License as  *
 *  published by the Free Software Foundation; either version 2 of the  *
 *  License, or (at your option) any later version.                     *
 *                                                                      *
 *    This program is  distributed in the hope that it will be useful,  *
 *  but WITHOUT  ANY WARRANTY;  without even  the implied  warranty of  *
 *  MERCHANTABILITY  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  *
 *  General Public License for more details.                            *
 *                                                                      *
 *    You can view the GNU  General Public License, online, at the GNU  *
 *  project's  web  site;  see <http://www.gnu.org/licenses/gpl.html>.  *
 *  The full text of the license is also included in the file COPYING.  *
 *                                                                      *
 ************************************************************************/

#ifndef MISSION_H
#define MISSION_H

#include <string>
#include <vector>
#include <set>

#include "common.h"
#include "mapobject.h"
#include "map.h"
#include "model/leveldata.h"
#include "core/gameevent.h"

class Vehicle;
class PedInstance;
class Agent;
class ObjectiveDesc;
class Squad;
class ProjectileShot;
class GaussGunShot;
class Weapon;

/*!
 * A class that holds mission statistics.
 */
class MissionStats {
public:
    void init(int nbAgents);

    int nbOfShots() { return nbOfShots_; }
    int precision() { return (nbOfHits_ * 100) / nbOfShots_; }
    int enemyKilled() { return enemyKilled_;}
    int criminalKilled() { return criminalKilled_;}
    int civilKilled() { return civilKilled_;}
    int policeKilled() { return policeKilled_;}
    int guardKilled() { return guardKilled_;}
    int convinced() { return convinced_;}
    int missionDuration() { return missionDuration_; }
    int agents() { return agents_; }
    int agentCaptured() { return agentCaptured_; }

    //! Increments the number of shots by the given amount
    void incrShots(int shots) { nbOfShots_ += shots; }
    //! Increments the number of hits by one
    void incrHits() { nbOfHits_++; }
    //! Increments the number of hits by the given amount
    void incrHits(int hits) { nbOfHits_ += hits; }
    void incrEnemyKilled() { enemyKilled_++; }
    void incrCriminalKilled() { criminalKilled_++; }
    void incrCivilKilled() { civilKilled_++; }
    void incrGuardKilled() { guardKilled_++; }
    void incrPoliceKilled() { policeKilled_++; }
    //!
    void incrAgentCaptured() { agentCaptured_++; }
    //!
    void incrConvinced() { convinced_++; }
    //!
    void incrMissionDuration(int elapsed) { missionDuration_ += elapsed; }
private:
    /*! How many agents participated in the mission. */
    int agents_;
    /*! How many time did the mission last. */
    int missionDuration_;
    /*! How many opposing agents where captured.*/
    int agentCaptured_;
    /*! How many opposing agents where killed.*/
    int enemyKilled_;
    /*! How many criminal where killed.*/
    int criminalKilled_;
    /*! How many civilian where killed.*/
    int civilKilled_;
    /*! How many policemen where killed.*/
    int policeKilled_;
    /*! How many guards where killed.*/
    int guardKilled_;
    /*! How many people where convinced.*/
    int convinced_;
    /*! How many times did agents shoot.*/
    int nbOfShots_;
    /*! How many times did agents hit.*/
    int nbOfHits_;
};

/*!
 * Contains information read from original mission data file.
 */
class Mission {
public:
    /*!
     * List of all possible mission status.
     */
    enum Status {
        /*! Mission is currently running.*/
        kMissionStatusRunning = 0,
        /*! Player left the mission without finishing it.*/
        kMissionStatusAborted = 1,
        /*! Player finished the mission with failure.*/
        kMissionStatusFailed = 2,
        /*! Player finished the mission with success.*/
        kMissionStatusCompleted = 3
    };

    //! Bit mask for methods on checking on blockers
    static const uint8 kBMaskBlockerTargetOutOfMap;
    static const uint8 kBMaskBlockerTargetObjectUpdated;
    static const uint8 kBMaskBlockerTargetPosUpdated;

    Mission(const LevelData::MapInfos & map_infos);
    virtual ~Mission();

    //*************************************
    // Mission life cycle and objectives
    //*************************************

    //! Init mission data
    void start();
    //! Destroy mission
    void end();

    //! Ends mission with the given status
    void endWithStatus(Status status);
    //! Returns true if mission status is failed
    bool failed() { return status_ == kMissionStatusFailed; }
    //! Returns true if mission status is completed
    bool completed() { return status_ == kMissionStatusCompleted; }
    //! Returns mission status
    Status getStatus() { return status_; }

    //! Adds an objective for the mission
    void addObjective(ObjectiveDesc *pObjective) { objectives_.push_back(pObjective); }
    //! Check if objectives are completed or failed
    void checkObjectives();
    void objectiveMsg(std::string& msg);

    //*************************************
    // Map
    //*************************************
    /*!
     * Sets the given map for the mission.
     * If p_map is not null, creates a minimap from it.
     * \param p_map The map to set.
     */
    void set_map(Map *p_map);

    /*!
     * Returns the map used for the mission.
     */
    Map * get_map() {return p_map_; }

    /*!
     * Returns the map id used for the mission.
     */
    int map() { return i_map_id_; }
    uint16 mapId() { return i_map_id_; }

    int mapWidth();
    int mapHeight();

    int minX() { return min_x_; }
    int minY() { return min_y_; }
    int maxX() { return max_x_; }
    int maxY() { return max_y_; }

    //*************************************
    // Map objects
    //*************************************
    size_t numPeds() { return peds_.size(); }
    PedInstance *ped(size_t i) { return peds_[i]; }
    void addPed(PedInstance *p) { peds_.push_back(p); }

    size_t numVehicles() { return vehicles_.size(); }
    Vehicle *vehicle(size_t i) { return vehicles_[i]; }
    void addVehicle(Vehicle *pVehicle) { vehicles_.push_back(pVehicle); }

    size_t numWeaponsOnGround() { return weaponsOnGround_.size(); }
    WeaponInstance *weaponOnGround(size_t i) { return weaponsOnGround_[i]; }
    void addWeaponToGround(WeaponInstance *w);
    void removeWeaponOnGround(WeaponInstance *pWeapon);

    size_t numStatics() { return statics_.size(); }
    Static *statics(size_t i) { return statics_[i]; }
    void addStatic(Static *pStatic) { statics_.push_back(pStatic); }

    size_t numSfxObjects() { return sfx_objects_.size(); }
    SFXObject *sfxObjects(size_t i) { return sfx_objects_[i]; }

    void addSfxObject(SFXObject *so) {
        sfx_objects_.push_back(so);
    }
    /*!
     * Removes SfxObject at given position in the list of sfxobjects.
     * Object is freed only if not managed by another object.
     * \param i position of object in the list.
     */
    void delSfxObject(size_t i) {
        if (!sfx_objects_[i]->isManaged()) {
            // object is not managed so delete it
            delete sfx_objects_[i];
        }
        sfx_objects_.erase((sfx_objects_.begin() + i));
    }

    /*!
     * Adds the given ProjectileShot to the list of animated shots.
     * \param prj The projectile to add
     */
    void addPrjShot(ProjectileShot *prj) {
        prj_shots_.push_back(prj);
    }
    /*!
     * Returns the number of currently animated ProjectileShot.
     */
    size_t numPrjShots() { return prj_shots_.size(); }
    /*!
     * Return the projectile at the given index.
     * \param i Index of the projectile
     * \return The projectile found.
     */
    ProjectileShot *prjShots(size_t i) { return prj_shots_[i]; }
    /*!
     * Destroy the projectile at given index.
     * \param i Index of the projectile
     */
    void delPrjShot(size_t i);

    /*!
     * Adds the given PedInstance to the list of armed peds.
     * \param pPed The ped to add
     */
    void addArmedPed(PedInstance *pPed) {
        armedPedsVec_.push_back(pPed);
    }
    /*!
     * Returns the number of currently armed peds.
     */
    size_t numArmedPeds() { return armedPedsVec_.size(); }
    /*!
     * Return the PedInstance at the given index.
     * \param i Index of the projectile
     * \return The ped found.
     */
    PedInstance *armedPedAtIndex(size_t i) { return armedPedsVec_[i]; }
    /*!
     * Removes given ped from the list of armed peds.
     * \param pPed The ped to remove
     */
    void removeArmedPed(PedInstance *pPed);

    MapObject * findObjectWithNatureAtPos(int tilex, int tiley, int tilez,
        MapObject::ObjectNature *nature, int *searchIndex, bool only);

    /*! Return the mission statistics. */
    MissionStats *stats() { return &stats_; }

    bool setSurfaces();
    void clrSurfaces();
    bool getWalkable(TilePoint &mtp);
    bool getWalkableClosestByZ(TilePoint &mtp);
    bool getShootableTile(TilePoint *pLocT);
    bool isTileSolid(int x, int y, int z, int ox, int oy, int oz);

    //*************************************
    // Methods for shooting verification
    //*************************************
    //! Check if a tile is blocking the line between originLoc and pTargetPosW
    uint8 checkBlockedByTile(const WorldPoint & originLoc, WorldPoint *pTargetPosW, bool updateLoc, double distanceMax, double *pFinalDest = NULL);
    //! Check if an object is blocking the line between originLoc and pTargetPosW
    MapObject * checkBlockedByObject(WorldPoint * originLoc, WorldPoint * pTargetPosW,
        double *dist, const ShootableMapObject *pOrigin);
    //! Check if tile or object blocks the line between originLoc and pTarget
    uint8 checkIfBlockersInShootingLine(const WorldPoint & originLoc, ShootableMapObject **pTarget,
        WorldPoint *pTargetPosW = NULL, bool setBlocker = false,
        bool checkTileOnly = false, double maxr = -1.0, double * distTo = NULL, const ShootableMapObject *pOrigin = NULL);
    //! Returns the distance between a ped and a object if a path exists between the two
    uint8 getPathLengthBetween(PedInstance *pPed, ShootableMapObject* objectToReach, double distanceMax, double *length);

    //! map-tile surfaces : x + y * mmax_x_ + z * mmax_m_xy
    uint8 *mtsurfaces_;
    // map-directions points
    floodPointDesc *mdpoints_;
    // for copy in pathfinding
    floodPointDesc *mdpoints_cp_;
    // initialized in set_map, used for in-class calculations
    // map maximum x,y,z values
    int mmax_x_, mmax_y_, mmax_z_;
    // initialized in setSurfaces, used for in-class calculations
    int mmax_m_xy;

    MiniMap * getMiniMap() { return p_minimap_; }
    /*!
     * Returns the current squad.
     */
    Squad * getSquad() const { return p_squad_; }

protected:
    bool sWalkable(char thisTile, char upperTile);
    bool isSurface(char thisTile);
    bool isStairs(char thisTile);

    void transferWeaponsFromPedInstanceToAgent(PedInstance *p, Agent *pAg);

protected:

    /*! List of all vehicles, cars and train.*/
    std::vector<Vehicle *> vehicles_;
    std::vector<PedInstance *> peds_;
    //! List of all weapons that have no owner
    std::vector<WeaponInstance *> weaponsOnGround_;
    std::vector<Static *> statics_;
    std::vector<SFXObject *> sfx_objects_;
    std::vector<ProjectileShot *> prj_shots_;
    /*!
     * A vector constantly updated with the peds that hold a weapon.
     * It's used for performance reasons.
     */
    std::vector<PedInstance *> armedPedsVec_;

    std::vector <ObjectiveDesc *> objectives_;
    //std::vector <ObjectiveDesc> sub_objectives_;
    uint16 cur_objective_;
    /*!
     * Mission status.
     * By default, a mission is running but it can be
     * aborted if user escapes the mission, failed if
     * missions objectives are not fullfilled or completed.
     */
    Status status_;

    int min_x_, min_y_, max_x_, max_y_;
    /*!
     * The id of the map for that mission.
     */
    uint16 i_map_id_;
    /*!
     * A pointer to the map.
     */
    Map *p_map_;

    /*! Statistics : time, shots, ...*/
    MissionStats stats_;
    /*!
     * minimap in colours, map z = 0 tiles transformed based on
     * walkdata->minimap_colours_ in function createMinimap
     */
    MiniMap *p_minimap_;
    /*!
     * The squad selected for the mission. It contains only active agents.
     */
    Squad *p_squad_;
};

#endif
