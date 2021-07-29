(:*******************************************************:)
(:Test: op-yearMonthDuration-less-than-4                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "yearMonthDuration-less-than" function that  :)
(:return true and used together with fn:not (le operator):)
(:*******************************************************:)
 
fn:not(xs:yearMonthDuration("P20Y10M") le xs:yearMonthDuration("P20Y10M"))