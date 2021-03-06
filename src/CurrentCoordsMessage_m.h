//
// Generated file, do not edit! Created by nedtool 5.6 from CurrentCoordsMessage.msg.
//

#ifndef __INET_CURRENTCOORDSMESSAGE_M_H
#define __INET_CURRENTCOORDSMESSAGE_M_H

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <omnetpp.h>

// nedtool version check
#define MSGC_VERSION 0x0506
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of nedtool: 'make clean' should help.
#endif


namespace inet {

class CurrentCoordsMessage;
} // namespace inet

#include "inet/applications/base/ApplicationPacket_m.h" // import inet.applications.base.ApplicationPacket


namespace inet {

/**
 * Class generated from <tt>CurrentCoordsMessage.msg:32</tt> by nedtool.
 * <pre>
 * class CurrentCoordsMessage extends ApplicationPacket
 * {
 *     double x;
 *     double y;
 *     double z;
 * }
 * </pre>
 */
class CurrentCoordsMessage : public ::inet::ApplicationPacket
{
  protected:
    double x = 0;
    double y = 0;
    double z = 0;

  private:
    void copy(const CurrentCoordsMessage& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const CurrentCoordsMessage&);

  public:
    CurrentCoordsMessage();
    CurrentCoordsMessage(const CurrentCoordsMessage& other);
    virtual ~CurrentCoordsMessage();
    CurrentCoordsMessage& operator=(const CurrentCoordsMessage& other);
    virtual CurrentCoordsMessage *dup() const override {return new CurrentCoordsMessage(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    // field getter/setter methods
    virtual double getX() const;
    virtual void setX(double x);
    virtual double getY() const;
    virtual void setY(double y);
    virtual double getZ() const;
    virtual void setZ(double z);
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const CurrentCoordsMessage& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, CurrentCoordsMessage& obj) {obj.parsimUnpack(b);}

} // namespace inet

#endif // ifndef __INET_CURRENTCOORDSMESSAGE_M_H

