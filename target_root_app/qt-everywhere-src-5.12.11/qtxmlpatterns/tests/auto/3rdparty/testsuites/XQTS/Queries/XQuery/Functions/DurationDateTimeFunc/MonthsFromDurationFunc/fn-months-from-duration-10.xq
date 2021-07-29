(:*******************************************************:)
(:Test: fn-months-from-duration-10                       :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "months-from-duration" function  :)
(:as part of a "*" expression.                           :) 
(:*******************************************************:)

fn:months-from-duration(xs:yearMonthDuration("P20Y09M")) * fn:months-from-duration(xs:yearMonthDuration("P02Y10M"))