/*
  Created on 19.06.18.
*/

#pragma once

namespace glez::detail::record
{
class RecordedCommands;
}

namespace glez::record
{

class Record
{
public:
    Record();
    ~Record();

    void begin();
    void end();
    void replay();

    detail::record::RecordedCommands *commands{ nullptr };
};

} // namespace glez::record