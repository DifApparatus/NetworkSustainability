//
// Generated file, do not edit! Created by nedtool 5.6 from NewPathSearchMessage.msg.
//

#ifndef __INET_NEWPATHSEARCHMESSAGE_M_H
#define __INET_NEWPATHSEARCHMESSAGE_M_H

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

class NewPathSearchMessage;
} // namespace inet

#include "inet/networklayer/contract/ipv4/Ipv4Address_m.h" // import inet.networklayer.contract.ipv4.Ipv4Address

#include "inet/applications/base/ApplicationPacket_m.h" // import inet.applications.base.ApplicationPacket


namespace inet {

/**
 * Class generated from <tt>NewPathSearchMessage.msg:21</tt> by nedtool.
 * <pre>
 * class NewPathSearchMessage extends ApplicationPacket
 * {
 *     Ipv4Address destAddr;
 *     double distance;
 * }
 * </pre>
 */
class NewPathSearchMessage : public ::inet::ApplicationPacket
{
  protected:
    Ipv4Address destAddr;
    double distance = 0;

  private:
    void copy(const NewPathSearchMessage& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const NewPathSearchMessage&);

  public:
    NewPathSearchMessage();
    NewPathSearchMessage(const NewPathSearchMessage& other);
    virtual ~NewPathSearchMessage();
    NewPathSearchMessage& operator=(const NewPathSearchMessage& other);
    virtual NewPathSearchMessage *dup() const override {return new NewPathSearchMessage(*this);}
    virtual void parsimPack(omnetpp::cCommBuffer *b) const override;
    virtual void parsimUnpack(omnetpp::cCommBuffer *b) override;

    // field getter/setter methods
    virtual const Ipv4Address& getDestAddr() const;
    virtual Ipv4Address& getDestAddrForUpdate() { handleChange();return const_cast<Ipv4Address&>(const_cast<NewPathSearchMessage*>(this)->getDestAddr());}
    virtual void setDestAddr(const Ipv4Address& destAddr);
    virtual double getDistance() const;
    virtual void setDistance(double distance);
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const NewPathSearchMessage& obj) {obj.parsimPack(b);}
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, NewPathSearchMessage& obj) {obj.parsimUnpack(b);}

} // namespace inet

#endif // ifndef __INET_NEWPATHSEARCHMESSAGE_M_H

