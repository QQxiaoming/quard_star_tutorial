(:*******************************************************:)
(:Test: op-subtract-yearMonthDurations-15                :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 29, 2005                                    :)
(:Purpose: Evaluates The "subtract-yearMonthDurations" function used  :)
(:together with the numeric-equal operator "le".         :)
(:*******************************************************:)
 
(xs:yearMonthDuration("P10Y01M") - xs:yearMonthDuration("P17Y02M")) le xs:yearMonthDuration("P17Y02M")