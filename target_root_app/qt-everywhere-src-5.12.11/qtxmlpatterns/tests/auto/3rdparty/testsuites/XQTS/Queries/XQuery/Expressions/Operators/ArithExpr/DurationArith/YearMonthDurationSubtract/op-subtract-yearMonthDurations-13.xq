(:*******************************************************:)
(:Test: op-subtract-yearMonthDurations-13                :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 29, 2005                                    :)
(:Purpose: Evaluates The "subtract-yearMonthDurations" function used  :)
(:together with the numeric-equal- operator "eq".        :)
(:*******************************************************:)
 
(xs:yearMonthDuration("P23Y11M") - xs:yearMonthDuration("P23Y11M")) eq xs:yearMonthDuration("P23Y11M")