(:*******************************************************:)
(:Test: fn-months-from-duration-9                        :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 11, 2005                                    :)
(:Purpose: Evaluates The "months-from-duration" function  :)
(:as part of a "-" expression.                           :) 
(:*******************************************************:)

fn:months-from-duration(xs:yearMonthDuration("P30Y10M")) - fn:months-from-duration(xs:yearMonthDuration("P10Y09M"))
