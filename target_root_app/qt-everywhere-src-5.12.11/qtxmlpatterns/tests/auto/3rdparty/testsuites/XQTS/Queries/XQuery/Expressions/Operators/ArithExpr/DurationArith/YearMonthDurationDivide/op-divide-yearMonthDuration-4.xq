(:*******************************************************:)
(:Test: op-divide-yearMonthDuration-4                    :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 30, 2005                                    :)
(:Purpose: Evaluates The "divide-yearMonthDuration" operator that  :)
(:return true and used together with fn:not.             :)
(:*******************************************************:)
 
fn:not(fn:string(xs:yearMonthDuration("P11Y04M") div 2.0))