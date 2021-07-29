(:*******************************************************:)
(:Test: op-yearMonthDuration-greater-than-6              :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "yearMonthDuration-greater-than" function that :)
(:return false and used together with fn:not(ge operator):)
(:*******************************************************:)
 
fn:not(xs:yearMonthDuration("P10Y07M") ge xs:yearMonthDuration("P10Y09M"))