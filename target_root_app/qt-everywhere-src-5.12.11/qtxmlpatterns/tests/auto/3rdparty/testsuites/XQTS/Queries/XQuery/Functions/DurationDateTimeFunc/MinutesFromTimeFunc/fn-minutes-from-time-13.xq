(:*******************************************************:)
(:Test: minutes-from-time-13                             :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "minutes-from-time" function    :)
(:as part of a "mod" expression.                         :) 
(:*******************************************************:)

fn:minutes-from-time(xs:time("10:10:00Z")) mod fn:minutes-from-time(xs:time("03:03:00Z"))
