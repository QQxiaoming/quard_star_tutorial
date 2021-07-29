(:*******************************************************:)
(:Test: fn-timezone-from-time-9                          :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 27, 2005                                    :)
(:Purpose: Evaluates The "timezone-from-time" function   :)
(:as part of a "-" expression.                           :) 
(:*******************************************************:)

fn:timezone-from-time(xs:time("10:10:10Z")) - fn:timezone-from-time(xs:time("09:02:07Z"))
