(:*******************************************************:)
(:Test: fn-months-from-duration-8                        :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "months-from-duration" function  :)
(:as part of a "+" expression.                           :) 
(:*******************************************************:)

fn:months-from-duration(xs:yearMonthDuration("P21Y10M")) + fn:months-from-duration(xs:yearMonthDuration("P22Y11M"))