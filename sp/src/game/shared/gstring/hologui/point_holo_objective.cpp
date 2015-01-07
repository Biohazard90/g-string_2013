
#include "cbase.h"
#include "point_holo_objective.h"

#ifdef GAME_DLL
BEGIN_DATADESC( CPointHoloObjective )

	DEFINE_KEYFIELD( m_strDescription, FIELD_STRING, "Description" ),
	DEFINE_KEYFIELD( m_iObjectiveState, FIELD_INTEGER, "State" ),
	DEFINE_KEYFIELD( m_iCountMax, FIELD_INTEGER, "CountMax" ),
	DEFINE_KEYFIELD( m_iCountCurrent, FIELD_INTEGER, "CountCurrent" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "setStateDisabled", InputSetStateDisabled ),
	DEFINE_INPUTFUNC( FIELD_VOID, "setStateActive", InputSetStateActive ),
	DEFINE_INPUTFUNC( FIELD_VOID, "setStateCompleted", InputSetStateCompleted ),
	DEFINE_INPUTFUNC( FIELD_INTEGER, "setCurrentCount", InputSetCountCurrent ),
	DEFINE_INPUTFUNC( FIELD_INTEGER, "addToCurrentCount", InputAddToCountCurrent ),

	DEFINE_OUTPUT( m_OnObjectiveDisabled, "OnObjectiveDisabled" ),
	DEFINE_OUTPUT( m_OnObjectiveActive, "OnObjectiveActive" ),
	DEFINE_OUTPUT( m_OnObjectiveCompleted, "OnObjectiveCompleted" ),

END_DATADESC()
#else

static CUtlVector< CPointHoloObjective* > g_HoloObjectives;
const CUtlVector< CPointHoloObjective* > &GetHoloObjectives()
{
	return g_HoloObjectives;
}

void AddHoloObjective( CPointHoloObjective *pEntity )
{
	Assert( !g_HoloObjectives.HasElement( pEntity ) );
	if ( !g_HoloObjectives.HasElement( pEntity ) )
	{
		g_HoloObjectives.AddToTail( pEntity );
	}
}

void RemoveHoloObjective( CPointHoloObjective *pEntity )
{
	if ( g_HoloObjectives.HasElement( pEntity ) )
	{
		g_HoloObjectives.FindAndRemove( pEntity );
	}
}

#endif

IMPLEMENT_NETWORKCLASS_DT( CPointHoloObjective, CPointHoloObjective_DT )

#ifdef GAME_DLL
	SendPropString( SENDINFO( m_szDescription ) ),
	SendPropInt( SENDINFO( m_iObjectiveState ), 3, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_iCountMax ) ),
	SendPropInt( SENDINFO( m_iCountCurrent ) ),
#else
	RecvPropString( RECVINFO( m_szDescription ) ),
	RecvPropInt( RECVINFO( m_iObjectiveState ) ),
	RecvPropInt( RECVINFO( m_iCountMax ) ),
	RecvPropInt( RECVINFO( m_iCountCurrent ) ),
#endif

END_NETWORK_TABLE();

LINK_ENTITY_TO_CLASS( point_holo_objective, CPointHoloObjective );

CPointHoloObjective::CPointHoloObjective()
{
}

CPointHoloObjective::~CPointHoloObjective()
{
#ifdef CLIENT_DLL
	RemoveHoloObjective( this );
#endif
}

#ifdef GAME_DLL

void CPointHoloObjective::InputSetStateDisabled( inputdata_t &inputdata )
{
	if ( m_iObjectiveState != OBJECTIVESTATE_DISABLED )
	{
		m_iObjectiveState = OBJECTIVESTATE_DISABLED;
		m_OnObjectiveDisabled.FireOutput( inputdata.pActivator, inputdata.pCaller );
		DispatchUpdateTransmitState();
	}
}

void CPointHoloObjective::InputSetStateActive( inputdata_t &inputdata )
{
	if ( m_iObjectiveState != OBJECTIVESTATE_ACTIVE )
	{
		m_iObjectiveState = OBJECTIVESTATE_ACTIVE;
		m_OnObjectiveActive.FireOutput( inputdata.pActivator, inputdata.pCaller );
		DispatchUpdateTransmitState();
	}
}

void CPointHoloObjective::InputSetStateCompleted( inputdata_t &inputdata )
{
	CompleteObjective( inputdata.pActivator, inputdata.pCaller );
}

void CPointHoloObjective::InputSetCountCurrent( inputdata_t &inputdata )
{
	if ( inputdata.value.Convert( FIELD_INTEGER ) )
	{
		m_iCountCurrent = inputdata.value.Int();
		if ( m_iCountCurrent >= m_iCountMax )
		{
			CompleteObjective( inputdata.pActivator, inputdata.pCaller );
		}
	}
}

void CPointHoloObjective::InputAddToCountCurrent( inputdata_t &inputdata )
{
	if ( inputdata.value.Convert( FIELD_INTEGER ) )
	{
		m_iCountCurrent += inputdata.value.Int();
		if ( m_iCountCurrent >= m_iCountMax )
		{
			m_iCountCurrent = m_iCountMax;
			CompleteObjective( inputdata.pActivator, inputdata.pCaller );
		}
	}
}

void CPointHoloObjective::Activate()
{
	BaseClass::Activate();

	Q_strncpy( m_szDescription.GetForModify(), STRING( m_strDescription ), 32 );
}

int CPointHoloObjective::UpdateTransmitState()
{
	return SetTransmitState( m_iObjectiveState == OBJECTIVESTATE_DISABLED ? FL_EDICT_DONTSEND : FL_EDICT_ALWAYS );
}

void CPointHoloObjective::CompleteObjective( CBaseEntity *pActivator, CBaseEntity *pCaller )
{
	if ( m_iObjectiveState != OBJECTIVESTATE_COMPLETED )
	{
		m_iObjectiveState = OBJECTIVESTATE_COMPLETED;
		m_OnObjectiveCompleted.FireOutput( pActivator, pCaller );
	}
}

#else

const char *CPointHoloObjective::GetDescription() const
{
	return m_szDescription.Get();
}

CPointHoloObjective::ObjectiveState CPointHoloObjective::GetObjectiveState() const
{
	return (ObjectiveState)m_iObjectiveState.Get();
}

int CPointHoloObjective::GetCountMax() const
{
	return m_iCountMax;
}

int CPointHoloObjective::GetCountCurrent() const
{
	return m_iCountCurrent;
}

void CPointHoloObjective::NotifyShouldTransmit( ShouldTransmitState_t state )
{
	// TODO: Called with SHOULDTRANSMIT_START on map shutdown? fucking source.
	BaseClass::NotifyShouldTransmit( state );
	switch( state )
	{
	case SHOULDTRANSMIT_START:
		AddHoloObjective( this );
		break;

	case SHOULDTRANSMIT_END:
		RemoveHoloObjective( this );
		break;
	}
}

void CPointHoloObjective::OnDataChanged( DataUpdateType_t type )
{
	BaseClass::OnDataChanged( type );
}

#endif
