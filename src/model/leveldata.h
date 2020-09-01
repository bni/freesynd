/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
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

#ifndef MODEL_LEVELDATA_H_
#define MODEL_LEVELDATA_H_

#include "common.h"

/*!
 * This namespace holds structures that represent a game file (GAMEXX.DAT).
 */
namespace LevelData {
    /*!
     * This is a header. Unknown use.
     * total: 8 bytes
     */
    struct Unkn01 {
        uint8 unkn10[6];
    };  

    /*!
     * The objs field is an array that represent the tiles of the map
     * (every map are 128x128 tiles). The values plus 32774 give an offset
     * in this file that is the entity placed on this tile.
     * The resulting offset can be 98309 max and only peds, vehicle, objects
     * and weapons can be indexed.
     * Was not able to verify correctness
     * total: 32768 bytes
     */
    struct Map {// 
        //! 128*128 offsets size 2 bytes(LE)
        uint8 objs[32768];
    };

    /*! 
     * This structure contains all people in the game, including agents
     * This struct size is 92.
     * most 2 byte variable have little-endianess byte order(LE)
     */
    struct People {
        // (LE data)'offset + 32774' gives the offset in this
        // file of the next object
        uint8 offset_next[2];
        // (LE data)'offset + 32774' gives the offset in this
        // file of the previous object (sometimes weapon, or
        // the next target for example ???)
        uint8 offset_prev[2];
        // [0] - offset, [1] - tile
        uint8 mapposx[2];
        uint8 mapposy[2];
        // to convert to our system use as uint16(LE)
        // tile = (uint16)/128, offz =(uint16)%128 or offz = mapposz[0] & 0x1F
        uint8 mapposz[2];
        /*!
         * This field gives information on the ped location. Values are:
         * 0x04 - ped on map
         * 0x05 - ped not on map(driving)
         * 0x0D - they are not visible/present on original map(on water located), purpose?
         * 0x0C - located level above possible walking surface, purpose?
         * 0x0D and 0x0C are excluded from being loaded
         */
        uint8 location;
        /*!
         * This field gives information on the ped's state. Values are:
         * 0x0 - standing
         * 0x10 - walking
         * 0x11 - dead
         */
        uint8 state;
        uint8 unkn3[2];         // nothing changes when this changes
        uint8 index_base_anim[2];  //index in (HSTA-0.ANI)
        uint8 index_current_frame[2];   //index in (HFRA-0.ANI)
        uint8 index_current_anim[2]; // index in (HSTA-0.ANI)
        uint8 health[2];
        uint8 offset_last_enemy[2];
        //0x01 ped; 0x02 vehicle; 0x04 weapon;
        //0x05 object; allow to display a target, a pickup, and for minimap
        uint8 type;
        uint8 status; // this can be sub type(?)
        //from 0xF0 to 0x10 : south = 0
        //from 0x10 to 0x30 : south-east = 1
        //from 0x30 to 0x50 : east = 2
        //from 0x50 to 0x70 : east-north = 3
        //from 0x70 to 0x90 : north = 4
        //from 0x90 to 0xB0 : north-west = 5
        //from 0xB0 to 0xD0 : west = 6
        //from 0xD0 to 0xF0 : west-south = 7
        // surface is mapped not to 360 degrees/surface, but 256 degrees/surface
        uint8 orientation;
        uint8 unkn4;
        // when 01 pedestrian, 02 agent, 04 police, 08 guard, 16 criminal
        uint8 type_ped;
        uint8 unkn5[3];
        uint8 offset_of_persuader[2];
        uint8 unkn6[2];
        uint8 offset_of_vehicle[2];
        // currently executed scenario
        uint8 offset_scenario_curr[2];
        // starting point for current scenario
        uint8 offset_scenario_start[2];
        uint8 unkn7[2];
        uint8 offset_of_vehicle_2[2]; // ??
        uint8 goto_mapposx[2];
        uint8 goto_mapposy[2];
        uint8 goto_mapposz[2];
        uint8 unkn8[6];
        uint8 offset_equipment[2];
        //bitmask, 0b - gender, 1-2b - leg, 3-4b - arm, 5-6b - chest
        // 7-8b - heart, 9-10b - eye, 11-12b - brain, 13-15b - unknown
        uint8 mods_info[2];
        uint8 unkn9[6];
        uint8 offset_cur_weapon[2];
        // IPA levels: white bar level,set level,exhaused level and forced level
        uint8 unkn10;
        uint8 adrena_amount;
        uint8 adrena_dependency;
        uint8 adrena_effect;
        uint8 unkn11;
        uint8 inteli_amount;
        uint8 inteli_dependency;
        uint8 inteli_effect;
        uint8 unkn12;
        uint8 percep_amount;
        uint8 percep_dependency;
        uint8 percep_effect;
        uint8 unkn13;
        uint8 unkn14[9];
    };

    /*!
     * This structure contains all cars in the game, including destroyed
     * This struct size is 42.
     */
    struct Cars {
        uint8 offset_next[2];
        uint8 offset_prev[2];
        uint8 mapposx[2];
        uint8 mapposy[2];
        uint8 mapposz[2];
        // 0x04 on map(visible)
        // 0x05 not on map(hidden)
        uint8 desc;
        uint8 unkn1;
        uint8 unkn2;
        uint8 unkn3;
        uint8 index_base_anim[2];
        uint8 index_current_frame[2];
        uint8 index_current_anim[2];
        uint8 health[2];
        uint8 offset_last_enemy[2];
        uint8 type; // 0x02
        uint8 sub_type;
        uint8 orientation;
        uint8 unkn4;
        uint8 offset_of_driver; //driver
        uint8 unkn5[11];
        uint8 speed;
        uint8 unkn6;
    };

    /*!
     *
     * total: 30 bytes
     */
    struct Statics {
        uint8 offset_next[2];
        uint8 offset_prev[2];
        uint8 mapposx[2];
        uint8 mapposy[2];
        uint8 mapposz[2];
        // 0x04 - on map;
        // 0x06 - on map, but why not 0x04?
        // 0x07 - on map, objects visibility is dependent on
        // orientation 0x40, 0x80 are drawn
        uint8 desc;
        uint8 unkn11;
        uint8 unkn12;
        uint8 unkn13;
        uint8 index_base_anim[2];
        uint8 index_current_frame[2];
        uint8 index_current_anim[2];
        uint8 unkn20[4];
        uint8 type;
        // more info mapobject.cpp
        uint8 sub_type;
        uint8 orientation;
        uint8 unkn27[3];
    };

    /*!
     *
     * total: 36 bytes
     */
    struct Weapons {
        uint8 offset_next[2];
        uint8 offset_prev[2];
        uint8 mapposx[2];
        uint8 mapposy[2];
        uint8 mapposz[2];
        // 0x04 on map
        // 0x05 not on map(in inventory)
        uint8 desc;
        uint8 unkn2;
        uint8 unkn3;
        uint8 unkn4;
        uint8 index_base_anim[2];
        uint8 index_current_frame[2];
        uint8 index_current_anim[2];
        uint8 unkn5[4];
        // look peds for more info
        uint8 type; // 0x04
        /* 0x01 persuadertron; 0x02 pistol; 0x03 gauss gun; 0x04 shotgun;
         * 0x05 uzi; 0x06 minigun; 0x07 laser; 0x08 flamer; 0x09 long range;
         * 0x0A scanner; 0x0B medikit; 0x0C time bomb; 0x0D access card;
         * 0x0E invalid; 0x0F invalid; 0x10 invalid; 0x11 energy shield;
        */
        uint8 sub_type;
        uint8 unkn6[2];
        uint8 offset_next_inventory[2];
        uint8 offset_prev_inventory[2];
        uint8 offset_owner[2];
        uint8 unkn7[2];
    };

    /*!
     * total: 30 bytes
     */
    struct Sfx {
        uint8 offset_next[2];
        uint8 offset_prev[2];
        uint8 mapposx[2];
        uint8 mapposy[2];
        uint8 mapposz[2];
        uint8 unkn1[4];
        uint8 index_base_anim[2];
        uint8 index_current_frame[2];
        uint8 index_current_anim[2];
        uint8 unkn2[8];
        uint8 offset_owner[2];
    };

    /*!
     * total: 8 bytes
     */
    struct Scenarios {
        // first scenario should be skipped, type 0x00
        uint8 next[2];
        uint8 offset_object[2];
        // tile_x_ = tilex >> 1
        // off_x_ = (tilex & 0x01) << 7
        uint8 tilex;
        // tile_y_ = tiley >> 1
        // off_y_ = (tiley & 0x01) << 7
        uint8 tiley;
        // tile_z_ = tilez
        uint8 tilez;
        /*!
         * Gives the type of scenario. Values are:
         * 0x00 - unset scenario type, is found at start of array and end;
         * 0x01 - walking/driving to pos, x,y defined, no object offset;
         * 0x02 - vehicle to use and goto 
         * 0x03 - ?(south africa)
         * 0x05 - ?(kenya)
         * 0x06 - (kenya) - ped offset when in vehicle, and? (TODO)
         * 0x07 - assasinate target escaped, mission failed
         * 0x08 - walking to pos, triggers on our agents in range, x,y defined
         * 0x09 - repeat from start, actually this might be end of script
         * 0x0A - train stops and waits
         * 0x0B - protected target reached destination(kenya) (TODO properly)
         */
        uint8 type;
    };

    /*!
     * total: 448 bytes
     */
    struct Unkn09{
        uint8 unkn10[448];
    };

    /*!
     * total: 14 bytes
     */
    struct MapInfos {
        uint8 map[2];
        uint8 min_x[2];
        uint8 min_y[2];
        uint8 max_x[2];
        uint8 max_y[2];
        //status flag is set to 1 if the mission has been successfully completed
        uint8 status;
        uint8 unkn1[3];
    };

    /*!
     * total: 14 bytes
     */
    struct Objectives {
        /* only max 5 objectives are non-zero, we will read 6
         * 0x00 action for non-agent(?) ;0x01 persuade; 0x02 assassinate;
         * 0x03 protect; 0x05 equipment aquisition; 0x0a combat sweep (police);
         * 0x0b combat sweep; 0x0e destroy vehicle; 0x0f use vehicle;
         * 0x10 evacuate
         * more info in mission.cpp : loadLevel()
        */
        uint8 type[2];
        // 'offset + 32774' gives the offset in this file of the first objective
        uint8 offset[2];
        uint8 mapposx[2];
        uint8 mapposy[2];
        uint8 mapposz[2];
        /* If "protect", the next objective are the goals and their type is zero.
         * The list finish with zero and the offset of the protected item ?
         * The status flag is set to 1 if the objective has to be completed
        */
        uint8 status;
        uint8 unkn1[3];
    };

    /*!
     * total: 1896 bytes
     */
    struct Unkn11 {
        uint8 unkn10[1952];
    };

    /*!
     * This is the main structure.
     */
    struct LevelDataAll {
        /*      0 */ Unkn01 u01;
        /*      6 */ Map map;
        /*  32774 0x0*/ uint8 offset_ref[2];
        /*  32776 0x02*/ People people[256];
        /*  56328 0x5C02*/ Cars cars[64];
        /*  59016 0x6682*/ Statics statics[400];
        /*  71016 0x9562*/ Weapons weapons[512];
        /*  89448 0xDD62*/ Sfx sfx[256];
        /*  97128 0xFB62*/ LevelData::Scenarios scenarios[2048];
        /* 113512 */ Unkn09 u09;
        /* 113960 */ MapInfos mapinfos;
        /* 113974 */ Objectives objectives[6];
        /* 114058 */ Unkn11 u11;
    };

    /*! Constant for field People::location : Ped is on the map.*/
    static const int kPeopleLocOnMap = 0x04;
    /*! Constant for field People::location : Ped is in a vehicle.*/
    static const int kPeopleLocInVehicle = 0x05;
    /*! Constant for field People::location : located level above possible walking surface, purpose?*/
    static const int kPeopleLocAboveWalkSurf = 0x0C;
    /*! Constant for field People::location : they are not visible/present on original map(on water located), purpose?*/
    static const int kPeopleLocNotVisible = 0x0D;

    /*! Constant for field People::state : ped is walking.*/
    static const int kPeopleStateWalking = 0x10;
    /*! Constant for field People::state : ped is dead.*/
    static const int kPeopleStateDead = 0x11;

    /*! Constant for field Scenario::type :  Use vehicle to go somewhere.*/
    static const int kScenarioTypeUseVehicle = 0x02;
    /*! Constant for field Scenario::type :  Target has escape the map.*/
    static const int kScenarioTypeEscape = 0x07;
    /*! Constant for field Scenario::type : this is a trigger. 
     * Agents will trigger it when they enter the circle defined by the center and a fixed radius.*/
    static const int kScenarioTypeTrigger = 0x08;
    /*! Constant for field Scenario::type : Reset all scripted action.*/
    static const int kScenarioTypeReset = 0x09;
}

#endif  // MODEL_LEVELDATA_H_
