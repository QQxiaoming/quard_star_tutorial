(:*******************************************************:)
(:Test: op-subtract-yearMonthDurations-16                :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 29, 2005                                    :)
(:Purpose: Evaluates The "subtract-yearMonthDurations" function used  :)
(:together with the numeric-equal operator "ge".         :)
(:*******************************************************:)
 
(xs:yearMonthDuration("P13Y09M") - xs:yearMonthDuration("P18Y02M")) ge xs:yearMonthDuration("P18Y02M")