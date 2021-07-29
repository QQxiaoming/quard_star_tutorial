(:*******************************************************:)
(:Test: fn-months-from-duration-16                       :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "months-from-duration" function :)
(:as part of a "numeric-equal" expression (eq operator)  :) 
(:*******************************************************:)

fn:months-from-duration(xs:yearMonthDuration("P22Y10M")) eq fn:months-from-duration(xs:yearMonthDuration("P22Y09M"))
