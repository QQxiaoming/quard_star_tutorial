(:*******************************************************:)
(:Test: op-yearMonthDuration-greater-than-3              :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "yearMonthDuration-greater-than" function that :)
(:return true and used together with fn:not (gt operator):)
(:*******************************************************:)
 
fn:not((xs:yearMonthDuration("P20Y123M") gt xs:yearMonthDuration("P20Y11M")))