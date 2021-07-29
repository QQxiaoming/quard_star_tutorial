(:*******************************************************:)
(:Test: op-yearMonthDuration-less-than-5                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "yearMonthDuration-less-than" function that  :)
(:return false and used together with fn:not (lt operator):)
(:*******************************************************:)
 
fn:not(xs:yearMonthDuration("P20Y10M") lt xs:yearMonthDuration("P20Y09M"))