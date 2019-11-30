#ifndef C_SPACE_CRAFT_H
#define C_SPACE_CRAFT_H

#ifdef CLIENT_DLL
#include "c_baseanimating.h"
#define CBaseAnimating C_BaseAnimating
#include "hologui/point_holo_target.h"
#else
#include "baseanimating.h"
#include "pathtrack.h"
#endif

#include "igamemovement.h"
#include "gstring_player_shared_forward.h"
#include "cspacecraft_config.h"

#define SPACECRAFT_SPAWNFLAG_IGNORE_PLAYER (1 << 0)
#define SPACECRAFT_SPAWNFLAG_INVINCIBLE (1 << 1)
#define SPACECRAFT_SPAWNFLAG_AVOID_OTHER_SHIPS (1 << 2)

#define SHIPNAME_MAX_LENGTH 32

class CEnvHoloSystem;
#ifdef CLIENT_DLL
class CHudCrosshair;
#else
class ISpacecraftAI
{
public:

	enum AISTATE_e
	{
		AISTATE_IDLE = 0,
		AISTATE_ATTACK_AND_CHASE,
		AISTATE_ATTACK_AND_IDLE,
		AISTATE_APPROACH_TARGET
	};

	virtual ~ISpacecraftAI() {}

	virtual void Run(float flFrametime) = 0;
	virtual void EnterState(AISTATE_e state) = 0;
};
#endif

class ISpacecraftData
{
public:
	virtual ~ISpacecraftData() {}

	enum EngineLevel_e
	{
		ENGINELEVEL_STALLED = 0,
		ENGINELEVEL_IDLE,
		ENGINELEVEL_NORMAL,
		ENGINELEVEL_BOOST,
	};

	virtual int GetShield() const = 0;
	virtual int GetMaxShield() const = 0;
	virtual int GetHull() const = 0;
	virtual int GetMaxHull() const = 0;

	virtual CBaseEntity *GetEntity() = 0;

	virtual const QAngle &GetAngularImpulse() const = 0;
	virtual const Vector &GetPhysVelocity() const = 0;
	virtual EngineLevel_e GetEngineLevel() const = 0;
	virtual bool IsBoostSuspended() const = 0;

#ifdef CLIENT_DLL
	virtual int GetThrusterCount() const = 0;
	virtual float GetThrusterPower( int index ) const = 0;
#endif
};

class CSpacecraft : public CBaseAnimating, public ISpacecraftData
#ifdef CLIENT_DLL
	, public IHoloTarget
#endif
{
	DECLARE_CLASS(CSpacecraft, CBaseAnimating);
	DECLARE_NETWORKCLASS();

#ifdef GAME_DLL
	DECLARE_DATADESC();
#endif

public:
	CSpacecraft();
	virtual ~CSpacecraft();

	// IHoloTarget
#ifdef CLIENT_DLL
	virtual const char *GetName() const;
	virtual float GetSize() const;
	virtual float GetHealthPercentage() const;
	virtual IHoloTarget::TargetType GetType() const;
	virtual float GetMaxDistance() const;
	virtual bool IsActive() const;
	virtual const C_BaseEntity *GetEntity() const;
#endif

	enum AITEAM_e
	{
		AITEAM_MARTIAN = 0,
		AITEAM_NATO,

		AITEAM_BITS = 1
	};

	virtual int ObjectCaps( void ) {
		return BaseClass::ObjectCaps() & ~FCAP_ACROSS_TRANSITION;
	};

	bool IsPlayerControlled() const;
	bool ShouldPlaySounds() const;
	const SpacecraftSettings_t &GetSettings() { return m_Settings; }

	// ISpacecraftData implementation
	virtual int GetShield() const { return m_iShield; }
	virtual int GetMaxShield() const { return m_iMaxShield; }
	virtual int GetHull() const { return GetHealth(); }
	virtual int GetMaxHull() const { return GetMaxHealth(); }

	virtual CBaseEntity *GetEntity() { return this; }
	virtual const QAngle &GetAngularImpulse() const { return m_AngularImpulse.Get(); }
	virtual const Vector &GetPhysVelocity() const { return m_PhysVelocity.Get(); }
	virtual EngineLevel_e GetEngineLevel() const { return (EngineLevel_e) m_iEngineLevel.Get(); }
	virtual bool IsBoostSuspended() const { return m_bBoostSuspended; }

#ifdef CLIENT_DLL
	virtual int GetThrusterCount() const { return m_ThrusterAttachments.Count(); }
	virtual float GetThrusterPower( int index ) const { return m_flThrusterPower[ index ]; }
#endif

#ifdef GAME_DLL
	void SetAI(ISpacecraftAI *pSpacecraftAI);
	ISpacecraftAI *GetAI() { return m_pAI; };

	AITEAM_e GetTeam() { return (AITEAM_e)m_iAITeam.Get(); };

	virtual int UpdateTransmitState() { return SetTransmitState(FL_EDICT_ALWAYS); } // GSTRING_INF

	virtual void Precache();
	virtual void Spawn();
	virtual void Activate();
	void OnPlayerEntered(CGstringPlayer *pPlayer);

	void InputEnterVehicle(inputdata_t &inputdata);
	void InputExitVehicle(inputdata_t &inputdata);
	virtual void PhysicsSimulate();
	virtual void VPhysicsCollision(int index, gamevcollisionevent_t *pEvent);
	virtual void VPhysicsFriction( IPhysicsObject *pObject, float energy, int surfaceProps, int surfacePropsHit );
	virtual bool WillSimulateGamePhysics() { return true; }
	virtual int OnTakeDamage(const CTakeDamageInfo &info);
	virtual void Event_Killed(const CTakeDamageInfo &info);

	void RegisterHoloSystem( CEnvHoloSystem *pHoloSystem );

	// AI
	void InputSetAIState(inputdata_t &inputdata);
	CBaseEntity *GetEnemy() const;
	void SetEnemy(CBaseEntity *pEnemy);
	void InputSetEnemy(inputdata_t &inputdata);
	void InputClearEnemy( inputdata_t &inputdata );

	CPathTrack *GetPathEntity() const;
	void SetPathEntity(CPathTrack *pPathEntity);
	void InputSetPathEntity(inputdata_t &inputdata);

	float GetSpeedMultiplier() const { return m_flSpeedMultiplier; }
	void SetSpeedMultiplier(float flSpeedMultiplier) { m_flSpeedMultiplier = flSpeedMultiplier; }
	void InputSetSpeedMultiplier(inputdata_t &inputdata);

	bool IsInvincible() const { return m_bInvincible; }
	bool IsFrozen() const { return m_bIsFrozen; }
	void InputSetFrozen(inputdata_t &inputdata);
#else
	virtual void NotifyShouldTransmit( ShouldTransmitState_t state );
	virtual int GetHealth() const { return m_iHealth; }
	virtual int GetMaxHealth() const { return m_iMaxHealth; }

	//virtual bool IsTransparent() { return false; }
	virtual bool IsTwoPass() { return true; }
	virtual ShadowType_t ShadowCastType() { return SHADOWS_SIMPLE; }
	virtual bool ShouldReceiveProjectedTextures( int flags ) { return true; }
	virtual RenderGroup_t GetRenderGroup();
	virtual bool IsViewModel() const;
	//virtual int DrawModel( int flags );

	virtual void OnDataChanged( DataUpdateType_t t );
	virtual void UpdateOnRemove();
	virtual void ClientThink();

	void UpdateCrosshair( CHudCrosshair *pCrosshair );

	float GetEngineAlpha() const { return m_flEngineAlpha; }

	int m_iAttachmentGUI;
#endif

	virtual CStudioHdr *OnNewModel();
	virtual void SimulateMove(CMoveData &moveData, float flFrametime);

private:
#ifdef GAME_DLL
	void SimulateFire(CMoveData &moveData, float flFrametime);
	void OnPlayerTeamAttack(const CTakeDamageInfo &info);

	string_t m_strSettingsName;
	string_t m_strShipName;

	float m_flFireDelay;
	int m_iNextWeaponIndex;

	ISpacecraftAI *m_pAI;
	bool m_bHasExternalHoloSystem;

	string_t m_strInitialEnemy;
	EHANDLE m_hEnemy;

	// AI
	int m_iAIControlled;
	int m_iAIState;
	string_t m_strPathStartName;
	CHandle<CPathTrack> m_hPathEntity;
	float m_flSpeedMultiplier;
	bool m_bInvincible;
	bool m_bIsFrozen;

	float m_flPlayerTeamAttackCooldown;
	float m_flPlayerTeamAttackToleration;
	bool m_bShouldTeamkillPlayer;

	IMPLEMENT_NETWORK_VAR_FOR_DERIVED(m_iMaxHealth);
	float m_flCollisionDamageProtection;
	float m_flShieldRegenerationTimer;
	float m_flShieldRegeneratedTimeStamp;
	float m_flHealthRegenerationTimer;
	float m_flHealthRegeneratedTimeStamp;

	COutputEvent m_OnKilled;
	COutputEvent m_OnPlayerTeamAttack;
	COutputEvent m_OnPlayerTeamAttackRetaliation;
#else
	CUtlVector< int > m_ThrusterAttachments;
	CUtlVector< int > m_ThrusterSounds;
	CUtlVector< int > m_EngineAttachments;
	float *m_flThrusterPower;

	CUtlVector< CSmartPtr< CNewParticleEffect > > m_ThrusterParticles;
	CUtlVector< CSmartPtr< CNewParticleEffect > > m_EngineParticles;

	CSmartPtr< CNewParticleEffect > m_SpaceFieldParticles;

	int m_iMaxHealth;

	int m_iEngineLevelLast;
	int m_iProjectileParityLast;
	int m_iGUID_Engine;
	int m_iGUID_Boost;

	float m_flEngineAlpha;
	float m_flEngineVolume;
	float m_flShakeTimer;
#endif
	IMPLEMENT_NETWORK_VAR_FOR_DERIVED(m_iHealth);

	CUtlVector< int > m_WeaponAttachments;
	float m_flBoostUsage;

	CNetworkString( m_szShipName, SHIPNAME_MAX_LENGTH );

	CNetworkVar(int, m_iShield);
	CNetworkVar(int, m_iMaxShield);
	CNetworkQAngle(m_AngularImpulse);
	CNetworkVector(m_PhysVelocity);
	CNetworkVar(int, m_iEngineLevel);
	CNetworkVar(int, m_iProjectileParity);
	CNetworkVar(float, m_flMoveX);
	CNetworkVar(float, m_flMoveY);
	CNetworkVar(bool, m_bBoostSuspended);

	// AI
	CNetworkVar(int, m_iAITeam);

	CNetworkVar(UtlSymId_t, m_iSettingsIndex);
	SpacecraftSettings_t m_Settings;

	CNetworkHandle(CEnvHoloSystem, m_hHoloSystem);
};


#endif