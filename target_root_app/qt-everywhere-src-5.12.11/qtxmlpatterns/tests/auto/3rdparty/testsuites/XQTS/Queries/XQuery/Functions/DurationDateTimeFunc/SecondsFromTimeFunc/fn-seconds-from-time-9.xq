(:*******************************************************:)
(:Test: seconds-from-time-9                              :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "seconds-from-time" function    :)
(:as part of a "-" expression.                           :) 
(:*******************************************************:)

fn:seconds-from-time(xs:time("10:10:10Z")) - fn:seconds-from-time(xs:time("09:02:07Z"))
