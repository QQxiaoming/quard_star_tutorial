(:*******************************************************:)
(:Test: op-divide-yearMonthDuration-2                    :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 30, 2005                                    :)
(:Purpose: Evaluates The "divide-yearMonthDuration" operator:)
(:used as part of a boolean expression (and operator) and the "fn:false" function. :)
(:*******************************************************:)

fn:string((xs:yearMonthDuration("P10Y11M") div 2.0)) and fn:false()