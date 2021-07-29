(:*******************************************************:)
(:Test: op-add-yearMonthDurations-8                      :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 29, 2005                                    :)
(:Purpose: Evaluates The "add-yearMonthDurations" function that  :)
(:returns a negative value.                              :)
(:*******************************************************:)
 
(xs:yearMonthDuration("P10Y01M") + xs:yearMonthDuration("-P11Y02M"))