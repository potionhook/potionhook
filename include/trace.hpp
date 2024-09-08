/*
 * trace.h
 *
 *  Created on: Oct 10, 2016
 *      Author: nullifiedcat
 */

#pragma once

#include <engine/IEngineTrace.h>

// This file is a mess. I need to fix it. TODO

class IClientEntity;

namespace trace
{
class FilterDefault : public ITraceFilter
{
public:
    IClientEntity *m_pSelf;

    virtual ~FilterDefault();
    FilterDefault();
    bool ShouldHitEntity(IHandleEntity *entity, int mask) override;
    void SetSelf(IClientEntity *self);
    TraceType_t GetTraceType() const override;
};

class FilterNoPlayer : public ITraceFilter
{
public:
    IClientEntity *m_pSelf;

    virtual ~FilterNoPlayer();
    FilterNoPlayer();
    bool ShouldHitEntity(IHandleEntity *entity, int mask) override;
    void SetSelf(IClientEntity *self);
    TraceType_t GetTraceType() const override;
};

class FilterNoTeammates : public ITraceFilter
{
public:
    IClientEntity *m_pSelf;

public:
    virtual ~FilterNoTeammates();
    FilterNoTeammates();
    virtual bool ShouldHitEntity(IHandleEntity *entity, int mask);
    void SetSelf(IClientEntity *self);
    virtual TraceType_t GetTraceType() const;
};

class FilterNavigation : public ITraceFilter
{
public:
    virtual ~FilterNavigation();
    FilterNavigation();
    bool ShouldHitEntity(IHandleEntity *entity, int mask) override;
    TraceType_t GetTraceType() const override;
};

class FilterNoEntity : public ITraceFilter
{
public:
    IClientEntity *m_pSelf;

    virtual ~FilterNoEntity();
    FilterNoEntity();
    bool ShouldHitEntity(IHandleEntity *entity, int mask) override;
    void SetSelf(IClientEntity *self);
    TraceType_t GetTraceType() const override;
};

class FilterPenetration : public ITraceFilter
{
public:
    IClientEntity *m_pSelf;
    IClientEntity *m_pIgnoreFirst;

    virtual ~FilterPenetration();
    FilterPenetration();
    bool ShouldHitEntity(IHandleEntity *entity, int mask) override;
    void SetSelf(IClientEntity *self);
    void Reset();
    TraceType_t GetTraceType() const override;
};

extern FilterDefault filter_default;
extern FilterNoPlayer filter_no_player;
extern FilterNavigation filter_navigation;
extern FilterNoEntity filter_no_entity;
extern FilterPenetration filter_penetration;
extern FilterNoTeammates filter_teammates;
} // namespace trace