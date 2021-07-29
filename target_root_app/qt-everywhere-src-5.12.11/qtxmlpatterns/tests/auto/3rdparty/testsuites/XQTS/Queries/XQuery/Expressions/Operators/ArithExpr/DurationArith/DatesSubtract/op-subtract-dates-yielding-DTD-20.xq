(:*******************************************************:)
(:Test: op-subtract-dates-yielding-DTD-20                :)
(:Date: July 6, 2005                                     :)
(:Purpose: Evaluates The "subtract-dates-yielding-DTD" operator as :)
(:part of a addition expression                          :)
(:*******************************************************:)
 
(xs:date("1999-10-23Z") - xs:date("1998-09-09Z")) + xs:dayTimeDuration("P17DT10H02M")