(:*******************************************************:)
(:Test: fn-months-from-duration-18                       :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 13, 2005                                    :)
(:Purpose: Evaluates The "months-from-duration" function :)
(:as part of a "numeric-equal" expression (le operator)  :) 
(:*******************************************************:)

fn:months-from-duration(xs:yearMonthDuration("P21Y01M")) le fn:months-from-duration(xs:yearMonthDuration("P21Y15M"))