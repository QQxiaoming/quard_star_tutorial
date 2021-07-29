(:*******************************************************:)
(:Test: op-yearMonthDuration-greater-than-5              :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "yearMonthDuration-greater-than" function that :)
(:return false and used together with fn:not (gt operator):)
(:*******************************************************:)
 
fn:not(xs:yearMonthDuration("P20Y09M") gt xs:yearMonthDuration("P20Y10M"))