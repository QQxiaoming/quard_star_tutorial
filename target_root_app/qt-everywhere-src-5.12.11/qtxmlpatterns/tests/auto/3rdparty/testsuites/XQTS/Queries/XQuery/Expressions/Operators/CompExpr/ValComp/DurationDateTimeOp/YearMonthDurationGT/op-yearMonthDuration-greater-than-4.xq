(:*******************************************************:)
(:Test: op-yearMonthDuration-greater-than-4              :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "yearMonthDuration-greater-than" function that :)
(:return true and used together with fn:not (ge operator):)
(:*******************************************************:)
 
fn:not(xs:yearMonthDuration("P20Y10M") ge xs:yearMonthDuration("P20Y10M"))