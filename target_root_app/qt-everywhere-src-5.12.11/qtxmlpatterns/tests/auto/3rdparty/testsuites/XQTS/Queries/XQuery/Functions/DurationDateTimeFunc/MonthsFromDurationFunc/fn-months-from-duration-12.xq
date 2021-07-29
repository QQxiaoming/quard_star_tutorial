(:*******************************************************:)
(:Test: fn-months-from-duration-12                       :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "months-from-duration" function :)
(:as part of a "idiv" expression.                        :) 
(:*******************************************************:)

fn:months-from-duration(xs:yearMonthDuration("P25Y10M")) idiv fn:months-from-duration(xs:yearMonthDuration("P05Y02M"))
