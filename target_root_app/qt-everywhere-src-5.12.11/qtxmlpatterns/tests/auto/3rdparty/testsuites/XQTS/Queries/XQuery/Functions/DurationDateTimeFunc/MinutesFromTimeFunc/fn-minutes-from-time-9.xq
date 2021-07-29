(:*******************************************************:)
(:Test: minutes-from-time-9                              :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "minutes-from-time" function    :)
(:as part of a "-" expression.                           :) 
(:*******************************************************:)

fn:minutes-from-time(xs:time("10:10:00Z")) - fn:minutes-from-time(xs:time("09:02:00Z"))
