#include "event_manager.h"
#include "file_signal_reader.h"
#include "event_table_file_reader.h"

#include <cassert>

namespace BioSig_
{

//-----------------------------------------------------------------------------
EventManager::EventManager (FileSignalReader& reader,
                            EventTableFileReader const& event_table_reader)
    : reader_ (reader),
      event_table_reader_ (event_table_reader)
{
    assert (reader_.isOpen());
    FileSignalReader::SignalEventVector signal_events;
    reader_.loadEvents (signal_events);
    next_free_id_ = 0;
    for (int index = 0; index < signal_events.size(); index++)
    {
        event_map_.insert (next_free_id_,
                           QSharedPointer<SignalEvent> (
                                   new SignalEvent(signal_events[index],
                                                   next_free_id_)));
        next_free_id_++;
    }
    sample_rate_ = reader_.getBasicHeader()->getEventSamplerate();
}

//-----------------------------------------------------------------------------
EventManager::~EventManager ()
{
    // nothing to do here
}

//-----------------------------------------------------------------------------
QSharedPointer<SignalEvent const> EventManager::getEvent (EventID id) const
{
    EventMap::ConstIterator event_it = event_map_.find (id);
    if (event_it == event_map_.end())
        return QSharedPointer<SignalEvent const> (0);
    else
        return event_it.value();
}

//-----------------------------------------------------------------------------
QSharedPointer<SignalEvent> EventManager::getEventForEditing (EventID id)
{
    EventMap::Iterator event_it = event_map_.find (id);
    if (event_it == event_map_.end())
        return QSharedPointer<SignalEvent> (0);
    else
        return event_it.value();
}

//-----------------------------------------------------------------------------
void EventManager::updateEvent (EventID id)
{
    emit eventChanged (id);
}

//-----------------------------------------------------------------------------
QSharedPointer<SignalEvent const> EventManager::createEvent (
        ChannelID channel_id, unsigned pos, unsigned duration, EventType type,
        EventID id)
{
    if (id == SignalEvent::UNDEFINED_ID)
    {
        id = next_free_id_;
        next_free_id_++;
    }
    else
        if (event_map_.contains(id))
            return QSharedPointer<SignalEvent>(0);

    QSharedPointer<SignalEvent> new_event (
            new SignalEvent(pos, type, sample_rate_, channel_id, duration, id));
    event_map_[id] = new_event;

    emit eventCreated (new_event);
    return new_event;
}

//-----------------------------------------------------------------------------
void EventManager::removeEvent (EventID id)
{
    EventMap::iterator event_iter = event_map_.find (id);
    if (event_iter == event_map_.end())
        return;

    event_map_.remove (id);
    emit eventRemoved (id);
}

//-----------------------------------------------------------------------------
std::set<EventID> EventManager::getEventsAt (unsigned pos,
                                             unsigned channel_id) const
{
    std::set<EventID> bla;
    pos++;
    channel_id++;
    return bla;
}

//-----------------------------------------------------------------------------
double EventManager::getSampleRate () const
{
    return sample_rate_;
}

//-----------------------------------------------------------------------------
QString EventManager::getNameOfEventType (EventType type) const
{
    return event_table_reader_.getEventName (type);
}

//-----------------------------------------------------------------------------
QList<EventID> EventManager::getAllEvents () const
{
    return event_map_.keys ();
}


//-----------------------------------------------------------------------------
QList<EventID> EventManager::getEventsOfType (EventType type) const
{
    QList<EventID> event_ids;
    for (EventMap::ConstIterator event_iter = event_map_.begin ();
         event_iter != event_map_.end ();
         ++event_iter)
        if (event_iter.value ()->getType () == type)
            event_ids.push_back (event_iter.key ());

    return event_ids;
}

//-----------------------------------------------------------------------------
void EventManager::getAllEvents (FileSignalReader::SignalEventVector&
                                 event_vector) const
{
    for (EventMap::ConstIterator event_iter = event_map_.begin ();
         event_iter != event_map_.end ();
         ++event_iter)
    {
        event_vector.push_back (*(event_iter.value ()));
    }
}




}