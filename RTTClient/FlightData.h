#pragma once

#ifndef _FLIGHT_DATA_H
#define _FLIGHT_DATA_H

#include <vector>
#include <memory>
#include <cstring>


#define FLIGHTDATA_VERSION 118
// changelog:
// 110: initial BMS 4.33 version
// 111: added SysTest to LightBits3
// 112: added MCAnnounced to LightBits3
// 113: added AllLampBits2OnExceptCarapace to LightBits2 and AllLampBits3OnExceptCarapace to LightBits3
// 114: renamed WOW LightBit to ONGROUND, added "real" (AFM) WOW to LightBits3
// 115: renamed "real" WOW in MLGWOW, added NLGWOW
// 116: bitfields are now unsigned instead of signed
// 117: added ATF_Not_Engaged to LightBits3
// 118: added Inlet_Icing to LightBits3

// *** "FalconSharedMemoryArea" ***
class FlightData 
{
public:
	// GENERAL NOTE FOR ALL LIGHTBITS:
	//
	// The lightbits contain status about whether a lamp is activated or deactivated. A *blinking* lamp
	// is always activated, even if it is in the "off" phase of the blinking! To check whether an activated
	// lamp is blinking or just "on", use the BlinkBits in FlightData2. A blinkbit does NOT alternate on/off
	// either, it will just state *if* a lamp is blinking. This construct might seem strange at 1st sight,
	// but only like this it can be guaranteed that even low-freq shared mem readers will pick up the info
	// about blinking lamps correctly. Obviously, it is up to the external program to implement the actual
	// blinking logic/freq etc.
	//
	// Summary:
	// a) The LightBit says "lamp is active (LightBit 1) or inactive (LightBit 0)".
	// b) The BlinkBit says "if the lamp is active (see LightBit 1), is it steady (BlinkBit 0)
	//    or is it blinking (BlinkBit 1)"
	// c) If a lamp has no BlinkBit, it is always assumed to be steady if active (LightBit 1).
	
	enum LightBits
    {
        MasterCaution = 0x1,  // Left eyebrow

        // Brow Lights
        TF        = 0x2,   // Left eyebrow
        OXY_BROW  = 0x4,   // repurposed for eyebrow OXY LOW (was OBS, unused)
        EQUIP_HOT = 0x8,   // Caution light; repurposed for cooling fault (was: not used)
        ONGROUND  = 0x10,  // True if on ground: this is not a lamp bit!
        ENG_FIRE  = 0x20,  // Right eyebrow; upper half of split face lamp
        CONFIG    = 0x40,  // Stores config, caution panel
        HYD       = 0x80,  // Right eyebrow; see also OIL (this lamp is not split face)
        Flcs_ABCD = 0x100, // TEST panel FLCS channel lamps; repurposed, was OIL (see HYD; that lamp is not split face)
        FLCS      = 0x200, // Right eyebrow; was called DUAL which matches block 25, 30/32 and older 40/42
        CAN       = 0x400, // Right eyebrow
        T_L_CFG   = 0x800, // Right eyebrow

        // AOA Indexers
        AOAAbove  = 0x1000,
        AOAOn     = 0x2000,
        AOABelow  = 0x4000,

        // Refuel/NWS
        RefuelRDY = 0x8000,
        RefuelAR  = 0x10000,
        RefuelDSC = 0x20000,

        // Caution Lights
        FltControlSys = 0x40000,
        LEFlaps       = 0x80000,
        EngineFault   = 0x100000,
        Overheat      = 0x200000,
        FuelLow       = 0x400000,
        Avionics      = 0x800000,
        RadarAlt      = 0x1000000,
        IFF           = 0x2000000,
        ECM           = 0x4000000,
        Hook          = 0x8000000,
        NWSFail       = 0x10000000,
        CabinPress    = 0x20000000,

        AutoPilotOn   = 0x40000000,  // TRUE if is AP on.  NB: This is not a lamp bit!
        TFR_STBY      = 0x80000000,  // MISC panel; lower half of split face TFR lamp

        // Used with the MAL/IND light code to light up "everything"
        // please update this if you add/change bits!
        AllLampBitsOn     = 0xBFFFFFEF
    };

    enum LightBits2
    {
        // Threat Warning Prime
        HandOff = 0x1,
        Launch  = 0x2,
        PriMode = 0x4,
        Naval   = 0x8,
        Unk     = 0x10,
        TgtSep  = 0x20,

		// EWS
		Go		= 0x40,		// On and operating normally
		NoGo    = 0x80,     // On but malfunction present
		Degr    = 0x100,    // Status message: AUTO DEGR
		Rdy     = 0x200,    // Status message: DISPENSE RDY
		ChaffLo = 0x400,    // Bingo chaff quantity reached
		FlareLo = 0x800,    // Bingo flare quantity reached

        // Aux Threat Warning
        AuxSrch = 0x1000,
        AuxAct  = 0x2000,
        AuxLow  = 0x4000,
        AuxPwr  = 0x8000,

        // ECM
        EcmPwr  = 0x10000,
        EcmFail = 0x20000,

        // Caution Lights
        FwdFuelLow = 0x40000,
        AftFuelLow = 0x80000,

        EPUOn      = 0x100000,  // EPU panel; run light
        JFSOn      = 0x200000,  // Eng Jet Start panel; run light

        // Caution panel
        SEC          = 0x400000,
        OXY_LOW      = 0x800000,
        PROBEHEAT    = 0x1000000,
        SEAT_ARM     = 0x2000000,
        BUC          = 0x4000000,
        FUEL_OIL_HOT = 0x8000000,
        ANTI_SKID    = 0x10000000,

        TFR_ENGAGED  = 0x20000000,  // MISC panel; upper half of split face TFR lamp
        GEARHANDLE   = 0x40000000,  // Lamp in gear handle lights on fault or gear in motion
        ENGINE       = 0x80000000,  // Lower half of right eyebrow ENG FIRE/ENGINE lamp

        // Used with the MAL/IND light code to light up "everything"
        // please update this if you add/change bits!
		AllLampBits2On = 0xFFFFF03F,
		AllLampBits2OnExceptCarapace = AllLampBits2On ^ HandOff ^ Launch ^ PriMode ^ Naval ^ Unk ^ TgtSep ^ AuxSrch ^ AuxAct ^ AuxLow ^ AuxPwr
    };

    enum LightBits3
    {
        // Elec panel
        FlcsPmg = 0x1,
        MainGen = 0x2,
        StbyGen = 0x4,
        EpuGen  = 0x8,
        EpuPmg  = 0x10,
        ToFlcs  = 0x20,
        FlcsRly = 0x40,
        BatFail = 0x80,

        // EPU panel
        Hydrazine = 0x100,
        Air       = 0x200,

        // Caution panel
        Elec_Fault = 0x400,
        Lef_Fault  = 0x800,

		OnGround	  = 0x1000,   // weight-on-wheels
        FlcsBitRun    = 0x2000,   // FLT CONTROL panel RUN light (used to be Multi-engine fire light)
        FlcsBitFail   = 0x4000,   // FLT CONTROL panel FAIL light (used to be Lock light Cue; non-F-16)
        DbuWarn       = 0x8000,   // Right eyebrow DBU ON cell; was Shoot light cue; non-F16
        NoseGearDown  = 0x10000,  // Landing gear panel; on means down and locked
        LeftGearDown  = 0x20000,  // Landing gear panel; on means down and locked
        RightGearDown = 0x40000,  // Landing gear panel; on means down and locked
		ParkBrakeOn   = 0x100000, // Parking brake engaged; NOTE: not a lamp bit
        Power_Off     = 0x200000, // Set if there is no electrical power.  NB: not a lamp bit

		// Caution panel
		cadc	= 0x400000,
		
		// Left Aux console
		SpeedBrake = 0x800000,  // True if speed brake is in anything other than stowed position

        // Threat Warning Prime - additional bits
		SysTest  = 0x1000000,

		// Master Caution WILL come up (actual lightBit has 3sec delay like in RL),
		// usable for cockpit builders with RL equipment which has a delay on its own.
		// Will be set to false again as soon as the MasterCaution bit is set.
		MCAnnounced = 0x2000000,

		//MLGWOW is only for AFM , it means WOW switches on MLG are triggered => FLCS switches to WOWPitchRockGain
		MLGWOW = 0x4000000,
		NLGWOW = 0x8000000,

		ATF_Not_Engaged = 0x10000000,
		
		// Caution panel
		Inlet_Icing = 0x20000000,

		// Free bits in LightBits3		
		//0x40000000,
		//0x80000000,

		// Used with the MAL/IND light code to light up "everything"
        // please update this if you add/change bits!
		AllLampBits3On = 0x3147EFFF,
		AllLampBits3OnExceptCarapace = AllLampBits3On ^ SysTest
    };

    enum HsiBits
    {
        ToTrue        = 0x01,    // HSI_FLAG_TO_TRUE == 1, TO
        IlsWarning    = 0x02,    // HSI_FLAG_ILS_WARN
        CourseWarning = 0x04,    // HSI_FLAG_CRS_WARN
        Init          = 0x08,    // HSI_FLAG_INIT
        TotalFlags    = 0x10,    // HSI_FLAG_TOTAL_FLAGS; never set
        ADI_OFF       = 0x20,    // ADI OFF Flag
        ADI_AUX       = 0x40,    // ADI AUX Flag
        ADI_GS        = 0x80,    // ADI GS FLAG
        ADI_LOC       = 0x100,   // ADI LOC FLAG
        HSI_OFF       = 0x200,   // HSI OFF Flag
        BUP_ADI_OFF   = 0x400,   // Backup ADI Off Flag
        VVI           = 0x800,   // VVI OFF Flag
        AOA           = 0x1000,  // AOA OFF Flag
        AVTR          = 0x2000,  // AVTR Light
		OuterMarker   = 0x4000,  // MARKER beacon light for outer marker
		MiddleMarker  = 0x8000,  // MARKER beacon light for middle marker
		FromTrue      = 0x10000, // HSI_FLAG_TO_TRUE == 2, FROM

		Flying		  = 0x80000000, // true if player is attached to an aircraft (i.e. not in UI state).  NOTE: Not a lamp bit

		// Used with the MAL/IND light code to light up "everything"
        // please update this is you add/change bits!
        AllLampHsiBitsOn = 0xE000
    };

    // These are outputs from the sim
	// Note: some two-engine values removed in this version for compatibility
	// reasons.
    float x;            // Ownship North (Ft)
    float y;            // Ownship East (Ft)
    float z;            // Ownship Down (Ft) --- NOTE: use FlightData2 AAUZ for barometric altitude!
    float xDot;         // Ownship North Rate (ft/sec)
    float yDot;         // Ownship East Rate (ft/sec)
    float zDot;         // Ownship Down Rate (ft/sec)
    float alpha;        // Ownship AOA (Degrees)
    float beta;         // Ownship Beta (Degrees)
    float gamma;        // Ownship Gamma (Radians)
    float pitch;        // Ownship Pitch (Radians)
    float roll;         // Ownship Pitch (Radians)
    float yaw;          // Ownship Pitch (Radians)
    float mach;         // Ownship Mach number
    float kias;         // Ownship Indicated Airspeed (Knots)
    float vt;           // Ownship True Airspeed (Ft/Sec)
    float gs;           // Ownship Normal Gs
    float windOffset;   // Wind delta to FPM (Radians)
    float nozzlePos;    // Ownship engine nozzle percent open (0-100)
	//float nozzlePos2;   // MOVED TO FlightData2! Ownship engine nozzle2 percent open (0-100) 
    float internalFuel; // Ownship internal fuel (Lbs)
    float externalFuel; // Ownship external fuel (Lbs)
    float fuelFlow;     // Ownship fuel flow (Lbs/Hour)
    float rpm;          // Ownship engine rpm (Percent 0-103)
	//float rpm2;         // MOVED TO FlightData2! Ownship engine rpm2 (Percent 0-103)
    float ftit;         // Ownship Forward Turbine Inlet Temp (Degrees C)
	//float ftit2;        // MOVED TO FlightData2! Ownship Forward Turbine Inlet Temp2 (Degrees C)
    float gearPos;      // Ownship Gear position 0 = up, 1 = down;
    float speedBrake;   // Ownship speed brake position 0 = closed, 1 = 60 Degrees open
    float epuFuel;      // Ownship EPU fuel (Percent 0-100)
    float oilPressure;  // Ownship Oil Pressure (Percent 0-100)
	//float oilPressure2; // MOVED TO FlightData2! Ownship Oil Pressure2 (Percent 0-100)
    unsigned int   lightBits;    // Cockpit Indicator Lights, one bit per bulb. See enum

    // These are inputs. Use them carefully
	// NB: these do not work when TrackIR device is enabled
	// NB2: launch falcon with the '-head' command line parameter to activate !
    float headPitch;    // Head pitch offset from design eye (radians)
    float headRoll;     // Head roll offset from design eye (radians)
    float headYaw;      // Head yaw offset from design eye (radians)

    // new lights
    unsigned int   lightBits2;   // Cockpit Indicator Lights, one bit per bulb. See enum
    unsigned int   lightBits3;   // Cockpit Indicator Lights, one bit per bulb. See enum

    // chaff/flare
    float ChaffCount;   // Number of Chaff left
    float FlareCount;   // Number of Flare left

    // landing gear
    float NoseGearPos;  // Position of the nose landinggear; caution: full down values defined in dat files
    float LeftGearPos;  // Position of the left landinggear; caution: full down values defined in dat files
    float RightGearPos; // Position of the right landinggear; caution: full down values defined in dat files

    // ADI values
    float AdiIlsHorPos; // Position of horizontal ILS bar
    float AdiIlsVerPos; // Position of vertical ILS bar

    // HSI states
    int courseState;    // HSI_STA_CRS_STATE
    int headingState;   // HSI_STA_HDG_STATE
    int totalStates;    // HSI_STA_TOTAL_STATES; never set

    // HSI values
    float courseDeviation;     // HSI_VAL_CRS_DEVIATION
    float desiredCourse;       // HSI_VAL_DESIRED_CRS
    float distanceToBeacon;    // HSI_VAL_DISTANCE_TO_BEACON
    float bearingToBeacon;     // HSI_VAL_BEARING_TO_BEACON
    float currentHeading;      // HSI_VAL_CURRENT_HEADING
    float desiredHeading;      // HSI_VAL_DESIRED_HEADING
    float deviationLimit;      // HSI_VAL_DEV_LIMIT
    float halfDeviationLimit;  // HSI_VAL_HALF_DEV_LIMIT
    float localizerCourse;     // HSI_VAL_LOCALIZER_CRS
    float airbaseX;            // HSI_VAL_AIRBASE_X
    float airbaseY;            // HSI_VAL_AIRBASE_Y
    float totalValues;         // HSI_VAL_TOTAL_VALUES; never set

    float TrimPitch;  // Value of trim in pitch axis, -0.5 to +0.5
    float TrimRoll;   // Value of trim in roll axis, -0.5 to +0.5
    float TrimYaw;    // Value of trim in yaw axis, -0.5 to +0.5

    // HSI flags
    unsigned int hsiBits;      // HSI flags

    //DED Lines
    char DEDLines[5][26];  //25 usable chars
    char Invert[5][26];    //25 usable chars

    //PFL Lines
    char PFLLines[5][26];  //25 usable chars
    char PFLInvert[5][26]; //25 usable chars

    //TacanChannel
    int UFCTChan, AUXTChan;

    // RWR
    int           RwrObjectCount;
    int           RWRsymbol[40];
    float         bearing[40];
    unsigned long missileActivity[40];
    unsigned long missileLaunch[40];
    unsigned long selected[40];
    float         lethality[40];
	unsigned long newDetection[40];

    //fuel values
    float fwd, aft, total;

	void SetLightBit(LightBits newBit) { lightBits |= newBit; };
	void ClearLightBit(LightBits newBit) { lightBits &= ~newBit; };
	bool IsSet(LightBits newBit) { return ((lightBits & newBit) ? true : false); };

	void SetLightBit2(LightBits2 newBit) { lightBits2 |= newBit; };
	void ClearLightBit2(LightBits2 newBit) { lightBits2 &= ~newBit; };
	bool IsSet2(LightBits2 newBit) { return ((lightBits2 & newBit) ? true : false); };

	void SetLightBit3(LightBits3 newBit) { lightBits3 |= newBit; };
	void ClearLightBit3(LightBits3 newBit) { lightBits3 &= ~newBit; };
	bool IsSet3(LightBits3 newBit) { return ((lightBits3 & newBit) ? true : false); };

	void SetHsiBit(HsiBits newBit) { hsiBits |= newBit; };
	void ClearHsiBit(HsiBits newBit) { hsiBits &= ~newBit; };
	bool IsSetHsi(HsiBits newBit) { return ((hsiBits & newBit) ? true : false); };

    int VersionNum;    // Version of FlightData mem area

    // New values added here for header file compatibility but not implemented
    // in this version of the code at present.
    float headX;       // Head X offset from design eye (feet)
    float headY;       // Head Y offset from design eye (feet)
    float headZ;       // Head Z offset from design eye (feet)

    int MainPower;     // Main Power switch state, 0=down, 1=middle, 2=up
};


// OSB capture for MFD button labeling

#define OSB_STRING_LENGTH 8  // currently strings appear to be max 7 printing chars

typedef struct {
	char line1[OSB_STRING_LENGTH];
	char line2[OSB_STRING_LENGTH];
	bool inverted;
} OsbLabel;


// *** "FalconSharedOsbMemoryArea" ***
class OSBData
{
public:
	OsbLabel leftMFD[20];
	OsbLabel rightMFD[20];
};


#define FLIGHTDATA2_VERSION 17
// changelog:
// 1: initial BMS 4.33 version
// 2: added AltCalReading, altBits, BupUhfPreset, powerBits, blinkBits, cmdsMode
// 3: added VersionNum, hydPressureA/B, cabinAlt, BupUhfFreq, currentTime, vehicleACD
// 4: added fuelflow2
// 5: added RwrInfo, lefPos, tefPos
// 6: added vtolPos
// 7: bit fields are now unsigned instead of signed
// 8: increased RwrInfo size to 512
// 9: added human pilot names and their status in a session
// 10: added bump intensity while taxiing/rolling
// 11: added latitude/longitude
// 12: added RTT info
// 13: added IFF panel backup digits
// 14: added instrument backlight brightness
// 15: added MiscBits, BettyBits, radar altitude, bingo fuel, cara alow, bullseye, BMS version information, string area size/time, drawing area size
// 16: added turn rate
// 17: added Flcs_Flcc, SolenoidStatus to MiscBits

// do NOT change these w/o crosschecking the BMS code
#define RWRINFO_SIZE 512
#define CALLSIGN_LEN 12
#define MAX_CALLSIGNS 32

// *** "FalconSharedMemoryArea2" ***
class FlightData2
{
public:

	// TACAN
	enum TacanSources
	{
		UFC = 0,
		AUX = 1,
		NUMBER_OF_SOURCES = 2,
	};
	enum TacanBits
	{
		band = 0x01,   // true in this bit position if band is X
		mode = 0x02,   // true in this bit position if domain is air to air
	};

	// ALTIMETER
	enum AltBits
	{
		CalType  = 0x01,	// true if calibration in inches of Mercury (Hg), false if in hectoPascal (hPa)
		PneuFlag = 0x02,	// true if PNEU flag is visible
	};

	// POWER
	enum PowerBits
	{
		BusPowerBattery      = 0x01,	// true if at least the battery bus is powered
		BusPowerEmergency    = 0x02,	// true if at least the emergency bus is powered
		BusPowerEssential    = 0x04,	// true if at least the essential bus is powered
		BusPowerNonEssential = 0x08,	// true if at least the non-essential bus is powered
		MainGenerator        = 0x10,	// true if the main generator is online
		StandbyGenerator     = 0x20,	// true if the standby generator is online
		JetFuelStarter       = 0x40,	// true if JFS is running, can be used for magswitch
	};

	// BLINKING LIGHTS - only indicating *IF* a lamp is blinking, not implementing the actual on/off/blinking pattern logic!
	enum BlinkBits
	{
		// currently working
		OuterMarker  = 0x01,	// defined in HsiBits    - slow flashing for outer marker
		MiddleMarker = 0x02,	// defined in HsiBits    - fast flashing for middle marker
		PROBEHEAT    = 0x04,	// defined in LightBits2 - probeheat system is tested
		AuxSrch      = 0x08,	// defined in LightBits2 - search function in NOT activated and a search radar is painting ownship
		Launch       = 0x10,	// defined in LightBits2 - missile is fired at ownship
		PriMode      = 0x20,	// defined in LightBits2 - priority mode is enabled but more than 5 threat emitters are detected
		Unk          = 0x40,	// defined in LightBits2 - unknown is not active but EWS detects unknown radar

		// not working yet, defined for future use
		Elec_Fault   = 0x80,	// defined in LightBits3 - non-resetting fault
		OXY_BROW     = 0x100,	// defined in LightBits  - monitor fault during Obogs
		EPUOn        = 0x200,	// defined in LightBits3 - abnormal EPU operation
		JFSOn_Slow   = 0x400,	// defined in LightBits3 - slow blinking: non-critical failure
		JFSOn_Fast   = 0x800,	// defined in LightBits3 - fast blinking: critical failure
	};

	// CMDS mode state
	enum CmdsModes : int
	{
		CmdsOFF  = 0,
		CmdsSTBY = 1,
		CmdsMAN  = 2,
		CmdsSEMI = 3,
		CmdsAUTO = 4,
		CmdsBYP  = 5,
	};

	// HSI/eHSI mode state
	enum NavModes : unsigned char
	{
		ILS_TACAN   = 0,
		TACAN       = 1, 
		NAV         = 2,
		ILS_NAV     = 3,
	};

	// human pilot state
	enum FlyStates
	{
		IN_UI   = 0, // UI      - in the UI
		LOADING = 1, // UI>3D   - loading the sim data
		WAITING = 2, // UI>3D   - waiting for other players
		FLYING  = 3, // 3D      - flying
		DEAD    = 4, // 3D>Dead - dead, waiting to respawn
		UNKNOWN = 5, // ???
	};

	// RTT area indices
	enum RTT_areas
	{
		RTT_HUD       = 0,
		RTT_PFL       = 1,
		RTT_DED       = 2,
		RTT_RWR       = 3,
		RTT_MFDLEFT   = 4,
		RTT_MFDRIGHT  = 5,
		RTT_HMS       = 6,
		RTT_noOfAreas = 7,
	};

	// instrument backlight brightness
	enum InstrLight : unsigned char
	{
		INSTR_LIGHT_OFF = 0,
		INSTR_LIGHT_DIM = 1,
		INSTR_LIGHT_BRT = 2,
	};

	// Bitching Betty VMS sounds playing
	enum BettyBits : unsigned int
	{
		Betty_Allwords       = 0x00001,
		Betty_Pullup         = 0x00002,
		Betty_Altitude       = 0x00004,
		Betty_Warning        = 0x00008,
		Betty_Jammer         = 0x00010,
		Betty_Counter        = 0x00020,
		Betty_ChaffFlare     = 0x00040,
		Betty_ChaffFlare_Low = 0x00080,
		Betty_ChaffFlare_Out = 0x00100,
		Betty_Lock           = 0x00200,
		Betty_Caution        = 0x00400,
		Betty_Bingo          = 0x00800,
		Betty_Data           = 0x01000,
		Betty_IFF            = 0x02000,
		Betty_Lowspeed       = 0x04000,
		Betty_Beeps          = 0x08000,
		Betty_AOA            = 0x10000,
		Betty_MaxG           = 0x20000,
	};

	// various flags - chances are that by now, we'll add new flags rarely and sparsely, so having a single "bulk pool" seems reasonable, size-wise
	enum MiscBits : unsigned int
	{
		RALT_Valid			= 0x01, // indicates weather the RALT reading is valid/reliable
		Flcs_Flcc_A			= 0x02,
		Flcs_Flcc_B			= 0x04,
		Flcs_Flcc_C			= 0x08,
		Flcs_Flcc_D			= 0x10,
		SolenoidStatus		= 0x20, //0 not powered or failed or WOW  , 1 is working OK

		AllLampBitsFlccOn	= 0x1e,
	};

	// VERSION 1
	float nozzlePos2;       // Ownship engine nozzle2 percent open (0-100)
	float rpm2;             // Ownship engine rpm2 (Percent 0-103)
	float ftit2;            // Ownship Forward Turbine Inlet Temp2 (Degrees C)
	float oilPressure2;     // Ownship Oil Pressure2 (Percent 0-100)
	NavModes navMode;       // (unsigned char) current mode selected for HSI/eHSI, see NavModes enum for details
	float AAUZ;             // Ownship barometric altitude given by AAU (depends on calibration)
	char tacanInfo[NUMBER_OF_SOURCES]; // Tacan band/mode settings for UFC and AUX COMM

	// VERSION 2 / 7
	int AltCalReading;	// barometric altitude calibration (depends on CalType)
	unsigned int altBits;		// various altimeter bits, see AltBits enum for details
	unsigned int powerBits;		// Ownship power bus / generator states, see PowerBits enum for details
	unsigned int blinkBits;		// Cockpit indicator lights blink status, see BlinkBits enum for details
						// NOTE: these bits indicate only *if* a lamp is blinking, in addition to the
						// existing on/off bits. It's up to the external program to implement the
						// *actual* blinking.
	CmdsModes cmdsMode;	// (int) Ownship CMDS mode state, see CmdsModes enum for details
	int BupUhfPreset;	// BUP UHF channel preset

	// VERSION 3
	int BupUhfFreq;		// BUP UHF channel frequency
	float cabinAlt;		// Ownship cabin altitude
	float hydPressureA;	// Ownship Hydraulic Pressure A
	float hydPressureB;	// Ownship Hydraulic Pressure B
	int currentTime;	// Current time in seconds (max 60 * 60 * 24)
	short vehicleACD;	// Ownship ACD index number, i.e. which aircraft type are we flying.
	int VersionNum;		// Version of FlightData2 mem area

	// VERSION 4
	float fuelFlow2;    // Ownship fuel flow2 (Lbs/Hour)

	// VERSION 5 / 8
	char RwrInfo[RWRINFO_SIZE]; // New RWR Info
	float lefPos;               // Ownship LEF position
	float tefPos;               // Ownship TEF position

	// VERSION 6
	float vtolPos;      // Ownship VTOL exhaust angle

	// VERSION 9
	char pilotsOnline;                                // Number of pilots in an MP session
	char pilotsCallsign[MAX_CALLSIGNS][CALLSIGN_LEN]; // List of pilots callsign connected to an MP session
	char pilotsStatus[MAX_CALLSIGNS];                 // Status of the MP pilots, see enum FlyStates

	// VERSION 10
	float bumpIntensity; // Intensity of a "bump" while taxiing/rolling, 0..1

	// VERSION 11
	float latitude;      // Ownship latitude in degrees (as known by avionics)
	float longitude;     // Ownship longitude in degrees (as known by avionics)

	// VERSION 12
	unsigned short RTT_size[2];                 // RTT overall width and height
	unsigned short RTT_area[RTT_noOfAreas][4];  // For each area: left/top/right/bottom

	// VERSION 13
	char iffBackupMode1Digit1;                     // IFF panel backup Mode1 digit 1
	char iffBackupMode1Digit2;                     // IFF panel backup Mode1 digit 2
	char iffBackupMode3ADigit1;                    // IFF panel backup Mode3A digit 1
	char iffBackupMode3ADigit2;                    // IFF panel backup Mode3A digit 2

	// VERSION 14
	InstrLight instrLight;  // (unsigned char) current instrument backlight brightness setting, see InstrLight enum for details

	// VERSION 15
	unsigned int bettyBits;      // see BettyBits enum for details
	unsigned int miscBits;       // see MiscBits enum for details
	float RALT;                  // radar altitude (only valid/ reliable if MiscBit "RALT_Valid" is set)
	float bingoFuel;             // bingo fuel level
	float caraAlow;              // cara alow setting
	float bullseyeX;             // bullseye X in sim coordinates (same as ownship, i.e. North (Ft))
	float bullseyeY;             // bullseye Y in sim coordinates (same as ownship, i.e. East (Ft))
	int BMSVersionMajor;         // E.g.  4.
	int BMSVersionMinor;         //         34.
	int BMSVersionMicro;         //            1
	int BMSBuildNumber;          //              build 20050
	unsigned int StringAreaSize; // the overall size of the StringData/FalconSharedMemoryAreaString area
	unsigned int StringAreaTime; // last time the StringData/FalconSharedMemoryAreaString area has been changed - you only need to re-read the string shared mem if this changes
	unsigned int DrawingAreaSize;// the overall size of the DrawingData/FalconSharedMemoryAreaDrawing area
	
	// VERSION 16
	float turnRate;              // actual turn rate (no delay or dampening) in degrees/second

	// TACAN
	// setters for internal use only
	void SetUfcTacanToAA(bool t) { if (t) { tacanInfo[UFC] |= mode; } else { tacanInfo[UFC] &= ~mode; } }
	void SetAuxTacanToAA(bool t) { if (t) { tacanInfo[AUX] |= mode; } else { tacanInfo[AUX] &= ~mode; } }
	void SetUfcTacanToX(bool t)  { if (t) { tacanInfo[UFC] |= band; } else { tacanInfo[UFC] &= ~band; } }
	void SetAuxTacanToX(bool t)  { if (t) { tacanInfo[AUX] |= band; } else { tacanInfo[AUX] &= ~band; } }

	// getters for external reader programs
	bool UfcTacanIsAA(void) { return ((tacanInfo[UFC] & mode) ? true : false); }
	bool AuxTacanIsAA(void) { return ((tacanInfo[AUX] & mode) ? true : false); }
	bool UfcTacanIsX(void)  { return ((tacanInfo[UFC] & band) ? true : false); }
	bool AuxTacanIsX(void)  { return ((tacanInfo[AUX] & band) ? true : false); }

	// ALTIMETER
	void SetAltBit(AltBits newBit) { altBits |= newBit; };
	void ClearAltBit(AltBits newBit) { altBits &= ~newBit; };
	bool IsSetAlt(AltBits newBit) { return ((altBits & newBit) ? true : false); };

	// POWER
	void SetPowerBit(PowerBits newBit) { powerBits |= newBit; };
	void ClearPowerBit(PowerBits newBit) { powerBits &= ~newBit; };
	bool IsSetPower(PowerBits newBit) { return ((powerBits & newBit) ? true : false); };

    // BLINKING LIGHTS
	void SetBlinkBit(BlinkBits newBit) { blinkBits |= newBit; };
	void ClearBlinkBit(BlinkBits newBit) { blinkBits &= ~newBit; };
	bool IsSetBlink(BlinkBits newBit) { return ((blinkBits & newBit) ? true : false); };

	// CMDS mode state
	void SetCmdsMode(CmdsModes newMode) { cmdsMode = newMode; };
	CmdsModes GetCmdsMode(void) { return cmdsMode; };

	// HSI/eHSI mode state
	void SetNavMode(NavModes newMode) { navMode = newMode; };
	NavModes GetNavMode(void) { return navMode; };

	// BETTY BITS
	void SetBettyBit(BettyBits newBit) { bettyBits |= newBit; };
	void ClearBettyBit(BettyBits newBit) { bettyBits &= ~newBit; };
	bool IsSetBetty(BettyBits newBit) { return ((bettyBits & newBit) ? true : false); };

	// MISC BITS
	void SetMiscBit(MiscBits newBit) { miscBits |= newBit; };
	void ClearMiscBit(MiscBits newBit) { miscBits &= ~newBit; };
	bool IsSetMisc(MiscBits newBit) { return ((miscBits & newBit) ? true : false); };
};


#define STRINGDATA_VERSION 3
#define STRINGDATA_AREA_SIZE_MAX (1024 * 1024)
// - NOTE: Check StringAreaSize in FalconSharedMemoryArea2 for the actual size of this area!
// - NOTE: Treat this shared memory area as a pure "char*", not as "StringData*", since the size is not fixed!
// - NOTE: Once you have the shared mem mapped for reading, pass the stream to GetStringData to have it reconstruct the *actual* "StringData" object for you!
// - NOTE: This area is not updated every frame, but only when data changes. Check StringAreaTime in FalconSharedMemoryArea2 for the last update timestamp!

// changelog:
// 1: initial BMS 4.34 version
// 2: added 3dbuttons.dat/3dckpit.dat file paths
// 3: added NavPoints

// *** "FalconSharedMemoryAreaString" ***
class StringData
{
public:
	enum StringIdentifier : unsigned int
	{
		// VERSION 1
		BmsExe = 0,             // BMS exe name, full path
		KeyFile,                // Key file name in use, full path

		BmsBasedir,             // BmsBasedir to BmsPictureDirectory:
		BmsBinDirectory,        // - BMS directories in use
		BmsDataDirectory,
		BmsUIArtDirectory,
		BmsUserDirectory,
		BmsAcmiDirectory,
		BmsBriefingsDirectory,
		BmsConfigDirectory,
		BmsLogsDirectory,
		BmsPatchDirectory,
		BmsPictureDirectory,

		ThrName,                 // Current theater name
		ThrCampaigndir,          // ThrCampaigndir to ThrTacrefpicsdir:
		ThrTerraindir,           // - Current theater directories in use
		ThrArtdir,
		ThrMoviedir,
		ThrUisounddir,
		ThrObjectdir,
		Thr3ddatadir,
		ThrMisctexdir,
		ThrSounddir,
		ThrTacrefdir,
		ThrSplashdir,
		ThrCockpitdir,
		ThrSimdatadir,
		ThrSubtitlesdir,
		ThrTacrefpicsdir,

		AcName,                  // Current AC name
		AcNCTR,                  // Current AC NCTR

		// VERSION 2
		ButtonsFile,             // Current 3dbuttons.dat file full path
		CockpitFile,             // Current 3dckpit.dat file full path

		// VERSION 3
		NavPoint,                // Multiple entries, one for each NavPoint. Format for each entry is (NP, O1, O2, PT can be concatenated):
		                         // (NavPoint, mandatory) NP:<index>,<type>,<x>,<y>,<z>,<grnd_elev>;
		                         //     <index>        int            NavPoint number, 1-99
		                         //     <type>         two chars      GM (GMPOINT), PO (POSPOINT), WP (WAYPOINT), MK (MARKPOINT), DL (DATALINK)
		                         //                                   CB (CAMPBULLSEYE), L1 (LINE1), L2 (LINE2), L3 (LINE3), L4 (LINE4), PT (PREPLANNEDTHREAD)
		                         //     <x>,<y>        float          position in sim coordinates
		                         //     <z>            float          altitude in 10s of feet
		                         //     <grnd_elev>    float          ground elevation in 10s of feet
		                         // (OA1/OA2, optional) O1:<bearing>,<range>,<alt>; (and/or) O2:<bearing>,<range>,<alt>;
		                         //     <bearing>      float
		                         //     <range>        unsigned int
		                         //     <alt>          unsigned int
		                         // (PPT, optional) PT:<str_id>,<range>,<declutter>;
		                         //     <str_id>       "string"
		                         //     <range>        float
		                         //     <declutter>    int            0 = false, 1 = true

		// FIXED LAST ENTRY
		StringIdentifier_DIM     // (number of identifiers; add new IDs only *above* this one)
	};

	struct StringStruct
	{
		unsigned int strId;         // See StringIdentifier (not type-safe on purpose, so that older readers can still read "unknown" newer IDs)
		unsigned int strLength;     // The length of the string in "strData", *without* termination! Note that strData *does* have termination.
		std::string strData;        // Data storage blob for the string
		                            // If you parse the data yourself, it should be handled as char[strLength+1] by external apps, string is only used in BMS internally for storage purposes
	};

	unsigned int VersionNum;        // Version of the StringData shared memory area - only indicates changes to the StringIdentifier enum
	unsigned int NoOfStrings;       // How many strings do we have in the area?
	unsigned int dataSize;          // The overall size of the "data" blob that follows
	std::vector<StringStruct> data; // Data storage blob for all the strings
	                                // If you parse the data yourself, it should be handled as char[dataSize] by external apps, vector is only used in BMS internally for storage purposes
	                                // Note that this can NOT be treated as StringStruct[NoOfStrings] by external apps, due to the flexible size of StringStruct!


	// Helper for external(!) apps to reconstruct the actual source StringData from a shared memory char* view, in case you don't want to parse everything yourself
	static StringData GetStringData(const char* inputbuffer)
	{
		StringData result;

		if (inputbuffer)
		{
			size_t offset = 0;

			memcpy((char*)&result.VersionNum, (char*)inputbuffer + offset, sizeof(result.VersionNum));
			offset += sizeof(result.VersionNum);

			memcpy((char*)&result.NoOfStrings, (char*)inputbuffer + offset, sizeof(result.NoOfStrings));
			offset += sizeof(result.NoOfStrings);

			memcpy((char*)&result.dataSize, (char*)inputbuffer + offset, sizeof(result.dataSize));
			offset += sizeof(result.dataSize);

			result.data.resize(result.NoOfStrings);

			for (auto &s : result.data)
			{
				memcpy((char*)&s.strId, (char*)inputbuffer + offset, sizeof(s.strId));
				offset += sizeof(s.strId);

				memcpy((char*)&s.strLength, (char*)inputbuffer + offset, sizeof(s.strLength));
				offset += sizeof(s.strLength);

				s.strData.resize(s.strLength);

				memcpy((char*)s.strData.c_str(), (char*)inputbuffer + offset, s.strLength + 1);
				offset += s.strLength + 1;
			}
		}

		return result;
	}
};


#define DRAWINGDATA_VERSION 1
#define DRAWINGDATA_AREA_SIZE_MAX (1024 * 1024)
// - NOTE: Check DrawingAreaSize in FalconSharedMemoryArea2 for the actual size of this area!
// - NOTE: Treat this shared memory area as a pure "char*", not as "DrawingData*", since the size is not fixed!

// changelog:
// 1: initial BMS 4.35 version: added export of 2D drawing commands for HUD, RWR, and HMS in the following string variables
//     HUD_commands  (only populated if g_bExportDrawingCommandsForHUD is set to 1 in Falcon BMS config)
//     RWR_commands  (only populated if g_bExportDrawingCommandsForRWR is set to 1 in Falcon BMS config)
//     HMS_commands  (only populated if g_bExportDrawingCommandsForHMS is set to 1 in Falcon BMS config)
//
//         Common data format for 2D drawing command strings:
//         Command strings consist of a symbol indicating the command type, followed by a colon, followed by a comma-delimited list of arguments for that commmand, all followed by a terminating semicolon. e.g;
//	           COMMAND: arg1, arg2, ..., argN;  
//
//         The following table lists all the available commands and their arguments. 
//
//		   Command       Arguments          Type              Description
//         -------       ---------          ---------        -------------------------------------------------------------------
//          R:                                               Set canvas resolution
//                       width,               int                canvas width
//                       height;              int                canvas height
//
//			F:                                               Set font            
//                       "fontFile";          string              font texture file
//
//			P:                                               Draw point           
//                       x,                   float               X coordinate
//                       y;                   float               Y coordinate
//
//			L:                                               Draw line           
//                       x1,                  float               starting X coordinate
//                       y1,                  float               starting Y coordinate
//                       x2,                  float               ending X coordinate
//                       y2;                  float               ending Y coordinate
//
//			T:                                               Draw filled triangle           
//                       x1,                  float               vertex 1 X coordinate
//                       y1,                  float               vertex 1 Y coordinate
//                       x2,                  float               vertex 2 X coordinate
//                       y2,                  float               vertex 2 Y coordinate
//                       x3,                  float               vertex 3 X coordinate
//                       y3;                  float               vertex 3 Y coordinate
//
//			S:                                               Draw string
//                       xLeft,               float               X coordinate 
//                       yTop,                float               Y coordinate 
//                       "textString",        string              string to draw
//                       invert;              unsigned char       0=draw text normally; 1=draw text inverted
//
//			SR:                                               Draw string with rotated text
//                       xLeft,               float               X coordinate
//                       yTop,                float               Y coordinate 
//                       "textString",        string              string to draw
//                       angle;               float               rotation angle (radians)
//
//			FG:                                              Set foreground (text and line) color
//                       packedABGR;          unsigned int        foreground color in packed Alpha-Blue-Green-Red bit order (8 bits for each component)
//                                                                   alpha: bits 24-31 (most significant 8 bits)
//                                                                   blue:  bits 16-23
//                                                                   green: bits 8-15
//                                                                   red:   bits 0-7 (least significant 8 bits)
//
//			BG:                                              Set background (text and line) color
//                       packedABGR;          unsigned int        foreground color in packed Alpha-Blue-Green-Red bit order (8 bits for each component)
//                                                                   alpha: bits 24-31 (most significant 8 bits)
//                                                                   blue:  bits 16-23
//                                                                   green: bits 8-15
//                                                                   red:   bits 0-7 (least significant 8 bits)

// *** "FalconSharedMemoryAreaDrawing" ***
class DrawingData
{
public:
	unsigned int VersionNum;   // Version of the DrawingData shared memory area

	unsigned int HUD_length;   // The length of the string in "HUD_commands", *without* termination, note that HUD_commands *does* have termination
	std::string HUD_commands;  // Data storage blob for the string
	                           // Should be handled as char[HUD_length+1] by external apps, std::string is only used in BMS internally for storage purposes
	unsigned int RWR_length;   // The length of the string in "RWR_commands", *without* termination, note that RWR_commands *does* have termination
	std::string RWR_commands;  // Data storage blob for the string
	                           // Should be handled as char[RWR_length+1] by external apps, std::string is only used in BMS internally for storage purposes
	unsigned int HMS_length;   // The length of the string in "HMS_commands", *without* termination, note that HMS_commands *does* have termination
	std::string HMS_commands;  // Data storage blob for the string
	                           // Should be handled as char[HMS_length+1] by external apps, std::string is only used in BMS internally for storage purposes
};


extern OSBData cockpitOSBData;         // "FalconSharedOsbMemoryArea"
extern FlightData cockpitFlightData;   // "FalconSharedMemoryArea"
extern FlightData2 cockpitFlightData2; // "FalconSharedMemoryArea2"
extern StringData cockpitStringData;   // "FalconSharedMemoryAreaString"
extern DrawingData cockpitDrawingData; // "FalconSharedMemoryAreaDrawing"

#endif // _FLIGHT_DATA_H
