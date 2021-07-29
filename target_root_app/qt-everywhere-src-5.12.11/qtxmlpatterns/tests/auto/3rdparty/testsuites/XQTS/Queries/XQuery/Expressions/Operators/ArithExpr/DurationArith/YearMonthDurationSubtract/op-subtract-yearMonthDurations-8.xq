(:*******************************************************:)
(:Test: op-subtract-yearMonthDurations-8                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 28, 2005                                    :)
(:Purpose: Evaluates The "subtract-yearMonthDurations" function that  :)
(:returns a negative value.                              :)
(:*******************************************************:)
 
(xs:yearMonthDuration("P10Y01M") - xs:yearMonthDuration("P11Y02M"))