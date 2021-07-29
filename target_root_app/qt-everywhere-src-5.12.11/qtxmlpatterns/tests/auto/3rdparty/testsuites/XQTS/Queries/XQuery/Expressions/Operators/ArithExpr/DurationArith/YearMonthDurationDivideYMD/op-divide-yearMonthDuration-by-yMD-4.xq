(:*******************************************************:)
(:Test: op-divide-yearMonthDuration-by-YMD-4             :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 30, 2005                                    :)
(:Purpose: Evaluates The "divide-yearMonthDuration-by-YMD" operator that  :)
(:return true and used together with fn:not.             :)
(:*******************************************************:)
 
fn:not(xs:yearMonthDuration("P11Y04M") div xs:yearMonthDuration("P02Y11M"))