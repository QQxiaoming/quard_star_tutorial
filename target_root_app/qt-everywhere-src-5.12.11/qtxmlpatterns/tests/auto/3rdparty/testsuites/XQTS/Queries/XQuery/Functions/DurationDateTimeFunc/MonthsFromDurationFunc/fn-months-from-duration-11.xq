(:*******************************************************:)
(:Test: fn-months-from-duration-11                       :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "months-from-duration" function  :)
(:as part of a "div" expression.                         :) 
(:*******************************************************:)

fn:months-from-duration(xs:yearMonthDuration("P20Y10M")) div fn:months-from-duration(xs:yearMonthDuration("P05Y05M"))
