(:*******************************************************:)
(:Test: op-yearMonthDuration-less-than-6                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "yearMonthDuration-less-than" function that  :)
(:return false and used together with fn:not(le operator):)
(:*******************************************************:)
 
fn:not(xs:yearMonthDuration("P10Y09M") le xs:yearMonthDuration("P10Y07M"))