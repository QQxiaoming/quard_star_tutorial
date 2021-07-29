(:*******************************************************:)
(:Test: minutes-from-time-10                             :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "minutes-from-time" function    :)
(:as part of a "*" expression.                           :) 
(:*******************************************************:)

fn:minutes-from-time(xs:time("02:02:00Z")) * fn:minutes-from-time(xs:time("10:08:00Z"))
