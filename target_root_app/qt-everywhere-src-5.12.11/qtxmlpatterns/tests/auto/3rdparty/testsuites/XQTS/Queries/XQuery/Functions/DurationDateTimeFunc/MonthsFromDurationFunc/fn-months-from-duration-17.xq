(:*******************************************************:)
(:Test: fn-months-from-duration-17                       :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "months-from-duration" function :)
(:as part of a "numeric-equal" expression (ne operator)  :) 
(:*******************************************************:)

fn:months-from-duration(xs:yearMonthDuration("P22Y10M")) ne fn:months-from-duration(xs:yearMonthDuration("P23Y10M"))
