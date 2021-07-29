(:*******************************************************:)
(:Test: fn-months-from-duration-19                       :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "months-from-duration" function :)
(:as part of a "numeric-equal" expression (ge operator)  :) 
(:*******************************************************:)

fn:months-from-duration(xs:yearMonthDuration("P20Y09M")) ge fn:months-from-duration(xs:yearMonthDuration("P20Y01M"))