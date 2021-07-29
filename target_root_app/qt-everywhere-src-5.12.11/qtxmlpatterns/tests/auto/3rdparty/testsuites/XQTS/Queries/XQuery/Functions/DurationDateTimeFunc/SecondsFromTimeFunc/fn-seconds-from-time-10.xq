(:*******************************************************:)
(:Test: seconds-from-time-10                             :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "seconds-from-time" function    :)
(:as part of a "*" expression.                           :) 
(:*******************************************************:)

fn:seconds-from-time(xs:time("02:02:03Z")) * fn:seconds-from-time(xs:time("10:08:09Z"))
