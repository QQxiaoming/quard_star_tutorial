(:*******************************************************:)
(:Test: seconds-from-time-8                              :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "seconds-from-time" function    :)
(:as part of a "+" expression.                           :) 
(:*******************************************************:)

fn:seconds-from-time(xs:time("02:00:00Z")) + fn:seconds-from-time(xs:time("10:00:10Z"))
