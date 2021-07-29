(:*******************************************************:)
(:Test: fn-months-from-duration-13                       :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "months-from-duration" function  :)
(:as part of a "mod" expression.                         :) 
(:*******************************************************:)

fn:months-from-duration(xs:yearMonthDuration("P10Y10M")) mod fn:months-from-duration(xs:yearMonthDuration("P03Y03M"))
