#ifndef __assignment_h__
#define __assignment_h__

#include "type.h"
#include "ioclass.h"
#include "variable.h"
#include "operator.h"
#include "latency.h"

#include <list>
#include <iostream>
#include <sstream>

class Assignment
{
  friend class Assignments;
  
public:
  // constructor
  Assignment( int count, Operator& op, Variable& result, Variable& input1, Variable& input2 = dummyvar(), Variable& input3 =
                  dummyvar() )
      : mCount(count), mWidth(0), mLatency(0.0), mSigned(false), mOperator( op ), mResult( result ), mOperand1( input1 ), mOperand2( input2 ), mOperand3( input3 )
  {
    // determine operator width
    mWidth = result.width();
    if ((op.id() == Operator::GT) ||
        (op.id() == Operator::LT) ||
        (op.id() == Operator::EQ))
    {
      mWidth = (input1.width() > input2.width()) ? input1.width() : input2.width();
    }
    // calculate latency through the component
    mLatency = Latencies::instance().getLatency(op,mWidth);
    // determine whether this component should be signed
    mSigned = input1.isSigned() || input2.isSigned() || input3.isSigned();
  }
  
  // get the assignment operator
  const Operator& getOperator()
  {
    return mOperator;
  }
  
  // get the result variable
   Variable& getResult()
  {
    return mResult;
  }
  
  // get the number of input arguments
  const int getNumArgs()
  {
    return mOperator.nargs();
  }
  
  // get the first input argument
   Variable& getInput1()
  {
    return mOperand1;
  }
  
  // get the second input argument
   Variable& getInput2()
  {
    return mOperand2;
  }
  
  // get the third input argument
   Variable& getInput3()
  {
    return mOperand3;
  }
  
  // get the instantiation count
  int getCount()
  {
    return mCount;
  }
  
  // get the component width
  int getWidth()
  {
    return mWidth;
  }
  
  // get the component latency in nanoseconds
  double getLatency()
  {
    return mLatency;
  }
  
  // is the component signed?
  bool isSigned()
  {
    return mSigned;
  }
  
  // stream override to output the assignment in Verilog
  friend std::ostream& operator<<(std::ostream& out, Assignment& a)
  {
    out << a.mOperator.component(a.mSigned) << " #(.DATAWIDTH(" << a.mWidth << "))";
    out << " inst" << a.mCount << "_" << a.mOperator.component(a.mSigned); 
    out << " (" << extend(a.mOperand1,a.mWidth);
    if (a.mOperator.nargs() > 1)
      out << "," << extend(a.mOperand2,a.mWidth);
    if (a.mOperator.nargs() > 2)
      out << "," << extend(a.mOperand3,a.mWidth);
    out << "," << a.mResult.name() << ");";
    /*DEBUG*/ out << " // latency = " << a.mLatency << " ns";
    return out;
  }

private:
  // return reference to a dummy variable to fill in unused operands
  static Variable& dummyvar()
  {
    static Type dummytype( "dummy",1,false );
    static IOClass dummyio( "dummy",IOClass::INPUT );
    static Variable dummyvar( "dummy",dummytype,dummyio );
    return dummyvar;
  }
  // create, if necessary, a width-extended input
  static std::string extend(Variable& var, int width)
  {
    // first, assume no modification
    std::string result(var.name());
    // now evaluate variable width versus component width
    if (var.width() < width)
    {
      std::ostringstream tmp;
      if (var.isSigned())
      {
        // sign extend
        tmp << "{" << (width-var.width()) << "{" << var.name() << "[" << var.width()-1 << "]" << "}," << result << "}";
      }
      else
      {
        // pad with zeros
        tmp << "{" << (width-var.width()) << "'b0," << result << "}";
      }
      result = tmp.str();
    }
    return result;
  }

private:
  int mCount;
  int mWidth;
  double mLatency;
  bool mSigned;
  Operator& mOperator;
  Variable& mResult;
  Variable& mOperand1;
  Variable& mOperand2;
  Variable& mOperand3;
};

class Assignments
{
private:
  typedef std::list< Assignment > assignments_t;

public:
  static Assignments& instance()
  {
    static Assignments assignments;
    return assignments;
  }
  Assignment& addAssignment( Operator& op, Variable& output, Variable& input1, Variable& input2 = Assignment::dummyvar(),
                      Variable& input3 = Assignment::dummyvar() )
  {
    Assignment* pA = new Assignment( mCount++,op,output,input1,input2,input3 );
    mAssignments.push_back( *pA );
    return *pA;
  }

  typedef assignments_t::iterator iterator_t;
  iterator_t begin() { return mAssignments.begin(); }
  iterator_t end() { return mAssignments.end(); }
  size_t size() { return mAssignments.size(); }
  Assignment& assignment(iterator_t i) { return *i; }
  Assignment& operator[](iterator_t i) { return *i; }

private:
  Assignments():mCount(0)
  {
  }

  assignments_t mAssignments;
  int mCount;
};

#endif//__assignment_h__
