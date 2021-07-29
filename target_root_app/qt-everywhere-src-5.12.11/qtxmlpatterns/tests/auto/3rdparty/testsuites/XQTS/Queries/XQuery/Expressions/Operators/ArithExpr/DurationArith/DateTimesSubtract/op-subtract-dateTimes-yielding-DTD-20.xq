(:*******************************************************:)
(:Test: op-subtract-dateTimes-yielding-DTD-20            :)
(:Date: July 6, 2005                                     :)
(:Purpose: Evaluates The "subtract-dateTimes-yielding-DTD" operator as :)
(:part of a addition expression.                         :)
(:*******************************************************:)
 
(xs:dateTime("1999-10-23T09:08:07Z") - xs:dateTime("1998-09-09T04:03:02Z")) + xs:dayTimeDuration("P17DT10H02M")